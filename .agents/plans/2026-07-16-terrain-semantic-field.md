# Terrain Semantic Field Implementation Plan

**Goal:** Give generators and evaluators one deterministic, actor-free grid describing the materialized terrain as flat, slope, steep, or water.

**Scope:** Version 1 only contains semantics already consumed by chunk analysis. Visibility, cover, choke, and reservations wait until a real consumer exists.

## Task 1: Specify the pure field with failing automation tests

- Add `GatersTerrainSemanticFieldTests.cpp`.
- Require deterministic samples, a flattened Gate center, exhaustive category counts, water in an archipelago fixture, and steep terrain in a canyon fixture.
- Build to prove the missing contract fails before implementation.

## Task 2: Implement the minimum actor-free field

- Add `GatersTerrainSemanticField.h/.cpp`.
- Centralize the materialized terrain height function, including the Gate-pad blend.
- Sample the existing grid and classify with the existing water and normal-Z thresholds.
- Build and run the focused semantic-field tests.

## Task 3: Make chunk analysis consume the field

- Delegate `AGatersChunk::GroundHeight` to the shared materialized-height function.
- Replace its duplicate sampling/classification loop with the semantic field.
- Preserve reservation handling, build-plot discovery, resource mutation, visuals, and report format.
- Run all `Gaters.Worldgen` tests and a seed-7 commandlet sweep; compare the `PLOTS` evidence with the existing baseline.

## Task 4: Promote the machine contract

- Narrow the registry outcome to version-1 semantics that now exist and have consumers.
- Mark `world.terrain-semantic-field` verified and move focus to `runtime.navigation-query` only after all checks pass.
- Run registry validation and `git diff --check`.
