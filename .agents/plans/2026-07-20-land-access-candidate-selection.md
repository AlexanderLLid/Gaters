# Land-access Candidate Selection Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Deterministically select a landform-process candidate that satisfies
seed-declared walkable-land and connected-land targets without requiring optional sites.

**Architecture:** Extend the existing environment brief and Traversability Evaluator,
then add one pure selector over deterministic landform variants. Runtime integration is a
thin adapter and keeps the current terrain as champion when no candidate satisfies.

**Tech Stack:** Unreal Engine 5.8 C++, Unreal Automation, PowerShell evidence archive.

## Global Constraints

- `WORLD-1`: intentional scarcity is valid and optional content is never guaranteed.
- Versioned recipes and source evidence are authoritative; Actors are derived adapters.
- `Unreal Runner` alone executes Unreal build, Automation, and commandlet commands.
- Do not branch, commit, or push in this shared checkout.

---

### Task 1: Land-access request and measurement

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersEnvironmentBrief.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersEnvironmentBrief.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersTraversabilityEvaluator.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersTraversabilityEvaluator.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersEnvironmentBriefTests.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersTerrainNavigationTests.cpp`

**Interfaces:**
- Produces: `FGatersLandAccessTargetRanges`, `FGatersLandAccessTargetProfile`,
  `FGatersCompiledEnvironmentBrief::LandAccess`, and
  `FGatersTraversabilityEvaluation::WalkableFraction`.
- Preserves: `ReachableFraction` as connected walkable cells divided by all walkable
  cells and `bEscapesStart` as an independent hard fact.

- [ ] **Step 1: Write failing brief tests**

```cpp
TestEqual(TEXT("same seed preserves land-access target"),
    A.Intent.LandAccess, B.Intent.LandAccess);
TestTrue(TEXT("walkable target is bounded"),
    A.Intent.LandAccess.WalkableLand >= 0.f
        && A.Intent.LandAccess.WalkableLand <= 1.f);
TestTrue(TEXT("connected target is bounded"),
    A.Intent.LandAccess.ConnectedLand >= 0.f
        && A.Intent.LandAccess.ConnectedLand <= 1.f);
```

- [ ] **Step 2: Write failing traversal metric tests**

```cpp
TestEqual(TEXT("open field exposes all cells as walkable"),
    OpenResult.WalkableFraction, 1.f);
TestEqual(TEXT("water barrier leaves four fifths of the field walkable"),
    BarrierResult.WalkableFraction, 0.8f);
```

- [ ] **Step 3: Ask Unreal Runner to prove RED**

Run through Runner:
`UnrealEditor-Cmd.exe Prototype.uproject -unattended -nop4 -nullrhi -ExecCmds="Automation RunTests Gaters.Worldgen.EnvironmentBrief+Gaters.Worldgen.TerrainNavigation;Quit" -TestExit="Automation Test Queue Empty"`

Expected: compile failure because the land-access types and `WalkableFraction` do not
exist.

- [ ] **Step 4: Implement the minimum contracts**

```cpp
struct FGatersLandAccessTargetRanges
{
    bool operator==(const FGatersLandAccessTargetRanges& Other) const = default;
    FGatersEnvironmentSignalRange WalkableLand;
    FGatersEnvironmentSignalRange ConnectedLand;
};

struct FGatersLandAccessTargetProfile
{
    bool operator==(const FGatersLandAccessTargetProfile& Other) const = default;
    float WalkableLand = 0.f;
    float ConnectedLand = 0.f;
};
```

Compile both signals using distinct deterministic salts, validate both ranges with the
existing `ValidateRange`, advance the brief/compiler contract to version `2`, update its
direct consumers, and calculate:

```cpp
Result.WalkableFraction = Field.Cells.IsEmpty()
    ? 0.f
    : static_cast<float>(Result.Region.WalkableCount)
        / static_cast<float>(Field.Cells.Num());
```

- [ ] **Step 5: Ask Runner for GREEN and the complete Gaters suite**

Expected: focused tests pass; full suite has no regression.

---

### Task 2: Deterministic landform variants and selector

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersLandformProcessField.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersLandformProcessField.cpp`
- Create: `Unreal/Prototype/Source/Prototype/Public/GatersEnvironmentCandidateSelector.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersEnvironmentCandidateSelector.cpp`
- Create: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersEnvironmentCandidateSelectorTests.cpp`

**Interfaces:**
- Produces: `FGatersEnvironmentCandidateSelector::Select(const FGatersEnvironment&,
  const FGatersCompiledEnvironmentBrief&, const TArray<FGatersLandformProtectedRegion>&,
  const FGatersEnvironmentCandidateSelectionSettings&)`.
- Returns: selected recipe only when independently satisfied, plus evidence for every
  attempted candidate.

- [ ] **Step 1: Write failing selector tests**

Tests require:

```cpp
TestTrue(TEXT("same input reproduces selection"), A == B);
TestEqual(TEXT("candidate zero preserves the current process field"),
    BaselineHeight, CandidateZeroHeight);
