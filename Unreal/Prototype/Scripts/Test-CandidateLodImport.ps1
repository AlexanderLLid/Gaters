param(
	[string] $EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',
	[string] $Blender = 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe'
)

$ErrorActionPreference = 'Stop'
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$RepoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\..'))
$Project = Join-Path $ProjectRoot 'Prototype.uproject'
$EditorCmd = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$Importer = Join-Path $PSScriptRoot 'ImportCandidateLod.py'
$BlenderTest = Join-Path $RepoRoot 'SourceAssets\Blender\Test-CandidateLod.ps1'
$Manifest = Join-Path $RepoRoot 'SourceAssets\Blender\Derived\neutral-rock-v1\manifest.json'
$Report = Join-Path $ProjectRoot 'Saved\AssetImport\neutral-rock-lod.json'
$GeneratedAsset = Join-Path $ProjectRoot 'Content\Gaters\Generated\Candidates\SM_NeutralRock.uasset'

foreach ($Path in @($EditorCmd, $Importer, $BlenderTest)) {
	if (-not (Test-Path -LiteralPath $Path)) { throw "Missing required path: $Path" }
}

& $BlenderTest -Blender $Blender
if ($LASTEXITCODE -ne 0) { throw "Blender candidate validation failed with exit code $LASTEXITCODE." }
if (-not (Test-Path -LiteralPath $Manifest)) { throw "Missing candidate manifest: $Manifest" }
$Expected = Get-Content -Raw -LiteralPath $Manifest | ConvertFrom-Json

function Invoke-CandidateImport([int] $Iteration) {
	Remove-Item -LiteralPath $Report -Force -ErrorAction SilentlyContinue
	$env:GATERS_CANDIDATE_MANIFEST = $Manifest
	$env:GATERS_IMPORT_DESTINATION = '/Game/Gaters/Generated/Candidates'
	$env:GATERS_IMPORT_NAME = 'SM_NeutralRock'
	$env:GATERS_IMPORT_REPORT = $Report
	try {
		& $EditorCmd $Project -unattended -nop4 -nosplash -nullrhi -nosound `
			"-ExecutePythonScript=$Importer" "-log=CandidateLodImport-$Iteration.log"
		if ($LASTEXITCODE -ne 0) {
			throw "Candidate LOD import iteration $Iteration failed with exit code $LASTEXITCODE."
		}
	}
	finally {
		Remove-Item Env:GATERS_CANDIDATE_MANIFEST, Env:GATERS_IMPORT_DESTINATION, `
			Env:GATERS_IMPORT_NAME, Env:GATERS_IMPORT_REPORT -ErrorAction SilentlyContinue
	}

	$ImportLog = Join-Path $ProjectRoot "Saved\Logs\CandidateLodImport-$Iteration.log"
	$Failures = Select-String -LiteralPath $ImportLog -Pattern `
		'Ensure condition failed|LogPython: Error|LogInterchangeEngine: Error|LogInterchangeImport: Error'
	if ($Failures) {
		throw "Candidate LOD import iteration $Iteration logged an error: $($Failures[0].Line)"
	}
	if (-not (Test-Path -LiteralPath $Report)) { throw "Import iteration $Iteration produced no report." }
	return Get-Content -Raw -LiteralPath $Report | ConvertFrom-Json
}

$First = Invoke-CandidateImport 1
$Second = Invoke-CandidateImport 2
foreach ($Field in @('schemaVersion', 'importerVersion', 'engineVersion', 'manifestSha256', 'objectPath', 'assetClass', 'lodCount')) {
	if ($First.$Field -ne $Second.$Field) { throw "Repeated import changed $Field." }
}
foreach ($Field in @('sourceFiles', 'lodTriangles', 'boundsSizeCentimeters', 'lodScreenSizes')) {
	if (($First.$Field | ConvertTo-Json -Compress) -cne ($Second.$Field | ConvertTo-Json -Compress)) {
		throw "Repeated import changed $Field."
	}
}
if ($First.schemaVersion -ne 1 -or $First.importerVersion -ne 1) { throw 'Importer versions were not recorded.' }
if ($First.assetClass -ne 'StaticMesh') { throw "Expected StaticMesh, got $($First.assetClass)." }
if ($First.objectPath -ne '/Game/Gaters/Generated/Candidates/SM_NeutralRock.SM_NeutralRock') {
	throw "Unexpected imported object path: $($First.objectPath)"
}
if ($First.lodCount -ne 3) { throw "Expected three native LODs, got $($First.lodCount)." }
$ExpectedTriangles = @($Expected.representations.triangles)
if ((@($First.lodTriangles) -join ',') -ne ($ExpectedTriangles -join ',')) {
	throw "Imported triangle counts $(@($First.lodTriangles) -join '/') do not match manifest $($ExpectedTriangles -join '/')."
}
$ExpectedBounds = @($Expected.dimensionsMeters | ForEach-Object { [double]$_ * 100.0 })
for ($Index = 0; $Index -lt 3; ++$Index) {
	if ([math]::Abs([double]$First.boundsSizeCentimeters[$Index] - $ExpectedBounds[$Index]) -gt 0.1) {
		throw "Imported bounds $($First.boundsSizeCentimeters -join '/') do not match expected centimeters $($ExpectedBounds -join '/')."
	}
}
if ($First.fbxTransportDeterministicBytes -ne $false) { throw 'FBX transport nondeterminism was not recorded honestly.' }
if ($null -ne $First.selectedStyle) { throw 'Style-neutral candidate unexpectedly selected an art style.' }
if (-not (Test-Path -LiteralPath $GeneratedAsset)) { throw "Generated asset is missing: $GeneratedAsset" }

Write-Output "PASS candidate-lod importer=$($First.importerVersion) triangles=$(@($First.lodTriangles) -join '/') asset=$($First.objectPath)"
