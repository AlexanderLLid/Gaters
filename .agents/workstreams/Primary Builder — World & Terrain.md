# Primary Builder — World & Terrain

## Current objective

- Advance `world.environment-generator` toward broad Earth-like environment coverage.
  Climate Field v1 is verified and composed through Environment Recipe v2. Drainage
  Network v1 now proves the pure terrain-plus-precipitation graph and independently fits
  its cells, basins, terminals, channels, inlets, and outlets to declared regional water.
  It now derives deterministic neutral river-system, declared-lake, wetland, delta, and
  waterfall feature candidates and composes the accepted drainage graph, water fit, and
  feature recipes into Environment Recipe v5. Surface Condition Field v1 turns that root
  into continuous neutral ground evidence; Biome Opportunity Field v2 now combines it
  with accepted biome and climate evidence. Version 5 stores both recipes and exposes
  their authoritative queries. Content Cell Recipe v6 consumes that root directly and
  turns opportunity magnitude into deterministic semantic placement density. An
  independent evaluator now measures distribution before assets. Landmark/travel use,
  terrain carving, and rendering remain later adapters.
  Sparse/disconnected archipelago control remains the next
  landform frontier. Built Sites remain an optional downstream layer and are not inputs
  to terrain, climate, or `world.environment-generator`.

## Owned outputs

- Terrain, biomes, water, vegetation, resources, seeds, streaming, candidate-site
  discovery, environmental constraints, and shared world integration.
- Shared Unreal integration and `research/machines.json` changes after specialist evidence
  is handed over through an exchange.

## Boundaries

- Settlements, Bases & Dungeons owns parcels, paths, districts, modular buildings, villages, outposts,
  bases, settlement growth, and physical dungeon layouts.
- Raids & Dungeons owns encounters, objectives, attackers, defenders, traps, loot, and
  tactical simulation/evaluation.
- This workstream owns environmental inputs and shared integration, not built-site recipe
  iteration after the handoff.

## Evidence

- [`World layer separation and biome field v1`](../reports/world-layer-and-biome-v1.md)
  records world-only/merged runtime, visual, automation, and machine evidence.
- [`Village growth v1`](../reports/village-growth-v1.md) records the implementation and
  verification being transferred to Settlements, Bases & Dungeons.
- [`Settlement vertical slice v1`](../reports/settlement-vertical-slice-v1.md) records
  terrain-site integration and held-out settlement evidence.
- World-only and merged seed `7` runtime modes both produce valid recipes; world-only has
  zero built-site modules and zero stamped base pieces.
- `Gaters.Worldgen.Biomes` passes deterministic, bounded, continuity, water-evidence,
  held-out variety, arrival compatibility, regional intent, and dry-override checks.
- Content Cell Recipe v6 uses one shared root-aware generator path, records accepted
  Environment Recipe and Biome Opportunity provenance, and adds deterministic opportunity
  rejection so high and low opportunity no longer fill equal budgets. Runtime streaming
  passes the root instead of reconstructing evidence. Its focused suite passes `8/8`. The new
  independent Content Distribution Evaluator rejects constant-density and scarcity
  counterexamples, then passes `3/3`; a 66-cell held-out root sweep records `0.056` mean
  density error, `0.006` kind-mix error, and `0.781` opportunity-density correlation.
  `Gaters.ContentOpportunities [seconds]` is registered to draw loaded opportunity spheres
  and actual placement dots without altering recipes or assets; its visual output still
  needs a human Editor check. Worldgen is `106/107` and complete Gaters is `143/146`;
  only the three active Settlements-owned RED contracts fail.
- `FGatersEnvironmentRecipeCompiler` now composes the accepted terrain, regional intent,
  regional water, biome, and neutral-opportunity layers behind one pure-data root.
  `AGatersChunk` compiles it once and passes it into streamed terrain cells instead of
  letting each cell reconstruct global environment state. Site and traversal analysis
  now build their semantic grid from the same regional intent as rendered terrain.
