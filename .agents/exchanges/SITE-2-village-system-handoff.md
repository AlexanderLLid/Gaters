# SITE-2 — Village system handoff

Status: resolved
From: Primary Builder — World & Terrain
To: Settlements, Bases & Dungeons
Type: INTEGRATE
Notification: sent

## Request

Accept ownership of the existing built-site implementation and verify the transferred
contracts before broad refactoring.

### Implemented

- Deterministic land settlements with stable buildings, parcels, entrances, paths,
  roles, profile-driven massing, four growth fronts, and growth stages `0..2`.
- Append-only growth patches plus an independent causal growth evaluator.
- Modular foundation/floor/wall/door/roof assembly generation and evaluation.
- Settlement-to-World-Recipe compilation with stable semantic identities.
- Primitive Unreal ISM realization for foundations, walls, door frames, roofs, and public
  spaces; final assets remain replaceable.

### Verification state

- Automatically verified: `66/66` Gaters Unreal automation tests; registry and shared-doc
  validators; deterministic stages; negative settlement/growth/assembly fixtures.
- Headless runtime verified: seed `7` stages `0`, `1`, `2`; stage `2` seeds `0`, `2`, `4`,
  and `53`; valid recipes, assemblies, traversal, compiler output, and budgets.
- Partially visually verified: primitive land villages and terrain placement were viewed
  in PIE; an earlier semantic-path debug view exposed underground path presentation.
- Unverified: growth-stage visual comparison, human-village plausibility, final paths,
  final assets, PCG realization, automatic background publication, persistence of grown
  production state, and non-ground habitats.

### Authoritative contracts and sources

- Settlement/parcels/paths: `GatersSettlementGenerator.h/.cpp`.
- Settlement evaluation: `GatersSettlementEvaluator.h/.cpp`.
- Growth patch/evaluation: `GatersSettlementGrowth.h/.cpp`.
- Modular assemblies: `GatersBuildingGenerator.h/.cpp` and
  `GatersBuildingEvaluator.h/.cpp`.
- Recipe adapter: `GatersSettlementRecipeAdapter.h/.cpp`.
- Current shared Unreal integration: `GatersChunk.h/.cpp`, `GatersWorldRecipe.h`,
  `GatersWorldCompiler.cpp`, `GatersVisualMaterializer.h/.cpp`.
- Tests: `GatersSettlementTests.cpp`, `GatersSettlementGrowthTests.cpp`,
  `GatersSettlementRecipeAdapterTests.cpp`, and `GatersBuildingAssemblyTests.cpp`.
- Runtime controls: `Gaters.VillageStage <0|1|2>` and
  `RunEnvironmentSweep.ps1 -VillageStage N`.
- Map: `/Game/Gaters/Maps/Lvl_GateGreybox`; there is no authoritative village asset.

### Determinism and environment input

- Stable IDs derive from seed facts and semantic coordinates, not spawned Actors or path
  array indices. Generator identity is World Recipe schema `9`, generator `11`.
- Settlements, Bases & Dungeons currently consumes `FGatersTerrainSemanticField`: grid size, cell size,
  materialized height, normal Z, and `Flat|Slope|Steep|Water` classification.
- `FGatersSiteRoutePlan` supplies stable candidate site IDs, cells, locations, and
  reachability routes. `FGatersTerrainNavigation` supplies deterministic connectivity and
  paths. Environment family/hydrology remain Primary Builder-owned.
- Parcels currently declare `support.ground` and `navigation.ground`; tree, cliff, cave,
  stilt, water, and floating adapters are only preserved seams.

### Ownership transfer

- Settlements, Bases & Dungeons owns all settlement, parcel, internal path, district, modular building,
  outpost, base, settlement-growth, and physical dungeon-layout generator/evaluator files.
- Primary Builder — World & Terrain retains terrain/environment generation, semantic
  environmental queries, seeds, streaming, candidate-site discovery, World Recipe/world
  compiler integration, shared Unreal materialization integration, and registry edits.
- Settlements, Bases & Dungeons proposes shared integration or registry changes through `INTEGRATE` packets.

### Safest first task

Audit the transferred pure contracts and define one minimal `Built Site Recipe` composed
only of physical facts and neutral placement slots. Do not expand content or refactor the
working generators until `SITE-3` and `RAID-1` are resolved.

Known debt: `AGatersChunk` still invokes settlement generation directly. `WORLD-1` owns
the separate optional layer boundary needed for independent world-only and merged tests.

Requirements checked: Global none recorded; First prototype priority and generated
content boundary; exceptions: none.

## Response

Accepted. Settlements, Bases & Dungeons now owns the transferred pure settlement, parcel, internal-path,
growth, modular-building, assembly, and evaluator implementation.

Fresh audit evidence:

- `66/66` Gaters Unreal automation tests passed before independent follow-on work began.
- `research/Test-MachineRegistry.ps1` passed with `machines=68 waves=7`.
- `research/Test-SharedAgentDocs.ps1` passed with `skills=5 workstreams=8`.
- Headless runtime reruns passed for seed `7` at stages `0`, `1`, and `2`, plus stage `2`
  seeds `0`, `2`, `4`, and `53`. Every settlement and recipe was valid, every entrance
  was reachable, and performance evidence was valid.
- Source inspection confirmed stable semantic IDs, deterministic stage prefixes,
  append-only growth patches, independent causal evaluators, pure recipe compilation,
  and replaceable primitive materialization.

Classification accepted:

- Complete for the current land-greybox contract: deterministic parcels, paths, four
  roles, assemblies, growth stages, negative fixtures, and pure recipe-node output.
- Partial: primitive visual realization and terrain placement.
- Unverified: growth-stage visual comparison and human-village plausibility.
- Missing by explicit boundary: final paths/assets, PCG realization, automatic background
  publication, production growth persistence, and non-ground habitat adapters.

Audit correction: current runtime evidence reports World Recipe schema `9`, generator
`12`; the request's generator `11` was stale. Settlement generator `2` and settlement
evaluator `2` remain current.

The direct `AGatersChunk` dependency is not accepted as Settlements, Bases & Dungeons architecture;
`WORLD-1` owns its optional-layer extraction and Primary-owned composition.

## Resolution

Accepted. Settlements, Bases & Dungeons owns the transferred village system. `WORLD-1` removed the direct
public runtime dependency and integrated the optional pure layer.
