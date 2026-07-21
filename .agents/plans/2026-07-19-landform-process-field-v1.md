# Composable Landform Process Field v1

## Outcome

- A compiled environment brief drives reusable terrain-shaping processes without
  selecting a named world generator.
- The field exposes base height, individual process contributions, final height, and
  regional influence as pure data.
- Existing terrain remains the runtime champion until held-out evaluation proves the
  challenger improves coverage without breaking traversal, continuity, or budget.

## Approved design

- Compile a versioned process recipe from an existing environment and compiled brief.
- Reject seed, world-size, and version mismatches with causal diagnostics.
- Query relief scaling, broad deterministic uplift, volcanic mass, and glacial valley
  carving independently, then sum them into a final height.
- Blend regional target profiles smoothly over the global profile.
- Keep sites, settlements, Anchors, Rifts, encounters, species, and assets outside the
  field.

## Deferred

- Erosion and deposition need drainage evidence.
- Coasts need accepted water and surface-condition evidence.
- Runtime integration and visual promotion wait for isolated field evidence.

## Implementation

### Task 1 - Contract and RED evidence

- [x] Add pure recipe, query sample, diagnostics, and compiler declarations.
- [x] Test determinism, causal incompatibility, contribution accounting, controllable
  relief/volcanism/ice, and smooth regional influence.
- [x] Build before production implementation and record the expected missing-header RED.

### Task 2 - Minimum field

- [x] Implement deterministic compilation and coordinate queries without Actors or
  assets.
- [x] Run `Gaters.Worldgen.LandformProcesses`.

### Task 3 - Challenger evidence

- [x] Compare fixed low/high process briefs with the independent terrain metrics.
- [x] Run the complete `Gaters` automation suite.
- [x] Update machine truth and workstream status without promoting the challenger.
- [x] Run registry, shared-doc, and diff checks.
- [x] Obtain independent read-only review.

Requirements checked: none applicable; exceptions: none.
