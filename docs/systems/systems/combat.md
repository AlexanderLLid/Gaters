---
type: system
status: draft
tags: []
sources: [raw/design-overview.md]
aliases: [Combat]
updated: 2026-06-28
---

# Combat

Third-person, gear-and-positioning PvP — the model resolved enough to greybox (src: raw/design-overview.md). [tentative]

## Purpose

- "Real PvP combat" is a required ingredient of the first playable. Skill lives in **loadout and positioning**, not ability-twitch — which keeps the home-soil edge tunable as a _number_, not a reflex contest.

## How it works

- **The mask _is_ the resource — no separate health bar.** On your own soil the [[mask-energy|mask]] tops up (full durability + home edge); on hostile soil your **away reserve is both raid clock and durability** — taking fire drains it. The fair-but-beatable home edge falls out of the field gradient, with no bolted-on buff.
- **The aperture is a two-way fire-lane.** Fire and momentum pass through a live Gate ([[gate-physics|physics]]): the defender shoots _into_ the tunnel; the raider lays covering fire _up_ it.
- **Muster cap (COMBAT-2).** The dome admits only **K** combat-effective defenders at the aperture lip, K scaling with the **attacker's throat power** (the dialer sizes the dome). A skilled solo can beat a _mustered_ defence **at the breach** — but only there; the rest hold the perimeter at range, and the attacker's [[raiding|away-reserve clock]] bounds grab-and-go.

## The dome

- An active Gate projects a tier-scaled protective **dome** — a field, sized by the power driving the throat, that lets matter cross the otherwise-lethal aperture safely. The Builders engineered it **on purpose: a Gate can never be walled in.**
- **Protects transit, not occupation** — shields you while crossing (no spawn-camping) and from the aperture's own energy; once you stand in it, combat is normal.
- **Destroys manually-built structures** in its radius (it senses _construction, not material_, so natural terrain is spared) — so you cannot entomb a Gate or pack a kill-box against it. Players are shielded/recalled, never killed by it.
- **Scales with power** — bigger throat → bigger dome → a bigger protected enemy beachhead on your soil. A Supergate sits in an open plaza ("impossible to hide"). **In a raid the attacker sizes the live dome**, so a defender can't shrink it to a pinhole.
- Grounded in the tidal shear at a flaring throat mouth; the protective field and the build-vs-natural discrimination are Builder **conceits** — see [[gate-physics|Gate Physics]].

## Two venues, one model

- The same combat serves the **home raid** (at the aperture, under the clock) and the **standalone arena** ([[hub-worlds|hubs]] capture, Open [[rifts|Rifts]], space gates — quick in/out, no territory required).

## Tech spine (precursor-tech-hybrid)

- Combat verbs climb the **Standard → Heavy → Supergate** ladder: bow/melee + scavenged ballistics at the low-tech start (low-poly art as _canon, not budget_), scavenged Builder tech (mask, energy weapons, the driven dome) mid-game, fleet/ship combat at the top. Through-line: you fight on a Builder-tech budget you don't fully understand. [tentative]

## Why / rejected

- **Why the dome protects transit and destroys builds:** it closes the **doorstep kill-box** trap (the Rust/ARK "honeycomb the raid entrance" meta that would nullify raiding and revive the turtle) — a fair beachhead, not a cheese funnel — without being a persistent combat shield.
- **Why the attacker sizes the dome:** stops a defender shrinking their own dome to choke arrivals into a pinhole.
- **Rejected — a dome that protects the _defender's_ base** (a shield scaling with their power): rebuilds the turtle, the exact thing the dome and the driven aperture kill.

## Data

- TTK curve, K(throat) values, gear tiers — tunables in data. Numbers open.

## Code

Placeholder until the Unity project exists.

## Open questions

- Exact TTK, K-values, gear tiers; the second-to-second verbs (movement, abilities). Tracked in open-questions.md.
