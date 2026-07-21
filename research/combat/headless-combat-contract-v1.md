# Headless combat contract

Normative semantics for the provisional Combat & Classes fixtures consumed by Base & Raid
Lab. The machine-readable values live in
[`headless-combat-fixtures-v1.json`](headless-combat-fixtures-v1.json). This contract is
Actor-free, class-free, and non-canonical. It answers
[`BASE-1`](../../.agents/exchanges/BASE-1-headless-combat-contract.md) without settling the
open second-to-second verbs in [Combat](../../docs/systems.md#combat).

## Boundary

- Combat & Classes owns this field meaning, the capability profiles, policy definitions,
  provisional fixtures, and expected black-box outcomes.
- Base & Raid Lab owns catalog validation, conversion from verified navigation/visibility
  data, and the executable resolver/evaluator.
- A tactical graph is a derived evaluator input. It never overrides world geometry,
  navigation, visibility, recipes, or runtime state.
- Explicit graph links may be authored directly only for black-box fixtures.

## Machine fit

| Node | Unlocks | Relation | Source | Contract | Work deleted | Verifier | Challenge set | Promotion gate |
|---|---|---|---|---|---|---|---|---|
| Navigation and visibility | Derived tactical graph | SEQUENCE input | Borrow | Stable nodes, movement/sight links, live blocker IDs | A second geometry model | Existing navigation fixtures plus Base adapter tests | Blocked, split, and destructible routes | Derived links agree with held-out navigation/visibility evidence |
| Combat semantics and fixtures | Headless resolution | AND input | Adapt | This contract and its catalog | Per-base combat assumptions | Catalog contract assertions | Open, fortified, and sealed arenas across every policy pair | All references validate and the resolver matches the expected matrices |
| Headless raid resolver | Tactical base evidence | Unlock | Build, Base-owned | Deterministic events and terminal result from fixed arena and policies | Manual raids and one-bot judgment | Independent reruns and event diagnostics | Repeated fixed inputs plus held-out generated bases | Reproduction and tactical separation hold without missing diagnostics |

If these inputs hold, base triage no longer needs manual raids or the current one-naive-
raider oracle. The first falsifying experiment is the catalog's challenge matrix; a
failure must preserve the result, event trace, implicated stable IDs, and all versions.

## Catalog shape

- `contractVersion` identifies this field meaning and resolution contract.
- `fixtureVersion` identifies the catalog values, arenas, and expected outcomes.
- `runTunables` holds tick duration, stall threshold, and run limit.
- `traversalEnvelopes` defines reusable movement limits.
- `capabilityProfiles` defines side, mask behavior, defence, traversal reference, and
  attack modes.
- `policies` defines ordered, snapshot-only action priorities.
- `arenas` contains tactical graphs and starting state.
- `acceptanceCases` selects one arena and one policy per side, then names the exact
  expected outcome and causal evidence.

All cross-references use stable IDs. A consumer rejects unsupported versions, duplicate
IDs in one scope, wrong-side policy/profile references, and dangling node, link, blocker,
profile, traversal, policy, or causal IDs.

## Tactical graph

- A `node` has a stable ID, position in centimetres, and semantic tags. Positions supply
  range only; they are not a second collision model.
- A directed `movementLink` names its endpoints, length, width, height, step, jump gap,
  jump rise, and live blocker IDs. It is legal only when every requirement fits the
  agent's traversal envelope, its length fits the per-tick move distance, and every
  blocker is destroyed. Width and headroom stay separate because they fail independently.
- Movement is atomic: one legal link completes in one movement phase. Base subdivides
  longer derived paths before resolution. There is no mid-link state or rounding.
- A directed `sightLink` certifies visibility from one node to another, records distance,
  and names live blockers. It is live only while all blockers are destroyed or absent.
- A `structure` has a stable ID, node, integrity, defence, and targetability. Structure
  integrity is not an agent health resource.
- An `agent` has a stable ID, side, start node, and versioned capability profile.
- One objective names the loot, its node, and the extraction node.

## Tick and action semantics

- **Upkeep:** drain hostile-soil attacker mask, recharge home-defender mask to its cap,
  and advance cooldowns.
- **Decision snapshot:** freeze the post-upkeep state. Every policy reads this same
  immutable snapshot and selects one intent. Ordered policy priorities are
  lexicographic; stable IDs break every tie.
- **Combat:** resolve all legal attack intents from the snapshot simultaneously. Applied
  damage is `max(0, attack damage - target defence)`. An attack is progress-capable only
  when its applied damage is positive.
- **Death and destruction:** mask at or below zero kills an agent. Integrity at or below
  zero destroys a structure and removes it from every movement/sight blocker reference.
  A carrier death drops loot at that agent's node.
- **Movement:** surviving movers complete their selected legal links atomically. Agents
  do not block nodes or links in this contract.
- **Objective:** a living agent on uncarried loot picks it up. A living carrier at the
  extraction node extracts it.
- **Evidence:** record intents, selected paths/targets, state changes, and terminal choice.

Agent mask is the only agent durability resource. Attackers lose mask to the raid clock
and combat damage; defenders may receive home recharge. Breach is an ordinary attack
against a structure, not a parallel system. An attack mode is legal only for its declared
target kind, when cooldown and range pass, a live sight link exists if required, and
applied damage is positive.

## Policies

- An objective route may include a link whose traversal requirements fit but whose live
  blocker makes movement currently illegal. The agent moves across the next open link or
  attacks the next targetable blocker it can damage positively.
- `runner`: extract or pick up; otherwise follow the shortest objective route, moving its
  next open link or breaching its next blocker, then attack a defender only when objective
  progress is unavailable.
- `clear`: extract or pick up; otherwise attack the nearest reachable defender the
  attacker can damage positively in the decision snapshot, then advance or breach toward
  the objective. It never forecasts a duel outcome.
- `hold`: attack a legal target, preferring the loot carrier; otherwise remain on the
  assigned guard node.
- `intercept`: attack the carrier or nearest legal attacker; otherwise advance toward the
  nearest reachable attacker.

Policies name priorities, not scripts. They cannot read future state, hidden resolver
results, expected outcomes, or eventual kill/extraction forecasts.

## Progress and termination

Progress events are completed movement, positive damage, death, destruction, loot pickup
or drop, and extraction. Cooldown changes and intent selection are not progress.

Choose one terminal result using this precedence and record its decisive causal event:

- `extracted` when objective resolution extracts loot.
- `mask-expired` only when upkeep drain removes the final living attacker.
- `attackers-dead` only when combat removes the final living attacker.
- `no-effective-action` when the decision snapshot offers neither side a legal intent
  capable of progress toward changing attacker, blocker, loot, or extraction state.
- `stalled` after the catalog threshold of legal intents without a progress event.
- `time-limit` when the catalog run limit is reached and no earlier reason applies.

## Result and diagnostics

The canonical result contains no timestamp, archive ID, or machine-local path. An outer
run archive may add those without changing canonical equality.

- Record contract, fixture, traversal, capability-profile, and attacker/defender-policy
  IDs and versions.
- Record outcome, terminal reason, terminal tick, decisive causal event ID, and implicated
  stable IDs.
- Emit tick-stamped events for upkeep, path selection, target selection, attack, damage,
  movement, death, destruction, loot pickup/drop/extraction, and termination whenever
  those events occur.
- Fixed inputs must reproduce byte-identical canonical JSON after stable key ordering.

## Acceptance and exclusions

- The open arena must extract under every policy pair.
- The fortified arena must match its mixed matrix, proving policy choice changes evidence.
- The sealed arena exposes no legal movement, breach, or combat action and must terminate
  `no-effective-action` with its blocker/link IDs.
- These fixtures validate resolver semantics and tactical separation, not balance or fun.
- Deferred: final classes, named abilities, defender-cap values, gear tiers, animation,
  combat feel, calibrated TTK, Unreal integration, and canon changes.

Sources: (src: ../../docs/systems.md#Combat),
(src: ../../docs/evolution-plan.md#Cross-workstream-contracts),
(src: ../../research/machines.json#runtime.combat-simulation).
