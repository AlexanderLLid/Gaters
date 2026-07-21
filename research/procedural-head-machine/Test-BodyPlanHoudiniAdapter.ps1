param(
    [Parameter(Mandatory = $true)]
    [string]$BodyRun
)

$ErrorActionPreference = 'Stop'
$source = (Resolve-Path -LiteralPath $BodyRun).Path
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss-fff'
$runRoot = Join-Path $PSScriptRoot "BodyPlanAdapterRuns\$stamp"
New-Item -ItemType Directory -Path $runRoot -Force | Out-Null
$hython = 'C:\Program Files\Side Effects Software\Houdini 22.0.368\bin\hython.exe'
if (-not (Test-Path -LiteralPath $hython)) { throw 'HOUDINI_NOT_FOUND' }

$watch = [System.Diagnostics.Stopwatch]::StartNew()
& $hython "$PSScriptRoot\adapters\houdini_body_plan_build.py" $source $runRoot
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $hython "$PSScriptRoot\adapters\houdini_body_plan_readback.py" "$runRoot\stick-humanoid.hipnc" "$runRoot\readback.json"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
$watch.Stop()
py "$PSScriptRoot\src\body_plan_adapter_verifier.py" $source "$runRoot\readback.json" "$runRoot\verification.json"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$summary = [ordered]@{
    schema = 'body-plan-houdini-adapter-run/0'
    source_run = $source
    elapsed_ms = $watch.ElapsedMilliseconds
    scene = "$runRoot\stick-humanoid.hipnc"
}
$summary | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath "$runRoot\summary.json" -Encoding utf8
Write-Output "BODY_PLAN_HOUDINI_ADAPTER_PASS root=$runRoot elapsed_ms=$($watch.ElapsedMilliseconds)"
