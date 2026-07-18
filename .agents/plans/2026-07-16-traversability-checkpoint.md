# Traversability Checkpoint Implementation Plan

> **For agentic workers:** Use `superpowers:executing-plans` inline. Do not create a branch or commit without user authorization.

**Goal:** Detect trapped/disconnected generated terrain headlessly and let the user inspect the same evidence visually in Unreal PIE.

**Architecture:** A pure grid-navigation query consumes `FGatersTerrainSemanticField`; a separate traversability evaluator turns reachability into versioned metrics. `AGatersChunk` reports the metrics and owns only the Unreal debug drawing adapter.

**Tech Stack:** Unreal Engine 5.8 C++, Automation Tests, DrawDebugHelpers, existing commandlet sweep.

## Global Constraints

- Preserve the generated terrain, recipe checksum, scatter, base stamp, and existing site counts.
- Flat and slope cells are traversable; steep and water cells are blocked.
- Use four-neighbor connectivity in version 1; this is terrain evidence, not final character pathfinding.
- Debug rendering is opt-in through `Gaters.Traversal` and creates no persistent gameplay actors.

### Task 1: Pure terrain navigation query

**Files:**
- Create: `Unreal/Prototype/Source/Prototype/Public/GatersTerrainNavigation.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersTerrainNavigation.cpp`
- Create: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersTerrainNavigationTests.cpp`

- [ ] Write failing tests for a reachable corridor, water barrier, trapped start, deterministic shortest path, and connected-component count.
- [ ] Build and confirm failure because the query contract is missing.
- [ ] Implement breadth-first reachability and shortest-path reconstruction over semantic samples.
- [ ] Run `Gaters.Worldgen.Navigation` and confirm all fixtures pass.

### Task 2: Versioned traversability evaluation

**Files:**
- Create: `Unreal/Prototype/Source/Prototype/Public/GatersTraversabilityEvaluator.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersTraversabilityEvaluator.cpp`
- Extend test: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersTerrainNavigationTests.cpp`

- [ ] Write failing tests for reachable coverage, escape from the Gate region, and reachable/unreachable critical goals.
- [ ] Implement evaluator version 1 by composing the navigation query.
- [ ] Run focused automation and confirm the known-good and deliberately broken fixtures separate.

### Task 3: Chunk report and opt-in Unreal visualization

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersChunk.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersChunk.cpp`

- [ ] Store the generated semantic field and traversability result on the chunk.
- [ ] Emit one stable `TRAVERSE` report containing version, reachable/walkable counts, coverage, components, escape, and base-route result.
- [ ] Add `Gaters.Traversal` to draw reachable cells green, blocked steep cells red, water blue, unreachable walkable cells yellow, and the Gate-to-base path white.
- [ ] Run seed 7 and confirm all previous generation evidence remains unchanged.

### Task 4: Machine promotion and checkpoint verification

**Files:**
- Modify: `research/machines.json`

- [ ] Narrow and promote terrain navigation version 1 based on its actual terrain-only guarantee.
- [ ] Promote traversability version 1 only if fixture tests and real seed evidence pass.
- [ ] Run the Unreal build, all `Gaters.Worldgen` tests, seed sweep, registry checks, archive checks, and `git diff --check`.
