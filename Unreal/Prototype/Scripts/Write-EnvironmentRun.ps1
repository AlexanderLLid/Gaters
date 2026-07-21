param(
    [Parameter(Mandatory = $true)]
    [string] $LogPath,
    [Parameter(Mandatory = $true)]
    [string] $ArchivePath,
    [Parameter(Mandatory = $true)]
    [string] $EngineVersion,
    [Parameter(Mandatory = $true)]
    [string] $Platform
)

$ErrorActionPreference = 'Stop'
$Culture = [System.Globalization.CultureInfo]::InvariantCulture
$Text = Get-Content -LiteralPath $LogPath -Raw

function Get-RequiredMatch([string] $Name, [string] $Pattern) {
    $Matches = [regex]::Matches($Text, $Pattern)
    if ($Matches.Count -eq 0) {
        throw "Missing required $Name evidence in $LogPath"
    }
    return $Matches[$Matches.Count - 1]
}

function To-Int([string] $Value) {
    return [int]::Parse($Value, $Culture)
}

function To-Double([string] $Value) {
    return [double]::Parse($Value, $Culture)
}

$Number = '[-+]?[0-9]+(?:\.[0-9]+)?'
$Landform = Get-RequiredMatch 'LANDFORM' "LANDFORM v=([0-9]+) enabled=(yes|no) relief=($Number) volcanism=($Number) ice=($Number) protected=([0-9]+)"
$LandAccess = Get-RequiredMatch 'LAND_ACCESS' "LAND_ACCESS v=([0-9]+) enabled=(yes|no) evaluated=(yes|no) brief=([0-9]+) compiler=([0-9]+) target_walkable=($Number) target_connected=($Number) candidates=([0-9]+) world_cells=([0-9]+) arrival_cells=([0-9]+) arrival_cell=($Number) pad=($Number) semantic=([0-9]+) transition=($Number) flat=($Number) slope=($Number) escape_cells=([0-9]+) walkable_tol=($Number) connected_tol=($Number) satisfying=([0-9]+) rejected=([0-9]+) selected=(-?[0-9]+) selected_scale=($Number) selected_dissection=($Number) selected_ruggedness=($Number) selected_walkable=($Number) selected_connected=($Number) selected_world_access=(yes|no) selected_escape=(yes|no) best=(-?[0-9]+) best_scale=($Number) best_dissection=($Number) best_ruggedness=($Number) best_walkable=($Number) best_connected=($Number) best_world_access=(yes|no) best_escape=(yes|no)"
$Recipe = Get-RequiredMatch 'RECIPE' "RECIPE schema=([0-9]+) generator=([0-9]+) seed=(-?[0-9]+) chunk=($Number) checksum=([0-9A-Fa-f]+) nodes=([0-9]+) valid=(yes|no)"
$Evaluation = Get-RequiredMatch 'EVAL' "EVAL v=([0-9]+) relief=($Number) water=($Number) rough=($Number) cliff=($Number) buildable=($Number) mean=($Number) below=($Number) window=($Number)"
$Site = Get-RequiredMatch 'SITE' "SITE seed=(-?[0-9]+) environment=([A-Za-z0-9_-]+) water=(yes|no) base_valid=(yes|no) base=\(($Number),($Number)\) drop=($Number) hydrology=([A-Za-z0-9_-]+)"
$Plan = Get-RequiredMatch 'PLAN' "PLAN v=([0-9]+) valid=(yes|no) sites=([0-9]+) routes=([0-9]+) diagnostics=([0-9]+)"
$Materialization = Get-RequiredMatch 'VISUAL' "VISUAL v=([0-9]+) backend=([A-Za-z0-9_-]+) batches=([0-9]+) instances=([0-9]+) carriers=([0-9]+) valid=(yes|no)"
$Generation = Get-RequiredMatch 'GEN' "GEN ms=($Number)"
$Performance = Get-RequiredMatch 'PERF' "PERF v=([0-9]+) generation=($Number) frame=($Number) memory=($Number) actors=([0-9]+) scatter=([0-9]+) claims=([0-9]+) base=([0-9]+) cells=([0-9]+) static_components=([0-9]+) instanced_components=([0-9]+) instances=([0-9]+) static_tris=([0-9]+) dynamic_tris=([0-9]+) lod0_triangles=([0-9]+) valid=(yes|no) issues=([0-9]+)"
$BeautyMatches = [regex]::Matches($Text, '(?m)\[GatersTestSpawner\] gallery_beauty=(.+)$')
$TraversalMatches = [regex]::Matches($Text, '(?m)\[GatersTestSpawner\] gallery_traversal=(.+)$')
if (($BeautyMatches.Count -gt 0) -ne ($TraversalMatches.Count -gt 0)) {
    throw "Gallery evidence requires paired gallery artifacts in $LogPath"
}

