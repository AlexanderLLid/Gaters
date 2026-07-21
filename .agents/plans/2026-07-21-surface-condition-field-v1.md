# Surface Condition Field v1 Implementation Plan

> **For agentic workers:** Execute inline with red-green verification. Do not create a
> branch, commit, add materials/assets, carve terrain, or edit active Settlements work.

**Goal:** Derive reusable continuous physical surface evidence from the authoritative
terrain, climate, drainage, and declared-water root without named terrain-family rules.

**Architecture:** A pure evaluator converts explicit evidence into bounded soil, rock,
sediment, sand, snow, ice, saturation, shore, ridge, valley, and cliff weights. A thin
root adapter samples authoritative terrain/climate, bilinearly interpolates the accepted
drainage grid, and measures declared water geometry. A versioned recipe records settings
and root provenance. It chooses no biome name, resource, art, site, encounter, or Actor.

**Tech stack:** Unreal Engine 5.8 C++ and existing Unreal Automation.

## Global constraints

- `WORLD-1`: dry, snowless, iceless, shoreless, channel-free, or otherwise sparse
  surfaces are valid; no output is universally required.
- Drainage input is bilinearly interpolated to avoid introducing cell-boundary snapping.
- Thresholds live in the recipe settings and every output is finite in `[0,1]`.
- No `EGatersEnvironment` family branch, material, asset, PCG graph, or runtime Actor.
- The three active Settlements-owned RED tests remain outside this machine.

### Task 1: RED contract

- [x] Declare settings, evidence, sample, recipe, issue, and compile-result contracts.
- [x] Require exact deterministic recipes and queries with complete source provenance.
- [x] Add pure causal fixtures for cliff/rock, valley/sediment/saturation, shore/sand,
  and cold snow/ice responses.
- [x] Add corrupt root/settings plus valid dry-scarcity counterexamples.
- [x] Build and observe RED only because the field implementation is absent.

### Task 2: Minimum field

- [x] Implement bounded pure evaluation over explicit physical evidence.
- [x] Sample root terrain slopes and local relief without Arrival/site materialization.
- [x] Bilinearly interpolate drainage accumulation, channel, and drop evidence.
- [x] Measure continuous proximity to exact global and regional water geometry.
- [x] Prove adjacent drainage-cell queries do not snap.

### Task 3: Held-out evidence and truth

- [x] Challenge seeds `11`, `29`, `47`, `83`, and `131` over a coordinate matrix.
- [x] Prove exact repeats, bounded outputs, valid source cells, and optional absence.
- [x] Run focused Surface/Environment/Drainage and broad suites; preserve unrelated RED.
- [x] Run registry/docs validators and scoped diff checks.
- [x] Update `world.surface-condition-field` and Primary status without claiming visual
  materials, final beaches, ice meshes, terrain carving, or resources.

Requirements checked: `WORLD-1`; exceptions: none.
