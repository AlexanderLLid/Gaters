# Village Growth v1 Implementation Plan

> **For agentic workers:** execute inline with TDD. Do not branch, commit, or push;
> the human reviews the shared worktree first.

**Goal:** The same deterministic village expands through bounded stages without moving,
renaming, or removing any earlier building or path identity.

**Architecture:** The settlement generator produces a stable stage prefix. A pure growth
planner derives an append-only patch from an accepted plan, and an independent evaluator
proves preservation and validates the expanded plan. World Recipe remains authoritative;
Unreal rendering and future PCG graphs only materialize stable semantic nodes.

Land is the first habitat adapter, not a permanent assumption in the village contract:
habitat-specific placement produces semantic parcels with support/access keys, while
roles, identities, growth history, patches, and publication remain habitat-independent.

**Tech Stack:** Unreal Engine 5.8 C++, automation tests, World Recipe, existing ISM
materializer.

## Global Constraints

- Growth stages are bounded integers `0..2`; stage zero is the existing six-building
  village.
- Existing building records and path identities are byte-stable at every later stage.
- Expansion adds records only; player changes remain separate recipe diffs.
- No NPC simulation, economy, final assets, or authoritative PCG graph.
- PCG, Smart Objects, StateTree, and Mass remain replaceable Unreal adapters.
- Growth candidates are planned and evaluated in the background, then published only
  while the settlement region is unloaded.
- V1 accepts ground-supported land parcels only; future tree, cliff, cave, stilt, and
  floating adapters may change placement, navigation, fit, and building grammar without
  changing the village-growth protocol.

### Task 1: Stage-stable settlement generation

**Files:**

- Test: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersSettlementTests.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersSettlementGenerator.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersSettlementGenerator.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersSettlementEvaluator.cpp`

**Interfaces:**

- `FGatersSettlementGenerator::Generate(Field, Seed, Site, GrowthStage)` produces a full
  deterministic stage plan.
- `FGatersSettlementPlan::StablePathId(Cell)` converts path coordinates to identity
  without sort-index coupling.
- `FGatersSettlementParcel` carries stable support/access semantics independently of the
  mesh or PCG graph that will later materialize it.

- [x] Write tests asserting stage-zero compatibility, deterministic stages, strict
  building-prefix preservation, path-set preservation, explicit land parcels, immutable
  growth fronts, increasing building count, and invalid-stage rejection.
- [x] Run `Gaters.Worldgen.Settlement.GrowthContract` and require RED because growth
  stages do not exist.
- [x] Add `GrowthStage`, stage-owned role entries, and stage-owned search radii so later
  candidates cannot alter earlier placement.
- [x] Make settlement evaluation derive expected counts and radius from the stage.
- [x] Run all settlement tests until GREEN.

### Task 2: Pure expansion patch and independent evaluator

**Files:**

- Create: `Unreal/Prototype/Source/Prototype/Public/GatersSettlementGrowth.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersSettlementGrowth.cpp`
- Create: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersSettlementGrowthTests.cpp`

**Interfaces:**

- `FGatersSettlementGrowthPlanner::Plan(Field, Seed, Site, Existing, TargetStage)` returns
  only added buildings and path cells plus the expected expanded plan.
- `FGatersSettlementGrowthEvaluator::Evaluate(Field, Existing, Patch)` independently
  rejects modified prefixes, duplicate identities, invalid stages, and invalid expanded
  settlements.

- [x] Write contract and counterexample tests for deterministic patching, unchanged
  prefix, wrong source stage, tampered existing building, duplicate addition, and
  disconnected expansion.
- [x] Build and require RED because growth planner/evaluator interfaces do not exist.
- [x] Implement the minimum diff-and-apply planner and causal evaluator.
- [x] Run `Gaters.Worldgen.SettlementGrowth` until positive and negative cases classify.

### Task 3: Authoritative recipe integration

**Files:**

- Test: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersWorldRecipeTests.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersChunk.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersChunk.h`

**Interfaces:**

- `AGatersChunk::VillageGrowthStage` selects the generated stage.
- Building module IDs remain derived from stable building IDs.
- Settlement path node IDs use `StablePathId(Cell)` rather than array indices.

- [x] Write RED tests showing existing recipe node IDs remain a subset of every later
  stage and path insertion cannot rename an existing path.
- [x] Integrate the accepted expanded plan through the existing building generator,
  evaluator, World Recipe, and ISM materializer.
- [x] Report stage and total building/module/path counts without making PCG state
  authoritative.
- [x] Run recipe, compiler, materializer, building, and settlement suites.

### Task 4: Runtime selection and held-out evidence

**Files:**

- Test: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersSeedCommandTests.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersTestSpawner.cpp`
- Create: `.agents/reports/village-growth-v1.md`
- Modify: `.agents/workstreams/builder.md`
- Modify with verified evidence only: `research/machines.json`

**Interfaces:**

- `Gaters.VillageStage <0|1|2>` persists the requested stage and reloads the current map.
- `-GatersVillageStage=N` supports headless sweeps.

- [x] Add a RED command-registration test.
- [x] Add the minimum persisted-stage command and command-line adapter.
- [x] Sweep stages on contrasting terrain seeds and verify prefix preservation,
  structural validity, performance, and determinism.
- [x] Run full `Gaters` automation, registry tests, shared-doc tests, and
  `git diff --check`.
- [x] Record only observed guarantees and limitations; promote registry machines only
  where fresh evidence satisfies their contracts.

## Self-review

- Every mutable Unreal output is downstream from pure authoritative data.
- Growth planning and growth evaluation are separate implementations.
- Stable paths no longer depend on array order.
- The stage ceiling is deliberate v1 scope, not a final town-size limit.