$Artifacts = [ordered]@{ logPath = [IO.Path]::GetFullPath($LogPath) }
if ($BeautyMatches.Count -gt 0) {
    foreach ($Artifact in @(
        @{ Name = 'beauty'; Path = $BeautyMatches[$BeautyMatches.Count - 1].Groups[1].Value.Trim() },
        @{ Name = 'traversal'; Path = $TraversalMatches[$TraversalMatches.Count - 1].Groups[1].Value.Trim() }
    )) {
        $FullPath = [IO.Path]::GetFullPath($Artifact.Path)
        if (-not (Test-Path -LiteralPath $FullPath) -or (Get-Item -LiteralPath $FullPath).Length -eq 0) {
            throw "Missing or empty gallery artifact $($Artifact.Name): $FullPath"
        }
        $Artifacts[$Artifact.Name] = [ordered]@{
            path = $FullPath
            sha256 = (Get-FileHash -LiteralPath $FullPath -Algorithm SHA256).Hash.ToLowerInvariant()
        }
    }
}

$Seed = To-Int $Recipe.Groups[3].Value
if ((To-Int $Site.Groups[1].Value) -ne $Seed) {
    throw "SITE seed does not match RECIPE seed in $LogPath"
}
if ((To-Double $Generation.Groups[1].Value) -ne (To-Double $Performance.Groups[2].Value)) {
    throw "PERF generation does not match GEN evidence in $LogPath"
}

$CandidateText = @(
    'world.terrain-generator',
    $Recipe.Groups[2].Value,
    $Recipe.Groups[1].Value,
    $Recipe.Groups[3].Value,
    $Recipe.Groups[4].Value,
    $Recipe.Groups[5].Value.ToUpperInvariant(),
    $Plan.Groups[1].Value,
	$Landform.Groups[1].Value,
	$Landform.Groups[2].Value,
	$Landform.Groups[3].Value,
	$Landform.Groups[4].Value,
	$Landform.Groups[5].Value,
	$Landform.Groups[6].Value,
    'world.environment-candidate-selector',
    $LandAccess.Value,
    'runtime.visual-materializer',
    $Materialization.Groups[1].Value,
    $Materialization.Groups[2].Value
) -join '|'
$Sha = [System.Security.Cryptography.SHA256]::Create()
try {
    $CandidateId = ([BitConverter]::ToString(
        $Sha.ComputeHash([Text.Encoding]::UTF8.GetBytes($CandidateText))) -replace '-', '').ToLowerInvariant()
}
finally {
    $Sha.Dispose()
}

$RepoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../../..'))
$GitCommit = @(& git -C $RepoRoot rev-parse HEAD 2>$null)[0]
if ([string]::IsNullOrWhiteSpace($GitCommit)) { $GitCommit = 'unknown' }
$GitDirty = @(& git -C $RepoRoot status --porcelain 2>$null).Count -gt 0

