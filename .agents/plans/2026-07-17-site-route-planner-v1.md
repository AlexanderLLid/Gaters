# Site and Route Planner v1 Implementation Plan

> **For agentic workers:** Execute inline with TDD. Do not branch, commit, or push unless the human asks.

**Goal:** Deterministically place one village and one landmark around the existing arrival and raid-base anchors, connect every required site with terrain-valid routes, and expose the result in runtime evidence.

**Architecture:** A pure `FGatersSiteRoutePlanner` consumes the existing semantic field, seed, and recipe base location. It reuses `FGatersTerrainNavigation` for reachability and paths, emits one versioned plan, then `AGatersChunk` compiles that plan into recipe nodes and traversal debug drawing.

**Tech Stack:** Unreal Engine 5.8 C++, existing semantic-field/navigation/world-recipe types, Unreal automation tests, debug drawing.

## Global Constraints

- Keep the existing arrival and raid-base placement unchanged.
- Add no settlement layout, roads, buildings, assets, biome logic, new pathfinder, or route-carving algorithm.
- Required sites must be dry, walkable, reachable, separated, deterministic, and represented by stable IDs.
- A failed plan must preserve a specific diagnostic.
- Do not commit or branch.

### Task 1: Pure planner contract

**Files:**
- Create: `Unreal/Prototype/Source/Prototype/Public/GatersSiteRoutePlanner.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersSiteRoutePlanner.cpp`
- Create: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersSiteRoutePlannerTests.cpp`

- [x] Write automation fixtures for deterministic open terrain, blocked base, insufficient sites, and generated-seed sweeps.
- [x] Build and run the focused test; verify it fails because the planner contract does not exist.
- [x] Implement the minimum deterministic candidate scan and reuse `FGatersTerrainNavigation::FindPath`.
- [x] Run the focused test until it passes without weakening its constraints.

### Task 2: Runtime recipe and debug evidence

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersWorldRecipe.h`
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersChunk.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersChunk.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersWorldRecipeTests.cpp`

- [x] Add village, landmark, and route-waypoint recipe node kinds.
- [x] Generate one plan from the already-built runtime semantic field and compile stable plan nodes into the recipe.
- [x] Log planner version, validity, site count, route count, and diagnostic count.
- [x] Draw site markers and route lines as part of traversal debug capture.
- [x] Verify recipe identity changes when a planned site or route moves.

### Task 3: Promotion evidence

**Files:**
- Modify: `research/machines.json`
- Modify: `.agents/plans/2026-07-17-site-route-planner-v1.md`

- [x] Run the focused planner, world-recipe, navigation, and complete Gaters automation suites.
- [x] Capture representative terrain-family traversal galleries and inspect site/route overlays.
- [x] Mark the planner active with its implemented verifier and champion version.
- [x] Validate registry, experiment archive, gallery images, and `git diff --check`.