- Two environment-recipe tests pass across seeds `0`, `2`, `4`, `7`, and `53`, including
  direct-query parity and causal provenance counterexamples. Related focused automation
  passed `17/17`; the fresh complete Gaters suite passed `94/94`.
- A fresh post-compiler seed-`7` world-only runtime generated a valid `357`-node recipe,
  nine streamed terrain cells, `50.50%` traversal coverage with pad escape and base
  access, four valid planned sites, and valid performance evidence at `162.8 ms`
  generation time.
- World Intent recipe v2 lets the seed declare sparse, forestless, and no-content regions,
  preserves the existing arrival profile, and places deterministic terrain/hydrology
  overrides outside it. Streamed terrain consumes those overrides through a smooth pure
  adapter. Regional wet profiles now emit bounded stable water surfaces while dry profiles
  emit none. A pure physical-fit evaluator rejects missing surfaces, dry leaks, wrong
  water datums, escaped bounds, and surfaces without submerged terrain evidence while
  explicitly ignoring unfinished water art. Biome queries now consume the same smooth
  regional terrain/hydrology intent and content cells record those regional biome facts;
  that earlier stack passed `90/90` before the compiler tests expanded the suite.
- `world.environment-generator` is the top World & Terrain machine. It is active, not
  complete: climate, landform-process promotion, drainage,
  surface conditions, ice, and complete resource loops remain planned dependencies.
- Seed `53` exposed a real cross-layer failure: a declared river emitted water without
  river-cut terrain outside the canyon family. Generic river hydrology now contributes
  deterministic channel relief for every terrain family, and root validation rejects
  unsupported water evidence.
- `FGatersEnvironmentBriefCompiler` compiles eight bounded global/regional physical
  target signals from a seed without named world generators or downstream site, Anchor,
  Rift, encounter, species, or asset decisions. Its focused contract passes `2/2`; the
  fresh complete Gaters suite passes `96/96`. This is target intent only: current terrain
  is not claimed to satisfy cold, ice, wetland, or volcanic requests yet.
- `FGatersLandformProcessField` is an isolated challenger over the current terrain. Its
  versioned pure recipe exposes separately accountable relief, uplift, volcanic, and
  glacial contributions with smooth regional influence; a generic terrain evaluator can
  score either champion or challenger height queries without knowing their implementation.
  Focused contracts pass `3/3`; an off-by-default `Gaters.Landforms <0|1>` preview now
  composes the accepted environment root while preserving global process coordinates
  through regional terrain profiles. The fresh complete shared suite passes `103/103`.
  Runtime traversal, held-out coverage, galleries, and budget evidence remain open, so
  the terrain-family generator is still the default champion.
- Seed `7` world-only runtime comparison is now captured in both modes. The challenger
  increases measured relief from `3149` to `4577` while traversal stays effectively flat
  (`50.50%` to `50.44%`), preserves Arrival escape and base access, and adds about `6 ms`
  in the comparable nine-cell headless run (`281.1` to `287.2 ms`). Both fixed-camera
  beauty/traversal captures pass image validation; the overview difference is subtle, so
  broader held-out briefs and closer diagnostic views are still needed.
- SITE-5 shared integration now delegates legacy hut/compound/tower topology to the pure
  `FGatersLegacyBaseLayer` while `AGatersChunk` retains only EBS asset resolution,
  catalog composition, spawning, streaming, and diff replay. The embedded topology and
  hardcoded raid-loot marker are removed. Fresh focused automation passed `2/2`, the
  complete suite passed `111/111`, merged seed `7` preserved `99` stamped pieces, and
  world-only seed `7` preserved `0`.
- Environment Brief Compiler version `2` declares deterministic walkable-land and
  connected-land targets alongside its physical signals. Landform recipe version `5`
  exposes deterministic candidate identity, explicit feature scale, and bounded generic
  terrain-dissection scale. Candidate zero preserves the prior recipe with no dissection.
  `world.environment-candidate-selector` result version `4` independently scores
  full-world land coverage, coarse world access, high-resolution Arrival escape, and
  absolute-plus-relative target fidelity while preserving selected and best recipes
  separately. Unsatisfied selection retains the terrain-family champion with causal
  evidence.
