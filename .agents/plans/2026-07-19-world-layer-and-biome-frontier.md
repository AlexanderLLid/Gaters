# World Layer Separation and Biome Frontier Implementation Plan

> **For agentic workers:** execute inline with TDD. Do not branch, commit, or push; the
> human reviews the shared worktree. Coordinate Settlements, Bases & Dungeons changes through `WORLD-1`.

**Goal:** World & Terrain runs and can be visually inspected without the Built Site layer, then
exposes the first deterministic stream-safe biome field for later resource generation.

**Architecture:** Pure generators emit optional recipe layers. World & Terrain owns the
base environment recipe, generic layer composition, runtime mode selection, and biome
queries. Settlements, Bases & Dungeons owns its layer producer. Unreal only composes and materializes
accepted layers; empty Built Site layer output is valid.

**Tech Stack:** Unreal Engine 5.8 C++, automation tests, World Recipe, existing runtime
cell streaming and developer console commands.

## Global Constraints

- Dependency direction is `Settlements, Bases & Dungeons -> World & Terrain`; never the reverse in public
  terrain/environment contracts.
- World-only mode must generate, stream, compile, and materialize with zero Built Site
  nodes.
- Merged mode must preserve current recipe IDs and visible settlement behavior.
- No separate Unreal module/plugin in v1; temporary shared-build breakage during parallel
  edits is accepted by the human.
- Biome facts are deterministic coordinate queries and own no Actors or assets.
- Requirements checked: Global none recorded; generated content boundary and first
  prototype generated-world requirements; exceptions: none.

### Task 1: Optional recipe-layer composition

**Files:**

- Create: `Unreal/Prototype/Source/Prototype/Public/GatersWorldRecipeLayer.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersWorldRecipeLayer.cpp`
- Create: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersWorldRecipeLayerTests.cpp`

**Interfaces:**

- `FGatersWorldRecipeLayer`: `LayerId`, `SchemaVersion`, `GeneratorVersion`, `bGenerated`,
  `Nodes`, `Diagnostics`.
- `FGatersWorldRecipeLayerComposer::Append(Recipe, Layer)` returns causal diagnostics and
  changes nothing when the layer is empty or invalid.

- [x] Write a failing test proving empty layers are accepted, valid nodes append, and
  duplicate IDs reject without partially modifying the recipe.
- [x] Run `Gaters.Worldgen.WorldRecipeLayer` and verify RED because the API is absent.
- [x] Implement transactional ID validation and append only.
- [x] Run the focused test until GREEN.

### Task 2: World-only runtime mode

**Files:**

- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersChunk.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersChunk.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersTestSpawner.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersTestSpawner.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersSeedCommandTests.cpp`
- Modify: `Unreal/Prototype/Scripts/RunEnvironmentSweep.ps1`

**Interfaces:**

- `AGatersChunk::bEnableBuiltSites` defaults `true` for compatibility.
- `Gaters.BuiltSites <0|1>` persists mode and reloads the current map.
- `-GatersBuiltSites=0|1` and `RunEnvironmentSweep.ps1 -BuiltSites` support headless runs.

- [x] Add command-registration and mode-parsing tests; verify RED.
- [x] Add the minimum persisted/command-line mode adapter.
- [x] Separate terrain/site discovery analysis from optional Built Site layer invocation.
- [x] Remove settlement/growth implementation types from `GatersChunk.h`.
- [x] Verify world-only recipes contain no settlement modules, parcels, growth fronts, or
  settlement paths while terrain/content streaming remains valid.

### Task 3: Built Site adapter integration

**Files:** owned by Settlements, Bases & Dungeons under the existing settlement source family; exact names
are resolved in `WORLD-1` before shared integration.

- [x] Consume the `WORLD-1` response without changing its agreed types silently.
- [x] Replace direct settlement calls in `AGatersChunk` with the Built Site layer result.
- [x] Verify stage `0..2` node identity and counts match pre-split evidence.
- [x] Headless-run seed `7` in world-only and merged modes.
- [x] Capture one fixed-camera image per mode for human comparison.

### Task 4: Stream-safe biome semantic query

**Files:**

- Create: `Unreal/Prototype/Source/Prototype/Public/GatersBiomeField.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersBiomeField.cpp`
- Create: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersBiomeFieldTests.cpp`

**Interfaces:**

- `FGatersBiomeSample`: materialized height, normal Z, water proximity, elevation band,
  moisture, exposure, and semantic biome key.
- `FGatersBiomeField::Query(Environment, Point, Semantics)` is deterministic at arbitrary
  coordinates and independent of streamed-cell load order.

- [x] Write failing determinism, adjacent-cell seam, water-proximity, terrain-family
  distinction, bounded-value, and non-vacuous variation tests.
- [x] Verify RED because the query is absent.
- [x] Implement the minimum seeded low-frequency facts by reusing environment/materialized
  terrain queries; do not generate assets or sites.
- [x] Run held-out seeds `0`, `2`, `4`, `7`, and `53` until GREEN.

### Task 5: Close evidence

- [x] Run full `Gaters` automation, registry validator, shared-doc validator, and
  `git diff --check`.
- [x] Update Primary Builder status and `research/machines.json` only with observed
  guarantees; leave biome/resource promotion incomplete until resource loops exist.
- [x] Record world-only/merged commands and remaining limitations in a short report.

## Self-review

- The plan does not require Settlements, Bases & Dungeons to finish before pure biome work begins.
- World-only behavior is a tested product path, not a compile-time comment or hidden mock.
- Biome semantics are not coupled to final assets, PCG, sites, or spawned Actors.
- V1 avoids speculative plugin/module extraction and automatic background publication.
