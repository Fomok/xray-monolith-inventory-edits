param(
    [ValidateSet('sqa3_logical_unequipped','sqa3_logical_equipped','sqa3_logical_carried','sqa3_nested_manual','sqa3_purchase','sqa3_direct_equip','sqa3_direct_unequip','sqa3_world_prepared','sqa3_world_release','sqa3_world_return','sqa3_world_nofit','sqa3_ground_release','sqa3_ground_return','sqa3_ground_nofit','sqa3_ground_timeout','sqa3_ground_rollback_release','sqa3_ground_rollback_return','sqa3_corpse_release','sqa3_corpse_return','sqa3_corpse_nofit','sqa3_corpse_rollback_release','sqa3_corpse_rollback_return','sqa3_corpse_cleaned')]
    [string]$Save='sqa3_logical_carried',
    [string]$LabRoot=$PSScriptRoot,
    [switch]$StrictGroundPickup,
    [switch]$StrictOwnedPickup,
    [switch]$StrictTradePickup,
    [switch]$WorldStorage,
    [switch]$GroundStorage,
    [switch]$CorpseStorage
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
if($StrictTradePickup -and !$manifest.trade_pickup_opt_in){throw 'This engine package has no strict purchase mode.'}
if($Save -like 'sqa3_world_*'){$WorldStorage=$true}
if($Save -like 'sqa3_ground_*'){$GroundStorage=$true}
if($Save -like 'sqa3_corpse_*'){$CorpseStorage=$true}
if($GroundStorage){$WorldStorage=$true}
if($CorpseStorage){$WorldStorage=$true}
if($WorldStorage -and !$manifest.world_storage_opt_in){throw 'This engine package has no loaded world-storage mode.'}
if($GroundStorage -and !$manifest.ground_storage_opt_in){throw 'This engine package has no loaded ground-storage mode.'}
if($CorpseStorage -and !$manifest.corpse_storage_opt_in){throw 'This engine package has no loaded corpse-storage mode.'}
$engine=Join-Path $lab 'bin/AnomalyDX11AVX.exe'
$expected=$manifest.files.'bin/AnomalyDX11AVX.exe'.sha256
if((Get-FileHash -LiteralPath $engine -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expected){throw 'Lab engine hash does not match its package.'}
if(!(Test-Path -LiteralPath (Join-Path $lab ('appdata/savedgames/'+$Save+'.scop')))){throw 'The selected disposable logical-storage save is missing.'}
# Interactive user launch: no autorun fixture, no original test save, one window.
$arguments='-dbg -inventory-lab -sqa3-logical-storage -start server('+ $Save +'/single/alife/load)'
if($StrictGroundPickup){$arguments+=' -sqa3-strict-ground-pickup'}
if($StrictOwnedPickup){$arguments+=' -sqa3-strict-owned-pickup'}
if($StrictTradePickup){$arguments+=' -sqa3-strict-trade-pickup'}
if($WorldStorage){$arguments+=' -sqa3-world-storage'}
if($GroundStorage){$arguments+=' -sqa3-ground-storage'}
if($CorpseStorage){$arguments+=' -sqa3-corpse-storage'}
$game=Start-Process -FilePath $engine -WorkingDirectory $lab -ArgumentList $arguments -WindowStyle Normal -PassThru
Write-Output ('Logical-storage lab started, PID '+$game.Id+'. Close the game when finished.')
