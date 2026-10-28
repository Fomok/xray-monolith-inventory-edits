param([string]$GameRoot=$PSScriptRoot)
$ErrorActionPreference='Stop'
$taskRoot=(Resolve-Path -LiteralPath $GameRoot).Path
if($taskRoot -ne (Resolve-Path -LiteralPath 'C:/GAMMA/sqa-inventory-lab-game').Path){throw 'This development launcher is restricted to the isolated SQA3 lab.'}
if(@(Get-Process | Where-Object {$_.ProcessName -match '^(Anomaly.*|xrEngine|VerifiedDX.*)$'}).Count){throw 'Close the existing game before starting SQA3.'}
$taskManifest=Get-Content -LiteralPath (Join-Path $taskRoot 'sqa3-development-manifest.json') -Raw | ConvertFrom-Json
foreach($entry in $taskManifest.files.PSObject.Properties){
    $target=[IO.Path]::GetFullPath((Join-Path $taskRoot $entry.Name))
    if(!$target.StartsWith($taskRoot+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)){throw 'Manifest target outside the game folder.'}
    if((Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash.ToLowerInvariant() -ne $entry.Value.sha256){throw ('Development payload changed: '+$entry.Name)}
}
# Normal main-menu startup: New Game and Load are available. No autorun fixture,
# diagnostic scenario or forced save is selected by this launcher.
$taskArguments='-dbg -inventory-lab -sqa3-logical-storage -sqa3-field-kit -sqa3-world-storage -sqa3-ground-storage -sqa3-corpse-storage -sqa3-stash-storage -sqa3-strict-owned-pickup -sqa3-strict-trade-pickup'
Start-Process -FilePath (Join-Path $taskRoot 'bin/AnomalyDX11AVX.exe') -WorkingDirectory $taskRoot -ArgumentList $taskArguments -WindowStyle Normal
