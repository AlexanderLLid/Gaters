# Worldgen Plan

Living greybox checklist for procedural place generation in the Unreal slice. Sibling of
[[prototype-plan]] (the raid loop) and [[unreal-progress]] (what exists in the project).
Not canon; update in place. Canon constraints it serves: play stays Gate-proximate — one
generated chunk carries a session ([[systems#Design pillars]]); frontier worlds are cheap
rows when empty and real places when visited ([[systems#Gates|FRONTIER-1]]).

## The filter [current call]

Only **feasibility unknowns** get built here. If a problem has a hundred known solutions
(a "shooting gun"), it is borrowed or stubbed, never prototyped. Test per item: could a
tutorial or marketplace asset settle it? Then it proves nothing — out.

## What generation must deliver for the raid prototype

The raid loop ([[prototype-plan]]) needs exactly this from worldgen, per seed:

- A terrain chunk a session can live on, with the Gate pad flat and clear.
- A base location that is reachable and readable (approach, entrances).
- Ground that knows itself: where bases can go, what is reserved, what is off-limits.
- A validation report per seed so bad seeds are data, not mysteries.
- The same seed always producing the same place — including what has changed there.

Everything below exists to make those five lines true.

## The stack — each layer feeds the next

1. **Frame** — one chunk around one Gate is the whole generated world. [decided, done]
2. **Ground** — one 300 m seeded heightfield at 128x128 resolution; explicit lowlands,
   mountains, canyon, and archipelago families; a clear Gate pad; water where the family
   calls for it. [done v2]
   - **Ground is immutable [current call].** Nothing edits terrain after generation —
     not the generator, not players. Bases and content conform to the ground via
     foundation skirts (built down from a level floor to meet dirt); a **max-drop rule**
     in the spot test rejects sites that would need too much stilt. Terrain stays a pure
     function of the seed, so world reconstruction is exact math and diffs only ever
     describe objects. Bases use foundation skirts on the raw terrain; only the Gate has
     a generated clearing/platform.
   - Tunables: family shape parameters, chunk size/resolution, Gate clearing, water
     height, base search ring, footprint radius, max foundation drop.
3. **Categories** — the ground classifies itself: flat / slope / steep, buildable plots,
   reserved zones, water, **resource zones** — regions where harvestable scatter
   clusters. [done v2] [current call: richer tags (high ground, chokepoints) wait for a
   raid evaluator that reads them]
4. **Dressing with stable identity** — native greybox scatter (cone/sphere asset slots)
   where every instance has a deterministic seed-derived ID. [done v2 — ID = grid cell
   index; imported foliage is not part of generation]
   - The prototype question is **identity, not looks**: tree #47 is the same tree every
     regeneration. Pretty forests are a content pass (PCG), not a feasibility question.
5. **Persistence: seed + change-list** — the core unknown. [proven — two diff types]
   - Dial the same address twice → the same world, including what changed. Regenerate
     from seed, replay recorded diffs (tree #47 harvested, plot #3 claimed).
   - Proven: chop and claim entries in one per-seed SaveGame slot → redial replays both;
     survives editor restart; per-seed slots isolate cleanly ([[unreal-progress]]).
6. **Base stamping** — assemble real bases on buildable plots from modular pieces.
   [done v1: EBS pieces, three archetypes, terrain-aware site search]
7. **Content passes** — PCG visual dressing, biome art, creatures, and resources with
   real yields. [defer — content, not feasibility; greybox water surface exists]

## Feasibility unknowns (the actual prototype list)

- **U1 — stable scatter identity.** Same seed → same instances with same IDs, every time.
  [proven — ID = grid cell index; checksum + named instances identical across runs]
- **U2 — seed + diff reconstruction.** Destroy something, tear the world down, regenerate,
  replay the diff: the change is still there. One trivial diff type is enough to prove it.
  [proven — chop diff replays across PIE restarts and editor restart]
- **U3 — ground self-knowledge as gameplay data.** Categories and plots readable by game
  logic, not just visuals. [proven v1 — plots array + report]
- **U4 — generation budget.** A full seed (ground + analysis + scatter) must build fast
   enough to feel like "dialing", not "loading". Measure per seed; the report gets a
   generation-time line. Budget is a tunable, not a hope. [open after v2 — the rendered
   seed sweep is stable; remeasure warm PIE before setting the budget]
- **U5 — deterministic variety.** Nearby seeds must produce unrelated worlds. [proven —
  four family tests + 128-seed base-site sweep; keep both when generation changes]

## Borrow or stub (solved elsewhere — never prototype)

- Destructible-tree mechanics, harvest interactions, falling props (Valheim/Rust-proven).
- Scatter rendering at scale (instanced meshes; PCG when visuals matter).
- Water simulation, sky, weather, biome art, foliage.
- Building mechanics (Easy Building System V10 is the designated borrow, [[unreal-progress]]).
- AI navigation and defenders.
- Save-file plumbing (the diff format is ours; writing files is not).

## Defer

- Streaming / World Partition (a chunk fits in memory whole — the pillar guarantees it).
- Multi-gate worlds, multiple chunks, planet scale.
- Server authority, real DB rows, network replication.
- Terrain deformation by players (not part of the fantasy; building is).

## Order to develop — and the check that gates each step

1. [x] Ground: presets, plinths, seed variety. — _Check: seed sweep, distinct sites._
2. [x] Site analysis: categories, plots, report. — _Check: plains many plots, broken few,
       plinths marker-free._
3. [x] Resource zone tag in the category grid. — _Check: report counts resource cells;
       zones land on valid ground only._
4. [x] Greybox scatter with stable IDs (in resource zones + sparse elsewhere). — _Check:
       two runs of one seed list identical IDs/positions; report line `SCATTER n=`._
5. [x] Diff record + replay: destroy scatter instance → diff entry; regenerate → replayed.
       — _Check: "chop tree #47, redial, #47 still gone"; diff survives editor restart._
6. [x] Claim diff: mark a plot claimed via the same diff path. — _Check: claim survives
       regeneration; proves the diff format carries more than one change type._
7. [x] Generation budget: time the full build per seed, print it. — _Check: within budget
       tunable across presets; if not, that is real feasibility data._
8. [x] Base stamping + explicit environment families + water-aware placement. — _Check:
       seeds 0/2/4/7 produce mountains/archipelago/lowlands/canyon, each with a valid base._
9. [ ] Stop worldgen work until raid evaluation exposes a missing terrain tag or invalid
       site; PCG art remains content work.

## Validation report (extends the existing per-seed print)

`SITE environment= water= base_valid= base= drop=` / `PLOTS ... water=` / `BASE` /
`SCATTER` / `DIFF` / `STAMP` / `GEN ms=`. `Scripts/RunEnvironmentSweep.ps1` runs seed
sets through the real map and prints those lines; `-CaptureGallery` also writes one
fixed-camera image per seed. A failing line names the layer; the gallery catches shapes
that pass numeric checks but still read flat.

### Why / rejected

- **Why persistence is in scope:** it is the one worldgen claim Gaters depends on that no
  marketplace asset or tutorial proves — revisitable procedural worlds reconstructed from
  seed + change-list. Rejected: "assume it's just saving data" — the identity/replay
  coupling with regeneration is exactly where it can silently fail.
- **Why identity-not-looks for dressing:** all-trees-destructible is industry-proven
  (Valheim, Rust, Fortnite); only stable identity across regeneration is ours to prove.
- **Why explicit families:** traversal needs different shapes, not one Perlin surface with
  a larger amplitude. Rejected: more presets that only retune magnitude/frequency.
- **Rejected — proving PCG now:** tool choice, not feasibility; it joins at the content
  pass driven by the same seed and categories.
- **Why immutable ground / rejected terrain-bending:** flattening terrain under bases
  grooms the fight surface but turns terrain into state — every rebuild must re-apply
  every edit, which is exactly what the persistence proof does not want. Foundation
  skirts plus the max-drop placement rule solve the same fit problem from the base's
  side, work identically for generated and player building, and close the under-floor
  gap exploit with a visual apron. Bases conform to the world, never the reverse.
