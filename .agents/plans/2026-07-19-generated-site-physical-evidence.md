# Generated Site Physical Evidence Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to
> implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Export one deterministic generated settlement with proved neutral placement,
movement, clearance, visibility, blocker, and coverage facts that the production Raids
preflight accepts as `ready-for-scenario`.

**Architecture:** Enrich the existing pure building assembly with usable-volume and
opening facts. Extend Built Site Recipe v1 in place with per-connection movement modes
and recipe evidence coverage, then make the existing settlement adapter compile its
coarse plan into a fine physical evidence graph. Reuse the RAID-2 serializer and
commandlet; do not create a second generator or tactical adapter.

**Tech Stack:** Unreal Engine 5.8 Core C++, Unreal Automation Tests, existing JSON
serializer/commandlet, Python standard library and Raids preflight.

## Global Constraints

- `BUILD-1`: every connection declares physical movement modes; no site-wide mode.
- Settlements emits only physical facts, neutral slots, coverage, and provenance.
- Raids owns arrival, extraction, objective, guard, loot, trap, scenario, policy, and
  score decisions.
- Generated recipes and JSON are authoritative; Actors and final assets are not inputs.
- Keep contract/export version `1`; change the green-field schema in place.
- Do not edit the Raids-owned harness, shared `AGatersChunk`, runtime growth, aquatic
  generation, or `research/machines.json`.
- Do not branch, stage, commit, or push.
- Requirements checked: Global (none recorded), BUILD-1; exceptions: none.

---

### Task 1: Built Site movement and coverage contract

**Files:**

- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersBuiltSiteRecipe.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersBuiltSiteRecipe.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersBuiltSiteRecipeTests.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersBuiltSiteRecipeJson.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersBuiltSiteRecipeJsonTests.cpp`

**Interfaces:**

- Add `FGatersBuiltSiteEvidenceCoverage` with four booleans and `SourceIds`.
- Add `TArray<FString> MovementModeIds` to `FGatersBuiltSiteConnection`.
- Add `EvidenceCoverage` to `FGatersBuiltSiteRecipe`.
- JSON fields: `evidenceCoverage` and `movementModeIds`.

- [ ] **Step 1: Write failing contract tests**

Add assertions equivalent to:

```cpp
Recipe.EvidenceCoverage = {true, true, true, true, {TEXT("source:coverage")}};
Recipe.Connections[0].MovementModeIds = {TEXT("ground")};
TestTrue(TEXT("movement and coverage recipe validates"), Recipe.Validate(Issues));

Recipe.Connections[0].MovementModeIds.Reset();
TestTrue(TEXT("connection without a movement mode is causal"),
    HasIssue(Recipe, TEXT("site.movement-mode")));

Recipe.EvidenceCoverage.bTraversalClearance = true;
Recipe.Connections[0].Width = 0.f;
TestTrue(TEXT("covered traversal requires positive width"),
    HasIssue(Recipe, TEXT("site.evidence.traversal")));
```

Extend the serializer test to require ordered `movementModeIds` and the full coverage
object with source IDs in parsed JSON.

- [ ] **Step 2: Verify RED**

Run a fresh `PrototypeEditor Win64 Development` build after the process preflight.
Expected: compile failure because the new fields/types do not exist.

- [ ] **Step 3: Implement the minimum contract**

Use these exact public fields:

```cpp
struct PROTOTYPE_API FGatersBuiltSiteEvidenceCoverage
{
    bool bPlacement = false;
    bool bTraversalClearance = false;
    bool bVisibility = false;
    bool bBlockers = false;
    TArray<FString> SourceIds;
};

struct PROTOTYPE_API FGatersBuiltSiteConnection
{
    // existing fields...
    TArray<FString> MovementModeIds;
};

struct PROTOTYPE_API FGatersBuiltSiteRecipe
{
    // existing fields...
    FGatersBuiltSiteEvidenceCoverage EvidenceCoverage;
};
```

Canonical text must include coverage before elements and movement modes inside each
connection. Validation must reject empty/duplicate movement IDs, missing coverage
provenance when any coverage flag is true, and non-positive width/headroom when traversal
coverage is true. JSON always writes the coverage object and movement-mode arrays.

- [ ] **Step 4: Verify GREEN**

Build, then run `Gaters.BuiltSites.Recipe` and
`Gaters.BuiltSites.JsonExport.Serializer`. Expected: all selected tests pass.

---

### Task 2: Building usable-volume and opening facts

**Files:**

- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersBuildingGenerator.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersBuildingGenerator.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersBuildingEvaluator.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersBuildingAssemblyTests.cpp`

