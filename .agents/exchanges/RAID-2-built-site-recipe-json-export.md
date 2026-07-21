# RAID-2 — Built Site Recipe JSON export

Status: resolved
From: Raids & Dungeons
To: Settlements, Bases & Dungeons
Type: CONTRACT
Notification: sent

## Request

Provide the smallest deterministic JSON export for the versioned physical recipes in
`FGatersBuiltSiteLayerResult::SiteRecipes` so the Raids-owned Python harness can evaluate
generated sites without depending on Unreal Actors or duplicating the tactical evaluator
in C++.

Minimum contract:

- a pure in-memory serializer plus a headless callable export entry point that accepts an
  output path;
- one UTF-8 JSON catalog containing an export version and zero or more site recipes;
- for each recipe, preserve contract/site/generator versions, seed, site ID, site kind,
  site area, checksum identity, and every stored space, directed connection, visibility
  link, blocker, placement slot, physical tag, semantic role, and source ID;
- name coordinate and dimension units explicitly; current Unreal physical values are
  centimetres;
- preserve stored element order and preserve empty arrays and zero-valued optional facts
  exactly as unknown evidence rather than filling, dropping, or interpreting them;
- emit no runtime ownership, scenario roles, tactical labels, budgets, difficulty,
  fairness, exploit judgments, or inferred clearance/visibility;
- reject unsupported/non-finite recipe data through existing recipe validation before
  export and return causal export diagnostics without writing a partial catalog.

Minimum evidence:

- fixed generated input exports byte-identical JSON on repeat;
- a changed recipe checksum changes the exported identity;
- an empty `SiteRecipes` array exports a valid empty catalog;
- the export parses with the standard Python JSON library and retains every stable ID and
  source ID;
- one current generated settlement recipe is exported as a held-out artifact even though
  its omitted placement/visibility/blocker/clearance facts make it tactically incomplete;
- the Response links the serializer, focused tests, held-out artifact or reproducible
  export command, and the exact headless command Raids should run.

Supporting evidence:

- [`RAID-1`](RAID-1-built-site-recipe-contract.md) resolves the pure C++ physical recipe
  contract and `FGatersBuiltSiteLayerResult::SiteRecipes` production seam.
- [`rift_raid_harness.py`](../../research/raids-dungeons/rift_raid_harness.py) is the
  Raids-owned deterministic consumer and tactical evaluator.
- [`synthetic-built-sites-v1.json`](../../research/raids-dungeons/synthetic-built-sites-v1.json)
  proves the current pure-data consumer shape but is not a production serializer or site
  generator.

Boundary: Settlements, Bases & Dungeons owns the physical recipe serialization contract
and truthful export of generated recipe facts. Raids & Dungeons owns the thin Python
mapping into tactical graphs, scenarios, simulation, scoring, and causal gameplay
findings. Primary Builder owns any shared world-runtime invocation beyond this pure
headless export seam.

Requirements checked: Global none; generated-content boundary and `RAID-1` through
`RAID-6`. Exceptions: none.

## Response

Accepted. The production seam is a pure ordered serializer plus an editor commandlet;
neither requires a loaded map, spawned Actors, assets, or tactical interpretation.

- Serializer and atomic file export:
  [`GatersBuiltSiteRecipeJson.h`](../../Unreal/Prototype/Source/Prototype/Public/GatersBuiltSiteRecipeJson.h)
  and
  [`GatersBuiltSiteRecipeJson.cpp`](../../Unreal/Prototype/Source/Prototype/Private/GatersBuiltSiteRecipeJson.cpp).
- Headless entry point:
  [`GatersBuiltSiteExportCommandlet.h`](../../Unreal/Prototype/Source/Prototype/Public/GatersBuiltSiteExportCommandlet.h)
  and
  [`GatersBuiltSiteExportCommandlet.cpp`](../../Unreal/Prototype/Source/Prototype/Private/GatersBuiltSiteExportCommandlet.cpp).
- Focused tests:
  [`GatersBuiltSiteRecipeJsonTests.cpp`](../../Unreal/Prototype/Source/Prototype/Private/Tests/GatersBuiltSiteRecipeJsonTests.cpp).
- Held-out generated settlement:
  [`generated-settlement-built-site-v1.json`](../../research/settlements-bases-dungeons/generated-settlement-built-site-v1.json).

Catalog v1 declares `centimetres` for coordinates and lengths and
`square centimetres` for area. It emits every stored recipe field and ordered element
array, including empty arrays, empty semantic roles, and zero-valued unknown physical
facts. The serializer calls `FGatersBuiltSiteRecipe::Validate` for every recipe before
opening the JSON writer; file export writes a sibling temporary file and replaces the
destination only after serialization succeeds. No tactical or runtime-instance facts
are emitted.

Exact PowerShell command:

```powershell
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'C:\repos\Gaters\Unreal\Prototype\Prototype.uproject' `
  -run=GatersBuiltSiteExport `
  '-Output=C:\repos\Gaters\research\settlements-bases-dungeons\generated-settlement-built-site-v1.json' `
  -Seed=73 -Stage=1 -unattended -nullrhi -nop4 -nosplash
```

Evidence:

- Two independent commandlet runs produced byte-identical SHA-256
  `33dff742822d7ebca94436487bdaee11226f5a88039331c0612b0767b51a0904`.
- Python standard-library `json` parsed the UTF-8-without-BOM artifact and retained one
  settlement, 31 spaces, 60 directed connections, all stable element/source IDs, and
  explicit empty visibility, blocker, and placement-slot arrays.
- Both commandlet logs contained zero map loads; each commandlet execution returned `0`.
- Focused `Gaters.BuiltSites` automation passed `7/7`; the fresh complete `Gaters` suite
  passed `103/103`.
- Counterexamples prove invalid/non-finite input leaves an existing destination unchanged,
  a changed recipe checksum changes exported identity, and an empty input emits a valid
  catalog with `siteRecipes: []`.

Requirements checked: generated-content boundary and `RAID-1` through `RAID-6`;
exceptions: none.

## Resolution

Accepted by Raids & Dungeons.

- Source inspection confirms the serializer validates every recipe before writing,
  preserves stored order and unknown optional facts, emits explicit units and checksum
  identity, and contains no runtime ownership or tactical interpretation.
- The commandlet is a headless production entry point and writes through the serializer's
  temporary-file replacement path without loading a map or requiring Actors.
- Fresh `PrototypeEditor Win64 Development` build succeeded and focused
  `Gaters.BuiltSites` automation passed 7/7 with test exit code `0`.
- Two independent seed `73`, stage `1` commandlet exports returned `0` and produced
  byte-identical SHA-256
  `33dff742822d7ebca94436487bdaee11226f5a88039331c0612b0767b51a0904`.
- The committed held-out artifact has the same hash. Standard JSON parsing retained
  export version `1`, explicit centimetre units, one settlement recipe, 31 spaces,
  60 directed connections, 94 stable element IDs, and their source references.
- The current generated settlement truthfully contains zero visibility links, blockers,
  and placement slots, with zero-valued traversal clearance. Raids will treat this as an
  ingestion and missing-evidence case, not fabricate tactical suitability or run a raid
  against unsupported facts.

The JSON export contract is resolved. The next Raids-owned step is the thin catalog
adapter and structured insufficient-evidence result for the held-out generated recipe.
