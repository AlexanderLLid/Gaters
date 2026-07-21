param(
    [string] $UnrealEditor = 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe',
    [string] $Blender = 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe',
    [string] $Python = 'C:\Users\alexa\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
)

$ErrorActionPreference = 'Stop'
$RepoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
$Project = Join-Path $RepoRoot 'Unreal\Prototype\Prototype.uproject'
$Builder = Join-Path $PSScriptRoot 'Build-RecoveryHumanoid.ps1'
$LocomotionHarness = Join-Path $PSScriptRoot 'Test-LocomotionMachine.ps1'
$PluginBuilder = Join-Path $PSScriptRoot 'Build-CharacterPhysicsProfileAdapter.ps1'
$PackagedPlugin = Join-Path $PSScriptRoot 'Derived\P\CharacterPhysicsProfileAdapter.uplugin'
$Importer = Join-Path $PSScriptRoot 'unreal\import_generated_humanoid.py'
$Policy = Join-Path $PSScriptRoot 'unreal_intake_policy.py'
$Source = Join-Path $PSScriptRoot 'Derived\humanoid-recovery-v1\impact-forward'
$Manifest = Join-Path $Source 'manifest.json'
$Profile = Join-Path $Source 'physical-profile.json'
$LocomotionManifest = Join-Path $PSScriptRoot 'Derived\humanoid-locomotion-v1\locomotion-manifest.json'
$IkContract = Join-Path $PSScriptRoot 'species\ik-evaluation.json'
$GeneratedRoot = Join-Path $RepoRoot 'Unreal\Prototype\Content\Gaters\Generated\CharacterLab'
$EvidenceRoot = Join-Path $RepoRoot 'Unreal\Prototype\Saved\CharacterLab'
$Destination = '/Game/Gaters/Generated/CharacterLab'
$TestMap = "$Destination/L_CharacterLab"
$TestMapFile = Join-Path $GeneratedRoot 'L_CharacterLab.umap'

foreach ($RequiredPath in @(
        $UnrealEditor,
        $Blender,
        $Python,
        $Project,
        $Builder,
        $LocomotionHarness,
        $PluginBuilder,
        $Importer,
        $Policy,
        $IkContract
    )) {
    if (-not (Test-Path -LiteralPath $RequiredPath -PathType Leaf)) {
        throw "Required humanoid intake path is missing: $RequiredPath"
    }
}

$InteractiveEditor = Get-Process -Name 'UnrealEditor' -ErrorAction SilentlyContinue
if ($InteractiveEditor) {
    throw 'Unreal Editor is running. Close the interactive editor before running the humanoid intake challenger.'
}