- Runtime landform selection protects exactly one neutral region: Arrival. It does not
  query optional base discovery, site routes, villages, landmarks, or raids; a source
  regression enforces that boundary. Terrain Semantic Field version `2` makes zero pad
  radius preserve raw terrain and derives a local pad transition from pad radius rather
  than a fixed distance. The seed-`11` world-only no-route regression failed before this
  change and passes afterward. This treatment belongs to the replaceable terrain adapter,
  not the landform process or Built Sites.
- Landform recipe version `6` adds target-derived ruggedness without a named-world branch:
  seeded broad massifs and rounded internal ridges are cut by a generic erosion valley,
  fade at the water datum, and respect only the neutral Arrival protection region.
  Candidate zero remains the exact terrain champion and candidate seven retains the
  proven pre-ruggedness fallback. Cellular spikes, smooth domes, and an independent
  diagonal lattice were rejected by visual or held-out evidence rather than promoted.
- The fresh 20-case held-out sweep at
  `Unreal/Prototype/Saved/EnvironmentRuns/landform-natural-final-20260721-200000.jsonl`
  selects `2/20` candidates versus the frozen `1/20` baseline, reduces mean best target
  error from `0.3930` to `0.3027`, raises best-candidate coarse world access from `16/20`
  to `18/20`, and records `20/20` Arrival escape and performance validity. Seed `83`
  high-relief repeats candidate `4`, checksum `D9C773F3`, walkable `0.7382`, and connected
  `0.9104` in the lit gallery capture. The complete Gaters suite passes `121/121`.
  `world.landform-process-field` remains active: the natural-process geometry is a useful
  greybox direction, not final art, and sparse/disconnected archipelago intent still needs
  its own physical-process challenger. Optional sites remain outside selection.
- `FGatersClimateField` version `1` compiles deterministic prevailing wind and
  seasonality provenance from the accepted Environment Brief and Landform recipe, then
  exposes bounded temperature, precipitation, wind exposure, seasonality, freeze-thaw,
  regional influence, and terrain height at any coordinate. A pure evidence evaluator
  owns the climate math; its thin terrain adapter samples the existing height field and
  reuses the Landform field's one authoritative regional-profile blend. It chooses no
  biome, resource, asset, site, or runtime Actor. A held-out matrix first rejected
  identical maritime and continental seasonality; local-water damping and dry-land
  amplification corrected it. Five real seeded terrain fields now preserve exact accepted
  height and bounded deterministic climate output. Climate tests pass `4/4`, the focused
  dependency suite passes `15/15`, and the fresh complete Gaters suite passes `125/125`.
  `world.climate-field` is verified as `climate-field-1-physical-evidence`; root/runtime
  consumption now occurs through Environment Recipe v2.
- Environment Recipe v2 stores the compiled Environment Brief and Climate recipe, exposes
  `QueryClimate`, validates their joint provenance, and accepts the selected Landform
  recipe explicitly from `AGatersChunk`. Climate queries sample the root's accepted terrain
  exactly once, so disabled or rejected challengers cannot silently change climate height.
  Root/Climate/Selector/Landform automation passes `17/17`; the complete suite passes
  `125/125`. A fresh seed-`83`, world-only, selected-landform headless run reports
  `CLIMATE root=2 field=1 valid=yes` with bounded physical signals and preserves valid
  World Recipe and performance evidence. This adds data evidence only, not climate art.
- Root-authority review found that the first Climate and Drainage adapters sampled raw
  `FGatersEnvironment::HeightAt` even though regional environmental terrain is composed by
  `EnvironmentRecipe.QueryTerrain`. Both now consume one injected height sampler; root
  adapters pass `QueryTerrain(...).Height`, while the raw Environment Climate overload is
  retained only as an isolated thin adapter. A regression proves at least one regional
  point differs from raw terrain before asserting root parity. Focused root/Climate/
  Drainage tests pass `9/9` and the complete suite remains `128/128`. Workflow review
  classified this as execution, not a missing skill rule.
