---
name: lore-options
description: Grounded lore paths — propose two or three distinct ways to resolve a lore question or close a hole, each naming the buildable mechanic or server/scaling lever it licenses. Reached when resolving an open question, and by the skeptic and lore-architect agents when they need to offer paths.
---

# Lore Options

Turn a lore question or hole into a short menu of **grounded** paths the human can pick from.

## Grounded is the bar

A path is **grounded** when its fiction earns its keep by licensing a concrete thing the devs can build, tune, or scale. The test for every path: _name the lever it hands the devs._ Fiction with no lever is colour, not a path — cut it.

The levers already live in the tech model — `docs/technical-challenges.md` (Storage, Stack, planet runtime states) and [[mask-energy|Mask Energy]] (field radius). Map fiction onto these, or onto a new lever devs can clearly build:

- **Reachability** — which worlds are live. _Empty = a DB row only_, seed-plus-deltas, lazy-loaded. Lore that gates _which Gates open_ is really lore that decides _when a planet-server is provisioned_. ("Not all Gates are open" → a server spins up only when its Gate is dialed and fueled.)
- **World border** — how far a player roams. _Field radius_ is the dev-tunable knob; the _sustaining field_ is the soft border. Lore that bounds movement sets the streaming/border radius. ("Can't go past X from the Gate" → mask energy decays beyond the field.)
- **Instancing** — who is authoritative. _Solo-occupied = client-authoritative; Contested = server-authoritative instance._ Lore that decides when others reach you decides when an instance spins up.
- **Cost / scarcity** — _power cores_ gate activation; the _tithe_ gates holding. Lore that throttles activity maps to a resource sink devs balance.

## Balanced is the second bar

A grounded path can still be a _bad_ path if it breaks the game's balance thesis: the **aggressor fantasy without the defender tax** (no base-babysitting, no decay, no offline raiding, no forced presence — the ARK/Rust pain Gaters reacts to). For every path, test it against the known **design traps** ([[pillars|Pillars]]):

- **Turtle equilibrium** — sealed = safe forever with no downside, so nobody opens.
- **Trade-suicide** — any economic act risks the whole base, so nobody trades.
- **Offline-dodge** — you can dodge a raid by logging off (or can't ever close).
- **Zerg pile-on** — opening exposes you to everyone, so bodies win.

Plus the healthy/toxic line: players should be _excited_ to log in, **never** _obligated_ on pain of loss — anything that punishes an absent player has reinvented decay. And the guardrail: a path may cut the _cost to reach_ a target, never the _target's defenses_. A path that reopens a trap isn't disqualified — but say so, and say which knob keeps it closed.

- **Inheritance** — does this path mint a new knob, or spend one that already exists upstream? Flag a new knob and name why an existing one cannot carry it (see CLAUDE.md, Design order).

## Method

1. **Find the lever first.** Ask which buildable knob is really at stake (reachability / border / instancing / cost / a new one). The fiction exists to justify _that_.
2. Produce **two or three genuinely distinct paths** — distinct in _mechanism_, not flavour. Three re-wordings of one idea is one path.
3. Order them **cheapest first** — least existing canon and least new tech disturbed at the top, so the quick pick is usually #1.
4. For each path, one line: **the fiction → the lever it licenses → its trade-off → the trap it avoids or risks.**
5. **Don't pad.** Two real paths? Give two. If nothing is grounded, the finding is that the question needs a buildable lever before lore can answer it.
6. **Always recommend.** End by naming the path you'd pick and the one-line why. The pick need not be the cheapest — when it isn't, say what justifies the extra cost.

## Shape

> **Path A (cheapest)** — <fiction>. Licenses: <lever devs build/tune>. Trade-off: <what it costs>. Balance: avoids/risks <trap>.
> **Path B** — <fiction>. Licenses: <lever>. Trade-off: <…>. Balance: avoids/risks <trap>.
> **Path C** — <fiction>. Licenses: <lever>. Trade-off: <…>. Balance: avoids/risks <trap>.
>
> _Recommend:_ <which, and the one-line why>.
