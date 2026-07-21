# BASE-1 — Headless combat capability contract

Status: resolved
From: Raids & Dungeons
To: Combat & Classes
Type: CONTRACT
Notification: sent

## Request

Provide the smallest versioned, pure-data capability contract that lets Raids & Dungeons
judge generated bases with deterministic headless raids rather than the current
reachability-only probe.

The contract must give a consumer, without depending on an Unreal Actor or a named final
class:

- one baseline traversal envelope;
- the minimum targeting, line-of-sight, attack, breach, defence, damage, death, loot, and
  extraction semantics needed to resolve a raid;
- at least two materially different attacker policies and two defender policies, with
  their capability-profile and policy versions recorded in every result; and
- provisional fixture data for every required tunable, kept outside prose and explicitly
  non-canonical rather than leaving the evaluator to invent balance values.

Please return the schema and fixtures in Combat-owned artifacts, plus black-box acceptance
cases showing that fixed inputs reproduce, a known open base and a known fortified base
separate across the policy set, and an impossible or stalled raid ends with an explicit
reason. A run must be able to report paths, targets, attacks, damage, deaths, objective
events, outcome, and termination reason so a failure implicates either the base recipe or
the combat contract.

Ownership boundary: Combat & Classes delivers the schema, fixtures, policy definitions,
and expected black-box outcomes only. Raids & Dungeons owns the executable reference
resolver and headless evaluator that consume them.

This request does not require a final class roster, named abilities, animation, combat
feel, final TTK, defender-cap values, gear tiers, or canon changes. Those remain outside
the headless evaluator boundary.

Minimum supporting evidence:

- [`docs/evolution-plan.md#Cross-workstream contracts`](../../docs/evolution-plan.md#cross-workstream-contracts)
  assigns Combat & Classes the baseline traversal, attack, defence, destruction, and
  objective capabilities consumed by the Base loop.
- `research/machines.json` makes `runtime.combat-simulation` and multiple policies in
  `evaluation.encounter` prerequisites for tactical base evidence; `world.base-generator`
  consumes that simulation. The registry remains authoritative and is not changed here.
- [`docs/systems.md#Combat`](../../docs/systems.md#combat) fixes gear-and-positioning as
  the greybox shape while leaving exact verbs and tuning open, so the fixtures must be
  executable without pretending those open calls are final.
- The existing [`GatersRaider`](../../Unreal/Prototype/Source/Prototype/Public/GatersRaider.h)
  hard-codes traversal and instant breach behavior and only tests whether one agent can
  reach `RaidLoot`; it has no defenders, damage, death, extraction, or policy comparison.

## Response

Delivered:

- [`headless-combat-contract-v1.md`](../../research/combat/headless-combat-contract-v1.md)
  defines the Actor-free tactical graph, mask-only agent durability, ordinary
  attack-based breach, post-upkeep decision snapshot, atomic movement, simultaneous
  combat, objective rules, terminal precedence, diagnostics, and evidence versioning.
- [`headless-combat-fixtures-v1.json`](../../research/combat/headless-combat-fixtures-v1.json)
  carries every provisional value: one traversal envelope, attacker/defender capability
  profiles, `runner`/`clear` attacker policies, `hold`/`intercept` defender policies, and
  twelve policy-pair cases across three arenas.

Expected matrices:

- `open`: every policy pair extracts (`4/4`).
- `fortified`: only `clear` against `intercept` extracts (`1/4`).
- `sealed`: no pair extracts (`0/4`); every pair ends `no-effective-action` and implicates
  `sealed-wall` plus `sealed-route`.

Structural evidence:

- PowerShell parsing and contract assertions pass for versions, counts, unique scoped
  IDs, traversal/profile/policy/arena references, side/version agreement, complete policy
  matrices, split width/headroom, anti-oracle policy wording, and sealed actionlessness.
- The full design and verification steps are recorded in
  [`2026-07-18-headless-combat-contract-v1.md`](../plans/2026-07-18-headless-combat-contract-v1.md).

Machine fit: this Combat contract and catalog are an AND input beside verified
navigation/visibility data; Raids & Dungeons owns the adapter and resolver that can promote
`runtime.combat-simulation` evidence. No separate JSON Schema, resolver, Unreal code,
canon, or registry change was added.

The expected outcomes are acceptance vectors, not executed combat evidence yet. Base &
Raids & Dungeons resolves this packet only after its independently implemented resolver reproduces
fixed inputs, emits the required diagnostics, and matches the three matrices.

## Resolution

Resolved by Raids & Dungeons with an independent, Raids-owned headless resolver:

- [`headless_raid_resolver.py`](../../research/base-raid-lab/headless_raid_resolver.py)
  consumes Combat's fixture catalog and emits deterministic canonical raid results with
  version, policy, profile, event, outcome, terminal, and implicated-ID diagnostics.
- [`test_headless_raid_resolver.py`](../../research/base-raid-lab/test_headless_raid_resolver.py)
  reruns every acceptance case twice, requires byte-identical canonical output, checks the
  expected outcome/terminal/decisive event, and verifies the required version/profile
  diagnostics.

Acceptance evidence:

- `python research/base-raid-lab/test_headless_raid_resolver.py` passes.
- CLI rerun against
  [`headless-combat-fixtures-v1.json`](../../research/combat/headless-combat-fixtures-v1.json)
  reproduces the required matrices: `open` extracts `4/4`, `fortified` extracts `1/4`,
  and `sealed` extracts `0/4`.
- Sealed cases terminate `no-effective-action` with `sealed-wall` and `sealed-route`;
  fortified cases separate policy behavior, with only `clear` against `intercept`
  extracting.

This resolves the contract request only. Generated-site tactical adapter work remains
Raids & Dungeons-owned and is not yet tactical evidence for arbitrary generated sites.