**Interfaces:**

- `FGatersBuildingUsableSpace`: stable ID, floor index, center, extent, source IDs.
- `FGatersBuildingOpening`: stable ID, source module ID, floor index, transform, width,
  headroom, source IDs.
- `FGatersBuildingAssembly::UsableSpaces` and `Openings`.

- [ ] **Step 1: Write failing building tests**

Require one positive ground-floor usable space and one positive opening:

```cpp
TestEqual(TEXT("one ground usable space"), A.UsableSpaces.Num(), 1);
TestEqual(TEXT("one entrance opening"), A.Openings.Num(), 1);
TestTrue(TEXT("usable volume is positive"),
    A.UsableSpaces[0].Extent.X > 0.f && A.UsableSpaces[0].Extent.Y > 0.f
    && A.UsableSpaces[0].Extent.Z > 0.f);
TestTrue(TEXT("opening is usable"),
    A.Openings[0].Width > 0.f && A.Openings[0].Headroom > 0.f);
```

Add counterexamples for zero usable extent, zero opening width, and an opening whose
`SourceModuleId` does not name the assembly's `DoorWall` module. Require causal rules
`building.usable-space.invalid`, `building.opening.invalid`, and
`building.opening.source`.

- [ ] **Step 2: Verify RED**

Build. Expected: compile failure because the new building facts do not exist.

- [ ] **Step 3: Implement source facts and evaluation**

Use arrays so later multi-floor/multi-opening assemblies do not require a schema change:

```cpp
struct PROTOTYPE_API FGatersBuildingUsableSpace
{
    FString Id;
    int32 FloorIndex = 0;
    FVector Center = FVector::ZeroVector;
    FVector Extent = FVector::ZeroVector;
    TArray<FString> SourceIds;
};

struct PROTOTYPE_API FGatersBuildingOpening
{
    FString Id;
    FString SourceModuleId;
    int32 FloorIndex = 0;
    FTransform Transform = FTransform::Identity;
    float Width = 0.f;
    float Headroom = 0.f;
    TArray<FString> SourceIds;
};
```

Derive the ground usable volume from footprint span, wall thickness, foundation top,
wall height, and floor/roof underside. Derive opening width/headroom from the existing
door-frame proportions used by placeholder realization. Include both collections in
canonical assembly text. The independent evaluator validates identity, provenance,
positive finite dimensions, ground-floor containment, and the DoorWall source relation.

- [ ] **Step 4: Verify GREEN**

Build and run `Gaters.Worldgen.Buildings`. Expected: contract and counterexamples pass.

---

### Task 3: Generated settlement physical evidence graph

**Files:**

- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersSettlementSiteRecipeAdapter.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersSettlementSiteRecipeAdapter.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersBuiltSiteLayer.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersSettlementSiteRecipeAdapterTests.cpp`

**Interfaces:**

- Add `FGatersSettlementEvidenceSettings::Ground()`.
- Extend `FGatersSettlementSiteRecipeAdapter::Compile` with explicit settings.
- Output fine positive-volume spaces, directed connections, visibility, wall blockers,
  neutral slots, and complete generated-geometry coverage.

- [ ] **Step 1: Write failing adapter tests**

Replace the old unknown-evidence assertions with these black-box guarantees:

```cpp
const auto Settings = FGatersSettlementEvidenceSettings::Ground();
const auto B = FGatersSettlementSiteRecipeAdapter::Compile(Field, 73, Village, Settings);
TestTrue(TEXT("generated geometry coverage is complete"),
    B.Recipe.EvidenceCoverage.bPlacement
    && B.Recipe.EvidenceCoverage.bTraversalClearance
    && B.Recipe.EvidenceCoverage.bVisibility
    && B.Recipe.EvidenceCoverage.bBlockers);