- `FGatersDrainageNetwork` version `1` samples accepted root terrain and climate into a
  bounded deterministic D8 graph with precipitation accumulation, stable basin/segment
  identity, channel evidence, and waterfall-drop candidates. It permits equal-height flow
  only toward the boundary, preventing flat plateaus from becoming false lakes while true
  enclosed depressions remain sinks. It validates only the terrain/climate provenance it
  consumes, so unrelated water metadata cannot block isolated drainage analysis. Five
  real seeds preserve exact authoritative regional-root height and precipitation evidence. Drainage tests
  first passed `3/3`, with `16/16` focused and `128/128` complete shared evidence before
  the water-fit gate below extended the contract.
- Drainage-to-water fit version `1` preserves both source recipes and records each water
  surface's grid cells, basins, reached terminals, channel contacts, inlet/outlet counts,
  accumulation, and submerged-terrain support. Its first held-out run exposed that the
  old water evaluator and generator shared an unblended regional-profile assumption:
  seeds `11` and `29` declared lakes where authoritative root terrain stayed above datum.
  Water validation now injects `QueryTerrain`, and regional lake terrain/surfaces consume
  one radius-scaled footprint while global lakes remain unchanged. Seed `83` then proved
  33-by-33 evidence was too coarse; 65-by-65 is the first tested resolution that fits all
  declared water for seeds `11`, `29`, `47`, `83`, and `131` without relaxing datum
  tolerance. Worldgen first passed `93/93` and the complete suite passed `131/131` before
  the feature compiler extended this evidence.
- Drainage Feature Recipe version `1` compiles only accepted physical facts into neutral
  river-system, declared-lake, wet low-drop basin, supported ocean-delta, and waterfall
  candidates. It preserves seed plus drainage, regional-water, fit, basin, cell, segment,
  and surface provenance; it chooses no art, site, encounter, carving, or runtime Actor.
  Synthetic fixtures prove every supported kind and dry fixtures prove valid total
  scarcity. Five real roots repeat exactly and reference only accepted source evidence at
  65-by-65 resolution. Drainage passes `9/9`, Worldgen passes `96/96`, and the complete
  suite passes `134/134`. `world.drainage-network` remains active because richer
  depression handling, hierarchical/local scale, terrain carving, and rendering remain
  unverified.
- Environment Recipe version `3` now stores drainage, water-fit, and drainage-feature
  recipes after terrain, intent, regional water, environment brief, and climate. Root
  validation independently rebuilds and compares every derived layer from its recorded
  settings. The first 65-by-65 composition failed existing 400000-unit fixtures while
  their independent water evaluator remained valid; 129-by-129 is the first bounded
  challenger passing seeds `0`, `2`, `4`, `7`, `53`, selected-landform `83`, and held-out
  roots `11`, `29`, `47`, `83`, and `131` without changing water tolerance. Environment
  Recipe passes `3/3` and Drainage Network passes `9/9`.
- The fresh rebuild activated three Settlements-owned RED tests added after the prior
  shared binary: `Gaters.Worldgen.Settlement.Contract` and both
  `Gaters.BuiltSites.SettlementAdapter` tests. Worldgen is `96/97` and complete Gaters is
  `132/135`; all Environment and Drainage tests pass. Primary did not modify the active
  Settlements-owned facade/path/access work.
