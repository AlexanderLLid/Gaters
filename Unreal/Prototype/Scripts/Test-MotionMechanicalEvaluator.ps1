param(
    [string] $Python = 'python',
    [string] $EngineRoot = 'C:\Program Files\Epic Games\UE_5.8'
)

$ErrorActionPreference = 'Stop'
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$RepoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\..'))
$Manifest = Join-Path $RepoRoot 'SourceAssets\Blender\Derived\neutral-motion-v2\manifest.json'
$ImportReport = Join-Path $ProjectRoot 'Saved\AssetImport\neutral-motion.json'
$EvidenceReport = Join-Path $ProjectRoot 'Saved\AssetImport\motion-mechanical.json'
$ImportHarness = Join-Path $PSScriptRoot 'Test-MotionFixtureImport.ps1'
$Evaluator = Join-Path $PSScriptRoot 'EvaluateMotionMechanical.py'
$OutputDirectory = Join-Path $ProjectRoot 'Saved\Tests\MotionMechanical'

foreach ($Path in @($ImportHarness, $Evaluator)) {
    if (-not (Test-Path -LiteralPath $Path)) { throw "Missing required path: $Path" }
}
if (-not (Get-Command $Python -ErrorAction SilentlyContinue)) {
    throw "Python command not found: $Python"
}

& $ImportHarness -EngineRoot $EngineRoot
if ($LASTEXITCODE -ne 0) { throw "Motion import harness failed with exit code $LASTEXITCODE." }
foreach ($Path in @($Manifest, $ImportReport)) {
    if (-not (Test-Path -LiteralPath $Path)) { throw "Missing required path after import: $Path" }
}

New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null

function Write-Json($Value, [string] $Path) {
    $Value | ConvertTo-Json -Depth 20 | Set-Content -LiteralPath $Path -Encoding utf8
}

function Copy-Json($Value) {
    return $Value | ConvertTo-Json -Depth 20 | ConvertFrom-Json
}

function Invoke-Evaluation(
    [string] $Name,
    [string] $ManifestPath,
    [string] $ReportPath,
    [int] $ExpectedExit,
    [string] $ExpectedIssue = '') {
    $Output = Join-Path $OutputDirectory "$Name.json"
    Remove-Item -LiteralPath $Output -Force -ErrorAction SilentlyContinue
    & $Python $Evaluator --manifest $ManifestPath --import-report $ReportPath --output $Output
    $ActualExit = $LASTEXITCODE
    if ($ActualExit -ne $ExpectedExit) {
        throw "$Name exited $ActualExit, expected $ExpectedExit."
    }
    if (-not (Test-Path -LiteralPath $Output)) {
        throw "$Name produced no evaluation record."
    }
    $Evaluation = Get-Content -Raw -LiteralPath $Output | ConvertFrom-Json
    if ($ExpectedExit -eq 0 -and -not $Evaluation.passed) {
        throw "$Name was expected to pass: $($Evaluation.issues | ConvertTo-Json -Compress)"
    }
    if ($ExpectedIssue -and $Evaluation.issues.ruleId -notcontains $ExpectedIssue) {
        throw "$Name did not report $ExpectedIssue."
    }
    return $Evaluation
}

$Positive = Invoke-Evaluation 'positive' $Manifest $ImportReport 0
Copy-Item -LiteralPath (Join-Path $OutputDirectory 'positive.json') -Destination $EvidenceReport -Force
if ($Positive.schemaVersion -ne 1 -or $Positive.evaluatorVersion -ne 2) {
    throw 'Mechanical evaluation versions were not recorded.'
}
if (-not $Positive.checks.footSliding -or
    $Positive.footSliding.limitCentimeters -ne 2.0 -or
    $Positive.footSliding.stances.Count -ne 2) {
    throw 'Mechanical evaluation did not prove planted-foot stability.'
}
if (($Positive.gameplayEvents.name -join ',') -cne 'contact.left,contact.right,contact.left') {
    throw 'Mechanical evaluation did not preserve required gameplay events.'
}
$ExpectedTimes = @(0.0, 0.5, 1.0)
for ($Index = 0; $Index -lt $ExpectedTimes.Count; $Index++) {
    if ([Math]::Abs($Positive.gameplayEvents[$Index].timeSeconds - $ExpectedTimes[$Index]) -gt 0.0001) {
        throw "Gameplay event $Index has the wrong mapped time."
    }
}

$BaseManifest = Get-Content -Raw -LiteralPath $Manifest | ConvertFrom-Json
$BaseReport = Get-Content -Raw -LiteralPath $ImportReport | ConvertFrom-Json

$MissingBone = Copy-Json $BaseReport
$MissingBone.boneNames = @($MissingBone.boneNames | Where-Object { $_ -ne 'foot_l' })
$MissingBonePath = Join-Path $OutputDirectory 'missing-bone-report.json'
Write-Json $MissingBone $MissingBonePath
Invoke-Evaluation 'missing-bone' $Manifest $MissingBonePath 2 'motion.skeleton.bones' | Out-Null

$BadDuration = Copy-Json $BaseReport
$BadDuration.durationSeconds = 1.25
$BadDurationPath = Join-Path $OutputDirectory 'bad-duration-report.json'
Write-Json $BadDuration $BadDurationPath
Invoke-Evaluation 'bad-duration' $Manifest $BadDurationPath 2 'motion.timing.duration' | Out-Null

$BadRoot = Copy-Json $BaseReport
$BadRoot.rootSamples[-1].translationCentimeters[0] = 125.0
$BadRootPath = Join-Path $OutputDirectory 'bad-root-report.json'
Write-Json $BadRoot $BadRootPath
Invoke-Evaluation 'bad-root' $Manifest $BadRootPath 2 'motion.root.path' | Out-Null

$MissingEvent = Copy-Json $BaseManifest
$MissingEvent.clip.events = @($MissingEvent.clip.events | Select-Object -First 2)
$MissingEventPath = Join-Path $OutputDirectory 'missing-event-manifest.json'
Write-Json $MissingEvent $MissingEventPath
Invoke-Evaluation 'missing-event' $MissingEventPath $ImportReport 2 'motion.events.required' | Out-Null

$SlidingFoot = Copy-Json $BaseReport
$SlidingFoot.footWorldSamples.foot_l[8].translationCentimeters[0] += 10.0
$SlidingFootPath = Join-Path $OutputDirectory 'sliding-foot-report.json'
Write-Json $SlidingFoot $SlidingFootPath
Invoke-Evaluation 'sliding-foot' $Manifest $SlidingFootPath 2 'motion.contact.foot_sliding' | Out-Null

Write-Output "PASS motion-mechanical events=$($Positive.gameplayEvents.Count) counterexamples=5"
exit 0
