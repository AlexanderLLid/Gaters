# RAID-3 — Generated site tactical evidence

Status: resolved
From: Raids & Dungeons
To: Settlements, Bases & Dungeons
Type: CONTRACT
Notification: sent

## Request

Provide the smallest generated settlement or base recipe whose physical evidence lets
the production Raids preflight return `ready-for-scenario` without treating unknown facts
as absent or inventing tactical roles.

The held-out RAID-2 settlement currently proves 31 spaces and 60 directed connections,
but reports `insufficient-evidence` because it has no placement slots, visibility, or
blocker evidence and every connection has unknown width/headroom.

Minimum next challenge:

- emit enough neutral placement slots with positive physical clearance for Raids to
  derive distinct exterior arrival/extraction, interior objective, and guard candidates;
- assert width and headroom for the connected physical subgraph that can carry at least
  one exterior-to-interior-and-back route under the baseline Combat traversal envelope;
- emit directed visibility facts covering the relevant route/slot subgraph;
- emit blocker facts for that subgraph, or add the smallest explicit evidence-coverage
  declaration that distinguishes “proved no blocker” from “blockers not measured”;
- likewise distinguish complete-empty visibility/blocker evidence from unknown empty
  arrays if the generated challenge proves none are present;
- retain physical tags and provenance only—Raids still assigns arrival, extraction,
  objective, guard, loot, trap, and encounter roles;
- export the result through RAID-2 as a deterministic held-out JSON artifact and provide
  the exact reproduction command.

Acceptance evidence:

- the existing command
  `python research/raids-dungeons/rift_raid_harness.py --built-site-export <artifact> --summary`
  returns `ready-for-scenario` for at least one recipe;
- every readiness decision cites only exported physical facts and stable recipe/source
  IDs;
- removing any required evidence category returns the corresponding causal
  `*-evidence-unknown` result;
- Settlements-focused and full `Gaters` automation remain green.

Supporting evidence:

- [`RAID-2`](RAID-2-built-site-recipe-json-export.md) resolves deterministic production
  serialization and the current held-out artifact.
- [`rift_raid_harness.py`](../../research/raids-dungeons/rift_raid_harness.py) now consumes
  the production export and refuses unsupported contract versions or units.
- [`test_rift_raid_harness.py`](../../research/raids-dungeons/test_rift_raid_harness.py)
  verifies deterministic mapping and causal insufficient-evidence behavior.

Boundary: Settlements, Bases & Dungeons owns proved physical facts, coverage truth, and
generated topology. Raids & Dungeons owns candidate tactical assignment, scenarios,
simulation, scores, and gameplay findings. This request does not ask Settlements to label
or tune raid gameplay.

Requirements checked: Global none; generated-content boundary and `RAID-1` through
`RAID-6`. Exceptions: none.

## Response

Delivered through the existing pure Built Site Recipe and RAID-2 export seam. No tactical
roles, spawned Actors, map state, or final assets are inputs.

Contract and generator changes:

- [`GatersBuiltSiteRecipe.h`](../../Unreal/Prototype/Source/Prototype/Public/GatersBuiltSiteRecipe.h)
  now records ordered per-connection `MovementModeIds` and explicit placement,
  traversal-clearance, visibility, and blocker evidence coverage with provenance.
- [`GatersBuildingGenerator.h`](../../Unreal/Prototype/Source/Prototype/Public/GatersBuildingGenerator.h)
  exposes generated usable volumes and doorway openings; the independent building
  evaluator rejects missing or invalid facts.
- [`GatersSettlementSiteRecipeAdapter.cpp`](../../Unreal/Prototype/Source/Prototype/Private/GatersSettlementSiteRecipeAdapter.cpp)
  compiles the coarse settlement into bounded positive-clearance ground links, contained
  neutral slots, directed visibility, wall/frame blockers, and complete
  generated-geometry coverage. Connections physically intersecting generated blockers
  are omitted, and at least one exterior-to-interior route is required.
- Contract/export version remains `1`; the green-field schema changed in place.

Held-out artifact:

- [`generated-settlement-built-site-v1.json`](../../research/settlements-bases-dungeons/generated-settlement-built-site-v1.json)
- seed `73`, stage `1`, recipe checksum `389705565`
- `140` spaces, `194` directed connections, `194` directed visibility facts, `150`
  blockers, and `118` neutral placement slots
- two independent outputs were byte-identical at SHA-256
  `a2140de624e77913891deedb5716df2a146d0bd7d16a1b2814ec3236b099a4aa`

Exact reproduction command:

```powershell
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'C:\repos\Gaters\Unreal\Prototype\Prototype.uproject' `
  -run=GatersBuiltSiteExport `
  '-Output=C:\repos\Gaters\research\settlements-bases-dungeons\generated-settlement-built-site-v1.json' `
  -Seed=73 -Stage=1 -unattended -nullrhi -nop4 -nosplash
```

Independent production preflight:

```text
site:village:0: evaluation=ready-for-scenario spaces=140 connections=194 findings=none
```

Causal in-memory mutations return exactly:

- missing slots -> `placement-evidence-unknown`
- missing visibility -> `visibility-evidence-unknown`
- missing blockers -> `blocker-evidence-unknown`
- zero connection width/headroom -> `traversal-clearance-unknown`

Verification:

- fresh build succeeded;
- `Gaters.BuiltSites`: `9/9` success;
- focused building tests: `2/2` success;
- independent full `Gaters`: `114/114` success, exit code `0`;
- shared-agent docs and machine registry validators pass;
- `git diff --check` reports no whitespace errors.

Raids ingestion note: `test_rift_raid_harness.py` still hard-codes the RAID-2
missing-evidence artifact checksum/counts and therefore has one expected fixture failure
until Raids updates its owned assertions to this delivered artifact. The production
preflight and all four causal mutation checks already pass without harness changes.

Boundary preserved: Settlements proves physical facts, neutral slots, and coverage.
Raids derives arrival, extraction, objective, guard, loot, trap, scenario, fairness, and
other tactical meaning.

Requirements checked: generated-content boundary, `BUILD-1`, RAID-1 through RAID-3;
exceptions: none.

## Resolution

Accepted after independent reproduction through Unreal Runner:

- `GatersBuiltSiteExport`, seed `73`, stage `1`: exit `0`, commandlet success, no errors;
- artifact size: `308890` bytes;
- SHA-256: `a2140de624e77913891deedb5716df2a146d0bd7d16a1b2814ec3236b099a4aa`,
  matching the recipient's recorded output;
- fresh Raids preflight returns `ready-for-scenario` with 140 spaces, 194 connections,
  and no findings;
- fresh Raids harness tests pass `9/9`, including causal missing-evidence mutations and
  unchanged-source representative evaluation.

This resolves the requested physical-evidence contract only. Disconnected interior
topology remains tracked by `RAID-5`; world-to-site approach and production settlement
scenario evidence remain separate downstream work.
