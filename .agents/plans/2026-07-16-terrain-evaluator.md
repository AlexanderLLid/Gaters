# Terrain Evaluator Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:executing-plans`. Subagents are intentionally not used because repository instructions do not authorize delegation.

**Goal:** Produce a deterministic, versioned terrain metric vector that an optimizer/LLM researcher can compare across generator versions and seeds.

**Architecture:** Add one pure evaluator beside `FGatersEnvironment`. It samples only the environment's public height/water/footprint contract and returns independent observations; `AGatersChunk` only reports those observations for the existing sweep archive.

**Tech Stack:** Unreal Engine 5.8 C++, Unreal Automation Tests, command-line seed sweep.

## Global Constraints

- Record a metric vector, never a single goodness score.
- Keep evaluator version independent from recipe schema and generator version.
- Preserve terrain generation, random draws, recipes, diffs, and actor materialization.
- Use explicit unknown/new evaluator versions rather than rewriting historical meaning.
- Do not branch, commit, or push; preserve unrelated human and Claude changes.

---

### Task 1: Pure Versioned Terrain Evaluation

**Files:**
- Create: `Unreal/Prototype/Source/Prototype/Public/GatersTerrainEvaluator.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersTerrainEvaluator.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersEnvironmentTests.cpp`

**Interfaces:**
- Produces: `FGatersTerrainEvaluation` and `FGatersTerrainEvaluator::Evaluate(const FGatersEnvironment&)`.
- Consumes: public `HeightAt`, `FootprintDrop`, `IsFootprintDry`, `WaterHeight`, and `ChunkSize`.

- [ ] **Step 1: Write failing evaluator tests**

  Assert evaluator version 1; identical environments produce identical metrics; fractions stay within `[0,1]`; relief/steps are non-negative; all four environment families yield buildable terrain; mountains exceed lowlands relief; archipelago has meaningful water coverage.

- [ ] **Step 2: Build to verify RED**

  Expected: compile failure because `GatersTerrainEvaluator.h` does not exist.

- [ ] **Step 3: Implement the minimum fixed-grid evaluator**

  Sample a deterministic square grid. Record minimum/maximum height, water fraction, mean/max neighbor step, and fraction of sampled centers satisfying the existing dry footprint/drop rule. Do not inspect private generator parameters.

- [ ] **Step 4: Build and run focused tests**

  Expected: all `Gaters.Worldgen.Environment` tests pass.

---

### Task 2: Sweep Evidence

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersChunk.cpp`
- Modify: `Unreal/Prototype/Scripts/RunEnvironmentSweep.ps1`

**Interfaces:**
- Consumes: current recipe/environment and `FGatersTerrainEvaluation`.
- Produces: `EVAL v=<n> relief=<cm> water=<ratio> rough=<cm> cliff=<cm> buildable=<ratio>`.

- [ ] **Step 1: Add the evaluation report**

  Evaluate after recipe/environment creation and log the fixed-order metric vector without changing generation.

- [ ] **Step 2: Include EVAL in sweep extraction**

  Extend only the existing log filter.

- [ ] **Step 3: Verify all worldgen tests and repeat seed sweep**

  Expected: all tests pass; duplicate seed runs produce identical RECIPE/EVAL lines; existing SITE/BASE/SCATTER/STAMP facts remain unchanged.

## Self-review

- The evaluator observes the generator through its public artifact contract.
- Metrics retain failure explanations and can be re-evaluated under later evaluator versions.
- Visual preference, traversal connectivity, base quality, and optimization remain later independent machines.
