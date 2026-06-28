---
type: system
status: draft
tags: []
sources: [raw/design-overview.md]
aliases: [Rifts, Rift]
updated: 2026-06-28
---

# Rifts

Spontaneous _unstable_ wormholes — the dopamine engine and the **PvPvE contact surface** where hidden homes get found (src: raw/design-overview.md). [tentative]

## Purpose

- The everyday "what's open today" pull (variable-ratio reward), and — for Open Rifts — the genre's **extraction** loop (Tarkov / Hunt) that makes a network of [[coordinates|hidden homes]] reachable. Without a contact surface, nobody meets and the home-hunt never starts.

## How it works

- A Rift is the _unstable_ half of the cosmology: it opens on its own, can't be controlled, and **collapses on a countdown** (its instability decaying — see [[gate-physics|Gate Physics]]).
- It opens **next to** your Gate as a _separate_ hole — **never through your sealed Gate, never a backdoor for enemy players into a home.** You choose to enter; ignoring it costs only the loot.
- Each Rift telegraphs a **rank + countdown**; a rare top-rank Rift is a server-wide event.

## Two flavors (opener chooses, EXTRACT-1)

- **Sealed (PvE)** — solo/tribe-only, no outsiders, safe, baseline loot; **generates no shares**. The everyday loop and the homesteader's answer to "what do I do while building?"
- **Open (PvPvE)** — visible to players **≤ the opener's [[potential|Potential]]**, so bigger ops draw **Scavengers** and nobody above the opener sees it (`reach = exposure`). Better loot is the risk premium. **One-way, return-to-home-only** — nobody can tail you home, which is what protects [[coordinates|hidden coordinates]]. Player corpses yield **coordinate-shares**.
- **Exits:** **extract** (reach the door, keep avatar + loot) vs. **[[consciousness-link|recall]]** (emergency mind-snap home, abandon avatar + loot).

## Roles

- **Host** — the Rift's opener. **Scavenger** — a weaker entrant who crashes a bigger Host's open Rift to loot the edges and extract. A dead Scavenger hands the Host their shares + loot, so lurkers are income, not pure tax.

## Rifts → new Gates

- A stabilized Rift is where **new [[Gates]]** come from — you never manufacture one, you **tame** a natural unstable wormhole. Finite but slowly replenishing. Pacing (player-driven vs. authority-gated) is open.

## Taming

- Creatures come **only from Rifts** — see [[taming|Taming]].

## Why / rejected

- **Why one extraction contact-surface:** it subsumes the separate "arena" idea into one mechanism, and ephemeral Rift instances **scale better than persistent shards** — a bigger network gets cheaper to host, not harder.
- **Why one-way return-home:** it's what forces home-finding through avatar-shares instead of a cheap follow — the join that protects [[coordinates|hidden coordinates]].
- **Rejected — a bolted-on persistent arena:** two mechanisms where Rifts do it as one; persistent shards scale worse.
- **Rejected — two-way Rifts:** would let hunters tail players home, collapsing obscurity.
- **Rejected — peer-only matchmaking:** kills the Scavenger fantasy and the underdog's loot path; uphill visibility is the point.

## Data

- Rank scale, loot tables, share-drop rate, extraction difficulty (the stakes knob) — tunables in data. Numbers open.

## Code

Placeholder until the Unity project exists.

## Open questions

- Rank scale (E–S vs. 1–10); surface vs. orbit; the ignore-it downside (missable loot vs. a present-only PvE incursion); Rift-loot vs. raid-loot balance; whether the Authority seeds public coordinate-drop Rift-events. Tracked in open-questions.md.
