param(
    [string] $Blender = 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe',
    [string] $Python = 'C:\Users\alexa\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
)

$ErrorActionPreference = 'Stop'
$Builder = Join-Path $PSScriptRoot 'Build-LocomotionHumanoid.ps1'
$Output = Join-Path $PSScriptRoot 'Derived\humanoid-locomotion-v1'
$Manifest = Join-Path $Output 'locomotion-manifest.json'
$RequiredNames = @(
    'A_Idle',
    'A_TurnLeft',
    'A_Walk',
    'A_Run',
    'A_Stop',
    'A_Jump',
    'A_Fall',
    'A_Land'
)

& $Python -m unittest discover (Join-Path $PSScriptRoot 'tests') -v
if ($LASTEXITCODE -ne 0) {
    throw "Locomotion unit tests failed with exit code $LASTEXITCODE."
}

& $Builder -Blender $Blender
if ($LASTEXITCODE -ne 0) {
    throw "Locomotion build failed with exit code $LASTEXITCODE."
}
$First = Get-Content -Raw -LiteralPath $Manifest

& $Builder -Blender $Blender
if ($LASTEXITCODE -ne 0) {
    throw "Repeated locomotion build failed with exit code $LASTEXITCODE."
}
$Second = Get-Content -Raw -LiteralPath $Manifest

if ($First -cne $Second) {
    throw 'Repeated locomotion manifests differ.'
}

$Data = $Second | ConvertFrom-Json
if (($Data.clips.name -join ',') -cne ($RequiredNames -join ',')) {
    throw 'Locomotion manifest clip order differs from the contract.'
}

$Blend = Join-Path $Output $Data.blenderArtifact.file
if (-not (Test-Path -LiteralPath $Blend -PathType Leaf)) {
    throw "Missing locomotion source Blend: $Blend"
}
foreach ($Clip in $Data.clips) {
    $Fbx = Join-Path $Output $Clip.fbxFile
    if (-not (Test-Path -LiteralPath $Fbx -PathType Leaf)) {
        throw "Missing locomotion FBX: $($Clip.fbxFile)"
    }
}

Write-Host "PASS locomotion-machine clips=$($Data.clips.Count)"
