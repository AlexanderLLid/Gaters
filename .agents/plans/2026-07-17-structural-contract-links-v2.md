# Structural Contract Links v2 Implementation Plan

> **For agentic workers:** Execute inline with TDD. Do not branch, commit, or push unless the human asks.

**Goal:** Let recipes describe explicit piece-to-piece connections and reject missing nodes, contracts, ports, or misaligned connections without loading assets.

**Architecture:** Recipes gain canonical pure-data links. Structural evaluation optionally receives a read-only map of asset contracts; when supplied, it validates link endpoints and compares transformed contract ports. Existing recipe generation remains valid without contract-aware evaluation until generators emit links.

**Tech Stack:** Unreal Engine 5.8 C++, world recipes, asset contracts, structural evaluator, Unreal automation tests.

## Global Constraints

- Links are data, never Actor or socket references.
- Do not load meshes or query an Unreal world.
- Do not reinterpret route grouping as asset connectivity.
- Contract-aware failures must name the rule, link, and involved nodes.
- Overlap remains the next isolated slice after connection validity.
- Do not commit or branch.

### Task 1: Canonical recipe links

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersWorldRecipe.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersWorldRecipe.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersWorldRecipeTests.cpp`

- [x] Write a failing fixture proving links affect canonical identity and checksum.
- [x] Add the minimum stable link record and canonical serialization.
- [x] Verify the world-recipe suite.

### Task 2: Contract-aware connection evaluation

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersStructuralEvaluator.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersStructuralEvaluator.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersStructuralEvaluatorTests.cpp`

- [x] Write failing fixtures for duplicate link IDs, missing endpoint nodes, missing contracts, missing ports, and misaligned transformed ports.
- [x] Add an optional read-only contract context and connection rules.
- [x] Verify valid connected pieces pass and each broken fixture emits its intended stable rule.

### Task 3: Evidence

**Files:**
- Modify: `research/machines.json`
- Modify: `.agents/plans/2026-07-17-structural-contract-links-v2.md`

- [x] Run complete Gaters automation and workflow verifiers.
- [x] Record evaluator v2 and leave contract-bound overlap as the explicit promotion gap.
