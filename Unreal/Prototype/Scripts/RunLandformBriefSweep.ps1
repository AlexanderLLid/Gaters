param(
    [int[]] $Seeds = @(11, 29, 47, 83, 131),
    [int] $SecondsPerSeed = 12,
    [string] $EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',
    [string] $ArchivePath = '',
    [switch] $CaptureGallery
)

$ErrorActionPreference = 'Stop'
$Runner = Join-Path $PSScriptRoot 'RunEnvironmentSweep.ps1'
if (-not (Test-Path -LiteralPath $Runner)) { throw "Missing environment sweep: $Runner" }
if ([string]::IsNullOrWhiteSpace($ArchivePath)) {
    $ArchivePath = Join-Path (Split-Path -Parent $PSScriptRoot) `
        "Saved\EnvironmentRuns\landform-brief-$([DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss')).jsonl"
}

$Fixtures = @(
    @{ Label = 'baseline'; Relief = 0.0; Volcanism = 0.0; Ice = 0.0 },
    @{ Label = 'high-relief'; Relief = 1.0; Volcanism = 0.0; Ice = 0.0 },
    @{ Label = 'volcanic'; Relief = 0.45; Volcanism = 1.0; Ice = 0.0 },
    @{ Label = 'glacial'; Relief = 0.8; Volcanism = 0.0; Ice = 1.0 }
)

foreach ($Fixture in $Fixtures) {
    & $Runner -Seeds $Seeds -SecondsPerSeed $SecondsPerSeed -EngineRoot $EngineRoot `
        -ArchivePath $ArchivePath -VillageStage 0 -BuiltSites 0 -Landforms 1 `
        -LandformRelief $Fixture.Relief -LandformVolcanism $Fixture.Volcanism `
        -LandformIce $Fixture.Ice -ArtifactLabel $Fixture.Label `
        -CaptureGallery:$CaptureGallery
}

Write-Output "LANDFORM_SWEEP records=$($Seeds.Count * $Fixtures.Count) archive=$([IO.Path]::GetFullPath($ArchivePath))"
