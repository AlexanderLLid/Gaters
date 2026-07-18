# Structural Recipe Evaluator v1 Implementation Plan

> **For agentic workers:** Execute inline with TDD. Do not branch, commit, or push unless the human asks.

**Goal:** Give every structurally invalid world recipe a stable rule ID and causal diagnostic before Unreal creates actors.

**Architecture:** A pure `FGatersStructuralEvaluator` consumes a world recipe plus explicit limits and returns typed issues. `FGatersWorldRecipe::Validate` becomes a compatibility-shaped projection of those issues, leaving recipe identity separate from evaluation policy.

**Tech Stack:** Unreal Engine 5.8 C++, existing world-recipe types, Unreal automation tests.

## Global Constraints

- Add no actors, asset loading, terrain sampling, scoring, or automatic repair.
- Preserve all currently enforced recipe invariants.
- Diagnostics must carry a stable rule ID, involved node IDs, and measured/limit values when applicable.
- This slice covers representable identity, transform, cardinality, content-key, and node-budget rules.
- Keep the machine active until relationship, port, contract-bound, and overlap fixtures exist.
- Do not commit or branch.

### Task 1: Pure typed evaluator

**Files:**
- Create: `Unreal/Prototype/Source/Prototype/Public/GatersStructuralEvaluator.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersStructuralEvaluator.cpp`
- Create: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersStructuralEvaluatorTests.cpp`

- [x] Write failing fixtures for a valid recipe, duplicate IDs, invalid transforms, missing content keys, wrong Gate/Base cardinality, and node-budget overflow.
- [x] Build and run the focused suite; verify the evaluator contract is missing.
- [x] Implement the minimum pure evaluator and stable diagnostics.
- [x] Run the focused suite until it passes.

### Task 2: Recipe adapter

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersWorldRecipe.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersWorldRecipeTests.cpp`

- [x] Replace inline validation policy with evaluator delegation.
- [x] Preserve existing human-readable error strings through issue projection.
- [x] Run world-recipe and complete Gaters automation suites.

### Task 3: Evidence and registry

**Files:**
- Modify: `research/machines.json`
- Modify: `.agents/plans/2026-07-17-structural-recipe-evaluator-v1.md`

- [x] Record evaluator v1 as active with its implemented challenge set.
- [x] Validate the machine registry, gallery verifier, experiment archive, and diff whitespace.
- [x] Keep missing spatial rules visible as the next evaluator slice.
