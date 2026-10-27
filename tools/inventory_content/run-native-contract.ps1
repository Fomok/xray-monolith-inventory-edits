$ErrorActionPreference = 'Stop'
$contractOutput = Join-Path $PSScriptRoot 'generated'
New-Item -ItemType Directory -Force -Path $contractOutput | Out-Null
$contractPython = (Get-Command python).Source
$contractScript = Join-Path $PSScriptRoot 'run_native_contract.py'
$contractProcess = Start-Process -FilePath $contractPython -ArgumentList '-B', ('"' + $contractScript + '"') -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $contractOutput 'launcher.stdout.txt') -RedirectStandardError (Join-Path $contractOutput 'launcher.stderr.txt')
if (-not $contractProcess.HasExited) { $contractProcess.PriorityClass = 'Idle' }
if (-not $contractProcess.WaitForExit(55000)) {
    $contractProcess.Kill()
    throw 'Native content contract exceeded the 55-second orchestration bound'
}
Get-Content -LiteralPath (Join-Path $contractOutput 'launcher.stdout.txt'), (Join-Path $contractOutput 'launcher.stderr.txt')
exit $contractProcess.ExitCode
