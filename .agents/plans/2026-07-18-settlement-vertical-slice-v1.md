# Settlement Vertical Slice v1 Implementation Plan

> **For agentic workers:** use TDD. Do not commit, branch, or push; the human reviews
> the shared worktree first.

**Goal:** Prove that varied terrain can deterministically produce a small coherent,
reachable village recipe that materializes as visible Unreal placeholders.

**Architecture:** A pure settlement generator consumes the verified terrain semantic
field and planned village site. A separate evaluator rejects missing roles, duplicate
placements, and unreachable entrances. `AGatersChunk` adapts accepted plans into world
recipe nodes; the existing compiler and visual materializer remain the only Unreal
materialization boundary.

**Why now:** This is the earliest unresolved feasibility risk on the generated-world
critical path. It unlocks settlement evaluation, building-asset replacement, population,
and the playable-world assembler. Motion work is preserved but does not block this slice.

## Constraints

- Use semantic placeholders; finished building assets remain outside this slice.
- Generate six buildings: three homes plus community, workshop, and storage roles.
- Entrances and paths must remain walkable and connected to the village center.
- Keep generation and evaluation independent and deterministic.
- Do not add NPC behavior, interiors, decoration, economy, or production networking.

### Task 1: Pure settlement recipe and evaluator

**Files:**

- Test first: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersSettlementTests.cpp`
- Create after RED: `Unreal/Prototype/Source/Prototype/Public/GatersSettlementGenerator.h`
- Create after RED: `Unreal/Prototype/Source/Prototype/Private/GatersSettlementGenerator.cpp`
- Create after RED: `Unreal/Prototype/Source/Prototype/Public/GatersSettlementEvaluator.h`
- Create after RED: `Unreal/Prototype/Source/Prototype/Private/GatersSettlementEvaluator.cpp`

**Interfaces:**

- Consumes: `FGatersTerrainSemanticField`, seed, and `FGatersPlannedSite` village site.
- Produces: versioned buildings, role keys, entrance cells, path cells, diagnostics, and
  a deterministic canonical layout string.
- Evaluator produces named causal issues plus role, reachability, path, and variety
  evidence without modifying the candidate.

- [x] Write open-field determinism, role coverage, path reachability, scarce-field
  rejection, corrupted-entrance rejection, and real-family seed tests.
- [x] Build and require RED because the settlement interfaces do not exist.
- [x] Implement the minimum deterministic candidate selection and independent checks.
- [x] Rebuild and run `Gaters.Worldgen.Settlement`; require the positive and negative
  fixtures to classify for their intended reasons.

### Task 2: Recipe adapter and placeholder materialization

**Files:**

- Modify test first: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersVisualMaterializerTests.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersWorldRecipe.h`
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersVisualMaterializer.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersVisualMaterializer.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersChunk.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersChunk.cpp`

**Interfaces:**

- Produces recipe node kinds `SettlementBuilding` and `SettlementPath` with stable IDs,
  role content keys, ground anchors, and deterministic transforms.
- Visual plan batches buildings, paths, and the existing village site using native Unreal
  ISM primitives without changing gameplay identity.

- [x] Require settlement nodes to enter distinct native visual batches with exact
  compiled transforms and stable IDs.
- [x] Run focused tests and require RED because settlement batches do not exist.
- [x] Adapt accepted settlement plans into recipe nodes and extend the visual plan with
  cube buildings, flat path blocks, and a public-space marker.
- [x] Run focused world compiler, settlement, and visual-materializer automation.

### Task 3: Evidence and current truth

**Files:**

- Create: `.agents/reports/settlement-vertical-slice-v1.md`
- Modify: `research/machines.json`
- Modify: `.agents/workstreams/builder.md`

- [x] Run representative seed evidence across terrain families.
- [x] Record only proven layout, evaluator, and placeholder guarantees.
- [x] Update the registry without claiming finished assets, NPC activity, interiors, or
  perceptual village quality.
- [x] Run full `Gaters` automation, registry/shared-doc validators, and `git diff --check`.

## Self-review

- The slice tests the village-generation unknown without waiting for production art.
- The generator does not grade itself.
- Runtime activity and final building assets remain independent later machines.
