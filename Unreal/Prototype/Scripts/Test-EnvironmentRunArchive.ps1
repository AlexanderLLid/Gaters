$ErrorActionPreference = 'Stop'

$Writer = Join-Path $PSScriptRoot 'Write-EnvironmentRun.ps1'
if (-not (Test-Path -LiteralPath $Writer)) {
    throw "Missing experiment writer: $Writer"
}

$TempRoot = Join-Path ([System.IO.Path]::GetTempPath()) "gaters-run-archive-$([guid]::NewGuid())"
$Log = Join-Path $TempRoot 'seed-7.log'
$BrokenLog = Join-Path $TempRoot 'broken.log'
$PartialGalleryLog = Join-Path $TempRoot 'partial-gallery.log'
$DifferentCandidateLog = Join-Path $TempRoot 'different-candidate.log'
$Archive = Join-Path $TempRoot 'runs.jsonl'
$Beauty = Join-Path $TempRoot 'seed-7-beauty.png'
$Traversal = Join-Path $TempRoot 'seed-7-traversal.png'
New-Item -ItemType Directory -Force -Path $TempRoot | Out-Null

try {
    [IO.File]::WriteAllBytes($Beauty, [byte[]](1, 2, 3, 4))
    [IO.File]::WriteAllBytes($Traversal, [byte[]](5, 6, 7, 8))
    @"
[0]LogTemp: Display: [GatersChunk] RECIPE schema=3 generator=2 seed=7 chunk=30000 checksum=3BF97FCB nodes=577 valid=yes
[0]LogTemp: Display: [GatersChunk] LANDFORM v=6 enabled=yes relief=0.500 volcanism=0.500 ice=0.500 protected=1
[0]LogTemp: Display: [GatersChunk] LAND_ACCESS v=4 enabled=yes evaluated=yes brief=2 compiler=2 target_walkable=0.4000 target_connected=0.8000 candidates=8 world_cells=61 arrival_cells=61 arrival_cell=500 pad=1000 semantic=2 transition=2000 flat=0.940 slope=0.770 escape_cells=3 walkable_tol=0.1500 connected_tol=0.1500 satisfying=1 rejected=7 selected=3 selected_scale=0.8500 selected_dissection=1.1000 selected_ruggedness=0.6000 selected_walkable=0.3900 selected_connected=0.8200 selected_world_access=yes selected_escape=yes best=4 best_scale=1.2000 best_dissection=1.4000 best_ruggedness=0.9000 best_walkable=0.3800 best_connected=0.7900 best_world_access=no best_escape=no
[0]LogTemp: Display: [GatersChunk] EVAL v=3 relief=3149 water=0.0761 rough=301 cliff=2897 buildable=0.7855 mean=812 below=0.1246 window=30000
[0]LogTemp: Display: [GatersChunk] SITE seed=7 environment=canyon water=yes base_valid=yes base=(-4076,-9568) drop=23 hydrology=river
[0]LogTemp: Display: [GatersChunk] PLAN v=1 valid=no sites=0 routes=0 diagnostics=1
[0]LogTemp: Display: [GatersChunk] VISUAL v=2 backend=UnrealISM batches=4 instances=442 carriers=0 valid=yes
[0]LogTemp: Display: [GatersChunk] GEN ms=1792.5
[0]LogTemp: Display: [GatersChunk] PERF v=2 generation=1792.5 frame=16.667 memory=1400.0 actors=900 scatter=203 claims=239 base=39 cells=81 static_components=600 instanced_components=3 instances=700 static_tris=1200000 dynamic_tris=650000 lod0_triangles=1850000 valid=yes issues=0
[0]LogTemp: Display: [GatersTestSpawner] gallery_beauty=$Beauty
[0]LogTemp: Display: [GatersTestSpawner] gallery_traversal=$Traversal
"@ | Set-Content -LiteralPath $Log

    & $Writer -LogPath $Log -ArchivePath $Archive -EngineVersion '5.8' -Platform 'Win64' | Out-Null
    $FirstBytes = [System.IO.File]::ReadAllBytes($Archive)
    & $Writer -LogPath $Log -ArchivePath $Archive -EngineVersion '5.8' -Platform 'Win64' | Out-Null

    $Lines = @(Get-Content -LiteralPath $Archive)
    if ($Lines.Count -ne 2) { throw "Expected two records, got $($Lines.Count)." }
    $First = $Lines[0] | ConvertFrom-Json
    $Second = $Lines[1] | ConvertFrom-Json
    if ($First.runId -eq $Second.runId) { throw 'Repeated executions reused the run ID.' }
    if ($First.candidateId -ne $Second.candidateId) { throw 'Repeated candidate produced a different candidate ID.' }
    if ($First.schemaVersion -ne 15) { throw 'Archive schema version was not recorded.' }
    if ($First.machine.id -ne 'world.terrain-generator' -or $First.machine.version -ne 2) {
        throw 'Generator identity/version was not projected correctly.'
    }
    if ($First.recipe.schemaVersion -ne 3 -or $First.recipe.checksum -ne '3BF97FCB' -or $First.recipe.nodeCount -ne 577) {
        throw 'Recipe evidence was not projected correctly.'
    }
    if ($First.input.landform.version -ne 6 -or $First.input.landform.protectedRegionCount -ne 1) {
        throw 'Landform version/protected-region provenance was not projected correctly.'
    }
    if ($First.input.landAccess.machineId -ne 'world.environment-candidate-selector' -or
        $First.input.landAccess.version -ne 4 -or $First.input.landAccess.briefVersion -ne 2 -or
        $First.input.landAccess.compilerVersion -ne 2 -or
        -not $First.input.landAccess.enabled -or -not $First.input.landAccess.evaluated -or
        $First.input.landAccess.targetWalkable -ne 0.4 -or
        $First.input.landAccess.targetConnected -ne 0.8 -or
        $First.input.landAccess.candidateCount -ne 8 -or
        $First.input.landAccess.worldCellsPerAxis -ne 61 -or
        $First.input.landAccess.arrivalCellsPerAxis -ne 61 -or
        $First.input.landAccess.arrivalCellSizeCm -ne 500 -or
        $First.input.landAccess.padRadiusCm -ne 1000 -or
        $First.input.landAccess.terrainSemanticVersion -ne 2 -or
        $First.input.landAccess.arrivalTransitionWidthCm -ne 2000 -or
        $First.input.landAccess.escapeDistanceCells -ne 3 -or
        $First.input.landAccess.satisfyingCount -ne 1 -or
        $First.input.landAccess.rejectedCount -ne 7 -or
        $First.input.landAccess.selected.index -ne 3 -or
        $First.input.landAccess.selected.featureScale -ne 0.85 -or
        $First.input.landAccess.selected.dissectionScale -ne 1.1 -or
        $First.input.landAccess.selected.ruggednessScale -ne 0.6 -or
        $First.input.landAccess.selected.walkable -ne 0.39 -or
        $First.input.landAccess.selected.connected -ne 0.82 -or
        -not $First.input.landAccess.selected.hasWorldAccess -or
        -not $First.input.landAccess.selected.escapesArrival -or
        $First.input.landAccess.best.index -ne 4 -or
        $First.input.landAccess.best.featureScale -ne 1.2 -or
        $First.input.landAccess.best.dissectionScale -ne 1.4 -or
        $First.input.landAccess.best.ruggednessScale -ne 0.9 -or
        $First.input.landAccess.best.walkable -ne 0.38 -or
        $First.input.landAccess.best.connected -ne 0.79 -or
        $First.input.landAccess.best.hasWorldAccess -or
        $First.input.landAccess.best.escapesArrival) {
        throw 'Land-access selection provenance was not projected correctly.'
    }
    if ($First.evaluation.machineId -ne 'evaluation.terrain-metrics' -or $First.evaluation.version -ne 3 -or
		$First.evaluation.windowSizeCm -ne 30000) {
        throw 'Evaluator identity/version was not projected correctly.'
    }
    if ($First.sitePlan.machineId -ne 'world.site-route-planner' -or
		$First.sitePlan.version -ne 1 -or $First.sitePlan.valid -or
		$First.sitePlan.siteCount -ne 0 -or $First.sitePlan.routeCount -ne 0 -or
		$First.sitePlan.diagnosticCount -ne 1) {
		throw 'Site-plan evidence was not projected correctly.'
	}
    if ($First.materialization.machineId -ne 'runtime.visual-materializer' -or
        $First.materialization.version -ne 2 -or $First.materialization.backend -ne 'UnrealISM' -or
        $First.materialization.batchCount -ne 4 -or $First.materialization.instanceCount -ne 442 -or
        $First.materialization.interactionCarrierCount -ne 0 -or -not $First.materialization.valid) {
        throw 'Visual materializer identity/version was not projected correctly.'
    }
    if ($First.evaluation.metrics.buildableFraction -ne 0.7855 -or
		$First.evaluation.metrics.meanHeightCm -ne 812 -or
		$First.evaluation.metrics.belowDatumFraction -ne 0.1246 -or
		$First.environment.family -ne 'canyon' -or $First.environment.hydrology -ne 'river') {
        throw 'Environment metrics were not projected correctly.'
    }
    if ($First.performance.machineId -ne 'evaluation.performance' -or
        $First.performance.version -ne 2 -or $First.performance.generationMs -ne 1792.5 -or
        $First.performance.meanFrameMs -ne 16.667 -or $First.performance.usedPhysicalMB -ne 1400 -or
        $First.performance.totalActors -ne 900 -or $First.performance.scatterActors -ne 203 -or
        $First.performance.claimActors -ne 239 -or $First.performance.baseActors -ne 39 -or
        $First.performance.loadedTerrainCells -ne 81 -or $First.performance.staticMeshComponents -ne 600 -or
        $First.performance.instancedStaticMeshComponents -ne 3 -or
        $First.performance.staticMeshInstances -ne 700 -or
        $First.performance.staticMeshTriangles -ne 1200000 -or
        $First.performance.dynamicMeshTriangles -ne 650000 -or
        $First.performance.lod0Triangles -ne 1850000 -or -not $First.performance.withinBudget -or
        $First.performance.issueCount -ne 0) {
        throw 'Performance evidence was not projected correctly.'
    }
    if ($First.artifacts.beauty.path -ne [IO.Path]::GetFullPath($Beauty) -or
        $First.artifacts.beauty.sha256 -ne (Get-FileHash -LiteralPath $Beauty -Algorithm SHA256).Hash.ToLowerInvariant() -or
        $First.artifacts.traversal.path -ne [IO.Path]::GetFullPath($Traversal) -or
        $First.artifacts.traversal.sha256 -ne (Get-FileHash -LiteralPath $Traversal -Algorithm SHA256).Hash.ToLowerInvariant()) {
        throw 'Paired gallery artifacts were not recorded with their hashes.'
    }
    $CurrentBytes = [System.IO.File]::ReadAllBytes($Archive)
    for ($Index = 0; $Index -lt $FirstBytes.Length; ++$Index) {
        if ($CurrentBytes[$Index] -ne $FirstBytes[$Index]) {
            throw 'Appending changed bytes in the first immutable record.'
        }
    }

    ((Get-Content -LiteralPath $Log -Raw) -replace 'selected=3 selected_scale=0.8500', 'selected=5 selected_scale=1.4000') |
        Set-Content -LiteralPath $DifferentCandidateLog
    & $Writer -LogPath $DifferentCandidateLog -ArchivePath $Archive -EngineVersion '5.8' -Platform 'Win64' | Out-Null
    $Third = (Get-Content -LiteralPath $Archive)[2] | ConvertFrom-Json
    if ($Third.candidateId -eq $First.candidateId) {
        throw 'Changed land-access candidate identity was not reflected in candidate ID.'
    }

    @"
LANDFORM v=6 enabled=yes relief=0.500 volcanism=0.500 ice=0.500 protected=1
LAND_ACCESS v=4 enabled=yes evaluated=yes brief=2 compiler=2 target_walkable=0.4000 target_connected=0.8000 candidates=8 world_cells=61 arrival_cells=61 arrival_cell=500 pad=1000 semantic=2 transition=2000 flat=0.940 slope=0.770 escape_cells=3 walkable_tol=0.1500 connected_tol=0.1500 satisfying=1 rejected=7 selected=3 selected_scale=0.8500 selected_dissection=1.1000 selected_ruggedness=0.6000 selected_walkable=0.3900 selected_connected=0.8200 selected_world_access=yes selected_escape=yes best=4 best_scale=1.2000 best_dissection=1.4000 best_ruggedness=0.9000 best_walkable=0.3800 best_connected=0.7900 best_world_access=no best_escape=no
RECIPE schema=3 generator=2 seed=7 chunk=30000 checksum=3BF97FCB nodes=577 valid=yes
"@ | Set-Content -LiteralPath $BrokenLog
    $Rejected = $false
    try {
        & $Writer -LogPath $BrokenLog -ArchivePath $Archive -EngineVersion '5.8' -Platform 'Win64' | Out-Null
    }
    catch {
        $Rejected = $_.Exception.Message -match 'Missing required EVAL evidence'
    }
    if (-not $Rejected) { throw 'Incomplete evidence was not rejected for the expected reason.' }
    if (@(Get-Content -LiteralPath $Archive).Count -ne 3) {
        throw 'Rejected evidence changed the archive.'
    }

    ((Get-Content -LiteralPath $Log -Raw) -replace '(?m)^.*gallery_traversal=.*\r?\n?', '') |
        Set-Content -LiteralPath $PartialGalleryLog
    $PartialRejected = $false
    try {
        & $Writer -LogPath $PartialGalleryLog -ArchivePath $Archive -EngineVersion '5.8' -Platform 'Win64' | Out-Null
    }
    catch {
        $PartialRejected = $_.Exception.Message -match 'paired gallery artifacts'
    }
    if (-not $PartialRejected) { throw 'Partial gallery evidence was not rejected for the expected reason.' }

    Write-Output 'PASS records=3 rejected=2 gallery=paired selector-identity=preserved'
}
finally {
    Remove-Item -LiteralPath $TempRoot -Recurse -Force -ErrorAction SilentlyContinue
}
