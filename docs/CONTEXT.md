# CONTEXT.md - Shared Language

The working vocabulary for Gaters. Use these terms consistently in pages, code,
commits, and chat. Add a term when a concept keeps needing a sentence to
explain. This is a glossary, not canon; it decodes jargon.

## Core concept

- Gate - the network device connecting planet-bases. Sealed by default. A _stable_
  wormhole; cannot be manufactured, only found/claimed/repaired.
- Planet-base - a single player's planet and home base.
- Gater - a player who goes out through the Gates into the world. The game
  is named after them.
- Hub world - a neutral, lawless crossroads world for routing trade to distant
  partners; capturable for tolls (open PvP contest lives in Rifts, not here).
- Rift - an _unstable_ wormhole: spontaneous, uncontrollable, self-collapsing on a
  countdown. Opens near Gates onto opt-in content. The PvE/loot event engine, and (Open Rifts)
  the PvPvE extraction contact-layer where homes get found.
  - Sealed Rift - solo/tribe PvE; safe; no shares shed. Open Rift - PvPvE, visible to players
    ≤ the opener's Potential; one-way, return-to-home-only.
- The Gate Network - the full web of every Gate.
- Coordinates - a re-dialable Gate address; intel you discover, trade, steal, hide.
  Stored as a geohash, **assembled from avatar-shares**, and **re-keyable**.
- Coordinate-share - a fragment of a tribe's home coordinate carried by every fielded avatar;
  loot a dead avatar to get one. Enough shares narrow to a dialable address.
- Re-key / epoch - rotating your home coordinate; **voids all shares already collected**. The
  master knob for the online:offline raid ratio.
- Potential - one sticky high-water-mark per tribe (Gate capability + footprint + tech + roster);
  drives Rift visibility and the raid clock. Solo = a tribe of one.
- Operator base - the cheap default respawn anchor you rebuild from; your home/civilization is
  mortal, your existence (body-on-Earth) is not.
- Power core - scarce, scavenged Gate fuel; the primary activation cost.
- Charter / tithe - the license to hold a Gate-world, and the cut owed for it.
- Holding - an asset staked _outside_ the home sustaining field (forward node, resource
  tap, cache); has no Gate to seal, so it is offline-contestable (the bounded offline-raid layer).

## Exposure states

- Sealed - gate closed; safe from a _cold_ breach but **not inviolable** — a found home is
  raidable. Safety is being **unfound**, not a wall.
- Port open - partial exposure for trade.
- Gate open - full exposure; raiding is possible.
- Siege timer - the committed window once a gate is opened.
- Mask energy - life-support drawn from the Gate; bounds survivable range and acts
  as the raider timer. Split into two numbers (below).
- Sustaining field - the bubble an active Gate projects; the mask tops up inside it.
  The soft world border (a cost gradient, not a wall).
- Field radius - how far you can roam/build; the dev-tunable expansion knob.
- Away reserve - how long the mask lasts once you leave a field; the raider clock.
  Kept decoupled from field radius on purpose.

## Identity & presence

- Avatar - the projected body you pilot through a Gate; the disposable layer. Only avatars
  die (gear drops, lootable); you develop several but pilot one at a time.
- Body-on-Earth / consciousness-link - your real body, held by the Coalition on Earth; presence
  is the linked mind projecting into an avatar. The in-world basis for respawn and sanctioned violence.
- Recall - snapping your mind home to defend; costs the away-avatar + gear left behind. A
  forced offense-vs-defense choice, not free teleport. In a Rift, the emergency exit.
- Extract - the normal Rift exit: reach an exit and go home with avatar + loot intact.
- Host / Scavenger - a Rift's opener / a weaker entrant who crashes a bigger Host's open Rift to
  loot the edges and extract.
- Mind-XP - permanent, account-wide, earned by discovery (first-time unlocks). Never lost.
- Avatar-XP - per-avatar proficiency, earned by doing; lost on avatar death, re-leveled fast up
  to your strongest-ever mark (high-water-mark).

## Planet runtime states

- Empty - a database row only.
- Solo-occupied - one owner present; a cheap server-side PvE tick.
- Contested - a raid or trade in progress; server-authoritative instance.

## Storage

- Seed-plus-deltas - planets stored as a seed plus changes, lazy-loaded on
  demand, so server cost scales with online players.

## Stack

- Unity with C#. Server-authoritative always; only online players' planets are simulated.

(Replace or extend as the design firms up. Keep each definition to a line.)