- Surface Condition Field version `1` compiles a settings/provenance recipe, then derives
  bounded soil, rock, sediment, sand, snow, ice, saturation, shore, ridge, valley, and
  cliff evidence from authoritative terrain, climate, drainage, and declared water. The
  adapter bilinearly interpolates drainage so queries do not snap at grid boundaries and
  contains no terrain-family branch. Pure fixtures causally exercise steep exposure,
  wet accumulated valleys, freeze-thaw weathering, gentle shores, cold wet surfaces, and
  valid dry scarcity. Five held-out roots repeat across `125` coordinates with valid
  source-cell provenance. Environment Recipe version `4` now stores that recipe,
  independently recompiles it from recorded settings, and exposes exact root-query parity
  without recursive validation. Focused Surface Condition passes `4/4` and Environment
  Recipe passes `3/3`; the fresh broad results are Worldgen `100/101` and complete Gaters
  `136/139`, with only the same three active Settlements-owned RED contracts failing.
  Materials, final beach/ice geometry, resource use, and terrain carving remain unverified.
- Biome Opportunity Field version `2` replaces named moist-flat/high-ground heuristics at
  the accepted root with explicit biome, climate, and surface-condition evidence. Pure
  fixtures prove soil/precipitation vegetation support, exposed-rock stone/landmark
  support, frozen and saturated travel friction, water exclusion, and valid barren
  scarcity. Environment Recipe version `5` stores its provenance recipe and delegates
  exact physical queries. Focused opportunities pass `2/2`, Environment Recipe passes
  `3/3`, Worldgen is `101/102`, and complete Gaters is `137/140`; only the three active
  Settlements-owned RED contracts fail. Content-cell density, resource identities,
  assets, and visual differentiation remain unverified.
- Seed `131` exposed circular archipelago stamps that determinism, traversal, and coarse
  topology checks could not reject. A pure contour-morphology evaluator now records
  closed-component size and radial circularity across arbitrary height bands. The
  archipelago candidate uses scale-relative deterministic deformation, removes its
  dedicated base-shelf stamp, and separates satellites without site dependencies.
  Focused morphology and Environment suites pass: seed `131` falls from `0.931` to
  `0.800` maximum circularity, retains a valid base site, and the 28-seed sweep preserves
  four-island layouts with 28 coastline signatures. World-only runtime preserves Arrival
  escape, base access, three valid routes, and performance validity. Human visual
  acceptance is recorded; the fresh Worldgen suite passes `76/76` and the complete
  shared suite passes `114/114`. `world.terrain-generator` is verified as
  `generator-10-morphology`. The broader morphology evaluator remains active until it
  proves it does not reject intentionally radial terrain such as requested craters or
  atolls.

## Exchanges

- [`ART-2 — Style-neutral requirement integration`](../exchanges/ART-2-style-neutral-requirement.md)
  — answered; global machine contracts now require recognizable, coherent output without
  fixing realism, proportions, shading, detail, or final visual style. Experimental
  champions remain evidence, not global art direction.
- [`WORLD-5 — Optional village scarcity counterexample`](../exchanges/WORLD-5-optional-village-scarcity.md)
  — resolved; optional Village and Landmark absence is valid scarcity, while a versioned
  site brief declares which stable site and route IDs are required. Primary owns the
  planner correction; the Built Site layer and environment selector remain unchanged.

- [`RAID-6 — World-to-site approach seam`](../exchanges/RAID-6-world-to-site-approach-seam.md)
  — answered; Primary accepts a pure neutral composer, but will not infer site edges
  from generic outdoor corridors.
- [`RAID-7 — Combat simulation registry integration`](../exchanges/RAID-7-combat-simulation-registry-integration.md)
  — answered; `runtime.combat-simulation` is active with the deterministic headless
  resolver champion. Settlement ensemble evidence remains analysis-only.
- [`WORLD-4 — Site-edge connector contract`](../exchanges/WORLD-4-site-edge-connector-contract.md)
  — open; requests stable neutral site-edge connector evidence from Settlements before
  shared world-to-site approach composition.

- [`WORLD-1 — Optional Built Site layer`](../exchanges/WORLD-1-optional-built-site-layer.md)
  — resolved and integrated; world-only and merged modes verified.
- [`SITE-2 — Village system handoff`](../exchanges/SITE-2-village-system-handoff.md)
  — resolved; ownership accepted by Settlements, Bases & Dungeons.
