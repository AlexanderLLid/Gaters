param(
    [string] $EngineRoot = 'C:\Program Files\Epic Games\UE_5.8'
)

$ErrorActionPreference = 'Stop'
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$RepoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\..'))
$Project = Join-Path $ProjectRoot 'Prototype.uproject'
$EditorCmd = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$Importer = Join-Path $PSScriptRoot 'ImportContractAsset.py'
$Source = Join-Path $RepoRoot 'research\fixtures\assets\wood-foundation.obj'
$Report = Join-Path $ProjectRoot 'Saved\AssetImport\wood-foundation.json'
$GeneratedAsset = Join-Path $ProjectRoot 'Content\Gaters\Generated\Fixtures\SM_WoodFoundation.uasset'

if (-not (Test-Path -LiteralPath $Importer)) { throw "Missing importer: $Importer" }
if (-not (Test-Path -LiteralPath $Source)) { throw "Missing source fixture: $Source" }
if (-not (Test-Path -LiteralPath $EditorCmd)) { throw "Missing UnrealEditor-Cmd: $EditorCmd" }

function Invoke-Import([int] $Iteration) {
    Remove-Item -LiteralPath $Report -Force -ErrorAction SilentlyContinue
    $env:GATERS_IMPORT_SOURCE = $Source
    $env:GATERS_IMPORT_DESTINATION = '/Game/Gaters/Generated/Fixtures'
    $env:GATERS_IMPORT_NAME = 'SM_WoodFoundation'
    $env:GATERS_IMPORT_REPORT = $Report
    try {
        & $EditorCmd $Project -unattended -nop4 -nosplash -nullrhi -nosound `
            "-ExecutePythonScript=$Importer" "-log=ContractAssetImport-$Iteration.log"
        if ($LASTEXITCODE -ne 0) {
            throw "Import iteration $Iteration failed with exit code $LASTEXITCODE."
        }
    }
    finally {
        Remove-Item Env:GATERS_IMPORT_SOURCE, Env:GATERS_IMPORT_DESTINATION, `
            Env:GATERS_IMPORT_NAME, Env:GATERS_IMPORT_REPORT -ErrorAction SilentlyContinue
    }
    $ImportLog = Join-Path $ProjectRoot "Saved\Logs\ContractAssetImport-$Iteration.log"
    $ImportFailures = Select-String -LiteralPath $ImportLog -Pattern `
        'Ensure condition failed|LogPython: Error|LogInterchangeEngine: Error|LogInterchangeImport: Error'
    if ($ImportFailures) {
        throw "Import iteration $Iteration logged an ensure or import error: $($ImportFailures[0].Line)"
    }
    if (-not (Test-Path -LiteralPath $Report)) { throw "Import iteration $Iteration produced no report." }

    $IntakeLogName = "ContractAssetIntake-$Iteration.log"
    & $EditorCmd $Project -unattended -nop4 -nosplash -nullrhi -nosound `
        '-ExecCmds=Automation RunTests Gaters.Content.AssetIntake;Quit' `
        '-TestExit=Automation Test Queue Empty' "-log=$IntakeLogName"
    if ($LASTEXITCODE -ne 0) {
        throw "Independent intake iteration $Iteration failed with exit code $LASTEXITCODE."
    }
    $IntakeLog = Join-Path $ProjectRoot "Saved\Logs\$IntakeLogName"
    if (-not (Select-String -LiteralPath $IntakeLog -Quiet -SimpleMatch `
        'Test Completed. Result={Success} Name={StaticMesh} Path={Gaters.Content.AssetIntake.StaticMesh}')) {
        throw "Independent intake iteration $Iteration did not report success."
    }
    return Get-Content -Raw -LiteralPath $Report | ConvertFrom-Json
}

if (Test-Path -LiteralPath $GeneratedAsset) {
    $ResolvedProjectRoot = [System.IO.Path]::GetFullPath($ProjectRoot).TrimEnd('\') + '\'
    $ResolvedGeneratedAsset = [System.IO.Path]::GetFullPath($GeneratedAsset)
    if (-not $ResolvedGeneratedAsset.StartsWith($ResolvedProjectRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to remove generated asset outside project: $ResolvedGeneratedAsset"
    }
    Remove-Item -LiteralPath $ResolvedGeneratedAsset -Force
}
$First = Invoke-Import 1
$Second = Invoke-Import 2
foreach ($Field in @('schemaVersion', 'importerVersion', 'engineVersion', 'sourceSha256', 'objectPath', 'assetClass')) {
    if ($First.$Field -ne $Second.$Field) { throw "Repeated import changed $Field." }
}
if (($First.importedObjectPaths | ConvertTo-Json -Compress) -ne ($Second.importedObjectPaths | ConvertTo-Json -Compress)) {
    throw 'Repeated import changed importedObjectPaths.'
}
if (($First.importSettings | ConvertTo-Json -Compress) -ne ($Second.importSettings | ConvertTo-Json -Compress)) {
    throw 'Repeated import changed importSettings.'
}
if ($First.schemaVersion -ne 1 -or $First.importerVersion -ne 2) { throw 'Importer versions were not recorded.' }
if ($First.assetClass -ne 'StaticMesh') { throw "Expected StaticMesh, got $($First.assetClass)." }
if ($First.objectPath -ne '/Game/Gaters/Generated/Fixtures/SM_WoodFoundation.SM_WoodFoundation') {
    throw "Unexpected imported object path: $($First.objectPath)"
}
if (-not (Test-Path -LiteralPath $GeneratedAsset)) { throw "Generated asset is missing: $GeneratedAsset" }

Write-Output "PASS importer=$($First.importerVersion) asset=$($First.objectPath) sha=$($First.sourceSha256.Substring(0, 12))"
