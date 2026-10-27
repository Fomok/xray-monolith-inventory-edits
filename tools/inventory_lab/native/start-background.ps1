param(
    [string]$Runner = (Join-Path $PSScriptRoot 'inventory-lab-release.exe'),
    [string]$Scenario = (Join-Path $PSScriptRoot 'scenarios/lifecycle.sqa'),
    [string]$ResultsRoot = (Join-Path $PSScriptRoot 'runs'),
    [string]$Resume = ''
)
$ErrorActionPreference = 'Stop'
function Get-LabSha256([string]$Path) {
    $stream = [System.IO.File]::OpenRead($Path)
    $hash = [System.Security.Cryptography.SHA256]::Create()
    try { return [System.BitConverter]::ToString($hash.ComputeHash($stream)).Replace('-', '').ToLowerInvariant() }
    finally { $stream.Dispose(); $hash.Dispose() }
}
$runnerPath = (Resolve-Path -LiteralPath $Runner).Path
$scenarioPath = (Resolve-Path -LiteralPath $Scenario).Path
New-Item -ItemType Directory -Force -Path $ResultsRoot | Out-Null
$resultsPath = (Resolve-Path -LiteralPath $ResultsRoot).Path
$runPath = Join-Path $resultsPath ((Get-Date -Format 'yyyyMMdd-HHmmss') + '-' + [guid]::NewGuid().ToString('N'))
# Windows paths cannot contain quotes. Keep each argument quoted for spaces.
$arguments = '--scenario "' + $scenarioPath + '" --output "' + $runPath + '"'
if ($Resume) {
    $resumePath = (Resolve-Path -LiteralPath $Resume).Path
    $arguments += ' --resume "' + $resumePath + '"'
}
# The runner is GUI-subsystem, sets IDLE_PRIORITY_CLASS before parsing/work, and
# terminates after <=512 lines/30 seconds of scenario processing. No resident loop.
$process = Start-Process -FilePath $runnerPath -ArgumentList $arguments -WindowStyle Hidden -PassThru
$record = [ordered]@{
    pid = $process.Id
    output = $runPath
    result = (Join-Path $runPath 'result.json')
    events = (Join-Path $runPath 'events.jsonl')
    scenario = $scenarioPath
    scenarioSha256 = (Get-LabSha256 $scenarioPath)
    runnerSha256 = (Get-LabSha256 $runnerPath)
    gameLaunched = $false
    note = 'Read result.json for completion; process launch alone is not success.'
}
$record | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $resultsPath 'last-run.json') -Encoding UTF8
$record | ConvertTo-Json
# Bounded watchdog belongs only to the process this launcher created.
if (!$process.WaitForExit(45000)) {
    $process.Kill()
    @{success=$false;error='launcher_timeout';pid=$process.Id} | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $resultsPath ('timeout-' + $process.Id + '.json'))
    exit 1
}
if ($process.ExitCode -ne 0) { exit $process.ExitCode }
