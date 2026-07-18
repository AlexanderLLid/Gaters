param(
    [string] $EngineRoot = 'C:\Program Files\Epic Games\UE_5.8'
)

$ErrorActionPreference = 'Stop'
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$RepoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\..'))
$Project = Join-Path $ProjectRoot 'Prototype.uproject'
$EditorCmd = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$BlenderTest = Join-Path $RepoRoot 'SourceAssets\Blender\Test-MotionFixture.ps1'
$Manifest = Join-Path $RepoRoot 'SourceAssets\Blender\Derived\neutral-motion-v1\manifest.json'
$Importer = Join-Path $PSScriptRoot 'ImportMotionFixture.py'
$Report = Join-Path $ProjectRoot 'Saved\AssetImport\neutral-motion.json'
$GeneratedDirectory = Join-Path $ProjectRoot 'Content\Gaters\Generated\Motion'

foreach ($Path in @($Project, $EditorCmd, $BlenderTest)) {
    if (-not (Test-Path -LiteralPath $Path)) { throw "Missing required path: $Path" }
}
& $BlenderTest
if ($LASTEXITCODE -ne 0) { throw "Blender motion fixture failed with exit code $LASTEXITCODE." }
if (-not (Test-Path -LiteralPath $Manifest)) { throw "Missing motion manifest: $Manifest" }
if (-not (Test-Path -LiteralPath $Importer)) { throw "Missing motion importer: $Importer" }

if (Test-Path -LiteralPath $GeneratedDirectory) {
    $ResolvedProject = [System.IO.Path]::GetFullPath($ProjectRoot).TrimEnd('\') + '\'
    $ResolvedGenerated = [System.IO.Path]::GetFullPath($GeneratedDirectory)
    if (-not $ResolvedGenerated.StartsWith($ResolvedProject, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to clear generated motion outside the project: $ResolvedGenerated"
    }
    Remove-Item -LiteralPath $ResolvedGenerated -Recurse -Force
}

function Invoke-MotionImport([int] $Iteration) {
    Remove-Item -LiteralPath $Report -Force -ErrorAction SilentlyContinue
    $env:GATERS_MOTION_MANIFEST = $Manifest
    $env:GATERS_IMPORT_DESTINATION = '/Game/Gaters/Generated/Motion'
    $env:GATERS_IMPORT_NAME = 'SK_NeutralMotionFixture'
    $env:GATERS_IMPORT_REPORT = $Report
    try {
        & $EditorCmd $Project -unattended -nop4 -nosplash -nullrhi -nosound `
            "-ExecutePythonScript=$Importer" "-log=MotionFixtureImport-$Iteration.log"
        if ($LASTEXITCODE -ne 0) {
            throw "Motion import iteration $Iteration failed with exit code $LASTEXITCODE."
        }
    }
    finally {
        Remove-Item Env:GATERS_MOTION_MANIFEST, Env:GATERS_IMPORT_DESTINATION, `
            Env:GATERS_IMPORT_NAME, Env:GATERS_IMPORT_REPORT -ErrorAction SilentlyContinue
    }
    $ImportLog = Join-Path $ProjectRoot "Saved\Logs\MotionFixtureImport-$Iteration.log"
    $Failures = Select-String -LiteralPath $ImportLog -Pattern `
        'Ensure condition failed|LogPython: Error|LogInterchangeEngine: Error|LogInterchangeImport: Error'
    if ($Failures) { throw "Motion import iteration $Iteration logged an error: $($Failures[0].Line)" }
    if (-not (Test-Path -LiteralPath $Report)) { throw "Motion import iteration $Iteration produced no report." }
    return Get-Content -Raw -LiteralPath $Report | ConvertFrom-Json
}

$First = Invoke-MotionImport 1
$Second = Invoke-MotionImport 2
foreach ($Field in @('schemaVersion', 'importerVersion', 'engineVersion', 'manifestSha256',
        'skeletalMeshPath', 'skeletonPath', 'animationPath', 'durationSeconds', 'sampledKeys')) {
    if ($First.$Field -ne $Second.$Field) { throw "Repeated motion import changed $Field." }
}
if (($First.boneNames -join ',') -cne ($Second.boneNames -join ',')) {
    throw 'Repeated motion import changed reference bones.'
}
foreach ($Field in @('requiredReferenceHierarchy', 'rootSamples', 'meshBoundsSizeCentimeters',
        'sourceContactEvents', 'importSettings')) {
    if (($First.$Field | ConvertTo-Json -Depth 8 -Compress) -cne
        ($Second.$Field | ConvertTo-Json -Depth 8 -Compress)) {
        throw "Repeated motion import changed $Field."
    }
}
if ($First.schemaVersion -ne 1 -or $First.importerVersion -ne 1) {
    throw 'Motion importer versions were not recorded.'
}
foreach ($ClassName in @('SkeletalMesh', 'Skeleton', 'AnimSequence')) {
    if ($First.assetClasses -notcontains $ClassName) { throw "Motion import is missing $ClassName." }
}
foreach ($Bone in @('root', 'pelvis', 'spine', 'foot_l', 'foot_r')) {
    if ($First.boneNames -notcontains $Bone) { throw "Imported skeleton is missing $Bone." }
}
if ([Math]::Abs($First.durationSeconds - 1.0) -gt 0.001 -or $First.sampledKeys -lt 30) {
    throw "Unexpected imported clip timing: duration=$($First.durationSeconds) keys=$($First.sampledKeys)"
}
$ExpectedRootX = @(0.0, 50.0, 100.0)
for ($Index = 0; $Index -lt $ExpectedRootX.Count; $Index++) {
    $ActualX = $First.rootSamples[$Index].translationCentimeters[0]
    if ([Math]::Abs($ActualX - $ExpectedRootX[$Index]) -gt 0.01) {
        throw "Imported root sample $Index has x=$ActualX cm, expected $($ExpectedRootX[$Index]) cm."
    }
}
if ($First.meshBoundsSizeCentimeters[2] -lt 100.0) {
    throw "Imported skeletal mesh is not meter-authored scale: $($First.meshBoundsSizeCentimeters -join 'x') cm"
}
if ($First.contactEventsTransported -ne $false) {
    throw 'The first fixture must not claim that FBX transported Blender timeline markers as Unreal notifies.'
}
if (($First.sourceContactEvents.name -join ',') -cne 'contact.left,contact.right,contact.left') {
    throw 'Source contact-event evidence was not preserved in the import report.'
}
if (-not (Test-Path -LiteralPath $GeneratedDirectory)) {
    throw "Generated Unreal motion directory is missing: $GeneratedDirectory"
}

Write-Output "PASS motion-import bones=$($First.boneNames.Count) duration=$($First.durationSeconds)s keys=$($First.sampledKeys)"
exit 0
