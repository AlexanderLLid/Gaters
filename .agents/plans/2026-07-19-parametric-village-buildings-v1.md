# Parametric Village Buildings v1 Implementation Plan

> **For agentic workers:** use TDD. Do not branch, commit, or push; the human reviews
> the shared worktree first.

**Goal:** Deterministic village profiles produce varied low, mixed, and tall greybox
buildings assembled from the same stable wall-sized modules later available to players.

**Architecture:** The settlement generator owns terrain-aware building anchors and a
continuous structural profile. A pure building generator expands each accepted anchor
into a versioned module graph. An independent evaluator rejects invalid identity,
unsupported structure, missing entrances, overlap, and terrain-fit failures. The existing
World Recipe/compiler/materializer boundary renders modules as native primitive batches.

**Tech stack:** Unreal Engine 5.8 C++, automation tests, World Recipe, ISM primitives.

## Global constraints

- No finished art, Blender dependency, interiors, NPC behavior, or economy.
- No fixed whole-house catalogue and no microscopic persistent pieces.
- Paths remain semantic and are not materially rendered in this slice.
- Module IDs and contracts are authoritative; visual boxes are replaceable outputs.
- Generated and future player-authored structures use the same module/evaluator types.

### Task 1: Village structural profile

**Files:**

- Test first: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersSettlementTests.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersSettlementGenerator.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersSettlementGenerator.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersSettlementEvaluator.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersSettlementEvaluator.cpp`

**Interfaces:**

- `FGatersSettlementProfile` exposes normalized `HeightBias`, `FootprintBias`, and
  `DensityBias` plus deterministic canonical text.
- Each `FGatersSettlementBuilding` records footprint width/depth cells and floor count.
- The evaluator records distinct height and footprint buckets without grading art.

- [x] Add tests proving same-seed determinism, changed-seed structural variation, low
  and tall held-out outcomes, bounded footprints/floors, and evaluator diagnostics.
- [x] Build and require RED because the profile and building dimensions do not exist.
- [x] Generate continuous profile values from the seed and derive bounded per-building
  massing while retaining the existing role and reachability contract.
- [x] Run `Gaters.Worldgen.Settlement` and preserve all counterexample classifications.

### Task 2: Pure modular building assembly

**Files:**

- Create test first:
  `Unreal/Prototype/Source/Prototype/Private/Tests/GatersBuildingAssemblyTests.cpp`
- Create after RED: `Unreal/Prototype/Source/Prototype/Public/GatersBuildingGenerator.h`
- Create after RED: `Unreal/Prototype/Source/Prototype/Private/GatersBuildingGenerator.cpp`
- Create after RED: `Unreal/Prototype/Source/Prototype/Public/GatersBuildingEvaluator.h`
- Create after RED: `Unreal/Prototype/Source/Prototype/Private/GatersBuildingEvaluator.cpp`

**Interfaces:**

- `FGatersBuildingGenerator::Generate(Field, SettlementBuilding)` produces foundation,
  floor, wall, door-wall, and roof modules with stable IDs and world transforms.
- `FGatersBuildingEvaluator::Evaluate(Field, Assembly)` independently reports identity,
  entrance, support, overlap, footprint, and terrain-contact issues.

- [x] Add determinism, module-kind coverage, multi-floor, different-footprint, missing
  entrance, duplicate ID, unsupported module, overlap, blocked-anchor, and buried
  foundation tests.
- [x] Build and require RED because building assembly interfaces do not exist.
- [x] Implement the minimum rectangular perimeter grammar and independent evaluator.
- [x] Run `Gaters.Worldgen.Buildings` until positive and negative fixtures classify.

### Task 3: World Recipe and primitive materialization

**Files:**

- Test first: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersWorldRecipeTests.cpp`
- Test first:
  `Unreal/Prototype/Source/Prototype/Private/Tests/GatersVisualMaterializerTests.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersWorldRecipe.h`
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersVisualMaterializer.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersVisualMaterializer.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersChunk.cpp`

**Interfaces:**

- Recipe kind `SettlementModule` carries stable module ID, semantic content key,
  transform, and positive scale; schema identity advances in place.
- The visual plan batches foundations/floors, walls/door frames, and roofs as replaceable
  cube instances. `SettlementPath` remains recipe data without a visible batch.

- [x] Add RED tests for module identity, compiler preservation, three primitive batches,
  door-frame expansion, and no forced path rendering.
- [x] Adapt accepted assemblies into World Recipe module nodes.
- [x] Replace whole-building cubes with module primitive batches and preserve ISM use.
- [x] Run focused World Recipe/compiler/materializer/building/settlement tests.

### Task 4: Held-out evidence and current truth

**Files:**

- Create: `.agents/reports/parametric-village-buildings-v1.md`
- Modify: `.agents/workstreams/builder.md`
- Modify only with verified evidence: `research/machines.json`

- [x] Run held-out seeds across terrain families and retain massing diversity descriptors.
- [x] Run a headless runtime sweep and capture standardized gallery images when possible.
- [x] Run full `Gaters` automation, registry checks, shared-doc checks, and
  `git diff --check`.
- [x] Record structural and primitive-rendering guarantees only; do not claim finished
  art, naturalness, interiors, activities, or player placement UI.

## Self-review

- The hierarchy supports varied buildings without fixed house templates.
- Runtime identity stops at meaningful modules; decorative micro-pieces remain optional
  offline generation detail.
- The generator and evaluator remain independent.
- Existing terrain, navigation, catalog, compiler, and ISM machinery is reused.
