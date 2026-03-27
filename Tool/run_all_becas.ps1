$exe = "E:\Qahse\build\release\Qahse.exe"
$base = "E:\Qahse\Demo\PCSL_Becas_Compare"

$dirs = Get-ChildItem $base -Directory | Sort-Object Name
$ok = 0; $fail = 0; $total = $dirs.Count
foreach ($d in $dirs) {
    $pcs = Join-Path $d.FullName "$($d.Name).pcs"
    if (-not (Test-Path $pcs)) { continue }
    Write-Host -NoNewline "$($d.Name)... "

    $p = New-Object System.Diagnostics.Process
    $p.StartInfo.FileName = $exe
    $p.StartInfo.Arguments = "--pcsl `"$pcs`""
    $p.StartInfo.UseShellExecute = $false
    $p.StartInfo.RedirectStandardInput = $true
    $p.StartInfo.RedirectStandardOutput = $true
    $p.StartInfo.RedirectStandardError = $true
    $p.StartInfo.CreateNoWindow = $true
    [void]$p.Start()
    $p.StandardInput.WriteLine("")
    $p.StandardInput.Close()
    [void]$p.StandardOutput.ReadToEnd()
    $p.WaitForExit()

    $outFile = Join-Path $d.FullName "Result1" "SectionResult_$($d.Name).out"
    if ($p.ExitCode -eq 0 -and (Test-Path $outFile)) {
        Write-Host "OK" -ForegroundColor Green
        $ok++
    } else {
        Write-Host "FAIL (exit=$($p.ExitCode))" -ForegroundColor Red
        $fail++
    }
    $p.Dispose()
}
Write-Host "`nDone: $ok OK, $fail FAIL, $total total"
