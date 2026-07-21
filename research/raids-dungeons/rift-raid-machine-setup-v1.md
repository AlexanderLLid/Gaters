# Rift raid machine setup v1

## Call

- Use one Rift-delivered raid machine for wild dungeons, AI/abandoned settlements, and
  player bases.
- A dungeon is a hostile world site reached through a Rift, not a menu room.
- A player-base raid is the same machine pointed at a claimed world's Built Site Recipe.
- Arrival must create an approach problem, not place attackers inside the final loot,
  boss, or vault space.

## Why

- `RIFT-4` already makes exploration, deliberate travel, and hostile entry one
  phenomenon.
- `RAID-3` requires world approach before breach and loot.
- `RAID-4` requires multiple possible approaches instead of one permanent door.
- `SITE-3` limits Settlements, Bases & Dungeons to physical facts and lets this workstream derive tactics.

## Machine outcome

Given:

- a Built Site Recipe for a world site or player base;
- Combat capability profiles and policies;
- a raid scenario;
- current Rift/trace access state;

produce:

- deterministic headless raid runs;
- outcome matrices across attacker/defender roles;
- tactical scores for difficulty, variety, fairness, repetition, and exploit risk;
- stable failure reasons that point back to recipe IDs, scenario IDs, and Combat inputs.

If this existed, Raids & Dungeons would no longer need manual raid reviews, one-off
dungeon scripts, or separate base-raid and dungeon-raid evaluators.

## Shared raid shape

- **Rift entry:** scenario selects one or more neutral arrival slots from the site recipe,
  biased by available trace/route evidence.
- **Approach:** attackers traverse world/site routes before the objective; the approach
  can include weather, wildlife pressure, patrols, visibility loss, and alternate paths.
- **Contact:** defenders, monsters, traps, and patrols react from scenario roles assigned
  by Raids & Dungeons.
- **Objective:** loot, boss, rescue, ritual, destroy, hold, scan, or steal-map objective.
- **Extraction:** attackers must return to a valid extraction slot or satisfy a scenario
  exit condition before the Rift/raid clock fails.
- **Feedback:** every bad result cites stable recipe/source IDs and says whether the fault
  is physical topology, scenario placement, Combat capability, or unresolved Rift rules.

## Player-base variant

- Input site kind: `claimed-base`.
- Access precondition: current trace/route evidence grants a temporary hostile Rift
  opportunity; map knowledge alone is not enough.
- Arrival: use neutral arrival slots tied to Anchors, ruins, edge approaches, breached
  perimeter paths, or unstable Rift landing areas.
- Defenders: use player-owned defenses, offline/automated defenders, present defenders,
  and scenario-specific reinforcement/retreat rules.
- Objectives: loot, stored resources, route knowledge, claimed-object disruption, rescue,
  sabotage, or timed extraction.
- Evaluation must flag:
  - doorstep kill-boxes;
  - vault-spawn shortcuts;
  - single-route solved defenses;
  - low-risk high-value loot;
  - routine offline loss;
  - zerg-friendly open approaches;
  - no-counterplay ambushes;
  - trivial extraction.

## Wild dungeon variant

- Input site kind: `wild-dungeon`, `ruin`, `monster-held-site`, or `rift-wounded-site`.
- Access precondition: natural Rift, called Rift, discovered Anchor, route clue, or
  expedition opportunity.
- Arrival: use neutral arrival slots near Anchor ruins, cave mouths, forest edges, frozen
  clearings, broken thresholds, or unstable Rift scars.
- Defenders: use monsters, intelligent factions, traps, patrols, boss/elite pressure, and
  environmental hazards.
- Objectives: boss clear, loot extraction, rescue, scan, survive, ritual break, route
  discovery, or resource harvest.
- Evaluation must flag:
  - repetitive straight-line clears;
  - no meaningful route choice;
  - unavoidable first-shot deaths;
  - environment pressure with no counterplay;
  - boss room skipping;
  - reward with no extraction risk.

## Inputs

- **Settlements, Bases & Dungeons:** physical facts, neutral slots, blockers, movement links, sight links,
  physical tags, physical budgets, and provenance IDs.
- **Combat & Classes:** movement, attacks, spells, AI capabilities, tactical policies, and
  balance assumptions.
- **Primary Builder - World & Terrain:** biome/weather/resource/environment facts and
  world integration.
- **Raids & Dungeons:** scenario catalog, tactical labels, tactical budgets, roles,
  objectives, traps, loot risk, spawn/reinforcement/retreat/extraction rules, scoring, and
  diagnostics.

