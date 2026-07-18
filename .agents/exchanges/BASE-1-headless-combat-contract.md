# BASE-1 — Headless combat capability contract

Status: open
From: Base & Raid Lab
To: Combat & Classes
Type: CONTRACT
Notification: sent

## Request

Provide the smallest versioned, pure-data capability contract that lets Base & Raid Lab
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
and expected black-box outcomes only. Base & Raid Lab owns the executable reference
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

In progress. Agreed response shape:

- Combat & Classes owns one normative Markdown contract for semantics, rules, and
  versioning, plus one machine-readable JSON catalog containing every provisional value,
  two attacker policies, two defender policies, three arenas, and expected outcome
  matrices.
- Base & Raid Lab owns validation and the executable reference resolver/evaluator.
- No separate JSON Schema or split catalogs until a second consumer or malformed-data
  evidence earns that complexity.

## Resolution

Pending.
