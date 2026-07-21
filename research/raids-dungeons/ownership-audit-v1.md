# Raids & Dungeons ownership audit v1

## Call

- This workstream owns raid and dungeon gameplay evaluation, not site generation.
- Settlements, Bases & Dungeons owns villages, buildings, bases, and physical dungeon topology.
- Primary Builder owns terrain, environmental simulation, shared Unreal integration, and
  machine registry integration.
- Combat & Classes owns movement, attacks, spells, AI capabilities, and balance
  assumptions.

## Existing work split

- Transfers to Settlements, Bases & Dungeons:
  - settlement and village generation;
  - building module generation;
  - physical base and dungeon topology;
  - path, entrance, parcel, room, corridor, doorway, and structure placement;
  - layout validity that does not require raid gameplay, such as structural support,
    terrain fit, reachability of entrances, duplicate modules, and visible materialization.
- Remains with Raids & Dungeons:
  - raid objectives and encounter composition;
  - attacker, defender, trap, reinforcement, retreat, and extraction rules;
  - loot placement risk/reward;
  - chokepoint, approach-route, visibility, and defensibility evaluation;
  - headless raid simulation and diagnostics;
  - difficulty, variety, fairness, repetition, and exploit evaluators;
  - dungeon gameplay placed into a physical layout supplied by Settlements, Bases & Dungeons.
- Preserved:
  - `research/base-raid-lab/headless_raid_resolver.py` stays useful as the first
    deterministic raid resolver.
  - `research/base-raid-lab/test_headless_raid_resolver.py` stays useful as the fixture
    acceptance gate.
  - `BASE-1` stays resolved as the Combat capability contract.
- Not preserved as this workstream ownership:
  - any claim to own base, village, building, settlement, or physical dungeon generation.

## Minimum Built Site Recipe input

A Built Site Recipe consumed by this workstream needs physical facts and neutral
placement slots, not Unreal Actors or tactical declarations:

- recipe identity: `siteId`, `siteVersion`, generator version, seed, site kind, and
  ownership context;
- spaces: stable IDs for rooms, yards, ledges, platforms, exterior zones, and anchor or
  arrival areas;
- routes: directed movement links with traversal requirements, width/headroom/step/jump
  constraints, tags, and stable blocker references;
- visibility: directed sight links with distance, occlusion/blocker references, height or
  cover facts;
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

## Structured evaluation result returned to Settlements, Bases & Dungeons

The output must explain why the site plays well or badly, not only who wins:

- result identity: evaluator version, Combat contract/profile/policy versions, site ID,
  scenario ID, seed, and run set;
- headline scores: tactical viability, difficulty band, variety, fairness, exploit risk,
  repetition risk, and diagnostic confidence;
- outcome matrix: attacker/defender/scenario policy results, terminal reasons, extraction
  rates, objective completion rates, casualties, time-to-outcome, and resource pressure;
- causal findings: stable IDs for overpowered chokepoints, dead routes, useless
  defenders, trivial loot paths, sealed objectives, spawn traps, trap spam, sightline
  dominance, unearned retreats, and extraction cheese;
- route findings: path diversity, route overlap, blocker criticality, flank value,
  backtracking burden, and dead-end severity;
- encounter findings: role usefulness, reinforcement impact, retreat success/failure,
  trap contribution, defender coverage, and attacker counterplay;
- loot/risk findings: loot exposure, defended value, extraction risk, reward clustering,
  and low-risk high-value flags;
- recommended feedback: recipe-stable adjustment hints such as "add second approach",
  "move loot behind a defended but breachable blocker", "reduce sight dominance from
  node X", or "mark slot Y unsuitable for trap";
- evidence payload: canonical run/event traces or links to immutable run artifacts.

## Magic-machine graph

| Node | Unlocks | AND/OR/SEQUENCE | Borrow/Adapt/Build | Contract | Work deleted | Verifier | Challenge set | Promotion gate |
|---|---|---|---|---|---|---|---|---|
| Combat capability catalog | Headless raid semantics | AND input | Borrow | Versioned movement, attack, policy, mask, damage, objective semantics | Inventing local combat assumptions | `BASE-1` acceptance test | Open, fortified, sealed fixtures | Existing matrices remain reproducible |
| Built Site Recipe | Generated raid/dungeon affordances | AND input | Adapt | Stable physical spaces, routes, blockers, sight, neutral slots, physical budgets, provenance | A second layout generator in this workstream | Recipe-shape validation | Village, base, dungeon, sealed, split-route, elevated-route sites | Diagnostics cite stable recipe IDs without Unreal Actors |
| Raid Scenario catalog | Encounter composition | SEQUENCE input | Build | Objectives, roles, spawn/reinforcement/retreat/extraction rules, trap and loot budgets | Per-site bespoke scenario decisions | Scenario fixture tests | Scout, smash-and-grab, guarded vault, ambush, boss/elite, retreat-pressure | Same site separates across scenarios with useful causes |
| Tactical evaluator | Feedback to Settlements, Bases & Dungeons | Unlock | Adapt | Scores plus causal failure reasons from repeated headless simulations | Manual raid review and win/loss-only judgments | Held-out generated sites | Repetitive, unfair, exploitable, sealed, trivial, and well-mixed sites | Findings predict actionable Settlements, Bases & Dungeons recipe changes |

## First build wave

- Borrow: keep the resolved Combat contract and existing headless resolver acceptance
  fixtures.
- Adapt: request a Built Site Recipe contract with semantic affordances and stable
  provenance IDs.
- Build: add abstract raid scenario fixtures now; adapt them to concrete Built Site
  Recipes only after `RAID-1` supplies the physical recipe shape.
- Selected frontier: Built Site Recipe contract, because without it this workstream would
  either depend on Unreal Actors or recreate topology generation.

If the Built Site Recipe contract existed, Raids & Dungeons would no longer need to infer
gameplay spaces from buildings, villages, or Unreal Actors.