TestTrue(TEXT("neutral slots are proved"), !B.Recipe.PlacementSlots.IsEmpty());
TestTrue(TEXT("directed visibility is proved"), !B.Recipe.Visibility.IsEmpty());
TestTrue(TEXT("wall blockers are proved"), !B.Recipe.Blockers.IsEmpty());
```

For every connection require `MovementModeIds == {"ground"}`, positive width/headroom,
and endpoint distance no greater than `Settings.MaxConnectionLength`. Require an
exterior-to-interior-and-back path, neutral slot tags only, slot containment, stable
provenance, repeat checksum identity, and complete earlier-stage fact preservation.

Add counterexamples for invalid settings, an opening too small for its interior link, and
a generated wall intruding into a declared open connection. Require causal diagnostics
and `bCompiled == false` without a partial accepted recipe.

Also extend `GatersBuiltSiteRecipeJsonTests.cpp` now, before production changes, to
require the generated seed-73 export to contain coverage, movement modes, positive
clearance, neutral slots, visibility, blockers, and no tactical slot tags.

- [ ] **Step 2: Verify RED**

Build. Expected: compile/test failure because the current adapter emits centerlines and
unknown evidence.

- [ ] **Step 3: Implement settings and evidence compilation**

Use this settings shape:

```cpp
struct PROTOTYPE_API FGatersSettlementEvidenceSettings
{
    int32 Version = 1;
    FString MovementModeId;
    float PathWidth = 0.f;
    float OutdoorHeadroom = 0.f;
    float MaxConnectionLength = 0.f;
    float SlotClearanceRadius = 0.f;
    float SlotClearanceHeight = 0.f;
    static FGatersSettlementEvidenceSettings Ground();
};
```

Implementation order must remain deterministic:

1. create positive outdoor path spaces in sorted plan order;
2. subdivide each canonical path adjacency into stable intermediate spaces and two-way
   links no longer than `MaxConnectionLength`;
3. create each building's usable interior and doorway space;
4. subdivide entrance-to-door and door-to-interior links;
5. emit one contained neutral center slot per original path/interior space;
6. emit solid wall blockers plus three frame blockers around each opening;
7. prove every declared open segment misses generated blockers;
8. emit one directed visibility fact per open directed connection;
9. declare generated-geometry coverage and validate the final recipe.

`FGatersBuiltSiteLayer` passes `FGatersSettlementEvidenceSettings::Ground()`. No tactical
tag is permitted; tests explicitly reject `arrival`, `extraction`, `objective`, `guard`,
`loot`, `trap`, `spawn`, or `patrol` tags.

- [ ] **Step 4: Verify GREEN**

Build, run `Gaters.BuiltSites.SettlementAdapter`, then all `Gaters.BuiltSites`. Expected:
focused and Built Sites automation pass.

---

### Task 4: Deterministic held-out export and Raids readiness

**Files:**

- Modify: `research/settlements-bases-dungeons/generated-settlement-built-site-v1.json`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersBuiltSiteRecipeJsonTests.cpp`
- Modify: `.agents/exchanges/RAID-3-generated-site-tactical-evidence.md`
- Modify: `.agents/workstreams/Settlements, Bases & Dungeons.md`

**Interfaces:**

- Existing commandlet remains the only producer.
- Existing Raids preflight remains the independent consumer.

- [ ] **Step 1: Verify the generated-export test is GREEN**

Run the generated-settlement JSON test added during Task 3. Require coverage, movement
modes, positive clearance, neutral slots, visibility, blockers, and physical-only tags.

- [ ] **Step 2: Regenerate twice and prove determinism**

Run the exact RAID-2 command twice to two output paths. Require byte equality and equal
SHA-256, then replace the authoritative held-out artifact with the generated bytes.

- [ ] **Step 3: Run the independent readiness and mutation checks**

Run:

```powershell
python research/raids-dungeons/rift_raid_harness.py `
  --built-site-export research/settlements-bases-dungeons/generated-settlement-built-site-v1.json `
  --summary
```

Expected: `ready-for-scenario` for the generated site.

Use a Python one-shot in memory to remove placement, visibility, blockers, and one
connection's width/headroom separately; call `inspect_built_site_export` and require the
four corresponding `*-evidence-unknown` findings. Do not write mutation artifacts.

- [ ] **Step 4: Complete verification**

Run fresh build, focused building/recipe/adapter/JSON tests, all Built Sites tests, full
`Gaters`, Raids Python tests, `research/Test-SharedAgentDocs.ps1`,
`research/Test-MachineRegistry.ps1`, and `git diff --check`.

- [ ] **Step 5: Answer RAID-3**

Write the recipient-owned Response with exact files, export command, held-out hash,
counts, readiness output, causal mutation evidence, tests, boundaries, and remaining
limitations. Set status `answered`, notify Raids, and update the owned workstream.
