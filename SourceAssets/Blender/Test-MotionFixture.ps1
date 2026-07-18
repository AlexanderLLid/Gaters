param(
    [string] $Blender = 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe'
)

$ErrorActionPreference = 'Stop'
$Runner = Join-Path $PSScriptRoot 'Build-MotionFixture.ps1'
$Output = Join-Path $PSScriptRoot 'Derived\neutral-motion-v1'
$Manifest = Join-Path $Output 'manifest.json'

foreach ($Path in @($Blender, $Runner)) {
    if (-not (Test-Path -LiteralPath $Path)) { throw "Missing required path: $Path" }
}

& $Runner -Blender $Blender
if ($LASTEXITCODE -ne 0) { throw "First motion build failed with exit code $LASTEXITCODE." }
$First = Get-Content -Raw -LiteralPath $Manifest
$Sentinel = Join-Path $Output 'stale-output-must-be-removed.txt'
[System.IO.File]::WriteAllText($Sentinel, 'stale')

& $Runner -Blender $Blender
if ($LASTEXITCODE -ne 0) { throw "Second motion build failed with exit code $LASTEXITCODE." }
$Second = Get-Content -Raw -LiteralPath $Manifest
if ($First -cne $Second) { throw 'Repeated motion builds produced different semantic manifests.' }
if (Test-Path -LiteralPath $Sentinel) { throw 'Repeated motion build did not clear stale derived output.' }
if (Get-ChildItem -LiteralPath $Output -Filter '*.blend1') { throw 'Motion output contains Blender backup files.' }

$Data = $Second | ConvertFrom-Json
if (-not $Data.validation.passed -or -not $Data.blenderArtifact.validation.passed) {
    throw 'Motion fixture validation did not pass.'
}
if ($null -ne $Data.sourceIdentity.selectedStyle -or -not $Data.sourceIdentity.styleNeutral) {
    throw 'Motion pipeline fixture must remain style-neutral.'
}
if (($Data.skeleton.bones.name -join ',') -ne 'root,pelvis,spine,foot_l,foot_r') {
    throw "Unexpected bone hierarchy: $($Data.skeleton.bones.name -join ',')"
}
if ($Data.clip.fps -ne 30 -or $Data.clip.startFrame -ne 1 -or $Data.clip.endFrame -ne 31 -or
    [Math]::Abs($Data.clip.durationSeconds - 1.0) -gt 0.000001) {
    throw 'Motion timing evidence does not match the brief.'
}
$ExpectedRoot = '1:0.0,0.0,0.0|16:0.5,0.0,0.0|31:1.0,0.0,0.0'
$ActualRoot = ($Data.clip.rootSamples | ForEach-Object {
    "$($_.frame):$($_.locationMeters -join ',')"
}) -join '|'
if ($ActualRoot -cne $ExpectedRoot) { throw "Unexpected root samples: $ActualRoot" }
$ExpectedEvents = 'contact.left:1|contact.right:16|contact.left:31'
$ActualEvents = ($Data.clip.events | ForEach-Object { "$($_.name):$($_.frame)" }) -join '|'
if ($ActualEvents -cne $ExpectedEvents) { throw "Unexpected contact events: $ActualEvents" }
if ($Data.fbxTransport.deterministicBytes -ne $false -or
    ($Data.fbxTransport.PSObject.Properties.Name -contains 'sha256')) {
    throw 'FBX byte nondeterminism must be recorded honestly.'
}
foreach ($RelativePath in @($Data.blenderArtifact.file, $Data.fbxTransport.file)) {
    $Artifact = Join-Path $Output $RelativePath
    if (-not (Test-Path -LiteralPath $Artifact) -or (Get-Item -LiteralPath $Artifact).Length -eq 0) {
        throw "Missing or empty motion artifact: $Artifact"
    }
}

Write-Output "PASS motion bones=$($Data.skeleton.bones.Count) clip=$($Data.clip.name) duration=$($Data.clip.durationSeconds)s"
exit 0
