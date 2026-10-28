param(
    [Parameter(Mandatory=$true)][string]$Arguments,
    [Parameter(Mandatory=$true)][string]$EvidenceDirectory,
    [Parameter(Mandatory=$true)][ValidatePattern('^[a-z0-9][a-z0-9-]*$')][string]$EvidenceStem,
    [ValidateRange(60,240)][int]$TimeoutSeconds=180,
    [string]$LabRoot='C:/GAMMA/sqa-inventory-lab-game'
)
$ErrorActionPreference='Stop'
$lab=(Resolve-Path -LiteralPath $LabRoot).Path
if($lab -ne (Resolve-Path -LiteralPath 'C:/GAMMA/sqa-inventory-lab-game').Path){throw 'Only the isolated inventory lab is supported.'}
if($Arguments -notmatch '(^|\s)-sqa3-fixture' -or $Arguments -notmatch 'server\(sqa3_[a-z0-9_]+/single/alife/load\)'){
    throw 'Use an explicit fixture and disposable sqa3_* save.'
}
$preview=(Resolve-Path -LiteralPath 'C:/GAMMA/sqa-previews').Path
$evidence=[IO.Path]::GetFullPath($EvidenceDirectory)
if(!$evidence.StartsWith($preview+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)){
    throw 'Evidence must stay inside the preview workspace.'
}
$engine=[IO.Path]::GetFullPath((Join-Path $lab 'bin/AnomalyDX11AVX.exe'))
$manifest=Get-Content -LiteralPath (Join-Path $lab 'sqa3-development-manifest.json') -Raw | ConvertFrom-Json
foreach($entry in $manifest.files.PSObject.Properties){
    $path=Join-Path $lab $entry.Name
    if((Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() -ne $entry.Value.sha256){throw ('Installed payload mismatch: '+$entry.Name)}
}
$mutex=[Threading.Mutex]::new($false,'Local\SQA3-Native-Lab-Test')
$locked=$false
$game=$null
$started=[DateTime]::UtcNow
$timedOut=$false
$exitCode=$null
try {
    $locked=$mutex.WaitOne(0)
    if(!$locked){throw 'Another supervised lab test is active.'}
    if(@(Get-Process | Where-Object {$_.ProcessName -match '^(Anomaly.*|xrEngine|VerifiedDX.*)$'}).Count){throw 'Another game instance is running.'}
    $game=Start-Process -FilePath $engine -WorkingDirectory $lab -ArgumentList $Arguments -WindowStyle Normal -PassThru
    Write-Output ('Started lab PID '+$game.Id)
    $timedOut=!$game.WaitForExit($TimeoutSeconds*1000)
    if(!$timedOut){$exitCode=$game.ExitCode}
} finally {
    if($game -and !$game.HasExited){
        $owned=Get-Process -Id $game.Id -ErrorAction SilentlyContinue
        if($owned -and $owned.Path -eq $engine){
            Stop-Process -Id $game.Id
            if(!$game.WaitForExit(10000)){throw 'The owned lab process did not exit after bounded shutdown.'}
        }
    }
    if($locked){$mutex.ReleaseMutex()}
    $mutex.Dispose()
}
New-Item -ItemType Directory -Path $evidence -Force > $null
$copied=@{}
$screenshots=@()
foreach($shot in @(Get-ChildItem -LiteralPath (Join-Path $lab 'appdata/screenshots') -Filter '*.jpg' -File -ErrorAction SilentlyContinue | Where-Object {$_.LastWriteTimeUtc -ge $started} | Sort-Object LastWriteTimeUtc)){
    $name=$EvidenceStem+'-screenshot-'+($screenshots.Count+1)+'.jpg'
    $destination=Join-Path $evidence $name
    Copy-Item -LiteralPath $shot.FullName -Destination $destination
    $screenshots+=@{file=$name;source=$shot.Name;sha256=(Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash.ToLowerInvariant()}
}
foreach($entry in @(
    @{source='appdata/logs/xray_fomok.log';suffix='.log'},
    @{source='appdata/sqa3-native-probe.inv';suffix='.inv'},
    @{source='appdata/sqa3-world-storage.inv';suffix='.world'}
)){
    $source=Join-Path $lab $entry.source
    if((Test-Path -LiteralPath $source) -and (Get-Item -LiteralPath $source).LastWriteTimeUtc -ge $started){
        $destination=Join-Path $evidence ($EvidenceStem+$entry.suffix)
        Copy-Item -LiteralPath $source -Destination $destination
        $copied[$entry.suffix]=(Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash.ToLowerInvariant()
    } else {$copied[$entry.suffix]='missing_or_stale'}
}
$report=@{
    pid=$game.Id;started_utc=$started.ToString('o');finished_utc=[DateTime]::UtcNow.ToString('o');
    exit_code=$exitCode;timed_out=$timedOut;engine_commit=$manifest.engine_commit;
    overlay_commit=$manifest.overlay_commit;arguments=$Arguments;evidence=$copied;screenshots=$screenshots;
    limits='Process supervision and evidence capture only; run the corresponding behavioral verifier.'
}
$json=$report | ConvertTo-Json -Depth 5
[IO.File]::WriteAllText((Join-Path $evidence ($EvidenceStem+'-run.json')),$json,[Text.UTF8Encoding]::new($false))
Write-Output $json
if($timedOut -or $exitCode -ne 0){exit 1}
exit 0
