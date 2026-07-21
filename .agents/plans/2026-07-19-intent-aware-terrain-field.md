# Intent-aware terrain field implementation plan

**Goal:** Seed-declared regional terrain and hydrology tendencies affect sampled terrain
without changing the arrival region or coupling World & Terrain to the Built Site layer.

**Architecture:** Keep `FGatersEnvironment` as the reusable terrain-family engine. A pure
intent-aware field selects a regional profile, evaluates that existing family in local
coordinates, and smoothly blends it into the global terrain. Runtime terrain sampling
then receives the intent recipe explicitly; no Actors or sites enter the pure contract.

**Tech stack:** Unreal Engine 5.8 C++, existing automation framework.

## Constraints

- The global intent profile must match the existing seed-derived environment.
- Regional influence must be zero inside the protected arrival/site zone.
- Region edges must return continuously to global terrain.
- No universal terrain, hydrology, biome, or resource guarantee.
- Local water-surface rendering is outside this slice; only terrain depression and
  semantic hydrology are consumed here.

### Task 1: Pure regional terrain adapter

**Files:**

- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersEnvironment.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersEnvironment.cpp`
- Create: `Unreal/Prototype/Source/Prototype/Public/GatersIntentTerrainField.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersIntentTerrainField.cpp`
- Create: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersIntentTerrainFieldTests.cpp`

- [x] Write failing tests for global-profile agreement, protected origin, regional core
  effect, deterministic identity, and continuous boundary fade.
- [x] Add the minimum environment-profile copy and pure intent-aware query.
- [x] Run focused automation until green.

### Task 2: Runtime terrain sampling adapter

**Files:**

- Modify: `GatersTerrainSemanticField.h/.cpp`
- Modify: `GatersTerrainCell.h/.cpp`
- Modify: `GatersChunk.h/.cpp`
- Modify relevant terrain semantic tests.

- [x] Write failing overload/integration tests using an explicit intent recipe.
- [x] Route chunk and streamed-cell height/normal sampling through the intent field.
- [x] Run held-out world-only runtime, full automation, and validators.

## Honest promotion boundary

- This challenger is not promoted as a complete hydrology machine until local water
  surfaces and an independent sampled-fidelity evaluator exist.