$PrototypeRoot = (Resolve-Path -LiteralPath (Join-Path $RepoRoot 'Unreal\Prototype')).Path
foreach ($CleanupPath in @($GeneratedRoot, $EvidenceRoot)) {
    $Candidate = [System.IO.Path]::GetFullPath($CleanupPath)
    if (-not $Candidate.StartsWith($PrototypeRoot + [System.IO.Path]::DirectorySeparatorChar, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing cleanup outside Unreal prototype: $Candidate"
    }
    if (Test-Path -LiteralPath $CleanupPath) {
        Remove-Item -LiteralPath $CleanupPath -Recurse -Force
    }
}
New-Item -ItemType Directory -Path $EvidenceRoot -Force | Out-Null

& $PluginBuilder
if ($LASTEXITCODE -ne 0) { throw "Character physics profile adapter build failed with exit code $LASTEXITCODE." }
if (-not (Test-Path -LiteralPath $PackagedPlugin -PathType Leaf)) {
    throw "Packaged character physics profile adapter is missing: $PackagedPlugin"
}

& $Builder -Blender $Blender -CaseId 'impact-forward'
if ($LASTEXITCODE -ne 0) { throw "Recovery humanoid build failed with exit code $LASTEXITCODE." }

& $LocomotionHarness -Blender $Blender -Python $Python
if ($LASTEXITCODE -ne 0) { throw "Locomotion humanoid harness failed with exit code $LASTEXITCODE." }
if (-not (Test-Path -LiteralPath $LocomotionManifest -PathType Leaf)) {
    throw "Locomotion manifest is missing: $LocomotionManifest"
}

function Invoke-HumanoidIntake([int] $Iteration) {
    if (Test-Path -LiteralPath $GeneratedRoot) {
        Remove-Item -LiteralPath $GeneratedRoot -Recurse -Force
    }
    $Report = Join-Path $EvidenceRoot "unreal-intake-$Iteration.json"
    $Evaluation = Join-Path $EvidenceRoot "unreal-intake-policy-$Iteration.json"
    $IkEvidence = Join-Path $EvidenceRoot "evaluated-ik-$Iteration.json"
    $IkLog = Join-Path $EvidenceRoot "evaluated-ik-$Iteration.log"
    $Log = Join-Path $EvidenceRoot "unreal-intake-$Iteration.log"
    Remove-Item -LiteralPath $Report, $Evaluation, $IkEvidence, $IkLog, $Log `
        -Force -ErrorAction SilentlyContinue

    $env:GATERS_CHARACTER_MANIFEST = $Manifest
    $env:GATERS_CHARACTER_PROFILE = $Profile
    $env:GATERS_LOCOMOTION_MANIFEST = $LocomotionManifest
    $env:GATERS_CHARACTER_DESTINATION = $Destination
    $env:GATERS_CHARACTER_REPORT = $Report
    try {
        & $UnrealEditor $Project '-unattended' '-nop4' '-nosplash' '-nullrhi' '-nosound' `
            "-PLUGIN=$PackagedPlugin" "-ExecutePythonScript=$Importer" "-abslog=$Log"
        if ($LASTEXITCODE -ne 0) {
            throw "Unreal humanoid intake iteration $Iteration failed with exit code $LASTEXITCODE. Log: $Log"
        }
    }
    finally {
        Remove-Item Env:GATERS_CHARACTER_MANIFEST, Env:GATERS_CHARACTER_PROFILE, `
            Env:GATERS_LOCOMOTION_MANIFEST, `
            Env:GATERS_CHARACTER_DESTINATION, Env:GATERS_CHARACTER_REPORT -ErrorAction SilentlyContinue
    }
    if (-not (Test-Path -LiteralPath $Log -PathType Leaf)) { throw "Unreal intake log is missing: $Log" }
    $LogFailures = Select-String -LiteralPath $Log -Pattern `
        'LogPython: Error|LogEditorPythonExecuter: Error|LogInterchangeEngine: Error|LogInterchangeImport: Error'
    if ($LogFailures) { throw "Unreal intake iteration $Iteration logged an error: $($LogFailures[0].Line)" }
    if (-not (Test-Path -LiteralPath $Report -PathType Leaf)) { throw "Unreal intake report is missing: $Report" }

    $env:GATERS_IK_EVIDENCE = $IkEvidence
    $env:GATERS_IK_CONTRACT = $IkContract
    try {
        & $UnrealEditor $Project '-unattended' '-nop4' '-nosplash' '-nullrhi' '-nosound' `
            "-PLUGIN=$PackagedPlugin" `
            '-ExecCmds=Automation RunTests Gaters.CharacterLab.EvaluatedIK;Quit' `
            '-TestExit=Automation Test Queue Empty' "-abslog=$IkLog"
        if ($LASTEXITCODE -ne 0) {
            throw "Evaluated IK iteration $Iteration failed with exit code $LASTEXITCODE. Log: $IkLog"
        }
    }
    finally {
        Remove-Item Env:GATERS_IK_EVIDENCE, Env:GATERS_IK_CONTRACT -ErrorAction SilentlyContinue
    }
    if (-not (Test-Path -LiteralPath $IkEvidence -PathType Leaf)) {
        throw "Evaluated IK evidence is missing: $IkEvidence"
    }

    & $Python $Policy `
        --manifest $Manifest `
        --profile $Profile `
        --locomotion-manifest $LocomotionManifest `
        --report $Report `
        --ik-evidence $IkEvidence `
        --ik-contract $IkContract `
        --output $Evaluation
    $PolicyExitCode = $LASTEXITCODE
    if ($PolicyExitCode -ne 0 -and $PolicyExitCode -ne 2) {
        throw "Intake policy iteration $Iteration failed to evaluate with exit code $PolicyExitCode."
    }
    if (-not (Test-Path -LiteralPath $Evaluation -PathType Leaf)) {
        throw "Unreal intake policy report is missing: $Evaluation"
    }
    $ReportData = Get-Content -Raw -LiteralPath $Report | ConvertFrom-Json
    if ($ReportData.testMap -cne $TestMap) {
        throw "Unreal intake iteration $Iteration did not report the CharacterLab map."
    }
    if (-not (Test-Path -LiteralPath $TestMapFile -PathType Leaf)) {
        throw "Unreal intake iteration $Iteration did not create the CharacterLab map: $TestMapFile"
    }
    return [PSCustomObject]@{
        ReportPath = $Report
        ReportText = Get-Content -Raw -LiteralPath $Report
        EvaluationPath = $Evaluation
        EvaluationText = Get-Content -Raw -LiteralPath $Evaluation
        Evaluation = Get-Content -Raw -LiteralPath $Evaluation | ConvertFrom-Json
        IkEvidencePath = $IkEvidence
        IkEvidenceText = Get-Content -Raw -LiteralPath $IkEvidence
        LogPath = $Log
    }
}

$First = Invoke-HumanoidIntake 1
$Second = Invoke-HumanoidIntake 2
if ($First.ReportText -cne $Second.ReportText) {
    throw 'Repeated Unreal humanoid intakes produced different stable reports.'
}
if ($First.EvaluationText -cne $Second.EvaluationText) {
    throw 'Repeated Unreal humanoid intakes produced different policy evaluations.'
}
if ($First.IkEvidenceText -cne $Second.IkEvidenceText) {
    throw 'Repeated evaluated IK runs produced different stable evidence.'
}
if (-not $Second.Evaluation.passed) {
    $RuleIds = $Second.Evaluation.issues.ruleId -join ','
    Write-Output "FALSIFIED unreal-humanoid-intake rules=$RuleIds report=$($Second.ReportPath) evaluation=$($Second.EvaluationPath)"
    exit 2
}

$MalformedProfile = Join-Path $EvidenceRoot 'malformed-physical-profile.json'
$MalformedReport = Join-Path $EvidenceRoot 'malformed-unreal-intake.json'
$MalformedLog = Join-Path $EvidenceRoot 'malformed-unreal-intake.log'
$Malformed = Get-Content -Raw -LiteralPath $Profile | ConvertFrom-Json
$Malformed.joints[0].child = 'missing_bone'
$MalformedJson = $Malformed | ConvertTo-Json -Depth 20
[System.IO.File]::WriteAllText(
    $MalformedProfile,
    $MalformedJson,
    [System.Text.UTF8Encoding]::new($false))
Remove-Item -LiteralPath $MalformedReport, $MalformedLog -Force -ErrorAction SilentlyContinue

$env:GATERS_CHARACTER_MANIFEST = $Manifest
$env:GATERS_CHARACTER_PROFILE = $MalformedProfile
$env:GATERS_LOCOMOTION_MANIFEST = $LocomotionManifest
$env:GATERS_CHARACTER_DESTINATION = $Destination
$env:GATERS_CHARACTER_REPORT = $MalformedReport
$env:GATERS_EXPECT_ADAPTER_ERROR = 'Joint references unknown skeleton bone: missing_bone'
try {
    & $UnrealEditor $Project '-unattended' '-nop4' '-nosplash' '-nullrhi' '-nosound' `
        "-PLUGIN=$PackagedPlugin" "-ExecutePythonScript=$Importer" "-abslog=$MalformedLog"
}
finally {
    Remove-Item Env:GATERS_CHARACTER_MANIFEST, Env:GATERS_CHARACTER_PROFILE, `
        Env:GATERS_LOCOMOTION_MANIFEST, `
        Env:GATERS_CHARACTER_DESTINATION, Env:GATERS_CHARACTER_REPORT, `
        Env:GATERS_EXPECT_ADAPTER_ERROR -ErrorAction SilentlyContinue
}
if (Test-Path -LiteralPath $MalformedReport -PathType Leaf) {
    throw "Malformed humanoid profile produced a success report: $MalformedReport"
}
if (-not (Test-Path -LiteralPath $MalformedLog -PathType Leaf)) {
    throw "Malformed humanoid intake log is missing: $MalformedLog"
}
$MalformedFailure = Select-String -LiteralPath $MalformedLog -Quiet -SimpleMatch `
    'GATERS_CHARACTER_IMPORT_REJECTED error=Joint references unknown skeleton bone: missing_bone'
if (-not $MalformedFailure) {
    throw "Malformed humanoid profile did not fail causally. Log: $MalformedLog"
}
if (Select-String -LiteralPath $MalformedLog -Quiet -Pattern `
        'LogPython: Error|LogEditorPythonExecuter: Error') {
    throw "Expected malformed-profile rejection logged a Python execution error: $MalformedLog"
}
if (Select-String -LiteralPath $MalformedLog -Quiet -SimpleMatch 'GATERS_CHARACTER_IMPORT_OK') {
    throw "Malformed humanoid profile logged an import success marker: $MalformedLog"
}

Write-Output "PASS unreal-humanoid-intake report=$($Second.ReportPath) evaluation=$($Second.EvaluationPath)"
exit 0
