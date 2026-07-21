param(
    [string]$Plan = "$PSScriptRoot\recipes\stick-humanoid-body-plan.json"
)

$ErrorActionPreference = 'Stop'
$planPath = (Resolve-Path -LiteralPath $Plan).Path
py "$PSScriptRoot\src\run_body_plan.py" $planPath --output-root "$PSScriptRoot\BodyPlanRuns"
exit $LASTEXITCODE
