# Regional water surfaces implementation plan

**Goal:** Seed-declared wet regional profiles emit deterministic bounded water surfaces
that Unreal can render without changing global water or the Built Site layer.

**Architecture:** A pure regional-water recipe adapts existing environment water
footprints into world-space surfaces with stable IDs. `AGatersChunk` materializes those
surfaces with Unreal's existing water material and primitive cylinder mesh; the recipe
remains authoritative and the components remain derived output.

**Tech stack:** Unreal Engine 5.8 C++, existing automation framework and engine assets.

## Constraints

- Dry regional intent emits no surface.
- Global water remains owned by the existing environment path and is not duplicated.
- Lake footprints reuse existing deterministic lake geometry.
- River/ocean footprints are capped inside the full-influence regional core.
- No Built Site layer, Actors, assets, or tactical data enter the pure recipe.
- Requirements checked: Global none recorded; exceptions: none.

### Task 1: Pure regional-water recipe

- [x] Write failing determinism, dry-suppression, lake-footprint, ocean-bound, and stable-ID tests.
- [x] Implement the minimum pure recipe by adapting `FGatersEnvironment::WaterSurfaces`.
- [x] Run focused automation until green.

### Task 2: Unreal renderer adapter

- [x] Materialize recipe surfaces as non-colliding primitive water components.
- [x] Add causal runtime evidence for regional surface count.
- [x] Run held-out wide world-only seed 7 capture, full automation, and validators.
