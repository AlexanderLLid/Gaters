---
type: system
status: draft
tags: []
sources: [raw/design-overview.md]
aliases: [Coordinates, Obscurity, Coordinate Database, Mortal Home]
updated: 2026-06-28
---

# Coordinates & Obscurity

How hidden homes get found — the mechanism behind the **mortal home**: protection is obscurity (a re-keyable share-trail), not invulnerability (src: raw/design-overview.md). [tentative]

## Purpose

- Gives real, ARK-style stakes back (a home _can_ be wiped) **without** the defender tax. The tax came from the **certainty** of offline loss; hidden coordinates remove the certainty, not the possibility.
- Only your **existence** is unloseable (the [[consciousness-link|body-on-Earth]]); your **civilization is mortal**.

## Coordinates as intel (COORD-1)

- You never build a new Gate; you **dial from the one you own** and save the **coordinate** — a re-dialable address in your homebase **coordinate database**.
- The DB is a **raidable asset** — a breach can steal the **map** (scouted planets and routes).
- A bookmark removes the _discovery_ cost, never the _travel_ cost (re-dialing still burns distance-scaled fuel).
- **Blind exploration-dialing** (dial the unknown for a fuel cost → a random planet of random richness/danger) is the variable-ratio hook for explorers. [leaning yes]

## Found by avatar-shares (SHARE-1)

- A home coordinate is a **geohash**; **every fielded avatar carries a share** of it.
- **Loot a dead avatar → get its share.** Below a threshold = a coarse region (hunt/scout, can't dial); at/above threshold = the exact dialable coordinate. Shares are **tradeable intel** — buy the last one you need in the [[world-types|Safe Core]].
- **Epoch-bound, re-keyable.** Re-keying your address **voids every share already collected**. Cheap/automatic while sealed at home. **Re-key cost/cadence is the master knob** for the online:offline raid ratio.
- **Anti-dodge:** every fielded avatar carries a _current-epoch_ share (no clean decoy avatars); shares die on re-key (no infinite banking).

## Consequence — uphill relocates

- Home raids are gated by **finding** (assembling shares), not a power wall. Anti-whale-farm protection moves to [[rifts|Rift-visibility]].
- Ownership is the **[[potential|tribe]]**: one shared coordinate, every member sheds shares of it — so big tribes are inherently **leaky** (endgame siege targets) and solos stay stealthy. `reach = exposure` at the group level.

## Why / rejected

- **Why mortal, not safe:** a perfectly-safe home makes the apex base un-raidable — inverts `reach = exposure` where it should bite, kills real stakes. Obscurity restores them without the tax (the tax was the _certainty_ of loss, not the possibility).
- **Rejected — inviolable home:** removes home stakes, makes the frontier pointless to leave.
- **Rejected — presence-gated wipe** (home falls only to a present defender): a no-show abuses it; obscurity makes it unneeded.
- **Rejected — permanent coordinates:** a leak is forever; re-key is the recovery valve.

## Data

- Threshold _k_, share decay, re-key cost/cadence, reveal flavor (prefix-narrowing default vs. triangulation) — tunables in data. Numbers open.

## Code

Placeholder until the Unity project exists.

## Open questions

- Where "return home" lands if the home was wiped while you were away; tribe shared-ownership sub-rules (who may re-key, member churn, defection). Tracked in open-questions.md.
