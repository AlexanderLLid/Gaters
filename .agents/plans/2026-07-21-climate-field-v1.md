# Climate Field v1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:test-driven-development and superpowers:verification-before-completion. Execute inline in the shared checkout; project instructions prohibit creating a branch or commit unless the human asks.

**Goal:** Add one deterministic coordinate-level climate field that converts environment intent and terrain evidence into bounded temperature, precipitation, wind exposure, seasonality, and freeze-thaw signals without biome names, assets, sites, or required content.

**Architecture:** `FGatersClimateField` compiles a small versioned recipe from the accepted environment brief and landform recipe. A pure evidence evaluator owns climate math; a thin terrain adapter samples the existing environment height field. The field reuses the landform recipe's authoritative regional profile blend and remains isolated from `FGatersEnvironmentRecipe` until its contract is independently green.

**Tech Stack:** Unreal Engine 5.8 C++, Unreal Automation Tests, existing `FGatersEnvironmentBrief`, `FGatersLandformProcessField`, and `FGatersEnvironment` pure-data APIs.

## Global Constraints

- `WORLD-1 [MUST]`: declared scarcity is valid; climate emits physical evidence and never adds optional content.
- Generated recipes are authoritative; Unreal Actors and assets remain derived adapters.
- Candidate zero and existing landform/selector behavior must remain unchanged.
- No named biome or terrain-family checks in climate logic.
- No new third-party dependency, Actor, Blueprint, asset, site, raid, or character coupling.

---

### Task 1: Authoritative regional profile query

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersLandformProcessField.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersLandformProcessField.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersLandformProcessFieldTests.cpp`

**Interfaces:**
- Produces: `static FGatersEnvironmentTargetProfile QueryProfile(const FGatersLandformProcessRecipe&, const FVector2D&, float* OutRegionInfluence = nullptr);`

- [x] Add a failing contract assertion that global, regional-core, boundary, and repeated queries return the same profile blend already consumed by landform height queries.
- [x] Run `Gaters.Worldgen.LandformProcesses` and confirm the missing public API fails compilation.
- [x] Expose the existing private blend through `QueryProfile`; do not duplicate profile blending.
- [x] Re-run the focused contract and require `4/4` success.

### Task 2: Pure climate recipe and evidence evaluator

**Files:**
- Create: `Unreal/Prototype/Source/Prototype/Public/GatersClimateField.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersClimateField.cpp`
- Create: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersClimateFieldTests.cpp`

**Interfaces:**
- Produces: `FGatersClimateRecipe`, `FGatersClimateTerrainEvidence`, `FGatersClimateSample`, `FGatersClimateCompileResult`.
- Produces: `FGatersClimateField::Compile(const FGatersEnvironment&, const FGatersCompiledEnvironmentBrief&, const FGatersLandformProcessRecipe&)`.
- Produces: `FGatersClimateField::Evaluate(const FGatersClimateRecipe&, const FGatersEnvironmentTargetProfile&, const FGatersClimateTerrainEvidence&)`.
- Produces: `FGatersClimateField::Query(const FGatersClimateRecipe&, const FGatersEnvironment&, const FVector2D&)`.

- [x] Write a failing contract test for recipe version/provenance validation, exact repeated compilation/query, changed-seed wind variation, bounded grid samples, and causal invalid seed/version/world-size inputs.
- [x] Write a failing control test using synthetic terrain evidence: higher altitude lowers temperature; greater declared moisture raises precipitation; rising upwind terrain raises precipitation; lee terrain lowers it; exposed terrain raises wind; wet near-freezing conditions raise freeze-thaw; dry or warm evidence lowers it.
- [x] Write a failing regional test proving a compiled regional temperature/moisture profile changes only through the shared profile-query seam and blends continuously at its boundary.
- [x] Implement the minimum recipe compiler, pure evaluator, and environment adapter. Clamp every output to `[0,1]`; keep all calibration constants local to the climate implementation.
- [x] Run `Gaters.Worldgen.ClimateField` and require all three tests to pass.

### Task 3: Verification and machine truth

**Files:**
- Modify: `research/machines.json`
- Modify: `.agents/workstreams/Primary Builder — World & Terrain.md`

**Interfaces:**
- Consumes: focused climate and landform evidence.
- Produces: truthful `world.climate-field` active or verified status; no runtime integration claim.

- [x] Run `Gaters.Worldgen.LandformProcesses+Gaters.Worldgen.EnvironmentCandidateSelector+Gaters.Worldgen.ClimateField`.
- [x] Run the complete `Gaters` automation suite and record exact pass/fail counts.
- [x] Run `research/Test-MachineRegistry.ps1`, `research/Test-SharedAgentDocs.ps1`, and `git diff --check`.
- [x] Record the pure field evidence and explicitly leave environment-root/runtime consumption for the next integration gate.

## Self-review

- Spec coverage: deterministic physical signals, altitude, water/moisture, windward/leeward, regional intent, bounds, provenance, and scarcity are each owned by a test above.
- Placeholder scan: no implementation placeholder or unrelated subsystem is included.
- Type consistency: Task 2 consumes Task 1's `QueryProfile` and exposes the same climate types named in Task 3.
- Requirements checked: `WORLD-1`; exceptions: none.
