# Root-Aware Biome Opportunities v2 Implementation Plan

> **For agentic workers:** Execute inline with red-green verification after the shared
> Unreal runner is free. Do not create a branch, commit, add assets, place resources, or
> edit active Settlements work.

**Goal:** Turn accepted biome, climate, and surface-condition evidence into one
versioned neutral opportunity recipe and deterministic coordinate query.

**Architecture:** Adapt the existing `FGatersBiomeOpportunityField`; do not create a
parallel environment field. Its pure evaluator consumes explicit biome, climate, and
surface samples. Environment Recipe stores only the field recipe/provenance and delegates
queries. Content-cell density, resource kinds, assets, placement and rendering remain
later consumers.

**Tech stack:** Unreal Engine 5.8 C++ and existing Unreal Automation.

## Global constraints

- `WORLD-1`: zero optional vegetation, stone or landmark opportunity is valid scarcity.
- No terrain-family branch, asset key, item/reward, site, species, Actor or placement.
- Every output is finite in `[0,1]` and exact for identical evidence.
- Water is physical exclusion evidence, not a named world-generator branch.
- Preserve source recipe versions so optimizer evidence remains comparable over time.

### Task 1: RED physical opportunity contract

**Files:**

- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersBiomeOpportunityFieldTests.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersEnvironmentRecipeTests.cpp`

**Interfaces:**

- Consume: `FGatersBiomeSample`, `FGatersClimateSample`,
  `FGatersSurfaceConditionSample`.
- Require: `FGatersBiomeOpportunityField::Evaluate(...)`, a versioned recipe compiler,
  and exact `FGatersEnvironmentRecipe::QueryOpportunities(...)` parity.

- [x] Replace named woodland/alpine fixtures with explicit warm-soil, exposed-rock,
  frozen, saturated, cliff, shore and water evidence.
- [x] Require warm moist soil to increase vegetation; exposed rock/cliff to increase
  stone and landmark opportunity; snow/ice/saturation/cliff to increase travel friction;
  water to emit no land vegetation or stone; dry barren evidence to remain valid.
- [x] Require exact root recipe/query repetition and causal rejection of corrupt
  opportunity provenance.
- [x] Build and observe RED only because the v2 recipe/evaluator is absent.

### Task 2: Minimum v2 field

**Files:**

- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersBiomeOpportunityField.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersBiomeOpportunityField.cpp`

**Interfaces:**

- Produce: one versioned provenance recipe and four neutral weights: `Vegetation`,
  `Stone`, `Landmark`, `TravelFriction`.

- [x] Add recipe and compile-result contracts containing field version, seed, world size,
  Environment Recipe version, Climate version and Surface Condition version.
- [x] Implement one bounded pure evaluator using only the supplied physical samples.
- [x] Keep water/dry scarcity valid and avoid content-specific thresholds or identities.
- [x] Run the focused field test and observe GREEN.

### Task 3: Environment Recipe v5 composition

**Files:**

- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersEnvironmentRecipe.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersEnvironmentRecipe.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersEnvironmentRecipeTests.cpp`

**Interfaces:**

- Store: accepted `FGatersBiomeOpportunityRecipe`.
- Query: biome, climate and surface from the same root, then delegate to the field.

- [x] Compile the opportunity recipe after Surface Conditions and bump root/compiler to
  version `5`.
- [x] Independently recompile and compare it during root validation.
- [x] Prove direct-source parity, held-out determinism and causal corruption diagnostics.
- [x] Run focused Environment Recipe and opportunity automation.

### Task 4: Evidence and truth

- [x] Run Worldgen and complete Gaters automation; preserve exact unrelated failures.
- [x] Update `world.biome-opportunity-field`, `world.environment-recipe-compiler`, the
  Primary status and machine validators.
- [x] Do not promote `world.biome-resource-generator`: content placement and visual
  differentiation are still unverified.

Requirements checked: `WORLD-1`; exceptions: none.
