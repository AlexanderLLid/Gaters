# Optional Built Site Layer Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use
> `superpowers:executing-plans` to implement this plan task-by-task. The repository owner
> authorized inline execution in the existing shared working tree; do not branch, commit,
> or modify Primary Builder-owned runtime integration.

**Goal:** Extract the existing accepted settlement/evaluation/recipe-adapter sequence into
a pure optional Built Site layer result that Primary Builder can compose or omit.

**Architecture:** Add one pure adapter owned by Settlements, Bases & Dungeons over the existing settlement
generator, evaluator, and recipe adapter. It accepts immutable terrain/site-plan inputs and
returns recipe nodes, diagnostics, versions, source IDs, and summary counts. Default output
is valid and empty; Primary Builder decides whether to call and compose the adapter.

**Tech Stack:** Unreal Engine 5.8 C++, existing Gaters pure structs, Unreal automation.

## Global Constraints

- Preserve every existing settlement, parcel, path, building, and module identity.
- No Actors, components, assets, materials, streaming state, or `AGatersChunk` mutation.
- Do not modify `AGatersChunk`, World Recipe composition, runtime toggles, or
  `research/machines.json`; Primary Builder owns those integrations.
- Empty `FGatersBuiltSiteLayerResult` is valid and means world-only mode.
- No new dependencies, modules, plugins, abstractions, or tactical semantics.
- Requirements checked: Global none recorded; generated content boundary; exceptions:
  none.

---

### Task 1: Pure optional layer contract

**Files:**

- Create: `Unreal/Prototype/Source/Prototype/Public/GatersBuiltSiteLayer.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersBuiltSiteLayer.cpp`
- Test: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersBuiltSiteLayerTests.cpp`

**Interfaces:**

- Consumes: `const FGatersTerrainSemanticField&`, seed, `const FGatersSiteRoutePlan&`,
  and growth stage.
- Produces: `FGatersBuiltSiteLayerResult` and
  `FGatersBuiltSiteLayer::Generate(...)`.

- [x] **Step 1: Write the failing contract tests**

Create tests that require:

```cpp
const FGatersBuiltSiteLayerResult Empty;
TestTrue(TEXT("default layer is valid and empty"), Empty.IsValid() && Empty.IsEmpty());

const FGatersBuiltSiteLayerResult Layer = FGatersBuiltSiteLayer::Generate(
    Field, 73, Sites, 1);
TestTrue(TEXT("accepted settlement produces a valid layer"), Layer.IsValid());
TestEqual(TEXT("layer contract is versioned"), Layer.ContractVersion, 1);
TestEqual(TEXT("settlement generator version is recorded"),
    Layer.SettlementGeneratorVersion, 2);
TestEqual(TEXT("settlement evaluator version is recorded"),
    Layer.SettlementEvaluatorVersion, 2);
TestEqual(TEXT("source site is recorded"), Layer.SourceIds[0], Site.Id);
TestEqual(TEXT("layer preserves legacy recipe node count"),
    Layer.Nodes.Num(), Legacy.Nodes.Num());
```

Compare every emitted node ID, kind, transform, and content key with the existing direct
`Generate -> Evaluate -> Compile` sequence. Add a counterexample proving an unsupported
stage returns no nodes and causal diagnostics.

- [x] **Step 2: Run the new test and verify RED**

Run:

```powershell
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'C:\repos\Gaters\Unreal\Prototype\Prototype.uproject' `
  -unattended -nop4 -nosplash -nullrhi `
  '-ExecCmds=Automation RunTests Gaters.Worldgen.BuiltSiteLayer;Quit' `
  '-TestExit=Automation Test Queue Empty' -log
```

Expected: compilation fails because `GatersBuiltSiteLayer.h` does not exist.

- [x] **Step 3: Implement the minimum pure result and adapter**

Define:

