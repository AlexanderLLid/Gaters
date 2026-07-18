# Prototype Plan

Living greybox brief for what the Unreal prototype must prove. It is not canon and does
not track implementation status; `research/machines.json` owns that.

## Current proof

Can the project repeatedly generate a reproducible, streamable, playable world with a
varied raidable base, then produce evidence that tells us what to improve?

The arrival device is deliberately unspecified while [[questions]] #27 remains open.
Generation must work for a Gate, Rift, spawn point, ship landing, or test marker without
changing terrain topology.

## Proof slices

1. **World** — deterministic terrain, water, semantic ground, routes, streamed content,
   resources, and persistence.
2. **Base** — varied layouts fitted to real terrain with multiple valid approaches,
   entrances or breach surfaces, defenders, loot, and extraction.
3. **Raid** — repeatable attacker/defender policies expose tactical strengths,
   degeneracy, exploits, and balance ranges.
4. **Content** — semantic placeholders can be replaced by contracted generated assets,
   rigs, and motion without changing world or gameplay identity.
5. **Research loop** — held-out seeds, immutable run evidence, independent evaluators,
   and champion/challenger promotion make each machine improvable in isolation.

## Priority

- Primary Builder: world/runtime contracts and integration.
- Base & Raid Lab: physical validity, tactical evaluation, and base variety.
- Combat & Classes: the capability envelope bases must support.
- Asset pipeline: enough contracted content to falsify integration assumptions.
- Scale & Persistence: budgets and high-risk experiments, not production networking.

Art and lore may explore in parallel but do not block these proofs until the human
selects a decision that changes a contract.

## Human checks machines cannot promote alone

- Terrain reads as intentional rather than noisy.
- Bases are interesting rather than merely different.
- Combat movement and impact feel powerful and readable.
- Style remains coherent across environment, buildings, characters, creatures, and VFX.
- The resulting loop is worth repeating.

## Current status

Run `research/Show-MachineRegistry.ps1 -Format Summary`. Plans and reports may explain
individual experiments, but their checkboxes and prose never override the registry.