## Outputs

- `scenarioId`, `scenarioVersion`, `siteId`, `siteVersion`, Combat contract versions, and
  Rift access condition.
- run matrix by attacker role, defender role, scenario, and arrival slot.
- terminal reasons: extracted, attackers-dead, objective-failed, no-effective-action,
  stalled, time-limit, retreat-success, retreat-failed, sealed, exploit-detected.
- scores: tactical viability, approach quality, objective pressure, extraction pressure,
  fairness, variety, repetition risk, exploit risk, defender value, attacker counterplay.
- causal findings with stable IDs:
  - route;
  - sight;
  - blocker;
  - slot;
  - objective;
  - trap;
  - defender;
  - reinforcement;
  - extraction.
- adjustment hints for Settlements, Bases & Dungeons and scenario tuning.

## Magic-machine graph

| Node | Unlocks | AND/OR/SEQUENCE | Borrow/Adapt/Build | Contract | Work deleted | Verifier | Challenge set | Promotion gate |
|---|---|---|---|---|---|---|---|---|
| Combat capability catalog | Headless combat semantics | AND input | Borrow | Versioned movement, attack, spell, policy, damage, death, objective rules | Local combat guesses | `BASE-1` resolver tests | Open, fortified, sealed cases | Matrices stay deterministic |
| Built Site Recipe | Physical raid/dungeon affordances | AND input | Adapt | Physical spaces, routes, sight, blockers, neutral slots, provenance | Actor/geometry inference and local layout generation | `RAID-1` recipe validation | Claimed base, wild ruin, cave, settlement edge, split-route, sealed-route | Diagnostics cite stable recipe IDs |
| Rift access state | Why this raid can happen now | AND input | Build | Trace/route evidence, arrival bias, raid clock, extraction condition | Separate dungeon queue/menu | Scenario fixtures | natural Rift, called Rift, traced player base, known Anchor, bad stale clue | Access grants bounded raid without permanent hostile access |
| Raid scenario catalog | Encounter composition | SEQUENCE input | Build | Objectives, roles, traps, loot, spawn, reinforcement, retreat, extraction budgets | Bespoke encounter scripts | Scenario fixture tests | ice-forest ambush, guarded vault, player-base steal, boss clear, rescue, sabotage | Same site separates across scenarios for clear causes |
| Tactical-role candidate ensemble | Generated-site scenario inputs | SEQUENCE adapter | Adapt | Deterministic representative arrival, objective, extraction, and guard tuples derived from neutral physical facts plus scenario constraints | Manual role labeling and one hand-picked oracle case | Raids-owned structural tests | shallow exploit, medium route, deep clock failure, isolated objective, exposed arrival | Stable exemplars cover distinct causal buckets without changing the source recipe |
| Tactical evaluator | Actionable feedback | Unlock | Adapt | Scores plus causal findings from repeated simulations | Manual review and win/loss-only judgment | Held-out generated sites | good, repetitive, unfair, exploitable, sealed, trivial, too lethal | Findings predict useful recipe/scenario changes |

## First build wave

- Borrow:
  - existing Combat fixture contract;
  - existing deterministic headless resolver;
  - current Settlements, Bases & Dungeons neutral-slot direction from `SITE-3`.
- Adapt:
  - [`rift_raid_harness.py`](rift_raid_harness.py) converts pure synthetic Built Site
    Recipe fixtures into resolver tactical graphs and maps production RAID-2 JSON without
    copying topology generation;
  - the harness reports approach, extraction, fairness, repetition, and exploit causes
    using stable recipe IDs;
  - production recipes stop at `insufficient-evidence` before scenario assignment or
    simulation when required physical facts remain unknown.
- Build:
  - Rift access state fixtures;
  - raid scenario catalog fixtures for wild dungeons and player bases.

Current fixture catalog:

- [`rift-raid-scenarios-v1.json`](rift-raid-scenarios-v1.json) defines the first Rift
  access states and scenario challenge set, including the minimum AI/abandoned settlement
  loot-and-extract scenario.
- [`Test-RiftRaidScenarios.ps1`](Test-RiftRaidScenarios.ps1) verifies catalog shape,
  stable references, player-base trace gating, direct-prize arrival rejection, and wild
  plus claimed-base coverage.

Current executable harness:

```powershell
python research/raids-dungeons/rift_raid_harness.py --summary
```

Production export preflight:

```powershell
python research/raids-dungeons/rift_raid_harness.py `
  --built-site-export research/settlements-bases-dungeons/generated-settlement-built-site-v1.json `
  --summary
```

