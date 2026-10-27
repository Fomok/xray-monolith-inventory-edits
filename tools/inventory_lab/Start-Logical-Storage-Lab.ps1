param(
    [ValidateSet('sqa3_logical_unequipped','sqa3_logical_equipped','sqa3_logical_carried','sqa3_nested_manual')]
    [string]$Save='sqa3_logical_carried',
    [string]$LabRoot=$PSScriptRoot,
    [switch]$StrictGroundPickup,
    [switch]$StrictOwnedPickup
)
$ErrorActionPreference='Stop'
$lab=(Resolve-Path -LiteralPath $LabRoot).Path
if(@(Get-Process | Where-Object {$_.ProcessName -match '^(Anomaly.*|xrEngine|VerifiedDX.*)$'}).Count){
    throw 'Another Anomaly instance is already running. Close it first.'
}
$manifest=Get-Content -LiteralPath (Join-Path $lab 'sqa3-development-manifest.json') -Raw | ConvertFrom-Json
if(!$manifest.logical_storage_opt_in){throw 'This lab currently has a different engine package installed.'}
if($StrictGroundPickup -and !$manifest.ground_pickup_opt_in){throw 'This engine package has no strict ground-pickup mode.'}
if($StrictOwnedPickup -and !$manifest.owned_pickup_opt_in){throw 'This engine package has no strict owned-source pickup mode.'}
$engine=Join-Path $lab 'bin/AnomalyDX11AVX.exe'
$expected=$manifest.files.'bin/AnomalyDX11AVX.exe'.sha256
if((Get-FileHash -LiteralPath $engine -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expected){throw 'Lab engine hash does not match its package.'}
if(!(Test-Path -LiteralPath (Join-Path $lab ('appdata/savedgames/'+$Save+'.scop')))){throw 'The selected disposable logical-storage save is missing.'}
# Interactive user launch: no autorun fixture, no original test save, one window.
$arguments='-dbg -inventory-lab -sqa3-logical-storage -start server('+ $Save +'/single/alife/load)'
if($StrictGroundPickup){$arguments+=' -sqa3-strict-ground-pickup'}
if($StrictOwnedPickup){$arguments+=' -sqa3-strict-owned-pickup'}
$game=Start-Process -FilePath $engine -WorkingDirectory $lab -ArgumentList $arguments -WindowStyle Normal -PassThru
Write-Output ('Logical-storage lab started, PID '+$game.Id+'. Close the game when finished.')
