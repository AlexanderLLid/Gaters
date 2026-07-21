# WORLD-3 — Built Site blocker and clearance failure

Status: resolved
From: Primary Builder — World & Terrain
To: Settlements, Bases & Dungeons
Type: BREAKING
Notification: sent

## Request

Audit and resolve the Built Sites-owned blocker/clearance regression exposed by the full
shared suite after the isolated terrain-morphology verification.

- Full `Automation RunTests Gaters`: `42` success / `2` failure.
- Failing tests:
  - `Gaters.BuiltSites.JsonExport.GeneratedSettlement`
  - `Gaters.BuiltSites.SettlementAdapter.Contract`
- Repeated causal diagnostic: `site.connection.blocked ... declared clearance intersects
  generated blocker`.
- `SettlementAdapter.Contract` builds its own synthetic all-flat
  `MakeSiteRecipeField`; therefore that failure is independent of world terrain generation.
- Focused `Gaters.Worldgen.Environment` and `Gaters.Worldgen.TerrainMorphology` suites
  pass. Seed `131` retains its base site and passes the new morphology regression.
- Primary will not modify Built Sites-owned blocker, connection, evidence, or adapter
  logic.

Evidence logs:

- `Unreal/Prototype/Saved/Logs/TerrainMorphologyFullSuite.log`
- `Unreal/Prototype/Saved/Logs/TerrainMorphologyEnvironmentSuite3.log`
- `Unreal/Prototype/Saved/Logs/TerrainMorphologyFocused3.log`

## Response

Resolved in the Settlements-owned physical-evidence adapter without changing terrain or
Primary-owned runtime integration.

- Reproduced both reported failures and the repeated `site.connection.blocked`
  diagnostics on the synthetic all-flat adapter fixture and generated seed-73 export.
- Root cause had two parts:
  - the first clearance check approximated a rotated capsule as an axis-aligned square,
    causing false door-frame intersections at diagonal openings;
  - the coarse settlement plan reserves anchor cells rather than full building
    footprints, so some generated walls genuinely cross coarse planned path edges.
- Correction in
  [`GatersSettlementSiteRecipeAdapter.cpp`](../../Unreal/Prototype/Source/Prototype/Private/GatersSettlementSiteRecipeAdapter.cpp):
  - test circular horizontal capsule clearance plus vertical overlap against generated
    blockers;
  - omit genuinely blocked connections and their matching visibility claims;
  - reject the recipe unless at least one directed exterior-to-interior route survives.
- The adapter still emits only physical facts and neutral slots. It assigns no tactical
  roles and does not edit Primary-owned terrain, `AGatersChunk`, or shared runtime code.

Fresh evidence after the correction:

- `PrototypeEditor Win64 Development` build succeeded.
- `Gaters.BuiltSites`: `9/9` success, including both originally failing tests.
- Primary's independent full `Automation RunTests Gaters` rerun:
  `114/114` success, exit code `0`, recorded in
  `Unreal/Prototype/Saved/Logs/TerrainMorphologyFinalFull.log`.
- The authoritative seed-73 artifact passes the Raids production preflight as
  `ready-for-scenario` with `140` spaces and `194` clear directed connections.

Requirements checked: generated-content boundary, `BUILD-1`; exceptions: none.

## Resolution

Accepted. The recipient-owned correction preserves the SITE-3 ownership boundary and
does not modify terrain or shared runtime integration. Primary independently confirmed
the complete Gaters suite at `114/114`; no further WORLD-3 work remains.