Production representative ensemble:

```powershell
python research/raids-dungeons/rift_raid_harness.py `
  --built-site-export research/settlements-bases-dungeons/generated-settlement-built-site-v1.json `
  --representative-ensemble `
  --summary
```

- [`synthetic-built-sites-v1.json`](synthetic-built-sites-v1.json) contains contract-test
  physical recipes for an open settlement, multi-route fortress, single-door bunker,
  and sealed dungeon. These are fixtures, not a site generator.
- [`test_rift_raid_harness.py`](test_rift_raid_harness.py) verifies deterministic policy
  matrices, version identity, direct-prize rejection, production export mapping,
  unsupported-version rejection, per-connection movement support, causal
  unknown-versus-proved-empty evidence findings, and a mutation that reintroduces one
  disconnected indoor objective.
- The current challenge result separates viable open/multi-route sites, a tactically
  exploitable but traversable bunker, and an unreachable sealed dungeon.
- The first generated settlement maps 171 spaces, 320 directed connections, 320 directed
  visibility facts, 150 blockers, and 129 neutral slots. Its complete evidence coverage
  and declared movement support now pass production preflight as `ready-for-scenario`;
  tactical roles and raid results remain Raids-owned downstream work.

## Current frontier

RAID-4 approved the deterministic representative ensemble. The production recipe is
physically safe to consume, and the evaluator now samples its tactical range without
writing roles back to the recipe:

- [`generated-settlement-frontier-v1.json`](generated-settlement-frontier-v1.json)
  preserves the role-free graph metrics and provisional shallow/deep counterexamples;

- 171 spaces form 13 movement components; the largest contains 159 spaces and all ten
  indoor neutral slots;
- all ten indoor objective candidates connect to the site path network and every one of
  the 1,190 directed outdoor-to-indoor slot pairs has a supported route;
- 119 slots are tagged `outdoors`, but none proves `settlement-edge` or a connection to
  surrounding terrain, so an internal path cannot yet be distinguished from a valid
  world approach;
- `abandoned-settlement-loot-v1` now supplies the catalogued objective, roles, pressure,
  and causal gates; runs remain analysis-only because no composed world approach proves
  an external arrival;
- 141,610 site-network-valid tuples reduce deterministically to five simulated
  representatives;
- a one-link provisional raid extracts under every Combat policy pair but is flagged for
  trivial extraction and unavoidable first fire;
- a 24-link medium provisional raid separates attacker policies: both `runner` cases extract
  while both `clear` cases expire, proving that the production graph can expose policy
  differences without changing physical facts;
- a 126-link deep provisional raid fails under every policy pair and is diagnosed as
  `raid-clock-impossible` before combat delay;
- route-choice and visibility-exposed representatives preserve different physical causes.

The adapter guarantee now passes deterministic reruns, unchanged-source checks, stable-ID
role references, site-network reachability for every indoor objective, a real settlement
scenario, and explicit shallow/medium/deep causes on the held-out settlement. `RAID-5`
is resolved. Results remain `analysis-only`: no composed world-to-site approach proves
an external arrival, and `WORLD-4` tracks the Settlements-owned site-edge prerequisite.

## First falsifying experiments

- **Ice-forest ambush:** hostile world site with low visibility, patrols, ranged ambush,
  shelter, boss/elite, and extraction pressure. Must not become straight-line boss rush.
- **Claimed-base steal:** player base with multiple neutral arrivals, perimeter defense,
  loot objective, defender automation, and extraction. Must not spawn attackers in the
  vault or create routine offline loss.
- **Single-door bunker:** one approach dominates. Evaluator must flag repetitive/solved
  defense and recommend another approach or arrival.
- **Unfair spawn trap:** defenders kill attackers before agency. Evaluator must flag
  no-counterplay arrival.
- **Low-risk jackpot:** high-value loot near extraction with weak coverage. Evaluator must
  flag low-risk high-value reward.

## Open rules that block promotion

- exact hostile Rift arrival selection;
- raid clock source and extraction condition;
- simultaneous hostile Rifts and caps;
- offline defender model;
- Rift cost/resource;
- stale trace expiry and when route knowledge is insufficient.

Requirements checked: Global none recorded; `RIFT-1` through `RIFT-6`, `ANCHOR-1`
through `ANCHOR-5`, `RAID-1` through `RAID-6`, `RETURN-1` through `RETURN-3`,
`CLAIM-1` through `CLAIM-4`, Combat section. Exceptions: none.