```cpp
struct PROTOTYPE_API FGatersBuiltSiteLayerResult
{
    bool IsValid() const { return Diagnostics.IsEmpty(); }
    bool IsEmpty() const { return Nodes.IsEmpty(); }

    int32 ContractVersion = 1;
    int32 SettlementGeneratorVersion = 0;
    int32 SettlementEvaluatorVersion = 0;
    int32 SiteCount = 0;
    int32 BuildingCount = 0;
    int32 ParcelCount = 0;
    int32 PathCount = 0;
    int32 ValidAssemblyCount = 0;
    int32 ModuleCount = 0;
    TArray<FString> SourceIds;
    TArray<FGatersRecipeNode> Nodes;
    TArray<FString> Diagnostics;
};

struct PROTOTYPE_API FGatersBuiltSiteLayer
{
    static FGatersBuiltSiteLayerResult Generate(
        const FGatersTerrainSemanticField& Field,
        int32 Seed,
        const FGatersSiteRoutePlan& Sites,
        int32 GrowthStage);
};
```

Implementation rules:

- Find `site:village:0`; missing village returns the default valid empty result.
- Generate and independently evaluate the settlement.
- Record generator/evaluator versions and causal diagnostics.
- Compile only an accepted settlement with `FGatersSettlementRecipeAdapter`.
- Copy its nodes and counts without changing order, identity, or transforms.
- Record the site ID plus stable building, parcel, growth-front, and path IDs in
  deterministic existing-plan order.
- Keep the function pure; do not access Actors or runtime state.

- [x] **Step 4: Run the focused tests and verify GREEN**

Run the Step 2 command.

Expected: every `Gaters.Worldgen.BuiltSiteLayer` test succeeds with exit code `0`.

- [x] **Step 5: Run all Gaters automation**

Run:

```powershell
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'C:\repos\Gaters\Unreal\Prototype\Prototype.uproject' `
  -unattended -nop4 -nosplash -nullrhi `
  '-ExecCmds=Automation RunTests Gaters;Quit' `
  '-TestExit=Automation Test Queue Empty' -log
```

Expected: all tests succeed; existing `66/66` plus the new focused tests.

### Task 2: Handoff evidence and integration request

**Files:**

- Modify: `.agents/exchanges/WORLD-1-optional-built-site-layer.md`
- Modify: `.agents/workstreams/Settlements, Bases & Dungeons.md`

**Interfaces:**

- Consumes: verified Task 1 output.
- Produces: an answered `WORLD-1` packet with exact files, tests, and Primary Builder
  integration boundary.

- [x] **Step 1: Run repository validators**

```powershell
& 'research/Test-MachineRegistry.ps1'
& 'research/Test-SharedAgentDocs.ps1'
git diff --check -- `
  'Unreal/Prototype/Source/Prototype/Public/GatersBuiltSiteLayer.h' `
  'Unreal/Prototype/Source/Prototype/Private/GatersBuiltSiteLayer.cpp' `
  'Unreal/Prototype/Source/Prototype/Private/Tests/GatersBuiltSiteLayerTests.cpp' `
  '.agents/exchanges/WORLD-1-optional-built-site-layer.md' `
  '.agents/workstreams/Settlements, Bases & Dungeons.md'
```

Expected: both validators pass and `git diff --check` emits no errors.

- [x] **Step 2: Answer `WORLD-1` and update Settlements, Bases & Dungeons status**

Record:

- exact result type and generation signature;
- default-empty/world-only behavior;
- legacy node parity and counterexample evidence;
- full automation result;
- no `AGatersChunk`, World Recipe, runtime toggle, registry, or Actor changes;
- Primary Builder must compose the result and remove site-generation implementation types
  from its public coordinator header.

- [x] **Step 3: Notify Primary Builder**

Send:

```text
EXCHANGE: WORLD-1 | Settlements, Bases & Dungeons -> Primary Builder — World & Terrain | CONTRACT | Answered: pure optional Built Site layer result and adapter are verified and ready for Primary-owned composition.
```

Do not commit; the human reviews the shared working tree first.
