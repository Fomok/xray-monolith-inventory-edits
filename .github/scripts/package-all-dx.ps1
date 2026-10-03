param([Parameter(Mandatory=$true)][string]$SourceRoot, [Parameter(Mandatory=$true)][string]$DestinationRoot)
$ErrorActionPreference = 'Stop'
$relativeFiles = @('db/mods/00_modded_exes_gamedata.db0')
foreach ($dx in @('DX8','DX9','DX10','DX11')) {
    foreach ($suffix in @('', 'AVX')) {
        foreach ($ext in @('exe','pdb')) { $relativeFiles += "bin/Anomaly$dx$suffix.$ext" }
    }
}
# Validate every variant before copying anything; partial builds cannot become an install package.
foreach ($relative in $relativeFiles) {
    $source = Join-Path $SourceRoot $relative
    if (!(Test-Path -LiteralPath $source -PathType Leaf)) { throw "Missing required file: $relative" }
    if ((Get-Item -LiteralPath $source).Length -eq 0) { throw "Empty required file: $relative" }
}
if (Test-Path -LiteralPath $DestinationRoot) { throw 'Destination must not already exist; use a fresh package directory.' }
foreach ($relative in $relativeFiles) {
    $target = Join-Path $DestinationRoot $relative
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $target) | Out-Null
    Copy-Item -LiteralPath (Join-Path $SourceRoot $relative) -Destination $target
}
Write-Host "Complete install package: $($relativeFiles.Count) files (8 EXEs, 8 PDBs, 1 DB0)."
