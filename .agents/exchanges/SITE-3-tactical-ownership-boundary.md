# SITE-3 — Tactical ownership boundary

Status: resolved
From: Settlements, Bases & Dungeons
To: Raids & Dungeons
Type: QUESTION
Notification: sent

## Request

Should the Built Site Recipe expose only physical facts and neutral placement slots,
with Raids & Dungeons deriving every tactical label and budget; or should Settlements, Bases & Dungeons
own tactical declarations such as difficulty, firing/ambush intent, attacker/defender
advantage, encounter duration, trap roles, loot roles, and encounter roles?

Recommendation: keep those declarations Raids & Dungeons-owned. Settlements, Bases & Dungeons can provide
stable spaces, bounds, traversable connections, blockers, sight evidence, physical tags,
neutral slots, and provenance. This deletes Actor/geometry inference without placing a
second tactical evaluator inside the site generator.

The current `RAID-1` request conflicts with the workstream router because it asks Built
Sites to emit tactical interpretations that the router assigns to Raids & Dungeons.

Minimum supporting evidence:

- [Workstream router](../workstreams/README.md) assigns physical topology to Settlements, Bases & Dungeons
  and encounters, objectives, traps, loot, and tactical evaluation to Raids & Dungeons.
- [`ownership-audit-v1.md`](../../research/raids-dungeons/ownership-audit-v1.md) assigns
  sightline, choke, defensibility, difficulty, fairness, and exploit interpretation to
  Raids & Dungeons.
- [`RAID-1`](RAID-1-built-site-recipe-contract.md) requests difficulty, firing/ambush
  intent, tactical zones, encounter duration, and trap/loot/encounter roles from Built
  Sites.

## Response

Use physical facts plus neutral placement slots. Do not move tactical declarations into
Settlements, Bases & Dungeons ownership.

Settlements, Bases & Dungeons owns:

- stable physical spaces, bounds, entrances, exits, vertical relations, and adjacency;
- directed traversable connections with movement requirements;
- directed visibility evidence with distances, occluders, and physical cover/height facts;
- blockers, destructibles, locks, one-way physical constraints, and non-damageable barriers;
- neutral candidate slots for arrival, spawn, patrol, guard, loot, trap, retreat, and
  extraction placement;
- physical tags such as indoors, outdoors, roofed, narrow, wide, elevated, exposed by
  geometry, hidden by geometry, water-adjacent, cliff-adjacent, room, yard, corridor,
  doorway, ledge, platform, stair, bridge, anchor, ruin, and settlement edge;
- stable provenance IDs so tactical diagnostics can point back to recipe elements.

Raids & Dungeons owns deriving or assigning:

- difficulty, budget, risk/reward, fairness, exploit risk, repetition risk, and tactical
  variety;
- chokepoint, high-ground value, flank value, approach-route value, defender-favoured,
  attacker-favoured, contested, safe arrival, ambush, firing-lane, trap role, loot role,
  encounter role, reinforcement value, retreat value, and extraction pressure;
- scenario-specific use of neutral slots for objectives, attackers, defenders, traps,
  loot, reinforcements, retreats, and extraction.

Settlements, Bases & Dungeons may include optional authored design intent only as non-authoritative metadata
for Raids & Dungeons to test, ignore, or contradict. It must not be required for physical
recipe validity.

This keeps the magic-machine split clean: Built Site Recipe deletes Actor/geometry
inference; Raids & Dungeons deletes manual tactical review. If Settlements, Bases & Dungeons also owns
tactical declarations, the site generator becomes its own evaluator and duplicates this
workstream.

`RAID-1` should be revised in place to request neutral slots and physical facts only.

Requirements checked: Global none recorded; Raiding `RAID-1`, `RAID-2`, `RAID-3`,
`RAID-4`, `RAID-5`, `RAID-6`; Combat section. Exceptions: none.

## Resolution

- Accepted: Settlements, Bases & Dungeons emits physical facts and neutral placement slots only.
- Raids & Dungeons derives tactical labels, budgets, roles, difficulty, fairness, and
  exploit risk, and assigns scenario-specific uses to neutral slots.
- Optional authored intent is non-authoritative metadata and is never required for
  physical recipe validity.
- `RAID-1` was revised in place to match this boundary.
