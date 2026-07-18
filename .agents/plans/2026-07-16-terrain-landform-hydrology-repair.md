# Terrain Landform and Hydrology Repair Implementation Plan

> **For agentic workers:** Execute inline with `superpowers:executing-plans`; do not branch or commit unless the user asks.

**Goal:** Make mountain seeds read as uplifted landforms instead of crater fields, allow water independently of the macro landform, and prevent the broken profile from being promoted again.

**Architecture:** Keep `FGatersEnvironment` as the single pure seed-derived terrain contract. Replace the thresholded mountain plateau with continuous broad uplift plus ridged peaks; add one orthogonal hydrology enum that controls the water surface without creating another generator. Extend the existing evaluator with signed height-distribution evidence so a high plateau cut by deep pits cannot satisfy the mountain contract merely through large relief.

**Tech Stack:** Unreal Engine 5.8 C++, Unreal Automation Tests, existing environment sweep/archive.

## Global Constraints

- Preserve deterministic seed generation and global-coordinate sampling across streamed cells.
- Keep the four macro landform families for this repair; hydrology is orthogonal metadata.
- Seed 53 is a held-out mountain regression case.
- Do not add biome, erosion simulation, or terrain assets in this repair.
- Bump generator and evaluator versions because seed output and metric identity change.

---

### Task 1: Reject crater-shaped mountains

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersTerrainEvaluator.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersTerrainEvaluator.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersEnvironmentTests.cpp`

- [x] Add a seed-53 regression asserting a mountain uplift score is positive and deep-basin coverage remains bounded.
- [x] Run `Gaters.Worldgen.Environment.MountainProfile` and confirm the current generator fails for the intended crater topology.
- [x] Add evaluator fields for mean elevation and low-tail coverage, derived from the existing sample grid.
- [x] Bump the evaluator version and include the fields in deterministic evaluator assertions.

### Task 2: Repair the mountain height function

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersEnvironment.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersWorldDiff.h`

- [x] Replace the thresholded `MountainMask * height - offset` formula with continuous broad uplift, ridged peaks, and small detail.
- [x] Run the focused mountain profile test until seed 53 passes without weakening its assertions.
- [x] Run environment variety, base-site, semantic-field, and traversability tests.
- [x] Bump `GatersGenVersion` so old diffs do not replay against new terrain.

### Task 3: Separate hydrology from landform

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersEnvironment.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersEnvironment.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersEnvironmentTests.cpp`

- [x] Add failing tests proving identical seeds reproduce hydrology and that at least one mountain seed and one lowland seed can contain water.
- [x] Add `EGatersHydrology` and deterministic selection independent of `EGatersEnvironment`.
- [x] Retain forced river/coast behavior for canyon/archipelago; let lowlands/mountains roll dry or lakes.
- [x] Make `HasWater()` and water height depend on hydrology rather than the landform enum.
- [x] Run all environment and worldgen tests.

### Task 4: Repair machine promotion evidence

**Files:**
- Modify: `research/machines.json`
- Modify: `.agents/workflow-feedback.md`

- [x] Record this as a machine failure: generator-3 produced crater mountains and evaluator-1 promoted them.
- [x] Update terrain-generator and terrain-metrics contracts with signed landform/hydrology challenge evidence only after tests pass.
- [x] Promote the new generator/evaluator versions only after a seed-53 headless run records the new evidence.

### Task 5: Continue with terrain streaming prefetch

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersWorldCellStreaming.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersWorldCellStreaming.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersWorldCellStreamingTests.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersChunk.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersChunk.cpp`

- [x] Add a failing pure test for loading the forward row before the source crosses a cell edge.
- [x] Add a velocity/direction input that expands the desired set by one forward row while preserving the normal 3x3 set.
- [x] Retain the rear row until after the boundary crossing to avoid unload flicker.
- [x] Run streaming tests and a real map smoke test; verify active cells remain bounded and the initial set stays nine.
