param(
    [string] $Blender = 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe',
    [string] $Python = 'C:\Users\alexa\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
)

$ErrorActionPreference = 'Stop'
$Runner = Join-Path $PSScriptRoot 'Build-Humanoid.ps1'
$Brief = Join-Path $PSScriptRoot 'species\humanoid.json'
$Output = Join-Path $PSScriptRoot 'Derived\humanoid-v1'
$Manifest = Join-Path $Output 'manifest.json'

& $Python -m unittest discover (Join-Path $PSScriptRoot 'tests') -v
if ($LASTEXITCODE -ne 0) { throw "Reaction unit tests failed with exit code $LASTEXITCODE." }

foreach ($Path in @($Blender, $Runner, $Brief)) {
    if (-not (Test-Path -LiteralPath $Path)) { throw "Missing required path: $Path" }
}

& $Runner -Blender $Blender
if ($LASTEXITCODE -ne 0) { throw "First humanoid build failed with exit code $LASTEXITCODE." }
$First = Get-Content -Raw -LiteralPath $Manifest
$Sentinel = Join-Path $Output 'stale-output-must-be-removed.txt'
[System.IO.File]::WriteAllText($Sentinel, 'stale')

& $Runner -Blender $Blender
if ($LASTEXITCODE -ne 0) { throw "Second humanoid build failed with exit code $LASTEXITCODE." }
$Second = Get-Content -Raw -LiteralPath $Manifest

if ($First -cne $Second) { throw 'Repeated humanoid builds produced different semantic manifests.' }
if (Test-Path -LiteralPath $Sentinel) { throw 'Repeated humanoid build did not clear stale output.' }
if (Get-ChildItem -LiteralPath $Output -Filter '*.blend1') { throw 'Humanoid output contains Blender backup files.' }

$Data = $Second | ConvertFrom-Json
if (-not $Data.validation.passed -or -not $Data.blenderArtifact.validation.passed) {
    throw 'Humanoid validation did not pass.'
}
$ExpectedBones = @(
    'root', 'ik_foot_l', 'ik_foot_r', 'pelvis', 'spine', 'chest', 'neck', 'head',
    'upper_arm_l', 'lower_arm_l', 'upper_arm_r', 'lower_arm_r',
    'thigh_l', 'shin_l', 'foot_l', 'ball_l',
    'thigh_r', 'shin_r', 'foot_r', 'ball_r'
)
if (($Data.skeleton.bones.name -join ',') -cne ($ExpectedBones -join ',')) {
    throw "Unexpected humanoid hierarchy: $($Data.skeleton.bones.name -join ',')"
}
if ($Data.motion.fps -ne 30 -or $Data.motion.startFrame -ne 1 -or
    $Data.motion.endFrame -ne 31 -or $Data.motion.durationSeconds -ne 1.0) {
    throw 'Humanoid reaction timing is invalid.'
}
$Final = $Data.motion.keyframes[-1]
$RecoveryMagnitude = [Math]::Sqrt(
    [Math]::Pow([double] $Final.rootLocationMeters[0], 2) +
    [Math]::Pow([double] $Final.rootLocationMeters[1], 2) +
    [Math]::Pow([double] $Final.rootLocationMeters[2], 2)
)
if ($RecoveryMagnitude -gt 0.000001) {
    throw "Humanoid reaction did not recover its root: $($Final.rootLocationMeters -join ',')"
}
if ($Data.fbxTransport.deterministicBytes -ne $false -or
    ($Data.fbxTransport.PSObject.Properties.Name -contains 'sha256')) {
    throw 'FBX byte nondeterminism must be recorded honestly.'
}
foreach ($RelativePath in @(
    $Data.blenderArtifact.file,
    $Data.fbxTransport.file,
    $Data.motion.recipeFile
)) {
    $Artifact = Join-Path $Output $RelativePath
    if (-not (Test-Path -LiteralPath $Artifact) -or (Get-Item -LiteralPath $Artifact).Length -eq 0) {
        throw "Missing or empty humanoid artifact: $Artifact"
    }
}

Write-Output "PASS humanoid bones=$($Data.skeleton.bones.Count) motion=$($Data.motion.name) seed=$($Data.motion.seed)"
exit 0
