param(
    [Parameter(Mandatory=$true)][string]$Compiler,
    [Parameter(Mandatory=$true)][string]$OutputDirectory
)
$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path (Join-Path $PSScriptRoot '../../..')).Path
$compilerPath = (Resolve-Path -LiteralPath $Compiler).Path
New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
$out = (Resolve-Path -LiteralPath $OutputDirectory).Path
$core = Join-Path $repo 'src/inventory_next'
$flags = @('-std=c++17','-Wall','-Wextra','-Wpedantic','-Werror','-O2',"-I$core")
& $compilerPath @flags -mwindows -municode (Join-Path $PSScriptRoot 'snapshot_viewer.cpp') (Join-Path $core 'model.cpp') (Join-Path $core 'persistence_win32.cpp') -lgdi32 -lshell32 -o (Join-Path $out 'inventory-snapshot.exe')
if ($LASTEXITCODE -ne 0) { throw 'Native snapshot renderer compilation failed' }
Get-FileHash -LiteralPath (Join-Path $out 'inventory-snapshot.exe')
