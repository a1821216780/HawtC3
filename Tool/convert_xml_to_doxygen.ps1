# Convert XML doc comments (/// <summary> style) to Doxygen format in a C++ header file
param(
    [string]$InputFile = "e:\Qahse\src\IO\Math\MathHelper.h"
)

$enc = [System.Text.Encoding]::UTF8
$lines = [System.IO.File]::ReadAllLines($InputFile, $enc)
$output = [System.Collections.Generic.List[string]]::new()

# Strip XML references like <paramref name="x"/> → `x`, <see cref="T:Foo"/> → Foo
function Clean-XmlRefs([string]$text) {
    $text = [regex]::Replace($text, '<paramref name="([^"]+)"/>', '`$1`')
    $text = [regex]::Replace($text, '<see langword="([^"]+)"/>', '`$1`')
    $text = [regex]::Replace($text, '<see cref="([^"]+)"/>', '$1')
    $text = [regex]::Replace($text, '<see cref="[^"]*"/>',   '')
    return $text.Trim()
}

# Strip leading "/// " from a comment line, return just content
function Strip-CommentPrefix([string]$line) {
    if ($line -match '^\s*/// (.*)$') { return $Matches[1] }
    if ($line -match '^\s*///$')       { return '' }
    return $line
}

$i = 0
$n = $lines.Count

while ($i -lt $n) {
    $line = $lines[$i]

    # Detect start of XML comment block
    if ($line -match '^(\s*)/// <summary>') {
        $indent = $Matches[1]

        # --- collect summary ---
        $summaryParts = [System.Collections.Generic.List[string]]::new()
        $i++
        while ($i -lt $n -and $lines[$i] -notmatch '/// </summary>') {
            $c = Strip-CommentPrefix $lines[$i]
            if ($c.Trim() -ne '') { $summaryParts.Add((Clean-XmlRefs $c)) }
            $i++
        }
        $i++ # skip </summary>

        $summaryText = ($summaryParts -join ' ').Trim()

        # --- collect param / returns / exception / typeparam / remarks ---
        $paramList    = [System.Collections.Generic.List[hashtable]]::new()
        $typeParamList= [System.Collections.Generic.List[hashtable]]::new()
        $exList       = [System.Collections.Generic.List[hashtable]]::new()
        $returnsText  = ''
        $detailLines  = [System.Collections.Generic.List[string]]::new()
        $codeLines    = [System.Collections.Generic.List[string]]::new()
        $noteLines    = [System.Collections.Generic.List[string]]::new()

        while ($i -lt $n -and $lines[$i] -match '^\s*///') {
            $l = $lines[$i]

            if ($l -match '/// <param name="([^"]+)">([^<]*(?:<[^/][^>]*>[^<]*)*)</param>') {
                $pName = $Matches[1]
                $pDesc = Clean-XmlRefs $Matches[2]
                $paramList.Add(@{ name=$pName; desc=$pDesc })
                $i++
            }
            elseif ($l -match '/// <typeparam name="([^"]+)">([^<]*)</typeparam>') {
                $tName = $Matches[1]
                $tDesc = Clean-XmlRefs $Matches[2]
                $typeParamList.Add(@{ name=$tName; desc=$tDesc })
                $i++
            }
            elseif ($l -match '/// <returns>(.*)</returns>') {
                $returnsText = Clean-XmlRefs $Matches[1]
                $i++
            }
            elseif ($l -match '/// <exception cref="([^"]+)">([^<]*)</exception>') {
                $exList.Add(@{ name=$Matches[1]; desc=Clean-XmlRefs $Matches[2] })
                $i++
            }
            elseif ($l -match '/// <remarks>') {
                $i++
                $inCode = $false
                while ($i -lt $n -and $lines[$i] -notmatch '/// </remarks>') {
                    $rl = Strip-CommentPrefix $lines[$i]
                    if ($rl -match '<code>') {
                        $inCode = $true
                        $codeLines.Add('@code')
                        $i++
                        continue
                    }
                    if ($rl -match '</code>') {
                        $inCode = $false
                        $codeLines.Add('@endcode')
                        $i++
                        continue
                    }
                    if ($inCode) {
                        $codeLines.Add($rl)
                    } else {
                        $cleaned = Clean-XmlRefs $rl
                        $detailLines.Add($cleaned)
                    }
                    $i++
                }
                $i++ # skip </remarks>
            }
            else {
                $i++
            }
        }

        # --- Build Doxygen block ---
        $dox = [System.Collections.Generic.List[string]]::new()
        $dox.Add("${indent}/**")
        $dox.Add("${indent} * @brief $summaryText")

        $hasExtra = ($detailLines.Count -gt 0) -or ($codeLines.Count -gt 0) -or
                    ($typeParamList.Count -gt 0) -or ($paramList.Count -gt 0) -or
                    ($returnsText -ne '') -or ($exList.Count -gt 0)

        if ($hasExtra) {
            $dox.Add("${indent} *")
        }

        # Detail lines (from remarks, non-code)
        $trimmedDetails = @($detailLines | Where-Object { $_ -ne '' })
        if ($trimmedDetails.Count -gt 0) {
            $dox.Add("${indent} * @details $($trimmedDetails[0])")
            for ($k=1; $k -lt $trimmedDetails.Count; $k++) {
                $dl = $trimmedDetails[$k]
                if ($dl.Trim() -eq '') {
                    $dox.Add("${indent} *")
                } else {
                    $dox.Add("${indent} * $dl")
                }
            }
            $dox.Add("${indent} *")
        }

        # tparam
        foreach ($tp in $typeParamList) {
            $dox.Add("${indent} * @tparam $($tp.name)  $($tp.desc)")
        }

        # params
        foreach ($p in $paramList) {
            $dox.Add("${indent} * @param[in] $($p.name)  $($p.desc)")
        }

        # returns
        if ($returnsText -ne '') {
            $dox.Add("${indent} * @return  $returnsText")
        }

        # exceptions
        foreach ($ex in $exList) {
            $dox.Add("${indent} * @throws $($ex.name)  $($ex.desc)")
        }

        # code example
        if ($codeLines.Count -gt 0) {
            if (-not $hasExtra -or ($trimmedDetails.Count -eq 0 -and $paramList.Count -eq 0 -and $returnsText -eq '')) {
                $dox.Add("${indent} *")
            }
            foreach ($cl in $codeLines) {
                if ($cl.Trim() -eq '') {
                    $dox.Add("${indent} *")
                } else {
                    $dox.Add("${indent} * $cl")
                }
            }
            $dox.Add("${indent} *")
        }

        $dox.Add("${indent} */")
        foreach ($dl in $dox) { $output.Add($dl) }
    }
    else {
        $output.Add($line)
        $i++
    }
}

# Write output
[System.IO.File]::WriteAllLines($InputFile, $output.ToArray(), $enc)
Write-Host "Conversion complete. Lines written: $($output.Count)"

# Verify
$remaining = (Select-String "/// <summary>" $InputFile | Measure-Object).Count
Write-Host "Remaining <summary> blocks: $remaining"
