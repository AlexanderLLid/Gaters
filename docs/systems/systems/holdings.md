---
type: system
status: draft
tags: []
sources: [raw/design-overview.md]
aliases: [Holdings, Holding]
updated: 2026-06-28
---

# Holdings

Assets staked _outside_ your home sustaining field — the one place the design admits **bounded offline raiding** without reviving the babysitting tax (src: raw/design-overview.md). [tentative]

## Purpose

- Serve three archetype gaps in one layer: the offline raider's _ambition_ (siphon exposed output, never the home), the builder's "build exposed for a payoff," and a forward-reach option the home field can't reach.

## What a holding is

- A new object class: a forward node, claimed resource tap, or off-site cache. **Space-gate holdings** and **captured hubs** are the top-tier cases.
- It has **no woken-sealed Gate of its own**, so there's **nothing to seal** — the same reason space gates can't be turtled. (The home itself is mortal-but-must-be-found; a holding is the **always-on, no-search** target — see [[coordinates|Coordinates & Obscurity]].)

## The mechanic — flow-tap, no window

- **Always contestable** by an **uphill** raider (potential computed from the _holding's own value_, so a whale can't farm a newbie's node). **No scheduled vulnerability window.**
- **Offline defence is a frozen snapshot** of the automated defences you left — the attacker fights AI, not the absent owner.
- **Winning denies; it doesn't bleed.** Taking a holding flips it **dormant/neutral** (owner stops the trickle) and grants the attacker a **one-time, capped buffer** — **never an ongoing stream diverted to the attacker.** So there's no "a rival is paid by my node while I sleep" meter, and **no reclaim obligation**: you re-tap it next time you play, as normal play.

## Why it's on-thesis

- The worst an offline player loses to this layer is one capped buffer of output they weren't collecting — never structures, stock, progress, or existence. That's "I missed a trickle," not the ARK/Rust offline-wipe.

## Conqueror's take-and-hold

- "Take **and** hold" routes here and to [[hub-worlds|hubs]] / space gates — capture is a **registry charter flip** defended by presence in an opt-in arena, not a decaying home-side claim.

## Cost fit

- A holding is a DB row at rest; it spins a Contested instance only when an attacker is online and pays to dial it, so server load stays ∝ active raiders.

## Why / rejected

- **Why a flow-tap with a capped buffer, no window:** the bound is _structural_ — at most a buffer of uncollected output, never structures or progress — so it's not the ARK/Rust offline-wipe and creates no reclaim chore.
- **Rejected — a scheduled vulnerability window:** pulls players onto the network's clock and is exploitable by timezone capture, zerg broadcast, and wait-out.
- **Rejected — an ongoing flow diverted to the attacker:** reads as a decay meter bleeding to a rival; the babysitting tax reborn on offence.
- **Rejected — a home-side capturable with a hold-or-lose timer:** "reverts if not held by presence" is base-decay in costume; take-and-hold is served at hubs / space gates instead.

## Data

- `buffer_cap`, dormancy / re-tap values — tunables in data. Numbers open.

## Code

Placeholder until the Unity project exists.

## Open questions

- Whether a holding ever gets a present-defender fight (leaning no). Tracked in open-questions.md.
