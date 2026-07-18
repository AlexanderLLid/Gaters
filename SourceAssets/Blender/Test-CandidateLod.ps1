param(
    [string] $Blender = 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe'
)

$ErrorActionPreference = 'Stop'
$Root = $PSScriptRoot
$Runner = Join-Path $Root 'Build-CandidateLod.ps1'
$Output = Join-Path $Root 'Derived\neutral-rock-v1'
$Manifest = Join-Path $Output 'manifest.json'

if (-not (Test-Path -LiteralPath $Blender)) { throw "Missing Blender: $Blender" }
if (-not (Test-Path -LiteralPath $Runner)) { throw "Missing runner: $Runner" }

& $Runner -Blender $Blender
if ($LASTEXITCODE -ne 0) { throw "First build failed with exit code $LASTEXITCODE." }
$First = Get-Content -Raw -LiteralPath $Manifest
$Sentinel = Join-Path $Output 'stale-output-must-be-removed.txt'
[System.IO.File]::WriteAllText($Sentinel, 'stale')

& $Runner -Blender $Blender
if ($LASTEXITCODE -ne 0) { throw "Second build failed with exit code $LASTEXITCODE." }
$Second = Get-Content -Raw -LiteralPath $Manifest
if ($First -cne $Second) { throw 'Repeated builds produced different manifests.' }
if (Test-Path -LiteralPath $Sentinel) { throw 'Repeated build did not clear stale derived output.' }
if (Get-ChildItem -LiteralPath $Output -Filter '*.blend1') { throw 'Generated candidate area contains Blender backup files.' }

$Data = $Second | ConvertFrom-Json
if (-not $Data.validation.passed) { throw 'Manifest validation did not pass.' }
if ($null -ne $Data.sourceIdentity.selectedStyle) { throw 'Neutral fixture selected a style.' }
if (-not $Data.authority.contract.authoritative -or -not $Data.authority.generator.authoritative) {
    throw 'JSON contract and generator are not explicitly authoritative.'
}
if (-not $Data.blenderArtifact.derived -or -not $Data.blenderArtifact.reproducible) {
    throw 'Blend is not explicitly recorded as derived and reproducible.'
}
if ($Data.PSObject.Properties.Name -contains 'sourceArtifact') {
    throw 'Manifest still describes the derived blend as a source artifact.'
}
if (-not $Data.blenderArtifact.validation.reopened -or -not $Data.blenderArtifact.validation.passed) {
    throw 'Derived blend was not reopened and validated.'
}
if ($Data.representations.Count -ne 3) { throw 'Expected near, mid, and far representations.' }
if (($Data.representations.role -join ',') -ne 'near,mid,far') { throw 'Representation roles are not ordered near, mid, far.' }
if (-not ($Data.representations[0].triangles -gt $Data.representations[1].triangles -and
          $Data.representations[1].triangles -gt $Data.representations[2].triangles)) {
    throw 'Triangle counts do not strictly decrease.'
}
if (($Data.portableAxisConversion.mapping -join ',') -ne 'x,z,-y') {
    throw 'Portable axis conversion is not explicit.'
}
foreach ($Representation in $Data.representations) {
	if ($null -eq $Representation.unrealFbx -or
		[System.String]::IsNullOrWhiteSpace($Representation.unrealFbx.file) -or
		$Representation.unrealFbx.deterministicBytes -ne $false) {
		throw "Missing Unreal FBX evidence for $($Representation.role)."
	}
	if ($Representation.unrealFbx.PSObject.Properties.Name -contains 'sha256') {
		throw "Nondeterministic FBX transport must not publish a misleading stable hash."
	}
	if ($null -eq $Representation.blenderBounds -or $null -eq $Representation.portableBounds) {
        throw "Missing dual-space bounds for $($Representation.role)."
    }
    $ExpectedMin = @(
        $Representation.blenderBounds.min[0],
        $Representation.blenderBounds.min[2],
        -$Representation.blenderBounds.max[1]
    )
    $ExpectedMax = @(
        $Representation.blenderBounds.max[0],
        $Representation.blenderBounds.max[2],
        -$Representation.blenderBounds.min[1]
    )
    if (($ExpectedMin | ConvertTo-Json -Compress) -cne ($Representation.portableBounds.min | ConvertTo-Json -Compress) -or
        ($ExpectedMax | ConvertTo-Json -Compress) -cne ($Representation.portableBounds.max | ConvertTo-Json -Compress)) {
        throw "Portable bounds do not match the recorded axis conversion for $($Representation.role)."
    }
}
if (-not $Data.validation.checks.blenderBoundsMatch -or
    -not $Data.validation.checks.portableBoundsMatch -or
    -not $Data.validation.checks.axisConvertedBoundsMatch) {
    throw 'Dual-space bounds validation did not pass.'
}
foreach ($RelativePath in @($Data.blenderArtifact.file) + @($Data.representations.file) +
		@($Data.representations.unrealFbx.file) + @($Data.preview.file)) {
	if (-not (Test-Path -LiteralPath (Join-Path $Output $RelativePath))) {
		throw "Missing generated artifact: $RelativePath"
	}
	if ((Get-Item -LiteralPath (Join-Path $Output $RelativePath)).Length -eq 0) {
		throw "Generated artifact is empty: $RelativePath"
	}
}

$InvalidPreservationSentinel = Join-Path $Output 'valid-output-survives-invalid-contract.txt'
[System.IO.File]::WriteAllText($InvalidPreservationSentinel, 'preserve')
$PreviousErrorActionPreference = $ErrorActionPreference
try {
	$ErrorActionPreference = 'Continue'
	$null = & powershell.exe -NoProfile -NonInteractive -ExecutionPolicy Bypass -File $Runner `
		-Blender $Blender `
		-Contract (Join-Path $Root 'contracts\neutral-rock-invalid.json') 2>&1
	$InvalidExitCode = $LASTEXITCODE
}
finally {
	$ErrorActionPreference = $PreviousErrorActionPreference
}
if ($InvalidExitCode -eq 0) { throw 'Deliberately invalid representation contract unexpectedly passed.' }
if (-not (Test-Path -LiteralPath $InvalidPreservationSentinel)) {
    throw 'Invalid contract cleared existing derived evidence before validation.'
}
Remove-Item -LiteralPath $InvalidPreservationSentinel -Force

Write-Output "PASS blender=$($Data.blenderVersion) triangles=$($Data.representations.triangles -join '/')"
exit 0
