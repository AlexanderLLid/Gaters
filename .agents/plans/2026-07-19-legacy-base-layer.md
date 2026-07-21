# Pure Legacy Base Layer Implementation Plan

> **For agentic workers:** execute inline with TDD. Do not branch, stage, commit, or push.

**Goal:** Extract the existing hut/compound/tower topology decision behind a deterministic
pure recipe layer without editing shared runtime integration.

**Architecture:** The generator consumes a base center, copied `FRandomStream` state,
foundation-drop limit, explicit module contracts, stable input source IDs, and a pure
terrain-height callback. It emits only stable physical piece recipes, used content
requirements, counts, and causal diagnostics. Primary later adapts content keys to EBS
classes/meshes and composes the pieces into the shared World Recipe.

**Tech stack:** Unreal Core C++, existing `FGatersAssetContract`, automation tests.

## Global constraints

- Preserve the current random draw order and hut/compound/tower topology.
- Never load classes, meshes, assets, worlds, or Actors in the generator.
- Emit no loot, raid objective, tactical label, or unproved placement slot.
- Do not edit `AGatersChunk`, shared catalog integration, spawning, streaming, or diffs.
- Requirements checked: Global (none recorded); subject requirements: none; exceptions:
  none. The generated-content and Raids ownership boundaries still apply as repository
  conventions.

---

### Task 1: Pure contract and validation

**Files:**

- Create: `Unreal/Prototype/Source/Prototype/Public/GatersLegacyBaseLayer.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersLegacyBaseLayer.cpp`
- Create: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersLegacyBaseLayerTests.cpp`

**Interfaces:**

- `FGatersLegacyBaseModuleDefinition`: one available/unavailable module backed by an
  explicit `FGatersAssetContract`.
- `FGatersLegacyBaseTierDefinition`: foundation, wall, door frame, optional window,
  ceiling, and optional fence definitions.
- `FGatersLegacyBaseLayerInput`: base center, random state, cell size,
  maximum foundation drop, three tiers, door definition, and stable source IDs.
- `FGatersLegacyBasePieceRecipe`: ID, transform, content key, and source IDs.
- `FGatersLegacyBaseLayerResult`: versions, archetype, main dimensions/tier, building
  count, pieces, used content requirements, and diagnostics.
- `FGatersLegacyBaseLayer::Generate(Input, HeightAt)`.

- [x] Write a failing contract test with three valid replaceable tiers and a flat
  deterministic terrain query.
- [x] Assert invalid versions, missing required modules, invalid bounds, duplicate
  content identities, non-finite inputs, and empty provenance produce causal diagnostics
  and no pieces.
- [x] Build to confirm RED because `GatersLegacyBaseLayer.h` is missing.
- [x] Add the minimum data types and pre-generation validation.
- [x] Rebuild and run `Gaters.BuiltSites.LegacyBase.Contract`; confirm GREEN.

### Task 2: Deterministic topology extraction

**Files:**

- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersLegacyBaseLayer.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersLegacyBaseLayerTests.cpp`

- [x] Add failing tests across copied random states that require all three archetypes,
  repeat-identical piece facts, stable `piece:<index>` ordering, used-requirement
  deduplication, terrain-following foundations/fences, and no loot or tactical facts.
- [x] Add counterexamples proving a changed random state changes topology and excessive
  main-footprint drop rejects the layer without partial output.
- [x] Implement the current random draw sequence and physical transforms using only
  module contract bounds and the supplied height query.
- [x] Run focused tests; confirm GREEN.

### Task 3: Independent evidence and Primary handoff

**Files:**

- Create: `.agents/exchanges/SITE-5-pure-legacy-base-layer-integration.md`
- Modify: `.agents/workstreams/Settlements, Bases & Dungeons.md`
- Modify: `.agents/plans/2026-07-19-legacy-base-layer.md`

- [x] Confirm Unreal Editor is closed before build or automation.
- [x] Run focused legacy-base tests, all Built Sites tests, and the complete Gaters suite.
- [x] Run `research/Test-SharedAgentDocs.ps1`, `research/Test-MachineRegistry.ps1`, and
  `git diff --check`.
- [x] Raise an `INTEGRATE` exchange to Primary containing the exact `AGatersChunk`,
  catalog, and materialization adapter work still required; do not perform those edits.
- [x] Update the owned workstream evidence and closeout packet links.
