param(
    [string] $EngineRoot = 'C:\Program Files\Epic Games\UE_5.8'
)

$ErrorActionPreference = 'Stop'
$RepoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
$Project = Join-Path $RepoRoot 'Unreal\Prototype\Prototype.uproject'
$RunUAT = Join-Path $EngineRoot 'Engine\Build\BatchFiles\RunUAT.bat'
$EditorCmd = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$SourcePlugin = Join-Path $PSScriptRoot 'unreal\CharacterPhysicsProfileAdapter\CharacterPhysicsProfileAdapter.uplugin'
$PackageRoot = Join-Path $PSScriptRoot 'Derived\P'
$PackagedPlugin = Join-Path $PackageRoot 'CharacterPhysicsProfileAdapter.uplugin'
$EvidenceRoot = Join-Path $RepoRoot 'Unreal\Prototype\Saved\CharacterLab'
$AutomationLog = Join-Path $EvidenceRoot 'physics-profile-adapter-tests.log'

foreach ($RequiredPath in @($Project, $RunUAT, $EditorCmd, $SourcePlugin)) {
    if (-not (Test-Path -LiteralPath $RequiredPath -PathType Leaf)) {
        throw "Required plugin build path is missing: $RequiredPath"
    }
}

if (Get-Process -Name 'UnrealEditor' -ErrorAction SilentlyContinue) {
    throw 'Unreal Editor is running. Close the interactive editor before building the character adapter.'
}

$ResolvedLab = [System.IO.Path]::GetFullPath($PSScriptRoot).TrimEnd('\') + '\'
$ResolvedPackage = [System.IO.Path]::GetFullPath($PackageRoot)
if (-not $ResolvedPackage.StartsWith($ResolvedLab, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "Refusing plugin cleanup outside the embodied species lab: $ResolvedPackage"
}
if (Test-Path -LiteralPath $PackageRoot) {
    Remove-Item -LiteralPath $PackageRoot -Recurse -Force
}

$BuildDrive = 'Q'
if (Get-PSDrive -Name $BuildDrive -ErrorAction SilentlyContinue) {
    throw "Temporary plugin build drive $BuildDrive`: is already in use."
}
subst.exe "$BuildDrive`:" $PSScriptRoot
if ($LASTEXITCODE -ne 0) {
    throw "Could not map temporary plugin build drive $BuildDrive`: to $PSScriptRoot."
}
try {
    & $RunUAT BuildPlugin `
        "-Plugin=$BuildDrive`:\unreal\CharacterPhysicsProfileAdapter\CharacterPhysicsProfileAdapter.uplugin" `
        "-Package=$BuildDrive`:\Derived\P" -TargetPlatforms=Win64 -Rocket
    $BuildExitCode = $LASTEXITCODE
}
finally {
    subst.exe "$BuildDrive`:" /D
}
if ($BuildExitCode -ne 0) {
    throw "Character adapter plugin build failed with exit code $BuildExitCode."
}
if (-not (Test-Path -LiteralPath $PackagedPlugin -PathType Leaf)) {
    throw "Packaged character adapter is missing: $PackagedPlugin"
}

New-Item -ItemType Directory -Path $EvidenceRoot -Force | Out-Null
Remove-Item -LiteralPath $AutomationLog -Force -ErrorAction SilentlyContinue
& $EditorCmd $Project -unattended -nop4 -nosplash -nullrhi -nosound `
    "-PLUGIN=$PackagedPlugin" `
    '-ExecCmds=Automation RunTests Gaters.CharacterLab.PhysicsProfileAdapter;Quit' `
    '-TestExit=Automation Test Queue Empty' "-abslog=$AutomationLog"
if ($LASTEXITCODE -ne 0) {
    throw "Character adapter automation failed with exit code $LASTEXITCODE. Log: $AutomationLog"
}
$Successes = @(Select-String -LiteralPath $AutomationLog -SimpleMatch 'Test Completed. Result={Success}' | `
    Where-Object Line -Like '*Gaters.CharacterLab.PhysicsProfileAdapter*')
if ($Successes.Count -ne 6) {
    throw "Expected six passing character adapter tests; found $($Successes.Count). Log: $AutomationLog"
}

Write-Output $PackagedPlugin
