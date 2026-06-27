# Open Questions & Conflicts

The project's living decision backlog. Unlike the contradiction discipline (which
handles canon/mechanics conflicts *between pages*), this tracks **undecided design
questions** — things not yet settled, so they don't belong in `severity: hard` blocks.

Resolve an item → write a numbered **DDR** (design) or **ADR** (architecture) with
the why and what you rejected, flip the relevant page tag `[tentative]→[decided]`,
and strike it here. Use /domain-modeling. Source: raw/design-overview.md.

Already graduated: players are gaters / game named after them (DDR-0002); reach-equals-exposure (DDR-0001); Authority as Patch Channel (DDR-0005).
Proposed and awaiting confirmation: players-are-human (DDR-0003), mesh-with-a-heart
topology + central nexus (DDR-0004).

## Conflicts / tensions to resolve
1. **Raid-clock governance** — siege timer (defender side, [[Exposure]]) vs. away reserve (attacker side, [[Mask Energy]]). Same clock or two? Which ends a raid? Can they contradict? Raiders extend away reserve with [[Power Core|power cores]]/beacons.
2. **Siege-timer number** — 20–30 min (design) vs. ~38 min (lore ceiling) vs. session length. Collapse to one value.
3. **"Gates unbuildable" vs. more Gates over time** — reconcile relocate/reassemble/stabilize-Rifts/saved-coordinates under one umbrella rule. ([[Gates]], [[Rifts]])
4. **Client-authoritative solo PvE vs. cheat-resistance** — pre-raid client tampering at the hand-off to server-authoritative; no mitigation decided.
5. **Stagnation pressure vs. "no obligation" promise** — strong enough to push players out, not so strong it's a disguised upkeep tax. ([[Economy]])
6. **Raider anonymity vs. the central registry** — "no caller ID" vs. the [[Central Authority]] knowing who holds what.
7. **Hub zerg problem** — [[Hub Worlds]] deliberately host many-vs-many chaos; is unbalanced hub PvP acceptable or does it need structure?
8. **"Sealed = isolated" vs. "comms pass through a sealed Gate"** — confirm the split (info yes; goods/energy/people no).
9. **NMS-aspirational visuals vs. low-poly-achievable-solo** — two target identities; look is deferred.
10. **Title "Gaters"** — phonetics ("gators" / "gaiters"), searchability, harassment risk. (Player term settled: gater — DDR-0002.)
11. **Player-driven Rift-stabilization vs. authority-paced expansion** — who controls world-growth pacing? ([[Rifts]], [[Central Authority]])
12. **Field radius and away reserve must stay decoupled** — standing rule; a "expand the field" patch must never lengthen the raid clock. ([[Mask Energy]])

## Biggest open design questions
1. **Is the core fun real?** Will players actually open their Gates? (The greybox must answer.)
2. **What governs the raid clock** — siege timer, away reserve, or both?
3. **How hard should stagnation push** without becoming upkeep?
4. **Which mystery truth** is canon (outside signal / tripwire / authority lie)? ([[The Wake]])
5. **Combat model** — perspective, gun/melee/ability/vehicle, TTK, aperture advantage.
6. **Protecting the client-authoritative solo planet** at hand-off.
7. **Final art identity** (NMS-lush vs. low-poly-stylized).
8. **Survival meters beyond mask energy** — any, and how minimal?
9. **Raider anonymity + central registry** coexistence.
10. **Does "Gaters" survive** the searchability/phonetics concern? (Player term settled: gater.)
11. **Can Rifts be stabilized into Gates** — player-driven or authority/tech-gated?
12. **Rift specifics** — rank scale (E–S vs. 1–10), spawn location, ignore-it downside, Rift-loot vs. raid-loot balance.
13. **Mask/field specifics** — gradient vs. hard radius; field radius authority-only vs. capped upgrade; confirm asymmetric mask-at-zero.
14. ~~**Lock "Authority as Patch Channel" as canon?**~~ — decided: yes; Gate availability = Authority-certified provisioning (DDR-0005).
