param(
    [Parameter(Mandatory = $true)] [string]$GuideRun,
    [string]$Recipe = "$PSScriptRoot\recipes\humanoid-guide-skeleton.json"
)

$ErrorActionPreference = 'Stop'
$guide = (Resolve-Path -LiteralPath $GuideRun).Path
$recipePath = (Resolve-Path -LiteralPath $Recipe).Path
py "$PSScriptRoot\src\run_guide_skeleton.py" $guide $recipePath --output-root "$PSScriptRoot\SkeletonRuns"
exit $LASTEXITCODE
