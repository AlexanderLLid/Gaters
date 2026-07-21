param(
    [string] $EngineRoot = 'C:\Program Files\Epic Games\UE_5.8'
)

$ErrorActionPreference = 'Stop'
$RunUAT = Join-Path $EngineRoot 'Engine\Build\BatchFiles\RunUAT.bat'
$SourcePlugin = Join-Path $PSScriptRoot 'unreal\CharacterMovementChallenger\CharacterMovementChallenger.uplugin'
$PackageRoot = Join-Path $PSScriptRoot 'Derived\M'
$PackagedPlugin = Join-Path $PackageRoot 'CharacterMovementChallenger.uplugin'

foreach ($RequiredPath in @($RunUAT, $SourcePlugin)) {
    if (-not (Test-Path -LiteralPath $RequiredPath -PathType Leaf)) {
        throw "Required movement challenger build path is missing: $RequiredPath"
    }
}

if (Get-Process -Name 'UnrealEditor' -ErrorAction SilentlyContinue) {
    throw 'Unreal Editor is running. Close the interactive editor before building the movement challenger.'
}

$ResolvedLab = [System.IO.Path]::GetFullPath($PSScriptRoot).TrimEnd('\') + '\'
$ResolvedPackage = [System.IO.Path]::GetFullPath($PackageRoot)
if (-not $ResolvedPackage.StartsWith($ResolvedLab, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "Refusing plugin cleanup outside the embodied species lab: $ResolvedPackage"
}
if (Test-Path -LiteralPath $PackageRoot) {
    Remove-Item -LiteralPath $PackageRoot -Recurse -Force
}

$BuildDrive = 'R'
if (Get-PSDrive -Name $BuildDrive -ErrorAction SilentlyContinue) {
    throw "Temporary plugin build drive $BuildDrive`: is already in use."
}
subst.exe "$BuildDrive`:" $PSScriptRoot
if ($LASTEXITCODE -ne 0) {
    throw "Could not map temporary plugin build drive $BuildDrive`: to $PSScriptRoot."
}
try {
    & $RunUAT BuildPlugin `
        "-Plugin=$BuildDrive`:\unreal\CharacterMovementChallenger\CharacterMovementChallenger.uplugin" `
        "-Package=$BuildDrive`:\Derived\M" -TargetPlatforms=Win64 -Rocket
    $BuildExitCode = $LASTEXITCODE
}
finally {
    subst.exe "$BuildDrive`:" /D
}
if ($BuildExitCode -ne 0) {
    throw "Character movement challenger build failed with exit code $BuildExitCode."
}
if (-not (Test-Path -LiteralPath $PackagedPlugin -PathType Leaf)) {
    throw "Packaged movement challenger is missing: $PackagedPlugin"
}

$Descriptor = Get-Content -Raw -LiteralPath $PackagedPlugin | ConvertFrom-Json
if ($Descriptor.Modules.Count -ne 1 -or $Descriptor.Modules[0].Type -cne 'Runtime') {
    throw 'Packaged movement challenger must contain exactly one Runtime module.'
}
$Forbidden = Select-String -LiteralPath (Join-Path $PackageRoot 'Source\CharacterMovementChallenger\CharacterMovementChallenger.Build.cs') `
    -Pattern 'UnrealEd|PhysicsUtilities|CharacterPhysicsProfileAdapter'
if ($Forbidden) {
    throw "Runtime challenger contains a forbidden editor dependency: $($Forbidden[0].Line)"
}

Write-Output $PackagedPlugin
