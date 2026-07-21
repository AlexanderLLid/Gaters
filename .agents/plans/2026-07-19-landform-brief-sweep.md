# Landform Brief Sweep Implementation Plan

> **For agentic workers:** Execute inline with test-first checkpoints. Do not branch,
> commit, or push unless the human asks.

**Goal:** Determine whether physical landform requests create distinct, controllable,
playable terrain across held-out seeds and preserve reproducible failure artifacts.

**Architecture:** A pure paired-response evaluator compares two height fields without
reading generator contributions. The runtime preview accepts normalized physical signals
and the existing environment sweep supplies those values, captures metrics and images,
and appends existing versioned run records. A thin orchestration script defines challenge
fixtures as test inputs, never as generator branches.

**Tech stack:** Unreal Engine 5.8 C++, Unreal Automation Tests, PowerShell, existing
`FGatersTerrainEvaluator`, traversal/performance evaluation, gallery capture, and run
archive.

## Global constraints

- Existing terrain remains the default champion.
- Challenges are physical signal tuples, not named world-generator implementations.
- Built Sites remain optional and disabled for terrain evaluation.
- Sites, settlements, Rifts, encounters, characters, and assets are not inputs.
- Every generated artifact records seed, signal tuple, machine versions, metrics, and
  source paths.

## Rejected approaches

- Start climate now: adds breadth while leaving the visually subtle landform layer
  unproven.
- Hand-amplify seed `7`: produces a screenshot but no reusable evidence.
- Let the process field score its own contribution channels: creates a self-judge.

---

### Task 1: Independent paired-response evaluator

**Files:**

- Create `Unreal/Prototype/Source/Prototype/Public/GatersLandformResponseEvaluator.h`.
- Create `Unreal/Prototype/Source/Prototype/Private/GatersLandformResponseEvaluator.cpp`.
- Create
  `Unreal/Prototype/Source/Prototype/Private/Tests/GatersLandformResponseEvaluatorTests.cpp`.

**Interfaces:**

- Consumes one `const FGatersEnvironment&` metric context, two
  `TFunctionRef<float(const FVector2D&)>` height queries, and an evaluation size.
- Produces `FGatersLandformResponseEvaluation` containing finite status, RMS height
  difference, positive/negative change fractions, relief delta, maximum-height delta,
  and mean-height delta.
- Does not consume `FGatersLandformProcessSample` or contribution fields.

- [ ] Write tests proving identical fields have zero response, uplift produces positive
  height/relief response, glacial carving produces negative coverage, and non-finite
  samples return a causal diagnostic.
- [ ] Build and record the expected missing-header RED.
- [ ] Implement one fixed `33 x 33` comparison grid and delegate generic terrain metrics
  to `FGatersTerrainEvaluator::EvaluateHeightField`.
- [ ] Build and run `Gaters.Worldgen.LandformResponse`.

### Task 2: Generic runtime physical-signal overrides

**Files:**

- Modify `Unreal/Prototype/Source/Prototype/Public/GatersChunk.h`.
- Modify `Unreal/Prototype/Source/Prototype/Private/GatersChunk.cpp`.
- Modify `Unreal/Prototype/Source/Prototype/Private/GatersTestSpawner.cpp`.
- Modify
  `Unreal/Prototype/Source/Prototype/Private/Tests/GatersLandformProcessFieldTests.cpp`.

**Interfaces:**

- Command-line inputs:
  `-GatersLandformRelief=<0..1>`, `-GatersLandformVolcanism=<0..1>`, and
  `-GatersLandformIce=<0..1>`.
- Negative or absent values retain seed-selected brief ranges.
- Valid override values replace only the matching global signal range with a fixed value.
- Runtime logs one `LANDFORM` evidence line containing enabled state and resolved values.

- [ ] Add a failing compiler/runtime-profile test for fixed values and bounded rejection.
- [ ] Add three `-1` default preview properties to `AGatersChunk`; clamp accepted command
  inputs and pass them from `UGatersTestSpawner`.
- [ ] In `RollSite`, fix only explicitly supplied ranges before compiling the brief.
- [ ] Add the `LANDFORM v=1 enabled=<yes|no> relief=<n> volcanism=<n> ice=<n>` evidence
  line without changing the default-off path.
- [ ] Build and run landform plus runtime command tests.

### Task 3: Reusable sweep and archive harness

**Files:**

- Modify `Unreal/Prototype/Scripts/RunEnvironmentSweep.ps1`.
- Modify `Unreal/Prototype/Scripts/Write-EnvironmentRun.ps1`.
- Create `Unreal/Prototype/Scripts/RunLandformBriefSweep.ps1`.
- Create `Unreal/Prototype/Scripts/Test-LandformBriefSweep.ps1`.

**Interfaces:**

- `RunEnvironmentSweep.ps1` gains normalized optional relief, volcanism, ice, and a safe
  artifact label; it forwards signal values and prevents filename collisions.
- `Write-EnvironmentRun.ps1` records the parsed `LANDFORM` evidence under input intent.
- `RunLandformBriefSweep.ps1` runs paired physical fixtures over held-out seeds
  `11, 29, 47, 83, 131`, with Built Sites disabled and archives JSONL plus gallery paths.
- Fixture tuples: baseline `(0,0,0)`, high relief `(1,0,0)`, volcanic `(0.45,1,0)`,
  glacial `(0.8,0,1)`. Names label experiments only.

- [ ] Write PowerShell checks that reject duplicate artifact paths, missing `LANDFORM`
  evidence, mismatched seed/signal provenance, absent paired images, and malformed JSONL.
- [ ] Run the checks and record the expected missing-script/parameter RED.
- [ ] Implement the smallest parameter forwarding, archive extension, and sequential
  orchestrator using existing scripts.
- [ ] Parse every script with the PowerShell AST and run `Test-LandformBriefSweep.ps1`.

### Task 4: Held-out evidence and truthful machine state

**Files:**

- Modify `research/machines.json` only after fresh evidence.
- Modify `.agents/workstreams/Primary Builder — World & Terrain.md`.

- [ ] Build `PrototypeEditor Win64 Development` and run focused automation.
- [ ] Run the complete `Gaters` suite.
- [ ] Run the held-out sweep first without galleries; preserve every run record.
- [ ] Capture paired galleries for the smallest separating and failing cases.
- [ ] Reject promotion if challenge responses overlap, traversal regresses materially,
  any critical route fails, output is non-finite, or performance evidence is invalid.
- [ ] Update the registry with observed facts only; keep the challenger active unless the
  full promotion gate actually passes.
- [ ] Run machine/shared-doc validators and scoped `git diff --check`.
- [ ] Obtain independent read-only review.

Requirements checked: generated content boundary; no canon requirement applies.
Exceptions: no commit because `AGENTS.md` requires explicit human authorization.
