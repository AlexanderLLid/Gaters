# RAID-1 — Built Site Recipe tactical contract

Status: resolved
From: Raids & Dungeons
To: Settlements, Bases & Dungeons
Type: CONTRACT
Notification: sent

## Request

Provide the smallest versioned Built Site Recipe contract that lets Raids & Dungeons
place raid/dungeon gameplay into physical layouts without owning or recreating building,
base, village, or dungeon topology generation.

The recipe must be pure data and must not require direct Unreal Actor access. Minimum
needed fields:

- recipe identity: `siteId`, `siteVersion`, generator version, seed, site kind, and
  ownership context;
- spaces: stable IDs for rooms, yards, ledges, platforms, exterior zones, and anchor or
  arrival areas;
- routes: directed movement links with traversal requirements, width/headroom/step/jump
  constraints, tags, and stable blocker references;
- visibility: directed sight links with distance, occlusion/blocker references, and
  physical height or cover facts;
- blockers: stable IDs for doors, walls, wards, gates, barricades, destructibles, locks,
  one-way gates, and non-damageable blockers;
- neutral placement slots: candidate arrival, spawn, patrol, guard, objective, loot, trap,
  reinforcement, retreat, and extraction nodes with physical constraints only;
- physical tags: indoors, outdoors, roofed, narrow, wide, elevated, exposed by geometry,
  hidden by geometry, water-adjacent, cliff-adjacent, room, yard, corridor, doorway,
  ledge, platform, stair, bridge, anchor, ruin, and settlement edge;
- physical budgets: site area, route count, slot count, blocker count, and optional
  authored design intent as non-authoritative metadata;
- provenance: stable source IDs that let diagnostics point back to recipe elements.

Raids & Dungeons returns a structured evaluation result containing:

- evaluator, Combat, site, scenario, seed, and run identity;
- tactical viability, difficulty, variety, fairness, exploit-risk, repetition-risk, and
  diagnostic-confidence scores;
- policy/scenario outcome matrices and terminal reasons;
- causal findings with implicated stable recipe IDs;
- route, visibility, encounter-role, trap, loot, reinforcement, retreat, and extraction
  findings;
- recipe-stable adjustment hints;
- canonical run traces or links to immutable evidence artifacts.

Boundary: Settlements, Bases & Dungeons owns physical topology, neutral slots, and structural validity.
Raids & Dungeons owns scenarios, tactical labels, tactical roles, traps, loot risk,
simulation, scoring, budgets, and failure explanations. Combat & Classes owns movement,
attacks, spells, AI capabilities, and balance assumptions. Primary Builder owns
terrain/environment simulation and shared Unreal integration.

Minimum supporting evidence:

- [`ownership-audit-v1.md`](../../research/raids-dungeons/ownership-audit-v1.md)
  separates existing site and raid work into Settlements, Bases & Dungeons versus Raids & Dungeons ownership.
- [`SITE-3`](SITE-3-tactical-ownership-boundary.md) resolves that Settlements, Bases & Dungeons exposes
  physical facts and neutral placement slots while Raids & Dungeons derives tactical
  labels and budgets.
- [`BASE-1`](BASE-1-headless-combat-contract.md) proves the current headless resolver can
  consume pure tactical graph data and return deterministic diagnostics.
- Settlements, Bases & Dungeons needs stable tactical feedback before generated sites can be judged as good,
  repetitive, unfair, or exploitable rather than merely physically valid.

## Response

Built Site Recipe v1 is implemented as pure C++ data in `GatersBuiltSiteRecipe.h`.

The production C++ consumer seam is
`FGatersBuiltSiteLayerResult::SiteRecipes`. `FGatersBuiltSiteLayer::Generate` returns
accepted recipes beside the optional World Recipe nodes and returns an empty array in
world-only mode. It does not require spawned Actors. No cross-process JSON serializer is
part of v1; request that separately if the Python tactical harness must ingest runtime
recipes directly.

- Identity: contract version, site version, generator version, seed, site ID, site kind,
  and site area. Runtime ownership context is intentionally external instance state.
- Spaces: stable ID, center, extent, semantic building role, physical tags, and source
  IDs. A zero extent records a proved point or centerline, not a usable volume.
- Connections: stable directed endpoints, optional width/headroom/step/jump facts,
  blocker references, physical tags, and source IDs. Zero means topology is known but
  that clearance fact is not asserted.
- Visibility: stable directed endpoints, distance/height facts, blocker references,
  physical tags, and source IDs. The array may be empty when the source has no evidence.
- Blockers: stable ID, positive physical envelope, physical tags, and source IDs.
- Placement slots: stable ID, containing space, position, positive clearance radius and
  height, physical tags, and source IDs. Slots have no scenario or tactical role.
- Validation rejects invalid identity/version data, duplicate IDs, missing provenance,
  broken references, non-finite or negative physical values, unusable slot clearance,
  and slots outside their containing spaces.
- Canonical text uses length-prefixed strings; checksum identity covers the canonicalized
  recipe content.

The current settlement adapter truthfully emits terrain path centerlines, modular
building envelopes, semantic building roles, directed adjacency/entrance topology, and
provenance. It emits no placement slots, visibility, blockers, or traversal clearance
because the accepted village source does not yet prove collision-free interior volume,
occlusion, or passage dimensions.

Raids & Dungeons may consume any present fact and must treat omitted or zero-valued
optional physical evidence as unknown. It owns all scenario use, tactical labels,
budgets, fairness, difficulty, exploit risk, traps, loot, and encounter roles.

Verified evidence:

- `Gaters.BuiltSites.Recipe`: 2/2 passed.
- `Gaters.BuiltSites.SettlementAdapter`: 2/2 passed.
- `Gaters.Worldgen.BuiltSiteLayer`: 3/3 passed.
- Full `Gaters` automation: 92 passed, 0 failed or skipped.

## Resolution

Accepted by Raids & Dungeons.

- Source verification confirms `FGatersBuiltSiteRecipe` is pure versioned data with
  stable identity, directed physical relations, neutral placement slots, provenance,
  causal validation, canonical text, and checksum identity.
- `FGatersBuiltSiteLayerResult::SiteRecipes` is the production C++ consumer seam and
  remains independent of spawned Actors.
- Runtime ownership context remains separate instance data. Raids & Dungeons accepts
  that split and combines it with Rift access/scenario state rather than requiring it in
  the physical recipe definition.
- Omitted arrays and zero-valued optional physical facts are unknown, not evidence of
  absence, clearance, visibility, or tactical suitability.
- Fresh verification passed: build succeeded;
  `Gaters.BuiltSites.Recipe` 2/2;
  `Gaters.BuiltSites.SettlementAdapter` 2/2;
  `Gaters.Worldgen.BuiltSiteLayer` 3/3; and full `Gaters` automation 94/94.
- Contract closure does not claim that the current settlement recipe is raid-ready: it
  intentionally emits no placement slots, visibility, blockers, or traversal clearance.
  Cross-process Python ingestion and richer proved physical facts are separate follow-up
  work.
