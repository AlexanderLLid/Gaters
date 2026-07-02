# Player Archetypes / Expected Playstyles

The playstyles Gaters expects, sorted by how the balance thesis treats each. **Test every proposal against this set** the same way it's tested against the design traps ([[pillars|Pillars]]) and the balance thesis ([[World Overview]]): a decision that silently starves a _served_ style, or revives a _denied_ one, is a blocking flag, not a detail.

Reference pull: **Rust** (raiding/offline-raiding, zergs, solo grinders, road PvP, trader-bandits — the toxicity reacted to); **ARK** (taming/breeding, mega-tribes farming newbies, elaborate bases, PvE homesteaders); **Valheim** (co-op, building-for-joy with no decay, exploration, boss progression — its largest audience never PvPs).

Mechanic IDs in parentheses point to the concept page that owns the mechanic; **⚠ = gap** (no mechanic yet).

## Served — the loved pillars; each needs a satisfying standalone loop

- **Raider** [decided — core fantasy]: open an uphill tunnel → breach the protected **dome** → take loot / the coordinate DB / the control device → withdraw. The headline.
  - **Find targets** — active gates radiate a traceable signature (running = detectable, sealed = dark), plus stolen coordinate DBs, blind-dial, bought intel. ([[coordinates|COORD-1/2]], [[the-wake|WAKE-1]])
  - Targets that hold **real value** — not husks worth less than the away-reserve window. ([[raiding|LOOT-1]])
  - A breach that's a **fight, not a wall** (dome + aperture combat) inside the clock. ([[gates|GATE-1]], [[raiding|SIEGE-1]], [[combat|COMBAT-1/2]])
  - An **extraction path** home, and **anonymity** (raid without leaking your own coordinate). ([[raiding|TUNNEL-1]], [[gate-uses|USE-1]])
- **Builder** [tentative]: build freely on sealed soil; no decay; defence is the perimeter at range, never doorstep cheese.
  - **Safe sealed soil** — no decay; offline loss only if the home is **found** (rare), never from neglect. ([[building|BUILD-1]], [[gates|Gates]])
  - **Build depth** worth the hours (structures, defences, production). (⚠ [[building|BUILD-2]] open)
  - A reason _and_ a way to build **exposed** too (forward bases) that pays for the risk. ([[holdings|HOLD-1]])
- **Explorer / scout** [tentative]: dial or probe unknown coordinates, map the network, blind-dial as a slot machine; early scouting becomes a late-game hit list.
  - **Unknown space** to probe/blind-dial, and **signatures to trace**. ([[coordinates|COORD-2]], [[the-wake|WAKE-1]])
  - **Coordinates as a valuable output** — tradeable, stealable intel (their "loot"). ([[coordinates|COORD-1]])
  - **Compounding discovery** — early scouting becomes a hit-list others pay for. ([[coordinates|COORD-1]])
- **Trader / economist** [tentative]: hub routing, tolls, tithe; how much a trade run risks vs. a raid is **open** (open-questions.md #23).
  - **Resource asymmetry** between worlds — a reason goods move. ([[world-types|REGION-1]])
  - **Routes and a market** — hub routing, direct dial, access lists. ([[hub-worlds|HUB-1]], [[economy|TRADE-1/ECON-2]])
  - **Risk on cargo vs. the vault** — ⚠ open. ([[gates|Gates]])
  - **Sinks** (tolls, tithe) that keep it flowing. ([[economy|ECON-3]])
- **Conqueror / warlord** [tentative]: the ARK "take a whole base" fantasy — take **and hold**, not just loot-and-leave.
  - **Exposed bases/territory worth capturing** with real value, not husks. ([[holdings|HOLD-1]], [[raiding|LOOT-1]])
  - A way to **take _and_ hold** — capture transfers the asset; defending it is the game. Routed to always-live capture targets (hubs for toll control, space gates) — a registry charter flip held by presence, not a decaying home-side claim. ([[holdings|HOLD-1]], [[hub-worlds|HUB-1]])
  - **Peer-or-greater targets only** (uphill) — conquest, not newbie-farming. ([[potential|REACH-1]])
  - **Takes the empire, never the _existence_** — conquest can wipe a found home/civilization, but never the player (you keep your existence). ([[consciousness-link|Consciousness Link]])

## Redirected — valid, but funnelled to a designated arena, never onto sleeping homes

- **PvP brawler / roamer** [tentative]: combat for its own sake → **Open Rifts, space gates** (no longer hubs).
  - An **always-live arena** (contested [[world-types|frontier worlds]], space gates) — no territory required to join.
  - **Combat fun on its own**, quick in/out. ([[combat|COMBAT-1]])
  - No need to own or defend anything to participate.
- **Clan / group / zerg** [tentative]: coordinated mass → **hub captures (toll control), big frontier pushes, Supergate events**; cannot punch down (uphill [[potential|frontier-visibility]]), so kept off solos and newbies.
  - **Large-scale objectives** that reward coordination (hub captures, Supergate events, top-rank Rifts). ([[hub-worlds|HUB-1]], [[progression|PROG-1]])
  - A hard **can't-punch-down** floor — kept off solos / newbies' existence. ([[potential|REACH-1]])
  - **Group-only payoffs** worth the org overhead. ([[progression|PROG-1]])

## Denied — the ARK/Rust pain the design exists to remove; scenarios must confirm each stays dead

- **Offline raider** [decided — reframed]: a sleeping player's **existence** stays denied (the body is on Earth). Their **home** is no longer immune — a **found** coordinate can be raided offline — but this is **rare** (you must find them; offline raids skew to the recently-active), not the ARK certainty. Their exposed [[holdings|holdings]] stay always-contestable without any search.
- **Megabase newbie-farmer** [decided — denied]: blocked by **obscurity + uphill [[potential|frontier-visibility]]** — a whale can't see a small player's frontier pushes to harvest their shares.
- **Turtle / babysitter** [decided — denied as a _dominant_ style]: sealing buys **obscurity, not immunity** (the home is mortal — [[coordinates|Coordinates & Obscurity]]) and stays stagnant; the aperture cannot be cheese-fortified.

## Resolved forks — were undecided; now served at a defined scope

- **Tamer / breeder** [tentative — narrow yes]: include [[taming|taming]] (Rift-sourced companions); decline deep ARK breeding/genetics (a babysitting chore by construction).
- **Peaceful homesteader** [tentative — served]: Valheim's largest audience never opens — build, explore, run solo PvE on the [[world-types|frontier]] forever, progressing to a **soft ceiling** ([[progression|PROG-2]]), never punished for staying sealed. Safe because a never-leaver stays **unfound**, not by an inviolable wall.
