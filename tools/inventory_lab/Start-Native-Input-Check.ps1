param([string]$LabRoot=$PSScriptRoot)
$ErrorActionPreference='Stop'
$lab=(Resolve-Path -LiteralPath $LabRoot).Path
$engine=Join-Path $lab 'bin/AnomalyDX11AVX.exe'
if(@(Get-Process | Where-Object {$_.ProcessName -match '^(Anomaly.*|xrEngine|VerifiedDX.*)$'}).Count){
    'Not started: another Anomaly instance is already running.' | Set-Content -LiteralPath (Join-Path $lab 'native-input-check-status.txt')
    exit 2
}
if(!(Test-Path -LiteralPath (Join-Path $lab 'appdata/savedgames/sqa3_layout.scop'))){throw 'Missing disposable sqa3_layout fixture save'}
$p=Start-Process -FilePath $engine -WorkingDirectory $lab -ArgumentList '-dbg -inventory-lab -nosound -sqa3-fixture-reload -sqa3-fixture-hold -start server(sqa3_layout/single/alife/load)' -WindowStyle Normal -PassThru
try {
    if(!$p.WaitForExit(180000)){
        $live=Get-Process -Id $p.Id -ErrorAction SilentlyContinue
        if($live -and $live.Path -eq $engine){Stop-Process -Id $p.Id;$p.WaitForExit(10000) | Out-Null}
        'Input-check timeout: owned process closed.' | Set-Content -LiteralPath (Join-Path $lab 'native-input-check-status.txt')
    }else{
        ('Input check exited: '+$p.ExitCode) | Set-Content -LiteralPath (Join-Path $lab 'native-input-check-status.txt')
    }
}finally{
    if(!$p.HasExited){$live=Get-Process -Id $p.Id -ErrorAction SilentlyContinue;if($live -and $live.Path -eq $engine){Stop-Process -Id $p.Id}}
}
