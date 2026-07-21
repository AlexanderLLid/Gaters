# Environment Brief Compiler v1

## Outcome

- A seed plus bounded global and regional physical requests compiles into deterministic
  pure-data environmental targets.
- The contract cannot choose sites, Anchors, Rifts, encounters, species, assets, or
  final biome names.
- Existing terrain remains the champion. This machine states what a later environment
  must satisfy; it does not claim the current generator already satisfies the brief.

## Approved design

- Each profile contains normalized ranges for relief, temperature, moisture, surface
  water, volcanism, ice, vegetation, and exposed rock.
- A regional request adds a stable ID, normalized center, bounded radius fraction, and
  its own profile. This is enough to express mixed worlds without separate named
  generators.
- Compilation samples every range deterministically from the seed. Fixed ranges remain
  exact; changed seeds vary values only where the caller allowed variation.
- Validation reports causal rule IDs for invalid versions, ranges, duplicate identity,
  non-finite geometry, and regions outside the bounded world.
- Arrival remains an independent neutral site contract supplied by
  `world.site-route-planner`; no Anchor-centered topology enters this compiler.

## Rejected approaches

- Named presets such as `GenerateGlacialWorld`: easy initially, but creates one generator
  branch per theme and prevents mixed physical causes.
- A general rule graph: expressive, but unnecessary before eight bounded signals prove
  the contract.

## Implementation

### Task 1 — Contract and RED evidence

Files:

- Create `Unreal/Prototype/Source/Prototype/Public/GatersEnvironmentBrief.h`.
- Create `Unreal/Prototype/Source/Prototype/Private/GatersEnvironmentBrief.cpp`.
- Create
  `Unreal/Prototype/Source/Prototype/Private/Tests/GatersEnvironmentBriefTests.cpp`.

- [x] Add contract tests for exact same-seed output, changed-seed variation within
  caller ranges, exact fixed values, two-region mixed output, bounded geometry, and an
  empty list of diagnostics for valid input.
- [x] Add counterexamples for reversed/out-of-range/non-finite ranges, duplicate region
  IDs, invalid region radius, and a region escaping world bounds. Require stable causal
  rule IDs.
- [x] Build before production implementation and record the expected missing-header RED.

### Task 2 — Minimum compiler

- [x] Implement `FGatersEnvironmentBriefCompiler::Compile` as a pure deterministic
  adapter with no Unreal objects, named environment presets, or downstream decisions.
- [x] Keep hashing local and deterministic; do not depend on mutable random streams.
- [x] Build and run `Gaters.Worldgen.EnvironmentBrief`.

### Task 3 — Truth and regression

- [x] Run the complete `Gaters` automation suite.
- [x] Update `world.environment-brief-compiler` and the Primary Builder status with only
  fresh evidence. Keep `world.environment-generator` active.
- [x] Run `research/Test-MachineRegistry.ps1`,
  `research/Test-SharedAgentDocs.ps1`, and scoped `git diff --check`.
- [x] Obtain independent read-only review before close.

Requirements checked: none applicable; exceptions: none.
