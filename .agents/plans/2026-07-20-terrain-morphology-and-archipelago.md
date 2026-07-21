# Terrain Morphology and Archipelago Plan

**Goal:** Reject exposed terrain-generator primitives and make seed `131` archipelago
silhouettes irregular without sacrificing deterministic arrival or base viability.

**Boundary:** Pure terrain height queries only. Sites, villages, assets, materials, and
runtime streaming remain consumers and do not participate.

Requirements checked: generated-content boundary, `ART-1`; exceptions: none.

## Wave 1 — morphology evidence

- Add a pure morphology evaluator that samples height contours, identifies closed
  components, and reports compact radial shapes as evidence rather than deciding art.
- Add synthetic-circle and irregular-shape contract tests.
- Add seed `131` as a held-out archipelago regression that fails on the current stamped
  satellite island.

## Wave 2 — smallest generator correction

- Domain-warp archipelago mass coordinates with deterministic low-frequency terrain
  noise before evaluating arrival and satellite masses.
- Remove the dedicated base-shelf stamp; continue selecting bases through the existing
  `FindBaseSite` contract.
- Preserve seed determinism, ocean coverage, arrival clearance, topology variety, and
  base-site tests.

## Verification

- Focused morphology and environment automation must pass.
- Full Gaters automation must pass.
- Seed `131` must preserve arrival escape and a valid base candidate.
- Capture seed `131` with landforms off and on for human visual acceptance.
- Do not promote the terrain challenger from automated evidence alone.

## Files

- Create `GatersTerrainMorphologyEvaluator.h/.cpp` and focused tests.
- Modify only the archipelago branch of `GatersEnvironment.cpp`.
- Update the Primary workstream, workflow evidence, and machine registry truthfully after
  verification.