TestTrue(TEXT("selected candidate escapes Arrival"), A.Selected.bEscapesArrival);
TestTrue(TEXT("selected candidate meets walkable target"),
    A.Selected.WalkableError <= Settings.WalkableTolerance);
TestTrue(TEXT("selected candidate meets connected target"),
    A.Selected.ConnectedError <= Settings.ConnectedTolerance);
TestFalse(TEXT("impossible brief does not replace champion"), Impossible.bSelected);
TestTrue(TEXT("impossible brief preserves rejection evidence"),
    Impossible.Candidates.Num() == Settings.CandidateCount);
```

- [ ] **Step 2: Ask Runner to prove RED**

Expected: compile failure because selector types do not exist.

- [ ] **Step 3: Add candidate identity without changing candidate zero**

Advance the landform recipe to version `3`, add `CandidateIndex` and the final optional
parameter to `Compile`. Offset existing process salts by `CandidateIndex * 16`; index zero
therefore uses the current salts unchanged. Reject negative indices causally.

- [ ] **Step 4: Implement the pure selector**

For each bounded candidate index:

1. Compile and attach the landform recipe.
2. Build one fixed-resolution semantic field spanning the full declared world with no
   optional route target; measure walkable and Arrival-connected fractions there.
3. Build a separate high-resolution local field and evaluate Arrival escape there.
4. Record full-world walkable fraction, full-world connected fraction, local Arrival
   escape, and absolute errors.
5. Select the lowest-error satisfying candidate; break ties by candidate index.
6. Return no selected recipe when none satisfy, while retaining every candidate record.

- [ ] **Step 5: Ask Runner for GREEN and full regression**

Expected: selector tests, Landform Process tests, Environment Brief tests, Terrain
Navigation tests, and the complete Gaters suite pass.

---

### Task 3: Runtime adapter and immutable evidence

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersChunk.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersChunk.h`
- Modify: `Unreal/Prototype/Scripts/Write-EnvironmentRun.ps1`
- Modify: `Unreal/Prototype/Scripts/Test-EnvironmentRunArchive.ps1`
- Modify: `Unreal/Prototype/Scripts/Test-LandformBriefSweep.ps1`

**Interfaces:**
- Consumes: selector result from Task 2.
- Produces: `LAND_ACCESS` runtime evidence and schema-versioned archive evidence without
  changing `FGatersSiteRoutePlanner`.

- [ ] **Step 1: Write failing archive tests**

Require `LAND_ACCESS` evidence to preserve brief/compiler versions, targets, selector
version, candidate count, selected index, satisfaction, walkable fraction, connected
fraction, Arrival escape, and rejection count. Require a valid environment record even
when `PLAN valid=no` because site planning is optional evidence.

- [ ] **Step 2: Run the PowerShell tests and prove RED**

Run:
`powershell -NoProfile -File Unreal/Prototype/Scripts/Test-EnvironmentRunArchive.ps1`

Expected: failure because `LAND_ACCESS` is not parsed.

- [ ] **Step 3: Integrate the thin runtime adapter**

Compile the selector with protected Arrival geometry only. Attach the selected recipe
only when `bSelected`; otherwise retain the current terrain champion. Do not query base,
settlement, landmark, or route discovery from this adapter.
Report selection evidence before environment-recipe compilation. Do not query or modify
the site plan.

- [ ] **Step 4: Advance the immutable archive schema**

Add the parsed selector provenance and metrics to candidate identity and the archive
record. Keep `sitePlan` as separate optional downstream evidence.

- [ ] **Step 5: Verify scripts, focused Unreal tests, and full suite**

Expected: script tests pass and Runner reports a green complete Gaters suite.

---

### Task 4: Held-out challenger evidence and registry decision

**Files:**
- Modify only if evidence supports it: `research/machines.json`
- Modify: `.agents/workstreams/Primary Builder — World & Terrain.md`

**Interfaces:**
- Consumes: immutable held-out run archive.
- Produces: a truthful champion/challenger decision and next falsifying experiment.

- [ ] **Step 1: Ask Runner for the held-out sweep**

Run the existing baseline, high-relief, volcanic, and glacial fixtures across seeds `11`,
`29`, `47`, `83`, and `131`, with world-only mode and no Built Sites requirement.

- [ ] **Step 2: Evaluate the result independently**

Require deterministic selection, declared target errors, Arrival escape, runtime budget,
and no dependency on site-plan validity. Preserve the JSONL archive and its SHA-256.

- [ ] **Step 3: Update the registry truthfully**

Add or activate `world.environment-candidate-selector` only with exact verifier,
challenge-set, failure-artifact, and promotion-gate evidence. Correct the top environment
generator from universal resource/traversal viability to brief-declared viability. Do not
promote `world.landform-process-field` unless held-out evidence passes its existing gate.

- [ ] **Step 4: Validate registry and shared coordination**

Run:

```powershell
research/Test-MachineRegistry.ps1
research/Test-SharedAgentDocs.ps1
git diff --check
```

Expected: all three checks pass.
