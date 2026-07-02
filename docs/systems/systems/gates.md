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
- **Overdrive** one ring past its ceiling and the throat destabilises into an uncontrolled [[gate-physics|Rift]] (the gate-overload bomb).

## Gate variation — the Builder "config" [tentative — rough idea]

- **Loose idea, needs tuning — nothing depends on it.** Gates differ (field size, stability, what they dial, what grows there) maybe because each carries a small Builder-seeded **config** the [[builders-truth|von Neumann probes]] tuned **blind** — they couldn't predict a config's behaviour, so gates came out imperfectly/variously tuned. That imperfection is the variety.
- Just an explainer for _why gates aren't all the same_. It doesn't touch dialing/findability (still [[coordinates|coordinates]]), it's **not** a selection system (explored and rejected — open-questions.md #22), and the _why_ behind the probes stays parked. Shape it properly if/when it matters.

## Presence (PRES-1)

- A Gate is **open only while the owner is online and choosing it**; logging off **seals** it (the Gate fails closed — a Builder conceit, like the [[combat|dome]]). Sealing is a **spin-down**, not a switch. Sealed = no cold breach through the shut throat (comms still pass — see open-questions.md #7).
- Presence stops a _cold_ breach, but no longer grants immunity: a **found** home can be raided, rarely, even offline (see [[coordinates|Coordinates & Obscurity]]). The babysitting tax died from **obscurity**, not a wall.
- **Offline tends _toward_ safety, not away from it** — sealing sheds no new shares and re-key voids those already taken, so logging off lowers your odds of being found, it doesn't raise them (see [[coordinates|the odds]]).

## Reaching the frontier — dial out (FRONTIER-1)

- **Two things you can dial:** a **claimed coordinate you've assembled** (a [[raiding|raid]] — needs [[coordinates|shares]]), or **out into the unclaimed frontier** (expansion). You can't pick _which_ frontier world — dialing an unclaimed address lands you at a **random** one; **claimed homes are off the pool**, so no one is ever dropped onto occupied soil (preserves obscurity).
- **Frontier travel is one-way** — you can't return the way you came; you arrive, and the [[mask-energy|away-reserve]] clock is already ticking. Push your luck against it for loot, or **claim the far Gate** before it runs out.
- **The frontier world _is_ the contact surface** — no separate Rift system. It runs on three planet runtime-states: a **database row** when empty, a **cheap solo PvE tick** when you're there alone (build/loot in peace), a **server-authoritative contested instance** when others arrive (PvPvE). Persistent as a row, ephemeral as an instance → a bigger frontier costs no more to host until it's actually fought over.
- **Who can reach you there** scales with [[potential|Potential]] and findability — small/quiet pushes draw few rivals; big ones are visible. Same obscurity economy as home, no separate rule.
- **Claiming the far Gate** (imprint) converts a frontier world into your soil and a forward respawn — this is how **new Gates** enter your holdings. Finite but slowly replenishing; pacing open ([[open-questions|Open Questions]]).
- **Creatures, biome variety, loot** come from the frontier world's tags — see [[world-types|World Types]] (REGION-1) and [[taming|Taming]] (TAME-1).

## Self-misdial — the uncontrolled lane (FRONTIER-2)

- The network is unstable (the [[the-wake|Wake]]). An **active** Gate channels energy through that frayed fabric and the strain occasionally **arcs open a lane by itself** — an un-propped throat (a **Rift** in the physics sense, [[gate-physics|Gate Physics]]) onto a random frontier world. You didn't dial it; you didn't pay a core.
- **Outbound only — not a backdoor.** It opens a lane _out_; it never drops anyone onto your soil (homes stay off the random pool). You can always **manually close** it; left open, the un-propped throat collapses on its own countdown.
- **Only an active Gate misdials.** A sealed/offline Gate channels nothing, so it can't — "never while logged off" follows from the physics, not a bolted-on rule.
- It's the in-world face of rising instability: **more Gate activity → more misdials → harsher pulls** — the difficulty ramp and the visible symptom of the central mystery.

## Why / rejected

- **Why ring-fixed, aperture-driven:** it keeps the canon rule "Gates are found, never fabricated" intact while still giving a tier ladder — you drive the same found ring harder, you never build a bigger one. Exceeding one ring means reassembling found segments, so the rule holds.
- **Rejected — "Builders made rings in many fixed sizes; tier up by claiming a bigger ring":** makes the aperture hardware-fixed, conflicting with the driven aperture and with the dome flexing as the throat widens.
- **Why Rifts fold into the Gate (FRONTIER-1):** a Rift was always "the same phenomenon uncontrolled" ([[gate-physics|Gate Physics]]) — so there is no second cosmology. Travel out, the push-your-luck timer, the contact surface, and the new-Gate pipeline are all the **Gate dialing the frontier**; the contest reuses the planet runtime-states instead of a bespoke instance type. One fewer system to balance, and the away-reserve replaces the old collapse clock.
- **Rejected — a standalone Rift extraction system** (Sealed/Open instances, Host/Scavenger, corpse-share drops as its own layer): hand-balanced and duplicative. The contested frontier world delivers the same extraction loop on instancing we already have; the underdog-crashes-a-bigger-op fantasy stays **emergent** (a weak player can still walk into a contested world). Note: this reverses the earlier "Rifts as the PvPvE extraction layer" call ([[open-questions|Open Questions]]).

## Data

- Gate tier, per-ring aperture cap, activation-cost bands — tunables in data; see [[economy|Economy]] and the Ford–Roman cap in [[gate-physics|Gate Physics]].

## Code

Placeholder until the Unity project exists.

## Open questions

- Dialing UX; spin-down / grace-window timings; the control-device component (the stealable "DHD-equivalent", name open). Tracked in open-questions.md.
