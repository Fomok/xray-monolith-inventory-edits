param(
    [Parameter(Mandatory=$true)][string]$Compiler,
    [Parameter(Mandatory=$true)][string]$OutputDirectory
)
$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path (Join-Path $PSScriptRoot '../../..')).Path
$compilerPath = (Resolve-Path $Compiler).Path
$archiver = Join-Path (Split-Path $compilerPath) 'ar.exe'
if (!(Test-Path $archiver)) { throw 'Companion ar.exe required' }
New-Item -ItemType Directory -Force $OutputDirectory | Out-Null
$out = (Resolve-Path $OutputDirectory).Path
$source = Join-Path $repo 'src/inventory_next'
foreach ($variant in @('debug','release')) {
    $flags = @('-std=c++17','-Wall','-Wextra','-Wpedantic','-Werror',"-I$source")
    if ($variant -eq 'debug') { $flags += @('-O0','-g','-D_GLIBCXX_ASSERTIONS') }
    else { $flags += @('-O2','-DNDEBUG') }
    $objects = @()
    foreach ($unit in @('model','persistence_win32','presentation','entity_registry','placement','identity_ledger','transfer_session','drag_session','recovery')) {
        $object = Join-Path $out "$unit-$variant.o"
        & $compilerPath @flags -c (Join-Path $source "$unit.cpp") -o $object
        if ($LASTEXITCODE -ne 0) { throw "Compilation failed: $unit ($variant)" }
        $objects += $object
    }
    $library = Join-Path $out "libinventory-next-$variant.a"
    & $archiver rcs $library @objects
    if ($LASTEXITCODE -ne 0) { throw 'Archive failed' }
    $exe = Join-Path $out "inventory-next-$variant.exe"
    & $compilerPath @flags (Join-Path $PSScriptRoot 'tests.cpp') $library -o $exe
    if ($LASTEXITCODE -ne 0) { throw "Link failed: $variant" }
    $runner = Join-Path $out "inventory-lab-$variant.exe"
    & $compilerPath @flags -mwindows (Join-Path $PSScriptRoot 'runner.cpp') $library -o $runner
    if ($LASTEXITCODE -ne 0) { throw "Runner link failed: $variant" }
    $fixture = Join-Path $out "$variant-fixture.inv"
    & $exe $fixture 2>&1 | Tee-Object (Join-Path $out "$variant-tests.log")
    if ($LASTEXITCODE -ne 0) { throw "Tests failed: $variant" }
    & $exe --read $fixture 2>&1 | Tee-Object (Join-Path $out "$variant-restart.log")
    if ($LASTEXITCODE -ne 0) { throw "Persistence restart failed: $variant" }
    & $exe --read-logical "$fixture.logical" 2>&1 | Tee-Object (Join-Path $out "$variant-logical-restart.log")
    if ($LASTEXITCODE -ne 0) { throw "Logical forest restart failed: $variant" }
    & $exe --read-world "$fixture.world" 2>&1 | Tee-Object (Join-Path $out "$variant-world-restart.log")
    if ($LASTEXITCODE -ne 0) { throw "Detached world subtree restart failed: $variant" }
}
