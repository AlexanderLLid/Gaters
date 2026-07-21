param(
    [string] $EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',
    [string] $Python = 'C:\Users\alexa\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
)

$ErrorActionPreference = 'Stop'
$RepoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
$Project = Join-Path $RepoRoot 'Unreal\Prototype\Prototype.uproject'
$EditorCmd = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$Builder = Join-Path $PSScriptRoot 'Build-CharacterMovementChallenger.ps1'
$PackagedPlugin = Join-Path $PSScriptRoot 'Derived\M\CharacterMovementChallenger.uplugin'
$Contract = Join-Path $PSScriptRoot 'species\character-movement-route.json'
$Policy = Join-Path $PSScriptRoot 'character_movement_policy.py'
$GeneratedRoot = Join-Path $RepoRoot 'Unreal\Prototype\Content\Gaters\Generated\CharacterLab'
$EvidenceRoot = Join-Path $RepoRoot 'Unreal\Prototype\Saved\CharacterLab'
$Map = '/Game/Gaters/Generated/CharacterLab/L_CharacterLab'

$GeneratedFiles = @(
    'SK_GeneratedHumanoid.uasset',
    'SK_GeneratedHumanoid_Skeleton.uasset',
    'SK_GeneratedHumanoid_PhysicsAsset.uasset',
    'IK_GeneratedHumanoid.uasset',
    'A_Idle.uasset',
    'A_Walk.uasset',
    'A_Run.uasset',
    'A_TurnLeft.uasset',
    'A_Stop.uasset',
    'A_Jump.uasset',
    'A_Fall.uasset',
    'A_Land.uasset',
    'L_CharacterLab.umap'
)
foreach ($RequiredPath in @($Project, $EditorCmd, $Python, $Builder, $Contract, $Policy)) {
    if (-not (Test-Path -LiteralPath $RequiredPath -PathType Leaf)) {
        throw "Required CharacterMovement path is missing: $RequiredPath"
    }
}
foreach ($GeneratedFile in $GeneratedFiles) {
    $GeneratedPath = Join-Path $GeneratedRoot $GeneratedFile
    if (-not (Test-Path -LiteralPath $GeneratedPath -PathType Leaf)) {
        throw "Run Test-UnrealHumanoidIntake.ps1 first; cooked CharacterLab asset is missing: $GeneratedPath"
    }
}

& $Builder -EngineRoot $EngineRoot
if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $PackagedPlugin -PathType Leaf)) {
    throw 'CharacterMovement challenger plugin did not package successfully.'
}
New-Item -ItemType Directory -Path $EvidenceRoot -Force | Out-Null

function Invoke-FlatRoute([int] $Iteration) {
    $Evidence = Join-Path $EvidenceRoot "character-movement-$Iteration.json"
    $Evaluation = Join-Path $EvidenceRoot "character-movement-policy-$Iteration.json"
    $Log = Join-Path $EvidenceRoot "character-movement-$Iteration.log"
    Remove-Item -LiteralPath $Evidence, $Evaluation, $Log -Force -ErrorAction SilentlyContinue

    $env:GATERS_CHARACTER_MOVEMENT_CONTRACT = $Contract
    $env:GATERS_CHARACTER_MOVEMENT_EVIDENCE = $Evidence
    try {
        & $EditorCmd $Project $Map -game -unattended -nop4 -nosplash -nullrhi -nosound `
            "-PLUGIN=$PackagedPlugin" `
            '-ExecCmds=Automation RunTests Gaters.CharacterLab.CharacterMovement.FlatRoute;Quit' `
            '-TestExit=Automation Test Queue Empty' "-abslog=$Log"
        $EditorExitCode = $LASTEXITCODE
    }
    finally {
        Remove-Item Env:GATERS_CHARACTER_MOVEMENT_CONTRACT, Env:GATERS_CHARACTER_MOVEMENT_EVIDENCE `
            -ErrorAction SilentlyContinue
    }
    if ($EditorExitCode -ne 0) {
        throw "CharacterMovement automation iteration $Iteration failed with exit code $EditorExitCode. Log: $Log"
    }
    if (-not (Test-Path -LiteralPath $Evidence -PathType Leaf)) {
        throw "CharacterMovement evidence is missing: $Evidence"
    }
    $Successes = @(Select-String -LiteralPath $Log -SimpleMatch 'Test Completed. Result={Success}' | `
        Where-Object Line -Like '*Gaters.CharacterLab.CharacterMovement.FlatRoute*')
    if ($Successes.Count -ne 1 -or (Select-String -LiteralPath $Log -SimpleMatch 'Test Completed. Result={Fail}')) {
        throw "Expected one passing CharacterMovement test in $Log"
    }

    & $Python $Policy --report $Evidence --contract $Contract --output $Evaluation
    if ($LASTEXITCODE -ne 0) {
        throw "CharacterMovement policy iteration $Iteration failed. Report: $Evaluation"
    }
    return [PSCustomObject]@{
        Evidence = $Evidence
        EvidenceText = Get-Content -Raw -LiteralPath $Evidence
        Evaluation = $Evaluation
        EvaluationText = Get-Content -Raw -LiteralPath $Evaluation
    }
}

$First = Invoke-FlatRoute 1
$Second = Invoke-FlatRoute 2
if ($First.EvidenceText -cne $Second.EvidenceText) {
    throw 'Repeated native CharacterMovement runs produced different evidence.'
}
if ($First.EvaluationText -cne $Second.EvaluationText) {
    throw 'Repeated CharacterMovement policies produced different evidence.'
}

$NativeHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $Second.Evidence).Hash
$PolicyHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $Second.Evaluation).Hash
Write-Output "PASS character-movement-machine native_sha256=$NativeHash policy_sha256=$PolicyHash"
