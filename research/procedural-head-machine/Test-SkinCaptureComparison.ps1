param(
    [Parameter(Mandatory = $true)] [string]$ChampionRun,
    [Parameter(Mandatory = $true)] [string]$ChallengerRun,
    [string]$Recipe = "$PSScriptRoot\recipes\elbow-capture-comparison.json"
)

$ErrorActionPreference = 'Stop'
$champion = (Resolve-Path -LiteralPath $ChampionRun).Path
$challenger = (Resolve-Path -LiteralPath $ChallengerRun).Path
$recipePath = (Resolve-Path -LiteralPath $Recipe).Path
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss-fff'
$root = Join-Path $PSScriptRoot "SkinCaptureComparisons\$stamp"
py "$PSScriptRoot\src\skin_capture_comparator.py" $champion $challenger $recipePath $root
exit $LASTEXITCODE