- [`SITE-4 — Legacy base layer handoff`](../exchanges/SITE-4-legacy-base-layer-handoff.md)
  — resolved; pure topology and the shared runtime adapter are integrated with ownership
  preserved.
- [`SITE-5 — Pure legacy base layer integration`](../exchanges/SITE-5-pure-legacy-base-layer-integration.md)
  — resolved; merged/world-only parity and shared automation are green, with no tactical
  loot semantics retained in the physical layer.
- [`LORE-1 — Fantasy Rift integration`](../exchanges/LORE-1-fantasy-rift-integration.md)
  — resolved; integrated and accepted by Lore.
- [`CHAR-1 — Generated character pipeline contract`](../exchanges/CHAR-1-character-pipeline-contract.md)
  — resolved; the specialist ownership and editor/runtime integration boundary are
  accepted.
- [`CHAR-2 — Unreal humanoid intake handoff`](../exchanges/CHAR-2-unreal-humanoid-intake.md)
  — resolved; the verified topology adapter is accepted as editor/cook-time
  infrastructure for later shared runtime adoption, without machine promotion.
- [`CHAR-3 — FullBodyIK and mechanical motion handoff`](../exchanges/CHAR-3-fullbodyik-motion-handoff.md)
  — resolved; deterministic editor/cook-time motion and native evaluated-IK evidence are
  accepted as a foundation only, without shared runtime integration or machine promotion.
- [`CHAR-4 — Native CharacterMovement flat-route handoff`](../exchanges/CHAR-4-native-character-movement-handoff.md)
  — answered; deterministic development-runtime flat-route evidence is accepted as the
  foundation for the uneven-terrain Foot Placement challenger. No shared-source change
  or machine promotion is warranted.
- [`CHAR-5 — Native uneven-terrain Foot Placement handoff`](../exchanges/CHAR-5-native-foot-placement-handoff.md)
  — resolved; deterministic development-runtime reference-pose grounding evidence is
  accepted narrowly. No shared-source integration, registry change, or promotion is
  warranted.
- [`CHAR-6 — CreatureDNA Houdini guide proof`](../exchanges/CHAR-6-creature-dna-houdini-proof.md)
  — answered; the structural proof is accepted as isolated Character-owned evidence
  behind the existing tool-neutral boundary, without integration or promotion.
- [`CHAR-7 — Procedural head and deformation proof`](../exchanges/CHAR-7-procedural-head-deformation-handoff.md)
  — answered; staged mechanical contracts are accepted narrowly. No shared Unreal or
  registry integration and no visual, production-body, runtime, or species promotion.
- [`CHAR-8 — Complete BodyPlan registry correction`](../exchanges/CHAR-8-body-plan-registry-correction.md)
  — answered; only the Body Plan compiler challenge now names the verified 11-node
  explicit-terminal fixtures. Machine status, graph, champion, gate, and limits remain
  unchanged.
- [`WORLD-2 — Built Site Recipe counterexample failure`](../exchanges/WORLD-2-built-site-recipe-counterexample-failure.md)
  — resolved as an invalid combined counterexample fixture; production validation was
  unchanged and the latest shared full suite is green.
- [`WORLD-3 — Built Site blocker and clearance failure`](../exchanges/WORLD-3-built-site-blocker-clearance-failure.md)
  — resolved; Settlements corrected its physical-clearance evidence, focused Built Sites
  passed `9/9`, and Primary independently confirmed the complete suite at `114/114`.
- [`SITE-6 — Settlement physical-evidence registry integration`](../exchanges/SITE-6-settlement-physical-evidence-registry-integration.md)
  — answered; only `world.settlement-generator` received the verified physical-recipe
  evidence and falsified full-footprint guarantee. It remains active without new
  dependencies or promotion.
- [`SITE-7 — Settlement access-fit registry integration`](../exchanges/SITE-7-settlement-access-fit-registry-integration.md)
  — answered; the settlement champion remains active, but its registry gate now requires
  every usable indoor slot to have a declared applicable route or explicit neutral seal.