$Record = [ordered]@{
    schemaVersion = 15
    runId = [guid]::NewGuid().ToString('D')
    candidateId = $CandidateId
    recordedAtUtc = [DateTime]::UtcNow.ToString('o', $Culture)
    machine = [ordered]@{
        id = 'world.terrain-generator'
        version = To-Int $Recipe.Groups[2].Value
    }
    input = [ordered]@{
        seed = $Seed
        chunkSizeCm = To-Double $Recipe.Groups[4].Value
		landform = [ordered]@{
			version = To-Int $Landform.Groups[1].Value
			enabled = $Landform.Groups[2].Value -eq 'yes'
			relief = To-Double $Landform.Groups[3].Value
			volcanism = To-Double $Landform.Groups[4].Value
			ice = To-Double $Landform.Groups[5].Value
			protectedRegionCount = To-Int $Landform.Groups[6].Value
		}
		landAccess = [ordered]@{
			machineId = 'world.environment-candidate-selector'
			version = To-Int $LandAccess.Groups[1].Value
			enabled = $LandAccess.Groups[2].Value -eq 'yes'
			evaluated = $LandAccess.Groups[3].Value -eq 'yes'
			briefVersion = To-Int $LandAccess.Groups[4].Value
			compilerVersion = To-Int $LandAccess.Groups[5].Value
			targetWalkable = To-Double $LandAccess.Groups[6].Value
			targetConnected = To-Double $LandAccess.Groups[7].Value
			candidateCount = To-Int $LandAccess.Groups[8].Value
			worldCellsPerAxis = To-Int $LandAccess.Groups[9].Value
			arrivalCellsPerAxis = To-Int $LandAccess.Groups[10].Value
			arrivalCellSizeCm = To-Double $LandAccess.Groups[11].Value
			padRadiusCm = To-Double $LandAccess.Groups[12].Value
			terrainSemanticVersion = To-Int $LandAccess.Groups[13].Value
			arrivalTransitionWidthCm = To-Double $LandAccess.Groups[14].Value
			flatNormalZ = To-Double $LandAccess.Groups[15].Value
			slopeNormalZ = To-Double $LandAccess.Groups[16].Value
			escapeDistanceCells = To-Int $LandAccess.Groups[17].Value
			walkableTolerance = To-Double $LandAccess.Groups[18].Value
			connectedTolerance = To-Double $LandAccess.Groups[19].Value
			satisfyingCount = To-Int $LandAccess.Groups[20].Value
			rejectedCount = To-Int $LandAccess.Groups[21].Value
			selected = [ordered]@{
				index = To-Int $LandAccess.Groups[22].Value
				featureScale = To-Double $LandAccess.Groups[23].Value
				dissectionScale = To-Double $LandAccess.Groups[24].Value
				ruggednessScale = To-Double $LandAccess.Groups[25].Value
				walkable = To-Double $LandAccess.Groups[26].Value
				connected = To-Double $LandAccess.Groups[27].Value
				hasWorldAccess = $LandAccess.Groups[28].Value -eq 'yes'
				escapesArrival = $LandAccess.Groups[29].Value -eq 'yes'
			}
			best = [ordered]@{
				index = To-Int $LandAccess.Groups[30].Value
				featureScale = To-Double $LandAccess.Groups[31].Value
				dissectionScale = To-Double $LandAccess.Groups[32].Value
				ruggednessScale = To-Double $LandAccess.Groups[33].Value
				walkable = To-Double $LandAccess.Groups[34].Value
				connected = To-Double $LandAccess.Groups[35].Value
				hasWorldAccess = $LandAccess.Groups[36].Value -eq 'yes'
				escapesArrival = $LandAccess.Groups[37].Value -eq 'yes'
			}
		}
    }
    recipe = [ordered]@{
        schemaVersion = To-Int $Recipe.Groups[1].Value
        checksum = $Recipe.Groups[5].Value.ToUpperInvariant()
        nodeCount = To-Int $Recipe.Groups[6].Value
        valid = $Recipe.Groups[7].Value -eq 'yes'
    }
    environment = [ordered]@{
        family = $Site.Groups[2].Value
		hydrology = $Site.Groups[8].Value
        hasWater = $Site.Groups[3].Value -eq 'yes'
        baseSiteValid = $Site.Groups[4].Value -eq 'yes'
        baseSite = [ordered]@{
            xCm = To-Double $Site.Groups[5].Value
            yCm = To-Double $Site.Groups[6].Value
        }
        baseDropCm = To-Double $Site.Groups[7].Value
    }
    evaluation = [ordered]@{
        machineId = 'evaluation.terrain-metrics'
        version = To-Int $Evaluation.Groups[1].Value
        windowSizeCm = To-Double $Evaluation.Groups[9].Value
        metrics = [ordered]@{
            reliefCm = To-Double $Evaluation.Groups[2].Value
            waterFraction = To-Double $Evaluation.Groups[3].Value
            meanNeighborStepCm = To-Double $Evaluation.Groups[4].Value
            maxNeighborStepCm = To-Double $Evaluation.Groups[5].Value
            buildableFraction = To-Double $Evaluation.Groups[6].Value
			meanHeightCm = To-Double $Evaluation.Groups[7].Value
			belowDatumFraction = To-Double $Evaluation.Groups[8].Value
        }
    }
    sitePlan = [ordered]@{
        machineId = 'world.site-route-planner'
        version = To-Int $Plan.Groups[1].Value
        valid = $Plan.Groups[2].Value -eq 'yes'
        siteCount = To-Int $Plan.Groups[3].Value
        routeCount = To-Int $Plan.Groups[4].Value
        diagnosticCount = To-Int $Plan.Groups[5].Value
    }
    materialization = [ordered]@{
        machineId = 'runtime.visual-materializer'
        version = To-Int $Materialization.Groups[1].Value
        backend = $Materialization.Groups[2].Value
        batchCount = To-Int $Materialization.Groups[3].Value
        instanceCount = To-Int $Materialization.Groups[4].Value
        interactionCarrierCount = To-Int $Materialization.Groups[5].Value
        valid = $Materialization.Groups[6].Value -eq 'yes'
    }
    performance = [ordered]@{
        machineId = 'evaluation.performance'
        version = To-Int $Performance.Groups[1].Value
        generationMs = To-Double $Performance.Groups[2].Value
        meanFrameMs = To-Double $Performance.Groups[3].Value
        usedPhysicalMB = To-Double $Performance.Groups[4].Value
        totalActors = To-Int $Performance.Groups[5].Value
        scatterActors = To-Int $Performance.Groups[6].Value
        claimActors = To-Int $Performance.Groups[7].Value
        baseActors = To-Int $Performance.Groups[8].Value
        loadedTerrainCells = To-Int $Performance.Groups[9].Value
        staticMeshComponents = To-Int $Performance.Groups[10].Value
        instancedStaticMeshComponents = To-Int $Performance.Groups[11].Value
        staticMeshInstances = To-Int $Performance.Groups[12].Value
        staticMeshTriangles = [long]::Parse($Performance.Groups[13].Value, $Culture)
        dynamicMeshTriangles = [long]::Parse($Performance.Groups[14].Value, $Culture)
        lod0Triangles = [long]::Parse($Performance.Groups[15].Value, $Culture)
        withinBudget = $Performance.Groups[16].Value -eq 'yes'
        issueCount = To-Int $Performance.Groups[17].Value
    }
    provenance = [ordered]@{
        engineVersion = $EngineVersion
        platform = $Platform
        gitCommit = $GitCommit.Trim()
        gitDirty = $GitDirty
    }
    artifacts = $Artifacts
}

$ArchiveFullPath = [IO.Path]::GetFullPath($ArchivePath)
$ArchiveDirectory = Split-Path -Parent $ArchiveFullPath
New-Item -ItemType Directory -Force -Path $ArchiveDirectory | Out-Null
$Json = $Record | ConvertTo-Json -Depth 8 -Compress
# ponytail: sequential append is enough for the current sweep; add a lock or per-run files when workers run concurrently.
[IO.File]::AppendAllText($ArchiveFullPath, $Json + [Environment]::NewLine, [Text.UTF8Encoding]::new($false))
Write-Output "RECORDED run=$($Record.runId) candidate=$CandidateId archive=$ArchiveFullPath"
