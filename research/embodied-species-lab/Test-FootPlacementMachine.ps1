param(
    [string] $EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',
    [string] $Python = 'C:\Users\alexa\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe',
    [switch] $Animated
)

$ErrorActionPreference = 'Stop'
$RepoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
$Project = Join-Path $RepoRoot 'Unreal\Prototype\Prototype.uproject'
$EditorCmd = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$Builder = Join-Path $PSScriptRoot 'Build-CharacterMovementChallenger.ps1'
$PackagedPlugin = Join-Path $PSScriptRoot 'Derived\M\CharacterMovementChallenger.uplugin'
$Contract = Join-Path $PSScriptRoot 'species\foot-placement-route.json'
$Policy = Join-Path $PSScriptRoot $(if ($Animated) { 'animated_foot_placement_policy.py' } else { 'foot_placement_policy.py' })
$GeneratedRoot = Join-Path $RepoRoot 'Unreal\Prototype\Content\Gaters\Generated\CharacterLab'
$EvidenceRoot = Join-Path $RepoRoot 'Unreal\Prototype\Saved\CharacterLab'
$Map = '/Game/Gaters/Generated/CharacterLab/L_CharacterLab'

$GeneratedFiles = @(
    'SK_GeneratedHumanoid.uasset',
    'SK_GeneratedHumanoid_Skeleton.uasset',
    'SK_GeneratedHumanoid_PhysicsAsset.uasset',
    'L_CharacterLab.umap'
)
foreach ($RequiredPath in @($Project, $EditorCmd, $Python, $Builder, $Contract, $Policy)) {
    if (-not (Test-Path -LiteralPath $RequiredPath -PathType Leaf)) {
        throw "Required Foot Placement path is missing: $RequiredPath"
    }
}
foreach ($GeneratedFile in $GeneratedFiles) {
    $GeneratedPath = Join-Path $GeneratedRoot $GeneratedFile
    if (-not (Test-Path -LiteralPath $GeneratedPath -PathType Leaf)) {
        throw "Run Test-UnrealHumanoidIntake.ps1 first; CharacterLab asset is missing: $GeneratedPath"
    }
}

& $Builder -EngineRoot $EngineRoot
if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $PackagedPlugin -PathType Leaf)) {
    throw 'Foot Placement challenger plugin did not package successfully.'
}
New-Item -ItemType Directory -Path $EvidenceRoot -Force | Out-Null

function Invoke-FootPlacement([int] $Iteration) {
    $Stem = if ($Animated) { 'animated-foot-placement' } else { 'foot-placement' }
    $Evidence = Join-Path $EvidenceRoot "$Stem-$Iteration.json"
    $Evaluation = Join-Path $EvidenceRoot "$Stem-policy-$Iteration.json"
    $Log = Join-Path $EvidenceRoot "$Stem-$Iteration.log"
    Remove-Item -LiteralPath $Evidence, $Evaluation, $Log -Force -ErrorAction SilentlyContinue

    $env:GATERS_FOOT_PLACEMENT_CONTRACT = $Contract
    $env:GATERS_FOOT_PLACEMENT_EVIDENCE = $Evidence
    if ($Animated) { $env:GATERS_FOOT_PLACEMENT_ANIMATED = '1' }
    try {
        & $EditorCmd $Project $Map -game -unattended -nop4 -nosplash -nullrhi -nosound `
            "-PLUGIN=$PackagedPlugin" `
            '-ExecCmds=Automation RunTests Gaters.CharacterLab.CharacterMovement.UnevenFootPlacement;Quit' `
            '-TestExit=Automation Test Queue Empty' "-abslog=$Log"
        $EditorExitCode = $LASTEXITCODE
    }
    finally {
        Remove-Item Env:GATERS_FOOT_PLACEMENT_CONTRACT, Env:GATERS_FOOT_PLACEMENT_EVIDENCE `
            -ErrorAction SilentlyContinue
        Remove-Item Env:GATERS_FOOT_PLACEMENT_ANIMATED -ErrorAction SilentlyContinue
    }
    if ($EditorExitCode -ne 0) {
        throw "Foot Placement automation iteration $Iteration failed with exit code $EditorExitCode. Log: $Log"
    }
    if (-not (Test-Path -LiteralPath $Evidence -PathType Leaf)) {
        throw "Foot Placement evidence is missing: $Evidence"
    }
    $Successes = @(Select-String -LiteralPath $Log -SimpleMatch 'Test Completed. Result={Success}' | `
        Where-Object Line -Like '*Gaters.CharacterLab.CharacterMovement.UnevenFootPlacement*')
    if ($Successes.Count -ne 1 -or (Select-String -LiteralPath $Log -SimpleMatch 'Test Completed. Result={Fail}')) {
        throw "Expected one passing Foot Placement test in $Log"
    }

    & $Python $Policy --report $Evidence --contract $Contract --output $Evaluation
    if ($LASTEXITCODE -ne 0) {
        throw "Foot Placement policy iteration $Iteration failed. Report: $Evaluation"
    }
    return [PSCustomObject]@{
        Evidence = $Evidence
        EvidenceText = Get-Content -Raw -LiteralPath $Evidence
        Evaluation = $Evaluation
        EvaluationText = Get-Content -Raw -LiteralPath $Evaluation
    }
}

$First = Invoke-FootPlacement 1
$Second = Invoke-FootPlacement 2
if ($First.EvidenceText -cne $Second.EvidenceText) {
    throw 'Repeated native Foot Placement runs produced different evidence.'
}
if ($First.EvaluationText -cne $Second.EvaluationText) {
    throw 'Repeated Foot Placement policies produced different evidence.'
}

$NativeHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $Second.Evidence).Hash
$PolicyHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $Second.Evaluation).Hash
$Machine = if ($Animated) { 'animated-foot-placement-machine' } else { 'foot-placement-machine' }
Write-Output "PASS $Machine native_sha256=$NativeHash policy_sha256=$PolicyHash"
