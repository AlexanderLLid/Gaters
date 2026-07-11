# Unreal progress

Working notes for the Unreal slice — what exists in the project right now and what is
being built. Not canon, not the plan (that's [[prototype-plan]]). Updated in place.

## Works now (map: /Game/Gaters/Maps/Lvl_GateGreybox)

- Home Gate (BP_Gate) with three lane-card pads: approach prints 3 rolled lane cards
  (danger / value / stability), stepping on a pad dials that lane and teleports to the
  far site. Readings are always true for now (lane twists not built).
- Far site: seeded terrain chunk (BP_TerrainChunk — Perlin heightfield, walkable, flat
  Gate plinth), far Gate ring, EXTRACT HOME pad teleports back.
- Run-pressure placeholder (BP_RunClock): drains while on the far site, resets on
  leaving, at zero recalls the player home ("run failed").
- All Blueprint, authored via the Unreal MCP; verified in PIE via spawn/teleport + log
  reading each step.

## Varied base generation (plan priority 4) — working

The terrain chunk IS the site generator (BP_TerrainChunk). One RandomStream(Seed) rolls
everything; the seed is the save. Base structures are appended into the same dynamic
mesh (no actors, collision free); only loot is a spawned actor (BP_LootCube, touch →
taken). Change Seed on the FarTerrain instance = new site.

- Rolls per seed: terrain preset (plains/hills/broken), base archetype, base
  bearing/distance from the Gate pad, layout pieces, defender positions, loot spot.
- Archetypes: compound (walled ring on a plinth, 1-2 entrances), outpost (tower +
  crates), ruin (scattered broken walls). Defenders are marker cones for now.
- Per-seed report prints on arrival: SITE (seed/preset/archetype), TAGS (bearing, dist,
  pieces, entrances, defenders), CHECK (pad_clear, loot_reach, triangle count).
- Verified across 6 seeds in one sweep: all checks pass, ~8.6-8.9k tris per site.
- Gotcha fixed: FRandomStream first draws correlate linearly for nearby seeds; the seed
  is sin-hashed before stream init (the shader-hash trick) to decorrelate.

Not done: real defender AI, per-piece destructible walls (structures are one mesh),
loot-reach check is by-construction only (no navmesh trace), lane twists / bad reads.

## Site analysis — where bases can go (working)

After terrain build, AnalyzeSite scans a 25x25 grid (5 m cells, trace-down) and
classifies each cell by surface normal: flat / slope / steep, with cells on the Gate
and base plinths marked reserved. 3x3 all-flat clusters become buildable plots
(spaced >= 15 m), drawn as green debug rings, stored in PlotCenters for later
consumers (base stamping, player-build rules). Report line per seed:
`PLOTS buildable=N flat=F slope=S steep=T`. Verified: plains seed -> 53 plots,
broken seed -> 2 plots; reserved plinths stay marker-free. Slope thresholds
(normal.z 0.94 / 0.77) and plot size/spacing are the tunables.

Direction note: base _content_ is intentionally not generated anymore — the plan is
plots + categories now, and stamping bases onto plots later (marketplace building
system pieces are the designated borrow; Easy Building System V10 on Fab is the
candidate, user claims/installs it via launcher when ready).

## Worldgen steps 3-5: resource zones, scatter identity, diff persistence — working

All on BP_TerrainChunk, chained after AnalyzeSite in BeginPlay:
MarkResourceZones → LoadDiff → SpawnScatter. New BPs: BP_Scatter (cone stand-in with
ScatterId + OwnerChunk, touch = chop), BP_WorldDiff (SaveGame holding the diff entries).

- **Resource zones (plan step 3).** 3 seeded cluster centers, radius-3-cell disks; only
  flat/slope cells get remarked to category 4 in FlatGrid (steep/reserved never). Report:
  `ZONES resource=N clusters=3`. Verified seed 7: two PIE runs → identical `resource=65`.
- **Scatter with stable IDs (plan step 4, U1).** One instance max per grid cell; **ID =
  cell index** (0-624), so identity is stable by construction. Trees on resource cells,
  sparse rocks (5%) on flat cells; jitter and density rolls drawn from the seed stream in
  fixed grid order, z from HeightGrid. Report: `SCATTER n= sum=` where sum is a checksum
  over all candidate ids+positions (diff-independent). Verified: two runs of seed 7 →
  identical `n=86 sum=1505505`, and named instances bit-exact (id=8 pos identical to the
  decimal). Screenshot check: cones sit on the terrain.
- **Diff record + replay (plan step 5, U2 — the core unknown, proven).** Chop = pawn
  overlap → ChopScatter(Id): append `chop:<id>` to DiffEntries, save to SaveGame slot
  `WorldDiff_<seed>`, destroy actor. LoadDiff (before scatter) loads the slot; SpawnScatter
  skips chopped ids (stream draws happen before the skip, so surviving instances never
  shift). Report: `DIFF entries= replayed=`. Verified end to end: chop tree id=8 →
  `CHOP id=8 entries=1`, `WorldDiff_7.sav` on disk; redial (PIE restart) →
  `SCATTER n=85 sum=1505505` (same checksum, one fewer spawn) + `DIFF entries=1
replayed=1`; **full editor restart → same lines**. Ground stays untouched — the diff
  describes objects only.

Determinism gotcha (cost a rewrite): RandomXFromStream nodes are pure — used as data
inputs they re-draw per consuming exec node. Every stream draw must be captured into a
member variable via a Set node before use (the BeginPlay pattern).

## Worldgen steps 6-7: claim diff + generation budget — working

- **Claim diff (plan step 6).** BP_ClaimMarker pads spawn on every plot (SpawnClaimMarkers,
  after scatter); touch one → ClaimPlot appends `claim:<plotIndex>` to the same DiffEntries
  and saves via the shared SaveDiff (ChopScatter now routes through it too). Replay: markers
  spawn already-claimed (blue, MI_Claimed). Report: `CLAIM plots= claimed=`. Verified seed 7:
  claim plot 0 → redial → `DIFF entries=14 replayed=13` + `CLAIM plots=53 claimed=1` —
  chop and claim coexist in one diff, both survive regeneration.
- **Generation budget (plan step 7, U4).** BeginPlay is wrapped in a DateTime wall-clock
  timer (frame-cached time nodes read 0 — GetAccurateRealTime is useless intra-frame);
  report line `GEN ms=`. Measured 170-260 ms per full seed across presets (seeds 3/7/11/21).
  Sweep also confirmed per-seed diff slots isolate (fresh seeds report `entries=0`).
- Scatter/claim visuals: green tree cones (MI_ScatterTree), brown rocks (MI_ScatterRock),
  resource-zone cells outlined with green debug boxes, claimed plots blue.

Not done: chop/claim are touch-triggered (real interactions are a borrow), markers/scatter
are actors (ISM when counts matter), debug rings/boxes are editor-only visualization.

## C++ — toolchain works, module compiles

VS 2026 (MSVC 14.51) + Windows SDK + .NET 8 verified; UE 5.8 rejects VS 2022's MSVC 14.41
(UBT bans 14.40-14.43). Module `Prototype` (Source/, GatersCore stub) builds via Build.bat
and loads in the editor. Standing call: new logic in C++, Blueprint for marketplace borrows
and glue; the worldgen/diff core is the first port candidate.

## Grid placement prototype (Fortnite-lite) — working

Content/Gaters/Prototype/Building/: BP_BuildPiece + BP_BuildPlacer + green/red ghost
materials. In Play mode: keys 1-5 select Floor_1x1 / Wall_1x1 / Ramp_1x1 /
RoomBlock_2x2 / CoverBlock_1x1, R rotates 90°, left click places. Ghost snaps to the
400 cm structural grid (even footprints snap to cell corners), Z follows the ground hit
with a 30 cm sink skirt, green when placeable, red when overlapping an existing piece
or with no ground hit. Overlap = live query of all BP_BuildPiece actors each tick
(no registration; fine below ~100 pieces). A BuildPlacer instance + one test piece sit
on the home floor in Lvl_GateGreybox.

Known simplifications: walls snap to cell centers (edge grid later), PieceType is a
string not an enum, no detail grid, ramp mesh pivot untested visually.

## Blocked / notes

- No C++ toolchain on the machine (VS install needs a UAC click). Blueprint-only until
  then. Install command lives in the AI memory notes.
- Terrain builds at BeginPlay — invisible in the editor viewport, press Play to see it.
