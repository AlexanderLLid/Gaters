---
type: system
status: draft
tags: []
sources: []
aliases: [Potential]
updated: 2026-06-28
---

# Potential

The measure of an actor's strength and progression — one sticky number that drives frontier visibility and the raid clock. [tentative]

## Purpose

- A single honest strength metric so near-peers meet (fair fights, peer-wipe selection) and the weak can't be trivially farmed.

## Model (POT-1)

- **One sticky high-water-mark per tribe**, aggregating:
  - **Gate capability** — highest Gate tier you can field/power (your reach ceiling).
  - **Footprint** — territory + [[holdings|holdings]] held.
  - **Tech depth** — how far up the tree you've unlocked.
  - **Roster** — active members/avatars (makes a tribe read bigger than a solo).
- **High-water-mark** — rises as you grow, **falls only on genuine dismantling**; no powering-down to dodge.
- **Account/tribe-wide** — closes smurfing; strength can't be hidden behind a small avatar.
- **Progression = Potential climbing.**

## What it drives

- **Frontier visibility** — a contested frontier world is visible/reachable to players ≤ the pusher's Potential, so a whale can't see (or farm) a small player's pushes. This is the anti-whale guardrail (`reach = exposure`); see [[Gates|Gates]] FRONTIER-1.
- **Frontier/hub tiering** — near-peers meet by construction.
- **[[raiding|Raid clock]]** — the away-reserve keys off home-Gate power, a Potential component.
- **Not** home-raid eligibility — that's gated by **finding** now ([[coordinates|shares]]).

## Tribe is the ownership primitive (TRIBE-1)

- The unit that owns a home, holds a [[coordinates|coordinate]], and carries Potential is a **tribe**; a **solo player is a tribe of one** (no separate code path).
- Roster is a real component, so a big tribe reads bigger — and **sheds more shares of the same coordinate**, so it's inherently findable. `reach = exposure` at the group level.

## Why / rejected

- **Why one sticky high-water-mark, account/tribe-wide:** it closes the two dodges at once — you can't power down to look weak, and you can't hide strength behind a small avatar (smurfing). Falling only on genuine dismantling means "raid longer = build more" feeds growth into capability.
- **Why the tribe is the primitive:** one ownership model, not two; and the big-tribe-is-leaky consequence (more members shedding the same coordinate) makes findability scale with size for free.
- **Rejected — live power draw:** dodgeable by powering down.
- **Rejected — separate metrics** for reach / stamina / visibility: one number is simpler; split only if tuning forces it.
- **Rejected — solo homes + loose alliances:** no shared fortress; "wipe a tribe" would mean hunting each member's base, undercutting the ARK fantasy.
- **Rejected — hybrid (solo homes + tribe-only holdings):** keeps two ownership models; tribe-of-one unifies them.

## Data

- The curve converting the four components into the number; component weights; whether roster counts — tunables in data. Numbers open.

## Code

Placeholder until the Unity project exists.

## Open questions

- Component weights and the climb curve; tribe shared-ownership sub-rules (shared with [[coordinates|Coordinates & Obscurity]]). Tracked in open-questions.md.
