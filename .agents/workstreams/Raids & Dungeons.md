# Raids & Dungeons

## Current objective

- Compose a valid world approach into the scenario-backed representative ensemble after
  Settlements repairs the held-out topology, without Actors or a second generator.

## Owned outputs

- Encounters, objectives, attackers, defenders, traps, loot, tactical-role fixtures,
  headless simulations, and explainable difficulty/variety/fairness/exploit evaluation.
- Legacy resolver artifacts under `research/base-raid-lab/` remain valid evidence.

## Boundaries

- Settlements, Bases & Dungeons owns villages, buildings, bases, and physical dungeon topology.
- Combat & Classes owns movement, attacks, spells, AI capabilities, and balance models.
- Primary Builder — World & Terrain owns environmental inputs and shared world integration.

## Evidence

- [`ownership-audit-v1.md`](../../research/raids-dungeons/ownership-audit-v1.md) defines
  the split and minimum tactical input/output boundary.
- [`rift-raid-machine-setup-v1.md`](../../research/raids-dungeons/rift-raid-machine-setup-v1.md)
  defines the shared Rift-delivered raid machine for wild dungeons and player bases.
- [`rift-raid-scenarios-v1.json`](../../research/raids-dungeons/rift-raid-scenarios-v1.json)
  and [`Test-RiftRaidScenarios.ps1`](../../research/raids-dungeons/Test-RiftRaidScenarios.ps1)
  define and verify the first Raids-owned Rift access states and scenario challenge set,
  including `abandoned-settlement-loot-v1`.
- [`rift_raid_harness.py`](../../research/raids-dungeons/rift_raid_harness.py),
  [`synthetic-built-sites-v1.json`](../../research/raids-dungeons/synthetic-built-sites-v1.json),
  and [`test_rift_raid_harness.py`](../../research/raids-dungeons/test_rift_raid_harness.py)
  run deterministic policy matrices, production JSON preflight, and causal topology or
  missing-evidence checks without Actors or a second site generator.
- [`headless_raid_resolver.py`](../../research/base-raid-lab/headless_raid_resolver.py)
  and its tests reproduce the required open, fortified, and sealed outcome matrices.
- The production adapter preserves explicit evidence coverage and per-connection movement
  support, rejects unsupported contracts/units, and distinguishes unknown evidence from
  proved-empty evidence.
- [`generated-settlement-built-site-v1.json`](../../research/settlements-bases-dungeons/generated-settlement-built-site-v1.json)
  maps deterministically to 171 spaces, 320 directed connections, 320 visibility facts,
  150 blockers, and 129 neutral slots, then returns `ready-for-scenario` without tactical
  inference. Settlements delivered two byte-identical exports at SHA-256
  `d9b6e6bc565884c33d8d1fc7206c00c09c176566a43364dbf868853019c01d6e`.
- Role-free graph analysis finds all ten indoor neutral slots connected to the site path
  network; all 1,190 directed outdoor-to-indoor slot pairs have a supported route.
- [`generated-settlement-frontier-v1.json`](../../research/raids-dungeons/generated-settlement-frontier-v1.json)
  preserves the exact input identity, role-free metrics, and analysis-only representative
  ensemble evidence.
- RAID-4's approved deterministic policy reduces 141,610 site-network-valid tuples to five
  simulated representatives. It exposes
  shallow shortcut, policy-sensitive medium, route-choice, visibility-exposed, and
  raid-clock-impossible deep cases without changing the source recipe.
- No production slot proves a settlement edge or terrain connection. `Outdoors` alone is
  insufficient to select a world approach without risking an internal arrival shortcut.
- Representative runs now use the catalogued AI/abandoned settlement loot-and-extract
  scenario. They remain analysis-only until a composed world approach exists.

## Exchanges

- [`BASE-1 — Headless combat capability contract`](../exchanges/BASE-1-headless-combat-contract.md)
  — resolved.
- [`RAID-1 — Built Site Recipe tactical contract`](../exchanges/RAID-1-built-site-recipe-contract.md)
  — resolved; the pure C++ production seam is accepted, with omitted optional evidence
  treated as unknown by the production JSON preflight.
- [`RAID-2 — Built Site Recipe JSON export`](../exchanges/RAID-2-built-site-recipe-json-export.md)
  — resolved; deterministic serializer, commandlet, focused tests, reproducible command,
  and held-out generated settlement artifact are independently accepted.
- [`RAID-3 — Generated site tactical evidence`](../exchanges/RAID-3-generated-site-tactical-evidence.md)
  — resolved; Unreal Runner reproduced the exact artifact, and fresh Raids preflight plus
  causal checks pass.
- [`RAID-4 — Production tactical-role candidate policy`](../exchanges/RAID-4-production-role-candidate-policy.md)
  — resolved; production analysis uses the deterministic representative ensemble while
  final runtime encounter placement remains a separate policy.
- [`RAID-5 — Disconnected generated interiors`](../exchanges/RAID-5-disconnected-generated-interiors.md)
  — resolved; the regenerated artifact makes all ten indoor slots reachable, preserves
  preflight, and the Raids regression mutation still diagnoses a reintroduced break.
- [`RAID-6 — World-to-site approach seam`](../exchanges/RAID-6-world-to-site-approach-seam.md)
  — resolved; Primary accepts the neutral pure-data composer boundary and rejects
  inferring site edges from generic outdoor corridors.
- [`RAID-7 — Combat simulation registry integration`](../exchanges/RAID-7-combat-simulation-registry-integration.md)
  — open; asks Primary to record the independently verified deterministic combat
  substrate as active without promoting the still-incomplete generated encounter loop.
- [`WORLD-4 — Site-edge connector contract`](../exchanges/WORLD-4-site-edge-connector-contract.md)
  — open; Primary requests the missing neutral connector evidence from Settlements before
  composing world-to-site approaches.
- [`SITE-3 — Tactical ownership boundary`](../exchanges/SITE-3-tactical-ownership-boundary.md)
  — resolved; Settlements, Bases & Dungeons owns physical facts and neutral slots, Raids & Dungeons derives
  tactical labels and budgets.
