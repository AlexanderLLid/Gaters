---
type: system
status: draft
tags: []
sources: []
aliases: [Gates]
updated: 2026-06-28
---

# Gates

The central object: every planet-base sits on a Gate, and all reach flows through it. The deep physics treatment is [[gate-physics|Gate Physics]]; this page is the game model.

## Purpose

- Owning a planet means **owning its Gate** — the reason bases are planets, not an arbitrary rule.
- Gate finiteness (found, never manufactured) is the root of all conflict; opening your Gate is how you reach the world _and_ how you become reachable.

## How it works

- **Woke sealed; unmanufacturable.** The network came online with every aperture shut (safe-by-default), and Gates can only be **found, claimed, repaired** — never built (GATE-1).
- **Travel is one-way and directional.** The _dialing_ side opens the tunnel and pays the power, so a raid is inherently the attacker pushing through a tunnel they opened.
- **Fire and momentum pass through** a live Gate — breaching is a firefight at the aperture, not a loading screen (see [[gate-physics|Gate Physics]], [[combat|Combat]]).
- **Dialing needs the target's [[coordinates|coordinates]]** — a planet whose coordinate nobody has assembled is effectively hidden.

## Tiers — driven, not built

- The **ring is fixed**; the **aperture** is a runtime variable set by how much exotic matter (power cores) you channel, capped per ring by the **Ford–Roman quantum inequality** ([[gate-physics|Gate Physics]]).
- **Standard → Heavy** = one ring fed lightly vs. hard (wider aperture, bigger payload, bigger dome, more cost).
- **Supergate** = past one ring's ceiling — combine multiple **found Builder segments** into a megastructure; recovered and repaired, never new-built.
- **Overdrive** one ring past its ceiling and the throat destabilises into a [[rifts|Rift]] (the gate-overload bomb).

## Gate variation — the Builder "config" [tentative]

- Beyond tier (aperture, above), gates **behave differently** — field size, stability, what they dial, what life/loot a world grows. Working idea: each Gate carries a compact Builder-seeded **configuration** (a "genome" / build recipe) that sets these intrinsic traits, distinct from the runtime aperture you drive.
- **Origin:** the variety comes from the **von Neumann probes** that built the network _experimenting_ with configs as they spread (cf. [[builders-truth|Builders' Truth]] — probes seeded the network). _Why_ they experimented stays parked (Builders-unknown).
- **Status:** idea only — it explains gate variety and nothing else depends on it. It does **not** change dialing/findability (still [[coordinates|coordinates]]). The larger "gate-DNA-as-key" / two-factor findability exploration is parked in open-questions.md, not adopted.

## Exposure states — sealed / port-open / gate-open (STATE-1)

- **Sealed** — safe _while unfound_, isolated, stagnant; **not inviolable** (a found home is raidable — see [[coordinates|Coordinates & Obscurity]]). No cold breach through a shut throat; comms still pass.
- **Port-open** — can trade; pirates can hit cargo/convoys but **cannot breach the vault**. Trading risks goods, never the whole civilization.
- **Gate-open** — can launch raids and is **fully breachable**.
- Each step out buys reach for risk. Thresholds/naming tentative.

## Presence (PRES-1)

- A Gate is **open only while the owner is online and choosing it**; logging off **seals** it (the Gate fails closed — a Builder conceit, like the [[combat|dome]]). Sealing is a **spin-down**, not a switch.
- Presence stops a _cold_ breach, but no longer grants immunity: a **found** home can be raided, rarely, even offline (see [[coordinates|Coordinates & Obscurity]]). The babysitting tax died from **obscurity**, not a wall.

## Why / rejected

- **Why ring-fixed, aperture-driven:** it keeps the canon rule "Gates are found, never fabricated" intact while still giving a tier ladder — you drive the same found ring harder, you never build a bigger one. Exceeding one ring means reassembling found segments, so the rule holds.
- **Rejected — "Builders made rings in many fixed sizes; tier up by claiming a bigger ring":** makes the aperture hardware-fixed, conflicting with the driven aperture and with the dome flexing as the throat widens.

## Data

- Gate tier, per-ring aperture cap, activation-cost bands — tunables in data; see [[economy|Economy]] and the Ford–Roman cap in [[gate-physics|Gate Physics]].

## Code

Placeholder until the Unity project exists.

## Open questions

- Dialing UX; spin-down / grace-window timings; the control-device component (the stealable "DHD-equivalent", name open). Tracked in open-questions.md.
