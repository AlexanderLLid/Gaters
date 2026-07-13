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
plots + categories now, and stamping bases onto plots later via the building-system
borrow.

## Easy Building System V10 — migrated in, demo verified

EBS V10 (free on Fab, Blueprint-only, ships as a 5.7 Complete Project) is migrated into
the project: `Content/EasyBuildingSystem/` copied whole, its legacy input mappings
appended to DefaultInput.ini, its game mode set as a per-map override on its
Demonstration map only (global defaults untouched — our maps keep our game mode).
Verified under 5.8: demo map PIEs clean (no BP compile errors), EBS builder character
spawns with the full loop — build menu, snapping, resources, damage tools, save/load.
Try it: open /Game/EasyBuildingSystem/Maps/Demonstration, press Play, Q for the menu.

EBS's own save system stays unused — the diff is ours. The run-pressure recall is
disabled meanwhile (BP_RunClock DrainRate 0).

## Base stamping from EBS pieces — working (C++ chunk)

`AGatersChunk::StampBase()` generates a base as **data**: rows of (piece class, local
position, yaw) built from a layout — currently one archetype, a 3x3-foundation hut with
perimeter walls, doorframe + door, and a ceiling roof (31 EBS pieces, 300-unit grid) on
the base plinth. EBS pieces spawn fine outside EBS's placement flow (plain SpawnActor).
Every stamped piece's `OnDestroyed` is bound to the chunk: destruction appends
`piece:<stampId>` to the same per-seed diff as chops/claims (with a world-teardown guard
so PIE shutdown doesn't count), and `StampBase` skips destroyed ids on rebuild. Report:
`STAMP pieces= replayed=`.

Verified on the CppTestChunk (seed 7): fresh dial `STAMP pieces=31 replayed=0`; destroy a
piece via the `bDebugSmashPiece` flag on the chunk (debug stand-in for raid damage) →
`PIECE destroyed=0 entries=1`, diff saved; redial → `STAMP pieces=30 replayed=1` — the
hole persists. Piece positions verified exact (foundations z=628 on the plinth, walls
z=906 on edges, ceilings z=1184).

Notes: first-dial GEN cost is ~2.5 s from cold EBS asset loads, warm dials 40-125 ms
(preload during the gate dial animation when that exists). The chunk is spawned at
runtime by `UGatersTestSpawner` (world subsystem) because the editor MCP tools can't
persist new external actors in the WP map — delete that subsystem when a real gate loop
is rebuilt against AGatersChunk.

## One-island flow + procedural base variety — working

The play path is now one island: `UGatersTestSpawner` destroys the legacy prototype
actors at BeginPlay (old dial network, old BP far site, run clock, grid-building
prototype — all proven or superseded) and teleports the player onto the island's gate
pad. Press Play → you're on the island. Seed override for testing: `Saved/TestSeed.txt`.

`StampBase` is a procedural generator now, all rolls from the seed stream: archetypes
hut / compound / tower; footprints 1x1-4x4; 1-4 stories; door on a rolled side (with EBS
door); ~22% of wall slots become window frames; compounds add 1-2 extra buildings and a
fence ring with a gate gap that follows the terrain; a max-drop check rejects buildings
on too-uneven ground (base fits the environment). Pieces spawn deferred with EBS's
`Built=true` so they arrive in finished state. The base plinth is gone — foundations sit
on raw terrain and bridge it (that's their job); only the Gate pad keeps its platform.

Verified: seed 3 → compound 4x4 main + 3 buildings + fence, seed 11 → compound with
2-story main, seed 42 → compound 3x2, seed 7 → 4-story tower; same seed run twice →
identical layout; smash a piece → redial → replayed. GEN 40-130 ms warm.

## Material tiers + ground fit + legacy cleanup

- **Material tiers.** Each building rolls wood/stone/metal from the seed (sheds never
  outclass the main building; the fence matches the main). Implementation: only the
  safe Modular actors are spawned; every tier's look comes from EBS's Polygonal mesh
  set swapped in post-spawn. The Chaos stone/metal ACTORS are explicitly not used —
  raw-spawned they self-collapse (no EBS support graph) and flooded the diff with 113
  false destructions in testing. Revisit Chaos variants only for real raid destruction
  physics, wired through EBS properly.
- **No hardcoded piece dimensions.** All vertical placement (foundation top, wall bands,
  roof, fence posts) is derived from each tier's actual mesh bounding boxes at load
  time. The first tier pass hardcoded the Dummy-mesh numbers (256/300) and every
  Polygonal band floated by the difference — visible as air gaps between foundation,
  walls, and roof. Mesh families differ in height AND pivot; measure, never assume.
- **Ground fit.** Buildings have no pedestal and no floating: each foundation cell is
  independently stretched (z-scale) from the shared level floor down past its own patch
  of dirt — foundation skirts exactly as the worldgen plan called for. Level floor rides
  the footprint's highest corner; the max-drop rule still rejects sites needing > 350 cm
  of stilt. Fence posts follow the ground per post.
- **Legacy cleanup.** Deleted (git history has them): the entire BP gate loop
  (BP_TerrainChunk, BP_Gate, BP_LanePad, BP_RunClock, BP_LootCube), the BP diff chain
  (BP_WorldDiff, BP_Scatter, BP_ClaimMarker — all superseded by C++), the Fortnite-lite
  grid placer (BP_BuildPlacer, BP_BuildPiece + ghost materials), their 10 placed level
  actors, and the GatersCore C++ stub. Content/Gaters keeps only Maps, Materials
  (MI_ScatterTree/Rock/Claimed — used by C++), and Meshes. Source/ is five files:
  GatersChunk, GatersScatter, GatersClaimMarker, GatersTestSpawner, GatersWorldDiff.

Verified after all three changes: seed 3 twice → identical `compound tier=metal 4x4x1
buildings=3, 136 pieces, replayed=0` (no self-destruction), seed 21 → `hut tier=wood
2x2x2` (25 pieces). Warm GEN 43-129 ms.

## Probe raider sim (base evaluation) — working

One scripted attacker (GatersRaider.cpp) measures how hard a generated base is to crack.
Generator-agnostic by contract: the raider consumes only world state — actors tagged
`Breakable` (every stamped piece), one actor tagged `RaidLoot` (spawned on the first
building's floor), and the navmesh — so a rewritten generator, or a player-built base,
is evaluable with zero sim changes.

- Behavior: pathfind to the loot; when stuck, break the nearest breakable piece (real
  destruction path → diff entry), jump through, retry. Direct-walk fallback when no
  navmesh path exists.
- Result: one JSON line per run appended to `Saved/RaidResults.jsonl` — context string
  (e.g. `seed=56`), genversion, success, why, time, pieces broken, whether pathfinding
  worked. This file is the future training/tuning dataset.
- Triggers: `Gaters.Raid <context>` console command (player position; also headless via
  `-ExecCmds`), or the `bDebugStartRaid` flag on the chunk (MCP-settable during PIE,
  spawns at the gate pad).
- GenVersion (the cheap half of questions #26): `GatersGenVersion` is stamped into every
  diff save; a mismatched diff is discarded on load instead of mis-replaying. No more
  stale-diff corruption after generator changes.
- Navmesh: RuntimeGeneration=Dynamic (DefaultEngine.ini) + a NavMeshBoundsVolume over
  the island. Navmesh builds fine on the dynamic-mesh ground. The volume was MCP-placed
  and is session-only until saved in-editor (Ctrl+Shift+S) — MCP cannot persist external
  actors.
- Scatter/claim overlaps are now player-only (IsPlayerControlled) so AI pawns don't chop
  trees or claim plots by walking around.

Verified seed 56 (wood tower, 19 pieces): raid 1 broke 1 piece, reached loot in 11.7 s;
raids 2-3 walked through the persisted hole in 6.7 s with 0 broken — raid damage is real
world damage, exactly the materialization model. Next rungs when wanted: N-seed headless
sweep, defender pawns (AIPerception patrol), per-tier piece HP.

## Environment pass — Megascans nature + atmosphere (working)

The island now reads as a real place: Megascans/Megaplants nature (free, claimed on
Fab by the user) + a lighting/post pass. Everything visual stays seed-safe: grass and
variant picks use a separate stream / id-hash, so existing seeds generate identical
worlds; GenVersion unchanged.

- **Trees**: Megaplants Norway Spruce + Common Hazel (skeletal, Nanite Assemblies).
  Attached as visuals on the scatter actors; the old cone stays as the invisible
  touch-trigger. Scale normalized from measured bounds per species (hazel is a bush —
  ~4.5 m, spruce 12-17 m). **Both `r.Nanite.AllowAssemblies=1` AND `r.Nanite.Foliage=1`
  are required** (DefaultEngine.ini [SystemSettings]) or the trees render as bare stems.
- **Rocks**: Nordic Beach Rock + Tundra Mossy Boulder meshes, bounds-normalized.
- **Grass**: ~7.8k instances across 4 HISM layers (4 Wild Grass variants), 14/cell on
  non-steep non-reserved cells, cull 70-140 m, no collision/shadow. Own random stream.
- **Ground**: Nordic Moss MI on the dynamic mesh, UVs tiled ~3 m via ScaleMeshUVs,
  green push via MID Albedo Tint. Gotcha: Mossy_Forest_Floor's MI parent is
  M_MS_Srf_**Trm** (transmission) — it renders a dynamic mesh invisible; use the
  plain M_MS_Srf surfaces.
- **Atmosphere** (map actors, tuned via MCP): sun pitch -32 warm 8 lux soft shadows,
  volumetric height fog, post: saturation 1.22, contrast 1.08, bloom 0.45, locked
  exposure, light vignette. UNSAVED until someone presses Ctrl+Shift+S in the editor —
  MCP cannot save map actors; same for the NavMeshBoundsVolume.
- Fab's Megaplants add enabled the ProceduralVegetationEditor plugin in the uproject;
  the editor's rebuild prompt after that is flaky — build from the command line.
- Cold first PIE after these assets: 1-3.5 min (shader + Nanite compile, one-time).

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

## C++ — toolchain works, worldgen/diff core ported

VS 2026 (MSVC 14.51) + Windows SDK + .NET 8 verified; UE 5.8 rejects VS 2022's MSVC 14.41
(UBT bans 14.40-14.43). Standing call: new logic in C++, Blueprint for marketplace borrows
and glue.

The worldgen/diff core now exists as reviewable C++ in `Source/Prototype/`:
`AGatersChunk` (terrain, rolls, category grid, resource zones, scatter, claim markers,
diff load/replay), `UGatersWorldDiff` (SaveGame), `AGatersScatter` / `AGatersClaimMarker`
(touch actors). A `CppTestChunk` instance sits in Lvl_GateGreybox at y=+40000, parallel to
the BP FarTerrain (which stays wired to the gate loop). Verified same as the BP version:
two PIE runs of seed 7 bit-identical (`SCATTER n=69 sum=4087639`), chop + claim recorded
to its own `CppWorldDiff_<seed>` slot and replayed on redial (`n=68`, same sum,
`replayed=1`, `claimed=1`). **GEN ms=25 vs the BP chunk's ~250** — the port is ~10x
faster, mostly from analytic heights: vertices are displaced by a pure `GroundHeight(x,y)`
function the analysis grid also samples, so no traces and heights are exact math per seed.

Deliberately not ported: base archetypes (base stamping replaces them), the lane-card gate
loop (BP glue). Numbers differ from the BP chunk (different draw order) — determinism is
per-implementation, which is fine; the BP chunk retires when the gate loop points at the
C++ one.

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

- Terrain builds at BeginPlay — invisible in the editor viewport, press Play to see it.
- The NavMeshBoundsVolume over the island is MCP-placed and session-only until someone
  saves the map in-editor (Ctrl+Shift+S).
