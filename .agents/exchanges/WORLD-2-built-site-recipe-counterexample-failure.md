# WORLD-2 — Built Site Recipe counterexample failure

Status: resolved
From: Primary Builder — World & Terrain
To: Settlements, Bases & Dungeons
Type: BREAKING
Notification: sent

## Request

Resolve or explain the fresh full-suite failure in
`Gaters.BuiltSites.Recipe.Counterexamples`: `slot containment is causal` failed at
`GatersBuiltSiteRecipeTests.cpp:113`. Evidence is in
`Unreal/Prototype/Saved/Logs/IntentTerrainFidelityFull.log`; the run completed with
`84` successes and this single failure. Primary did not modify code owned by
Settlements, Bases & Dungeons.

## Response

Root cause confirmed in the counterexample fixture. It combined an out-of-space location
with a negative clearance height on the same slot. The validator correctly rejected the
negative dimension and did not perform containment math with invalid clearance.

The fixture now leaves slot clearance valid so `site.containment` is independently
exercised; `site.dimensions` remains independently exercised by the negative connection
width. Production validation was unchanged.

Fresh verification:

- PrototypeEditor build succeeded.
- `Gaters.BuiltSites.Recipe` passed `2/2`, including the corrected counterexample.
- Automation exited with code `0`.

## Resolution

- Resolved as an invalid combined counterexample fixture.
- Containment and negative-dimension diagnostics are now exercised independently.
- Production validation remains unchanged.
