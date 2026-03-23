Add-Type -AssemblyName System.Drawing

function Draw-WindTurbine([int]$size) {
    $bmp = New-Object System.Drawing.Bitmap $size, $size, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode     = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $g.PixelOffsetMode   = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality

    $s = $size

    # ── Rounded green background ──
    $bgColor = [System.Drawing.Color]::FromArgb(255, 34, 139, 34)
    $bgBrush = New-Object System.Drawing.SolidBrush $bgColor
    $cornerR = [Math]::Max(2, $s * 0.18)  # rounded corner radius
    $bgPath  = New-Object System.Drawing.Drawing2D.GraphicsPath
    $d = $cornerR * 2
    $bgPath.AddArc(0,        0,        $d, $d, 180, 90)
    $bgPath.AddArc($s - $d,  0,        $d, $d, 270, 90)
    $bgPath.AddArc($s - $d,  $s - $d,  $d, $d, 0,   90)
    $bgPath.AddArc(0,        $s - $d,  $d, $d, 90,  90)
    $bgPath.CloseFigure()
    $g.FillPath($bgBrush, $bgPath)
    $bgPath.Dispose()

    # ── White "H" (slightly smaller, more breathing room) ──
    $white = [System.Drawing.Color]::White
    $penW  = [Math]::Max(2.0, $s * 0.16)
    $pen   = New-Object System.Drawing.Pen $white, $penW
    $pen.StartCap = [System.Drawing.Drawing2D.LineCap]::Round
    $pen.EndCap   = [System.Drawing.Drawing2D.LineCap]::Round

    $mx = $s * 0.26
    $my = $s * 0.18
    $left  = $mx
    $right = $s - $mx
    $top   = $my
    $bot   = $s - $my
    $midY  = $s / 2.0

    $g.DrawLine($pen, [float]$left,  [float]$top,  [float]$left,  [float]$bot)
    $g.DrawLine($pen, [float]$right, [float]$top,  [float]$right, [float]$bot)
    $g.DrawLine($pen, [float]$left,  [float]$midY, [float]$right, [float]$midY)

    $pen.Dispose(); $bgBrush.Dispose()
    $g.Dispose()
    return $bmp
}

# ═══════════════════════════════════════════════════════════
#  Generate images and assemble multi-size ICO
# ═══════════════════════════════════════════════════════════
$sizes = @(16, 32, 48, 256)
$imgDataList  = [System.Collections.ArrayList]::new()
$imgSizeList  = [System.Collections.ArrayList]::new()
$imgIsPngList = [System.Collections.ArrayList]::new()

foreach ($sz in $sizes) {
    Write-Host "  Drawing ${sz}x${sz} ..."
    $bmp = Draw-WindTurbine $sz

    if ($sz -eq 256) {
        # Store 256 as PNG (standard for large ICO images)
        $ms = [System.IO.MemoryStream]::new()
        $bmp.Save($ms, [System.Drawing.Imaging.ImageFormat]::Png)
        [void]$imgDataList.Add($ms.ToArray())
        [void]$imgIsPngList.Add($true)
        $ms.Dispose()
    }
    else {
        # Store as BMP DIB (BITMAPINFOHEADER + bottom-up BGRA + AND mask)
        $lockRect = New-Object System.Drawing.Rectangle 0, 0, $sz, $sz
        $bd = $bmp.LockBits($lockRect, [System.Drawing.Imaging.ImageLockMode]::ReadOnly,
                            [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
        $stride  = $bd.Stride
        $rawData = New-Object byte[] ($stride * $sz)
        [System.Runtime.InteropServices.Marshal]::Copy($bd.Scan0, $rawData, 0, $rawData.Length)
        $bmp.UnlockBits($bd)

        # Flip to bottom-up
        $rowBytes = $sz * 4
        $flipped  = New-Object byte[] ($rowBytes * $sz)
        for ($r = 0; $r -lt $sz; $r++) {
            [Array]::Copy($rawData, $r * $stride, $flipped, ($sz - 1 - $r) * $rowBytes, $rowBytes)
        }

        # BITMAPINFOHEADER (40 bytes)
        $bih = New-Object byte[] 40
        [Array]::Copy([System.BitConverter]::GetBytes([int32]40),      0, $bih, 0,  4)
        [Array]::Copy([System.BitConverter]::GetBytes([int32]$sz),     0, $bih, 4,  4)
        [Array]::Copy([System.BitConverter]::GetBytes([int32]($sz*2)), 0, $bih, 8,  4)  # height * 2
        [Array]::Copy([System.BitConverter]::GetBytes([int16]1),       0, $bih, 12, 2)  # planes
        [Array]::Copy([System.BitConverter]::GetBytes([int16]32),      0, $bih, 14, 2)  # bpp

        # AND mask (all 0 = opaque)
        $andRow = [int][Math]::Ceiling($sz / 8.0)
        if ($andRow % 4 -ne 0) { $andRow += 4 - ($andRow % 4) }
        $andMask = New-Object byte[] ($andRow * $sz)

        $ms = [System.IO.MemoryStream]::new()
        $ms.Write($bih,     0, $bih.Length)
        $ms.Write($flipped, 0, $flipped.Length)
        $ms.Write($andMask, 0, $andMask.Length)
        [void]$imgDataList.Add($ms.ToArray())
        [void]$imgIsPngList.Add($false)
        $ms.Dispose()
    }

    [void]$imgSizeList.Add($sz)
    $bmp.Dispose()
}

# ── Write ICO file ──
$icoPath = 'E:\HawtC3\src\HawtC3.ico'
$fs = [System.IO.File]::Create($icoPath)
$bw = New-Object System.IO.BinaryWriter($fs)

$count = $sizes.Count
$bw.Write([int16]0)        # reserved
$bw.Write([int16]1)        # type = ICO
$bw.Write([int16]$count)   # number of images

# Compute data offsets (after header 6 + entries 16*N)
$dataStart = 6 + 16 * $count
$curOff = $dataStart

# Write directory entries
for ($i = 0; $i -lt $count; $i++) {
    $sz   = [int]$imgSizeList[$i]
    $data = [byte[]]$imgDataList[$i]

    $bw.Write([byte]$(if ($sz -ge 256) { 0 } else { $sz }))   # width
    $bw.Write([byte]$(if ($sz -ge 256) { 0 } else { $sz }))   # height
    $bw.Write([byte]0)      # palette count
    $bw.Write([byte]0)      # reserved
    $bw.Write([int16]1)     # color planes
    $bw.Write([int16]32)    # bits per pixel
    $bw.Write([int32]$data.Length)   # image data size
    $bw.Write([int32]$curOff)       # offset from start of file
    $curOff += $data.Length
}

# Write image data
for ($i = 0; $i -lt $count; $i++) {
    $data = [byte[]]$imgDataList[$i]
    $bw.Write($data)
}

$bw.Close()
$fs.Close()

$fileSize = (Get-Item $icoPath).Length
Write-Host ""
Write-Host "Icon created: $icoPath ($fileSize bytes)"
Write-Host "Sizes: $($sizes -join ', ') px"

# ── Also generate 512x512 PNG ──
$pngPath = 'E:\HawtC3\src\HawtC3_512.png'
$pngBmp = Draw-WindTurbine 512
$pngBmp.Save($pngPath, [System.Drawing.Imaging.ImageFormat]::Png)
$pngBmp.Dispose()
$pngSize = (Get-Item $pngPath).Length
Write-Host "PNG created: $pngPath ($pngSize bytes)"
