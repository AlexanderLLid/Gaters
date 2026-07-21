param(
    [Parameter(Mandatory = $true)] [string]$BodyRun,
    [string]$Recipe = "$PSScriptRoot\recipes\humanoid-anatomical-guide.json"
)

$ErrorActionPreference = 'Stop'
$body = (Resolve-Path -LiteralPath $BodyRun).Path
$recipePath = (Resolve-Path -LiteralPath $Recipe).Path
py "$PSScriptRoot\src\run_anatomical_guide.py" $body $recipePath --output-root "$PSScriptRoot\AnatomyRuns"
exit $LASTEXITCODE
