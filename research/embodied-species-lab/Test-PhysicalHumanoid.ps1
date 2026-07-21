param(
    [string] $Blender = 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe',
    [string] $Python = 'C:\Users\alexa\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
)

$ErrorActionPreference = 'Stop'
$Runner = Join-Path $PSScriptRoot 'Build-PhysicalHumanoid.ps1'
$Output = Join-Path $PSScriptRoot 'Derived\humanoid-physical-v1'
$Manifest = Join-Path $Output 'manifest.json'

& $Python -m unittest discover (Join-Path $PSScriptRoot 'tests') -v
if ($LASTEXITCODE -ne 0) { throw "Physical profile tests failed with exit code $LASTEXITCODE." }

& $Runner -Blender $Blender
if ($LASTEXITCODE -ne 0) { throw "First physical humanoid build failed with exit code $LASTEXITCODE." }
$First = Get-Content -Raw -LiteralPath $Manifest
$Sentinel = Join-Path $Output 'stale-output-must-be-removed.txt'
[System.IO.File]::WriteAllText($Sentinel, 'stale')

& $Runner -Blender $Blender
if ($LASTEXITCODE -ne 0) { throw "Second physical humanoid build failed with exit code $LASTEXITCODE." }
$Second = Get-Content -Raw -LiteralPath $Manifest
if ($First -cne $Second) { throw 'Repeated physical builds produced different semantic manifests.' }
if (Test-Path -LiteralPath $Sentinel) { throw 'Repeated physical build did not clear stale output.' }
if (Get-ChildItem -LiteralPath $Output -Filter '*.blend1') {
    throw 'Physical humanoid output contains Blender backup files.'
}

$Data = $Second | ConvertFrom-Json
if (-not $Data.validation.passed) { throw 'Physical humanoid validation failed.' }
if (-not $Data.blenderArtifact.validation.passed) {
    throw 'Reopened physical humanoid artifact validation failed.'
}
if ($Data.physics.partCount -ne 14 -or $Data.physics.jointCount -ne 13) {
    throw "Unexpected physics counts: parts=$($Data.physics.partCount) joints=$($Data.physics.jointCount)"
}
if ($Data.physics.totalMassKg -ne 75.0) { throw "Unexpected mass: $($Data.physics.totalMassKg)" }
if ($Data.motion.source -ne 'blender-rigid-body-simulation') {
    throw "Unexpected motion source: $($Data.motion.source)"
}
if ($Data.motion.preimpactChestDisplacementMeters -gt 0.02) {
    throw "Body moved before impact: $($Data.motion.preimpactChestDisplacementMeters)m"
}
if ($Data.motion.peakChestDisplacementMeters -lt 0.01 -or
    $Data.motion.peakChestDisplacementMeters -gt 0.5) {
    throw "Impact reaction is outside its bound: $($Data.motion.peakChestDisplacementMeters)m"
}
if ($Data.motion.finalChestDisplacementMeters -gt 0.2 -or
    $Data.motion.finalFrameChestMotionMeters -gt 0.01) {
    throw "Impact reaction did not settle: displacement=$($Data.motion.finalChestDisplacementMeters)m motion=$($Data.motion.finalFrameChestMotionMeters)m/frame"
}
if ($Data.motion.bakedKeyframeCount -lt 100) {
    throw "Too few baked skeleton keyframes: $($Data.motion.bakedKeyframeCount)"
}
foreach ($RelativePath in @($Data.blenderArtifact.file, $Data.fbxTransport.file, $Data.physics.profileFile)) {
    $Artifact = Join-Path $Output $RelativePath
    if (-not (Test-Path -LiteralPath $Artifact) -or (Get-Item -LiteralPath $Artifact).Length -eq 0) {
        throw "Missing or empty physical humanoid artifact: $Artifact"
    }
}
if ($Data.previews.Count -ne 3) { throw "Expected three previews, found $($Data.previews.Count)." }
foreach ($Preview in $Data.previews) {
    $Artifact = Join-Path $Output $Preview.file
    if (-not (Test-Path -LiteralPath $Artifact) -or (Get-Item -LiteralPath $Artifact).Length -eq 0) {
        throw "Missing or empty physical humanoid preview: $Artifact"
    }
}

Write-Output "PASS physical-humanoid mass=$($Data.physics.totalMassKg)kg joints=$($Data.physics.jointCount) peak=$($Data.motion.peakChestDisplacementMeters)m"
exit 0
