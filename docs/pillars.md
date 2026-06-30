# Pillars

The cores that must work. Full detail lives in the concept pages — this is the focusing list.

> Organized by **failure mode**, not by object — each core is something that can fail on its own and take the game down. Don't fold cores into the Gate just because they route through it (reachability and mask energy both do); nesting by object collapses the list and hides the distinct failures.

**Thesis — reach equals exposure** (see [[World Overview]]). Aggressor fantasy, no defender tax.

1. **The Gate** — the one object everything routes through.
   - Travel, mask-energy source, exposure surface, economy sink.
   - Finite and unbuildable: found / claimed / repaired / relocated / tamed.
2. **Obscurity** — the home is safe while **unfound**, never by a wall.
   - No sealed state, no re-key; protection is low odds of being found + decaying share-trail + bounded loss.
3. **The siege clock** — the babysitting fix.
   - A raid is finite, bounded by the attacker's away reserve; no seal needed, no offline jump.
   - Raid commits → tunnel propped open for a bounded fight.
4. **Mask energy — two decoupled numbers.**
   - Field radius = soft world border + studio expansion lever.
   - Away reserve = raider's clock on hostile soil.
   - Must stay separate or world-growth nerfs raids.
5. **Contained conflict.**
   - Bilateral directional tunnels (anti-zerg).
   - Asymmetric uphill-only reach (anti-whale — see [[potential|Potential]]).
6. **Stagnation economy** — the counterforce that makes players open up.
7. **Gate-based instancing** — what makes it buildable solo.
   - empty row → solo (cheap server tick) → contested (full server instance).
   - Server cost ∝ online players, not world size.
8. **Misdials** — the engagement engine while you're home and quiet (a stray outbound lane to run); opportunity, not invasion.

**Gate on all of it:** will players actually open their Gates? Make-or-break — and the one thing a greybox **can't** settle: it's a population equilibrium, not a feel. The greybox proves the loop is _fun_; whether the population _opts in_ only shows at playtest scale (see Validation).

## Design traps (failure modes to avoid)

Each is a way the exposure model could collapse; the design's counter is named.

- **Turtle equilibrium** — if staying hidden = safe forever with no downside, everyone bunkers. _Counter:_ the [[economy|economy forces exposure]] (a base that never opens out stagnates), and staying hidden buys obscurity, not immunity ([[coordinates|the home is mortal]]).
- **Trade-suicide** — if trading exposes the vault like raiding, every economic act invites a wipe. _Counter:_ trade is **cargo out** — it risks the cargo, never the home vault (only a coordinate-holder can breach it).
- **Offline grief** — a found home cracked repeatedly while you sleep would be the ARK tax. _Counter:_ obscurity (hard to find), a **decaying share-trail** (the lock fades if a hunter stops collecting; relocate for a fresh coordinate), and **bounded loss** (a minted bounty, never your existence). No seal exists, so there's no instant-seal dodge to exploit either.
- **Zerg pile-on** — if opening exposes you to everyone, mass wins. _Counter:_ [[raiding|bilateral directional tunnels]] (contained duels).
- **Whale / newbie-farming** — a megabase griefing small players kills retention. _Counter:_ obscurity + uphill [[potential|frontier-visibility]] (a whale can't see a small player's frontier pushes to harvest their shares).
- **Doorstep kill-box** — walling the aperture into a box nullifies raiding. _Counter:_ the [[combat|dome]] destroys manual builds (not terrain) — fortify the perimeter at range, never the doorstep.

## Validation — two questions, two scales

Two risks, de-risked differently. Don't conflate them.

- **Is the loop fun? — greybox-answerable.** Do dialing, the bilateral tunnel, the siege clock, and real PvP combat feel good moment-to-moment? Build a **greybox proof** before any art. Cheap, early, and _necessary but not sufficient_.
- **Will the population open up rather than turtle? — only shows at scale.** The real make-or-break, and a greybox **cannot** settle it: open-vs-bunker is an emergent **equilibrium** of a live economy — it needs hundreds of players, market depth, and social pressure before it stabilises. Five testers say nothing about where 500 land. Treat it **assume-and-commit** ([[Systems Overview]]): reason from games that ran the experiment (Rust, EVE, Albion, Foxhole), model the [[economy|economic pressure]], and prove it only at a **scaled paid playtest**. The biggest risk is the one you can't cheaply prove.
- **Minimum viable slice:** dialing + obscurity (a decaying share-trail), a bilateral raid tunnel, loot worth taking, economic pressure against turtling, real PvP combat. **Ground Gates only**; space Gates later.
- **Design for success enabling expansion, not requiring it** — a working small version is a complete thing, not a broken large one. (Build-cost detail: [technical-challenges.md](technical-challenges.md).)

## Standing constraint — IP / originality

The Gate/wormhole _mechanic_ is genre-generic, but Gaters must avoid Stargate's _expression_: no "Stargate," no Egyptian skin, no chevron/glyph set, no copied proper nouns, no ascended-precursor lore, no named dialing device. Original names and story throughout. (A practical design line, not legal advice.)
