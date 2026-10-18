param(
    [Parameter(Mandatory=$true)][string]$SourceIni,
    [Parameter(Mandatory=$true)][string]$OutputDirectory
)
$ErrorActionPreference = 'Stop'
$source = (Resolve-Path -LiteralPath $SourceIni).Path
$out = [IO.Path]::GetFullPath($OutputDirectory)
$destination = Join-Path $out 'OptiScaler.ini'
if ($source -eq $destination) { throw 'Output must be separate from the source INI.' }
if (Test-Path -LiteralPath $destination) { throw 'Output already exists; choose a new output directory.' }
$text = [IO.File]::ReadAllText($source)
if ($text -notmatch '(?m)^\[DlssNr\]' -or $text -notmatch '(?m)^External\s*=') {
    throw 'Expected the DLSS-NR fork configuration with startup-only External FG support.'
}
function Set-IniValue([string]$section,[string]$key,[string]$value) {
    $pattern = '(?ms)(^\[' + [regex]::Escape($section) + '\]\r?\n)(.*?)(?=^\[|\z)'
    if ($script:text -notmatch $pattern) { throw "Missing [$section] section" }
    $script:text = [regex]::Replace($script:text,$pattern,{
        param($match)
        $body=$match.Groups[2].Value
        $line='(?m)^'+[regex]::Escape($key)+'\s*=.*$'
        if ($body -notmatch $line) { throw "Missing [$section] $key" }
        $match.Groups[1].Value + [regex]::Replace($body,$line,"$key=$value")
    })
}
Set-IniValue 'FrameGen' 'External' 'true'
Set-IniValue 'Upscalers' 'Dx11Upscaler' 'dlss_12'
Set-IniValue 'DlssNr' 'Enabled' 'true'
Set-IniValue 'Menu' 'OverlayMenu' 'false'
New-Item -ItemType Directory -Path $out -Force | Out-Null
[IO.File]::WriteAllText($destination,$text,[Text.UTF8Encoding]::new($false))
Write-Output "Prepared $destination. Source was not changed. Back up installed INI before manually copying; restart required."
