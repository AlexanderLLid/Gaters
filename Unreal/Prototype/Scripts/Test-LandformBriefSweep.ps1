$ErrorActionPreference = 'Stop'

$Sweep = Join-Path $PSScriptRoot 'RunEnvironmentSweep.ps1'
$LandformSweep = Join-Path $PSScriptRoot 'RunLandformBriefSweep.ps1'
$Writer = Join-Path $PSScriptRoot 'Write-EnvironmentRun.ps1'
$Spawner = Join-Path $PSScriptRoot '..\Source\Prototype\Private\GatersTestSpawner.cpp'
$Chunk = Join-Path $PSScriptRoot '..\Source\Prototype\Private\GatersChunk.cpp'

foreach ($Path in @($Sweep, $LandformSweep, $Writer)) {
    if (-not (Test-Path -LiteralPath $Path)) { throw "Missing landform sweep dependency: $Path" }
    $Tokens = $null
    $Errors = $null
    [void][Management.Automation.Language.Parser]::ParseFile($Path, [ref]$Tokens, [ref]$Errors)
    if ($Errors.Count -gt 0) { throw "PowerShell parse failure in $Path`: $($Errors[0].Message)" }
}

$SweepCommand = Get-Command $Sweep
foreach ($Name in @('ArtifactLabel', 'LandformRelief', 'LandformVolcanism', 'LandformIce')) {
    if (-not $SweepCommand.Parameters.ContainsKey($Name)) {
        throw "RunEnvironmentSweep is missing parameter $Name"
    }
}
$SweepText = Get-Content -LiteralPath $Sweep -Raw
$SpawnerText = Get-Content -LiteralPath $Spawner -Raw
$ChunkText = Get-Content -LiteralPath $Chunk -Raw
if ($SweepText -notmatch 'GatersArtifactLabel=') {
    throw 'RunEnvironmentSweep does not forward its artifact label to Unreal.'
}
if ($SpawnerText -notmatch 'GatersArtifactLabel=' -or
    $SpawnerText -notmatch 'CurrentArtifactLabel') {
    throw 'Unreal gallery naming does not consume the artifact label.'
}
$RollSite = [regex]::Match(
    $ChunkText,
    'void AGatersChunk::RollSite\(\)(?<body>[\s\S]*?)float AGatersChunk::GroundHeight')
if (-not $RollSite.Success) { throw 'Could not isolate AGatersChunk::RollSite.' }
if ($RollSite.Groups['body'].Value -match 'FindBaseSite|FGatersSiteRoutePlanner') {
    throw 'Landform candidate selection depends on optional site discovery or planning.'
}

$TempRoot = Join-Path ([IO.Path]::GetTempPath()) "gaters-landform-sweep-$([guid]::NewGuid())"
$Log = Join-Path $TempRoot 'seed-83-glacial.log'
$Archive = Join-Path $TempRoot 'runs.jsonl'
New-Item -ItemType Directory -Force -Path $TempRoot | Out-Null

try {
    @"
[0]LogTemp: Display: [GatersChunk] LANDFORM v=6 enabled=yes relief=0.800 volcanism=0.000 ice=1.000 protected=1
[0]LogTemp: Display: [GatersChunk] LAND_ACCESS v=4 enabled=yes evaluated=yes brief=2 compiler=2 target_walkable=0.4000 target_connected=0.8000 candidates=8 world_cells=61 arrival_cells=61 arrival_cell=500 pad=1000 semantic=2 transition=2000 flat=0.940 slope=0.770 escape_cells=3 walkable_tol=0.1500 connected_tol=0.1500 satisfying=1 rejected=7 selected=3 selected_scale=0.8500 selected_dissection=1.1000 selected_ruggedness=0.6000 selected_walkable=0.3900 selected_connected=0.8200 selected_world_access=yes selected_escape=yes best=4 best_scale=1.2000 best_dissection=1.4000 best_ruggedness=0.9000 best_walkable=0.3800 best_connected=0.7900 best_world_access=no best_escape=no
[0]LogTemp: Display: [GatersChunk] RECIPE schema=9 generator=12 seed=83 chunk=400000 checksum=3BF97FCB nodes=357 valid=yes
[0]LogTemp: Display: [GatersChunk] EVAL v=3 relief=4200 water=0.0761 rough=301 cliff=2897 buildable=0.7855 mean=812 below=0.1246 window=400000
[0]LogTemp: Display: [GatersChunk] SITE seed=83 environment=mountains water=yes base_valid=yes base=(-4076,-9568) drop=23 hydrology=river
[0]LogTemp: Display: [GatersChunk] PLAN v=1 valid=yes sites=4 routes=3 diagnostics=0
[0]LogTemp: Display: [GatersChunk] VISUAL v=2 backend=UnrealISM batches=4 instances=442 carriers=0 valid=yes
[0]LogTemp: Display: [GatersChunk] GEN ms=287.2
[0]LogTemp: Display: [GatersChunk] PERF v=2 generation=287.2 frame=16.667 memory=1400.0 actors=900 scatter=203 claims=239 base=0 cells=9 static_components=600 instanced_components=3 instances=700 static_tris=1200000 dynamic_tris=650000 lod0_triangles=1850000 valid=yes issues=0
"@ | Set-Content -LiteralPath $Log

    & $Writer -LogPath $Log -ArchivePath $Archive -EngineVersion '5.8' -Platform 'Win64' | Out-Null
    $Record = Get-Content -LiteralPath $Archive | ConvertFrom-Json
    if (-not $Record.input.landform.enabled -or
        $Record.input.landform.relief -ne 0.8 -or
        $Record.input.landform.volcanism -ne 0.0 -or
        $Record.input.landform.ice -ne 1.0 -or
        $Record.input.landform.protectedRegionCount -ne 1 -or
        $Record.schemaVersion -ne 15 -or
        $Record.input.landAccess.terrainSemanticVersion -ne 2 -or
        $Record.input.landAccess.arrivalTransitionWidthCm -ne 2000 -or
        $Record.input.landAccess.selected.featureScale -ne 0.85 -or
        $Record.input.landAccess.selected.dissectionScale -ne 1.1 -or
        $Record.input.landAccess.selected.ruggednessScale -ne 0.6 -or
        -not $Record.input.landAccess.selected.hasWorldAccess -or
        $Record.input.landAccess.best.featureScale -ne 1.2 -or
        $Record.input.landAccess.best.dissectionScale -ne 1.4 -or
        $Record.input.landAccess.best.ruggednessScale -ne 0.9 -or
        $Record.input.landAccess.best.hasWorldAccess) {
        throw 'LANDFORM provenance was not preserved in the run archive.'
    }

    $LandformText = Get-Content -LiteralPath $LandformSweep -Raw
    foreach ($Seed in @(11, 29, 47, 83, 131)) {
        if ($LandformText -notmatch "(?<![0-9])$Seed(?![0-9])") {
            throw "Held-out seed $Seed is missing from the landform sweep."
        }
    }
    foreach ($Label in @('baseline', 'high-relief', 'volcanic', 'glacial')) {
        if ($LandformText -notmatch [regex]::Escape($Label)) {
            throw "Physical fixture $Label is missing from the landform sweep."
        }
    }

    Write-Output 'PASS archive=landform-provenance seeds=5 fixtures=4 scripts=parsed'
}
finally {
    Remove-Item -LiteralPath $TempRoot -Recurse -Force -ErrorAction SilentlyContinue
}
