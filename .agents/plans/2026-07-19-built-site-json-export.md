# Built Site Recipe JSON Export Implementation Plan

> **For agentic workers:** execute inline with TDD. Do not branch, stage, commit, or push.

**Goal:** Export deterministic UTF-8 JSON from generated
`FGatersBuiltSiteLayerResult::SiteRecipes` through a standalone headless command.

**Architecture:** A pure serializer validates all recipes before writing ordered JSON.
A thin file exporter writes atomically, and a console command reproduces the current
pure settlement generation sequence from output path, seed, and stage without loading a
map or spawning Actors.

**Tech stack:** Unreal Engine Core JSON writer, C++ automation tests,
`UnrealEditor-Cmd.exe`, Python standard-library `json` verification.

## Global constraints

- Preserve stored array order, empty arrays, empty strings, and zero-valued unknown facts.
- Emit physical facts only; never infer tactical facts.
- Coordinates and lengths are centimetres; area is square centimetres.
- Validate every recipe before serialization; never replace a destination with invalid
  or partial output.
- Requirements checked: generated-content boundary and `RAID-1` through `RAID-6`;
  exceptions: none.

---

### Task 1: Pure deterministic serializer

**Files:**

- Create: `Unreal/Prototype/Source/Prototype/Public/GatersBuiltSiteRecipeJson.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersBuiltSiteRecipeJson.cpp`
- Create: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersBuiltSiteRecipeJsonTests.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Prototype.Build.cs`

**Interfaces:**

- `FGatersBuiltSiteRecipeJson::Serialize(const TArray<FGatersBuiltSiteRecipe>&,
  FString&, TArray<FString>&) -> bool`
- Catalog root: `exportVersion`, `coordinateUnit`, `lengthUnit`, `areaUnit`,
  `siteRecipes`.
- Recipe identity: versions, seed, ID, kind, site area, unsigned checksum.

- [x] Add one automation test constructing a valid recipe with every element type,
  repeated tags/source IDs, explicit zeros, and empty arrays.
- [x] Assert repeated serialization is byte-identical, changed recipe facts change the
  checksum field, empty input emits a valid empty catalog, and every stored fact appears.
- [x] Build and run `Gaters.BuiltSites.JsonExport.Serializer`; confirm RED because
  `GatersBuiltSiteRecipeJson.h` does not exist.
- [x] Add the header and ordered `TJsonWriter` implementation. Call `Recipe.Validate`
  for every item before opening the JSON writer. Prefix validation diagnostics with the
  recipe index and causal rule ID.
- [x] Add Unreal's native `Json` module as a private dependency.
- [x] Rebuild and rerun the focused test; confirm GREEN.

### Task 2: Atomic file export and standalone generator

**Files:**

- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersBuiltSiteRecipeJson.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersBuiltSiteRecipeJson.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersBuiltSiteRecipeJsonTests.cpp`

**Interfaces:**

- `Save(const TArray<FGatersBuiltSiteRecipe>&, const FString&,
  TArray<FString>&) -> bool`
- `GenerateSettlement(const FString& OutputPath, int32 Seed, int32 Stage,
  TArray<FString>&) -> bool`
- Console command: `Gaters.ExportBuiltSites <output-path> [seed] [stage]`.

- [x] Add tests proving invalid recipe data leaves an existing destination unchanged,
  valid data replaces it with UTF-8 JSON, and seed `73`, stage `1` generates one current
  settlement recipe without Actors.
- [x] Run the focused tests; confirm RED because file and generator entry points do not
  exist.
- [x] Implement temp-file-then-move output. Remove the temp file after any failed move.
- [x] Reproduce the current pure sequence using environment recipe compilation, world
  base-site discovery, semantic-field construction, route planning, and
  `FGatersBuiltSiteLayer::Generate`. Reject missing base/village/invalid layer with causal
  diagnostics.
- [x] Register the headless commandlet entry point. Defaults are seed `73` and stage `1`;
  malformed or rejected inputs return a non-zero headless exit.
- [x] Rebuild and rerun focused tests; confirm GREEN.

### Task 3: Held-out artifact and consumer proof

**Files:**

- Create: `research/settlements-bases-dungeons/generated-settlement-built-site-v1.json`
- Modify: `.agents/exchanges/RAID-2-built-site-recipe-json-export.md`
- Modify: `.agents/workstreams/Settlements, Bases & Dungeons.md`

- [x] Confirm Unreal Editor is closed before launching any Unreal build or headless run.
- [x] Run the standalone command twice for seed `73`, stage `1`; compare SHA-256 hashes.
- [x] Parse the artifact using Python `json`; assert one recipe, stable IDs/source IDs,
  preserved empty arrays, and centimetre unit declarations.
- [x] Run the focused exporter tests, all Built Sites tests, and the complete Gaters
  automation suite.
- [x] Run `research/Test-SharedAgentDocs.ps1`, `git diff --check`, and re-scan relevant
  exchanges.
- [x] Change RAID-2 from `open` to `answered`; link implementation, tests, artifact, and
  the exact reproducible command. Update the owned workstream status and notify Raids &
  Dungeons.
