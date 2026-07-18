# World Compiler v1 Implementation Plan

> **For agentic workers:** Execute inline with TDD. Do not branch, commit, or push unless the human asks.

**Goal:** Compile a World Recipe and content catalog into one deterministic materialization manifest without changing semantic identity.

**Architecture:** A pure `FGatersWorldCompiler` copies every recipe node/link into a compiled manifest, resolves asset-bearing node kinds through the catalog, chooses an Unreal representation from the selected contract, and emits typed diagnostics. Actual Actor/component creation remains the existing placeholder materializer until the manifest contract is proven.

**Tech Stack:** Unreal Engine 5.8 C++, world recipes, content catalog, asset contracts, Unreal automation tests.

## Global Constraints

- Preserve recipe checksum, node IDs, links, and transforms exactly.
- Only BasePiece and scatter kinds query assets in v1; route ContentKey remains semantic route grouping.
- Missing required BasePiece content is an error; missing optional scatter content retains a placeholder warning.
- Persistent state always selects a unique Actor representation.
- Do not commit or branch.

### Task 1: Pure compiler manifest

**Files:**
- Create: `Unreal/Prototype/Source/Prototype/Public/GatersWorldCompiler.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersWorldCompiler.cpp`
- Create: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersWorldCompilerTests.cpp`

- [x] Add failing fixtures for identity preservation, instanced/unique/persistent selection, missing required/optional content, and catalog swaps.
- [x] Implement the minimum compiler and typed diagnostics.
- [x] Run focused tests.

### Task 2: Evidence

**Files:**
- Modify: `research/machines.json`
- Modify: `.agents/plans/2026-07-17-world-compiler-v1.md`

- [x] Record compiler v1 as active and keep runtime Actor/component consumption explicit.
- [x] Run complete Gaters and workflow verifiers.
