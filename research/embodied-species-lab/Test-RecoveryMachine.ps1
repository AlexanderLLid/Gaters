param(
    [string] $Blender = 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe',
    [string] $Python = 'C:\Users\alexa\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
)

$ErrorActionPreference = 'Stop'
$Runner = Join-Path $PSScriptRoot 'Build-RecoveryHumanoid.ps1'
$CasesPath = Join-Path $PSScriptRoot 'species\recovery-cases.json'
$Cases = (Get-Content -Raw -LiteralPath $CasesPath | ConvertFrom-Json).cases

& $Python -m unittest discover (Join-Path $PSScriptRoot 'tests') -v
if ($LASTEXITCODE -ne 0) { throw "Recovery unit tests failed with exit code $LASTEXITCODE." }

foreach ($Case in $Cases) {
    $Output = Join-Path $PSScriptRoot "Derived\humanoid-recovery-v1\$($Case.caseId)"
    $Manifest = Join-Path $Output 'manifest.json'
    & $Runner -Blender $Blender -CaseId $Case.caseId
    if ($LASTEXITCODE -ne 0) { throw "First recovery build failed: $($Case.caseId)." }
    $First = Get-Content -Raw -LiteralPath $Manifest
    $Sentinel = Join-Path $Output 'stale-output-must-be-removed.txt'
    [System.IO.File]::WriteAllText($Sentinel, 'stale')

    & $Runner -Blender $Blender -CaseId $Case.caseId
    if ($LASTEXITCODE -ne 0) { throw "Second recovery build failed: $($Case.caseId)." }
    $Second = Get-Content -Raw -LiteralPath $Manifest
    if ($First -cne $Second) {
        throw "Repeated recovery builds produced different manifests: $($Case.caseId)."
    }
    if (Test-Path -LiteralPath $Sentinel) {
        throw "Recovery rebuild did not clear stale output: $($Case.caseId)."
    }
    if (Get-ChildItem -LiteralPath $Output -Filter '*.blend1') {
        throw "Recovery output contains Blender backup files: $($Case.caseId)."
    }

    $Data = $Second | ConvertFrom-Json
    $Metrics = $Data.recovery.metrics
    if (-not $Data.validation.passed -or -not $Data.blenderArtifact.validation.passed) {
        throw "Recovery validation failed: $($Case.caseId)."
    }
    if ($Data.recovery.plan.stepFoot -ne $Case.expectedStepFoot) {
        throw "Unexpected step foot for $($Case.caseId): $($Data.recovery.plan.stepFoot)."
    }
    if ($Data.physics.passiveHumanoidPartCount -ne 0) {
        throw "Passive humanoid part remained in $($Case.caseId)."
    }
    if ($Metrics.actualStepTravelMeters -lt 0.12 -or $Metrics.actualStepTravelMeters -gt 0.55) {
        throw "Step travel is outside bounds in $($Case.caseId): $($Metrics.actualStepTravelMeters)m."
    }
    if ($Metrics.stepDirectionDotMeters -lt 0.10) {
        throw "Step moved against impact in $($Case.caseId): $($Metrics.stepDirectionDotMeters)."
    }
    if ($Metrics.finalTargetErrorMeters -gt 0.18) {
        throw "Foot missed target in $($Case.caseId): $($Metrics.finalTargetErrorMeters)m."
    }
    if ($Metrics.lowestBodyPointMeters -lt -0.06) {
        throw "Body penetrated ground in $($Case.caseId): $($Metrics.lowestBodyPointMeters)m."
    }
    if ($Metrics.finalBodyMotionMetersPerFrame -gt 0.02) {
        throw "Body did not settle in $($Case.caseId): $($Metrics.finalBodyMotionMetersPerFrame)m/frame."
    }
    if (-not $Metrics.supportContainsCenterOfMass) {
        throw "Center of mass escaped support in $($Case.caseId)."
    }
    if ($Data.previews.Count -ne 4) {
        throw "Expected four recovery previews in $($Case.caseId), found $($Data.previews.Count)."
    }
    foreach ($RelativePath in @(
        $Data.blenderArtifact.file,
        $Data.fbxTransport.file,
        $Data.physics.profileFile,
        $Data.recovery.planFile
    ) + $Data.previews.file) {
        $Artifact = Join-Path $Output $RelativePath
        if (-not (Test-Path -LiteralPath $Artifact) -or (Get-Item -LiteralPath $Artifact).Length -eq 0) {
            throw "Missing recovery artifact: $Artifact"
        }
    }
    Write-Output "PASS recovery case=$($Case.caseId) foot=$($Data.recovery.plan.stepFoot) travel=$($Metrics.actualStepTravelMeters)m"
}

Write-Output "PASS recovery-machine cases=$($Cases.Count)"
exit 0
