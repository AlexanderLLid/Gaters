# Systems

The Gaters mechanics bible in one file. The world it serves is [[world]]; the deep physics
in [[gate-physics|Gate Physics]]. System-local open questions live beside the mechanics
they affect; big cross-cutting forks live in [[questions]]. Numbers never live in prose —
each mechanic names its tunables; values land in data at playtest.
Code references stay placeholders until the game project exists.

## Design pillars

- **Reach equals exposure** - growth requires opening outward; opening outward creates risk.
  - **Why:** the survival/PvP model depends on action creating vulnerability.
- **Mode by action, not by server** - players choose today's risk by what they do with Gates, not by joining a permanent PvE/PvP world.
  - **Why:** real life changes day to day; a player should be able to build quietly, explore, trade, raid, or join a war without abandoning their identity or swapping servers.
- **Bodies die, players persist** - avatars, gear, homes, holdings, and empires can be lost; player existence cannot.
  - **Why:** PvP needs real stakes without account deletion or an ARK/Rust sleeping-body punishment.
- **Safety is obscurity** - a quiet home is hard to locate, not invulnerable.
  - **Why:** invulnerable homes kill raid stakes; guaranteed vulnerability creates defender tax.
- **No defender tax** - absence must not create routine loss, decay, or chores.
  - **Why:** players should be excited to log in, never obligated on pain of loss.
- **Finite Gates drive conflict** - at the current layer, Gates are found, claimed, repaired, and fought over; whether players can ever manufacture or grow new Gates is open.
  - **Why:** finite infrastructure creates conflict without arbitrary scarcity, while leaving the long-term Gate-creation question undecided.
- **The Gate is the seam** - travel, trade, raiding, claiming, discovery, and exposure all route through Gates.
  - **Why:** one central object keeps the design legible and avoids parallel systems.
- **Play stays Gate-proximate** - important action must be pulled toward active Gates: homes, dead houses, raid fronts, high-value resources, AI bases, claims, and contested objectives should live close enough to Gates that one generated chunk can carry a session.
  - **Why:** this keeps the Gate as the real centre of play, makes local procedural generation viable for a solo build, and prevents "the whole planet" from becoming the actual required play space.
- **Explain rules, not the soul** - avatar continuity works operationally; the metaphysics stay part of the Builder mystery.
  - **Why:** over-explaining consciousness creates contradictions and weakens the central mystery.

## Core loop [tentative]

The cycle the thesis produces, at three scales. Documented here only; other sections
link here, they do not restate it.

- **Moment-to-moment** - build, gather, scan, and solo PvE while quiet; open a Gate to trade, raid, explore, or contest, accepting the exposure that action creates.
- **Session** - log in with no decay chore; choose today's risk surface through Gate use; resolve finite engagements; close tunnels when done.
- **Long-term** - grow the base, raise charter clearance, climb world tiers and Potential as more of the network wakes; no base to babysit.

- **Validation note:** whether players actually open up is a population equilibrium, not a greybox feel test; see [[#Validation — two questions, two scales|Validation]].

## First prototype priority — PvPvE raiding [current call]

The first real build should prove the **Gate-proximate PvPvE raid surface**: your home
Gate gets or finds a dial to a generated world chunk with one raidable AI/dead-house base,
basic defenders, loot, extraction, and reset/retry. Do not spend the first proof on
markets, galaxy-scale routing, full MMO hosting, space, taming depth, or perfect portal
rendering.

- **Core question** - can the game generate a local Gate site that is fun and valid to raid?
  - **Includes:** home-Gate dial offer, sensor reading, terrain chunk, Gate placement, dome/no-build clearance, base platform/foundation, modular AI/dead-house base, pathable defenders, reachable loot, extraction/recall, and save/reload of the generated result.
  - **Excludes:** proving that PvP itself is fun. Rust/ARK/EVE already prove the appetite for raiding, loss, alliances, betrayal, and loot pressure; Gaters must prove its Gate-native delivery.
- **AI/dead-house bases are the first PvE target** - they train the raid loop, work at low population, and let the generator be tested without waiting for a player economy.
  - **Test:** generate many seeds; reject any base that blocks the Gate dome, has unreachable loot, traps defenders, has no attack lane, or exceeds the object/performance budget.
- **Gate sensors make the raid a wager** - before committing, the home Gate can return an imperfect read on the far Gate's value, danger, base tier, defenders, loot signature, instability, or weirdness. Better sensors reduce uncertainty; no sensor or a bad read makes the dial a gamble.
- **Bad reads create the Solo Leveling twist** - a lane can be misranked, turn hostile after entry, lock recall/re-dial until a condition is cleared, reveal a hidden second base, or become a double-site. These are special lane modifiers, not a separate travel system.
- **Player bases come after generated raid sites** - free building matters, but the first prototype needs predictable platform/foundation sites so AI bases and validation can work.
- **Gate network logic is not the hard first proof** - coordinates, heat, ownership, and route records are normal software systems; keep them stubbed until generated raid sites work.
- **Success condition** - a solo dev can press "new seed" and repeatedly get a playable Gate-adjacent raid: approach, breach, fight AI, loot, extract, reload.

## Failure modes

Each is a way the exposure model can collapse. Test every proposal against these.

- **TURTLE-1 Turtle equilibrium** - if quiet play is optimal forever, everyone bunkers.
  - **Counter:** quiet play has a soft ceiling; growth needs exposure.
- **TRADE-SUICIDE-1 Trade-suicide** - if trade exposes the vault like raiding, nobody trades.
  - **Counter:** trade is emergent and in-person, never a required home storefront or remote market; PvE scarcity is solved primarily through exploration/frontier play.
- **OFFLINE-DODGE-1 Offline dodge** - instant closing mid-raid cancels raiding.
  - **Counter:** committed tunnels, siege clocks, and finite engagements.
- **ZERG-1 Zerg pile-on** - if opening exposes you to everyone, mass always wins.
  - **Counter:** bilateral directional tunnels and visibility limits.
- **WHALE-1 Newbie farming** - if large houses can locate small houses easily, retention dies.
  - **Counter:** obscurity plus uphill frontier visibility.
- **DOORSTEP-1 Doorstep kill-box** - if the Gate can be walled in, raids become spawn cheese.
  - **Counter:** the Gate dome clears the aperture.
- **PERMA-LEAK-1 Permanent coordinate leak** - if one leak lasts forever, one mistake dooms a home.
  - **Counter:** stale intel, address rotation, heat decay, relocation, or another recovery valve.

- **Healthy/toxic line:** players should be excited to log in, never obligated on pain of loss; anything that punishes an absent player has reinvented decay.

## Gates

The central object: every planet-base sits on a Gate, and all reach flows through it. The
deep physics treatment is [[gate-physics|Gate Physics]]; this section is the game model.

- Owning a planet means **owning its Gate** — the reason bases are planets, not an arbitrary rule.
- Gate finiteness at the current layer is the root of all conflict; opening your Gate is how you reach the world _and_ how you become reachable.
- **Woke closed; Gate creation open.** The network came online with every aperture shut. For the current design layer, Gates are **found, claimed, and repaired**; whether players can ever manufacture, grow, print, or otherwise create new Gates stays open (GATE-1).
- **Closed / open are connection states.** Closed = no active tunnel; open = one active tunnel exists. Safety comes from coordinate obscurity, not from the word "closed."
- **Travel is one-way and directional.** The _dialing_ side opens the tunnel and pays the power, so a raid is inherently the attacker pushing through a tunnel they opened.
- **One ring props one live tunnel at a time (GATE-2).** A throat joins exactly two mouths ([[gate-physics|Gate Physics]]), so a Gate never holds multiple simultaneous connections — multi-party arrivals at one world are **sequential dials**, never held tunnels. Multi-tunnel exists only as multiple rings (Supergates, deferred).
- **Fire and momentum pass through** a live Gate — breaching is a firefight at the aperture, not a loading screen (see [[gate-physics|Gate Physics]], [[#Combat|Combat]]).
- **Dialing needs the target's coordinates** — a planet whose coordinate nobody has assembled is effectively hidden (see [[#Coordinates & obscurity|Coordinates & Obscurity]]).

### Tiers — driven, not built

- The **ring is fixed**; the **aperture** is a runtime variable set by how much exotic matter (power cores) you channel, capped per ring by the **Ford–Roman quantum inequality** ([[gate-physics|Gate Physics]]).
- **Standard → Heavy** = one ring fed lightly vs. hard (wider aperture, bigger payload, bigger dome, more cost).
- **Supergate** = past one ring's ceiling — combine multiple **found Builder segments** into a megastructure; recovered and repaired at the current layer, with true new-building still open.
- **Overdrive** one ring past its ceiling and the throat destabilises into an uncontrolled, self-collapsing lane (the gate-overload bomb).

### Gate variation — the Builder "config" [tentative — rough idea]

- **Loose idea, needs tuning — nothing depends on it.** Gates differ (field size, stability, what they dial, what grows there) maybe because each carries a small Builder-seeded **config** the [[world#Behind the curtain — the Builders' truth|von Neumann probes]] tuned **blind** — they couldn't predict a config's behaviour, so gates came out imperfectly/variously tuned. That imperfection is the variety.
- Just an explainer for _why gates aren't all the same_. It doesn't touch dialing/findability (still coordinates), it's **not** a selection system (explored and rejected — [[questions]] #20), and the _why_ behind the probes stays parked. Shape it properly if/when it matters.

### Connection state (PRES-1)

- **Closed** — no active tunnel exists. Closing ends current contact; it does not erase coordinate intel someone already has.
- **Open** — one active tunnel exists. Travel, trade, raids, and exposure happen through open Gates.
- **Presence stops cold contact, not discovery.** A closed Gate cannot be entered through a shut throat, but a located home is still a valid raid target under the raid rules.
- **Offline tends _toward_ safety, not away from it.** A quiet closed Gate sheds fewer traces, so logging off lowers your odds of being located; it does not make a located home magically invulnerable.
- **Open:** exact spin-down, grace-window, and offline raid timings; what can pass through or be sensed while closed, including comms, diagnostics, registry state, trace sampling, and whether goods/energy/people are fully blocked.

### Reaching the frontier — dial out (FRONTIER-1)

- **Two things you can dial:** a **claimed coordinate you've assembled** (a raid — needs current intel), or **out into the unclaimed frontier** (expansion). You can't pick _which_ frontier world — dialing an unclaimed address lands you at a **random** one; **claimed homes are off the pool**, so no one is ever dropped onto occupied soil (preserves obscurity).
- **Raid-lane offers** — your home Gate can surface or discover candidate lanes to frontier Gates with AI/dead-house bases. A lane is described by a **sensor reading**, not perfect truth: estimated value, danger, base tier, defenders, loot signature, instability, and special modifiers can be missing, vague, or wrong.
- **Frontier travel is one-way** — you can't return the way you came; you arrive, and the away-reserve clock is already ticking. Push your luck against it for loot, or **claim the far Gate** before it runs out.
- **The frontier world _is_ the contact surface** — there is no second travel phenomenon. It runs on three planet runtime-states: a **database row** when empty, a **cheap solo PvE tick** when you're there alone (build/loot in peace), a **server-authoritative contested instance** when others arrive (PvPvE). Persistent as a row, ephemeral as an instance → a bigger frontier costs no more to host until it's actually fought over.
- **Who can reach you there** scales with Potential and findability — small/quiet pushes draw few rivals; big ones are visible. Same obscurity economy as home, no separate rule.
- **Claiming the far Gate** (imprint) converts a frontier world into your soil and a forward respawn — this is how **new Gates** enter your holdings. Finite but slowly replenishing; pacing open.
- **Creatures, biome variety, loot** come from the frontier world's tags — see [[world#World types|World Types]] (REGION-1) and [[#Taming|Taming]] (TAME-1).
- **A frontier world can seat multiple Gates (FRONTIER-3).** The seed rolls 1–N Gates per world; each can host a **dead-house base** ([[#Potential|HOUSE-2]]). Several houses reaching the same listed world arrive through **different Gates** (sequential dials — GATE-2 holds), which is the multi-party PvPvE mechanism: an event is just a multi-gate world getting listed (the broker patch channel, [[world#United Gate Coalition|Coalition]]). **Homes stay single-gate** — a second gate on claimed soil is an overland backdoor around obscurity.

### Self-misdial — the uncontrolled lane (FRONTIER-2)

- The network is unstable (the Wake). An **active** Gate channels energy through that frayed fabric and the strain occasionally **arcs open a lane by itself** — an un-propped throat ([[gate-physics|Gate Physics]]) onto a random frontier world. You didn't dial it; you didn't pay a core.
- **Outbound only — not a backdoor.** It opens a lane _out_; it never drops anyone onto your soil (homes stay off the random pool). You can always **manually close** it; left open, the un-propped throat collapses on its own countdown.
- **Only an active Gate misdials.** A closed/offline Gate channels nothing, so it can't — "never while logged off" follows from the physics, not a bolted-on rule.
- It's the in-world face of rising instability: **more Gate activity → more misdials → harsher pulls** — the difficulty ramp and the visible symptom of the central mystery.

### Open questions — Gates

- **Frontier pacing** — player-driven frontier claiming vs. Coalition-paced expansion: who controls world-growth pacing?
- **Frontier specifics** — rank scale, frontier-loot vs. raid-loot balance, dead-house discount, gates-per-world distribution, and self-misdial rate/trigger.
- **Multi-gate homes** — parked. Frontier/procedural worlds can seat multiple Gates; homes stay single-gate unless a closer solves the overland-backdoor problem. A possible closer is claiming the whole world gate set, with each extra Gate becoming its own exposure surface.

### Sensor readings & lane twists (FRONTIER-4)

- **Sensors read through the Gate before commitment** — either built into the home Gate or sent as a probe/scan. They estimate the far site's rank, value, defenders, terrain, loot signature, instability, and special rules.
- **Readings are probabilistic** — sensor tier controls confidence, not truth. A cheap scan might say "high value / low hostile count" and be wrong; a better scan narrows ranges, detects instability, or spots hidden structures.
- **Lane twists are the dungeon surprise** — a lane can be misranked, locked, blind, double-layered, fast-collapsing, defender-heavy, or richer than advertised. The player sees enough warning to choose a risk, never enough to solve the run from the menu.
- **A red/locked lane is opt-in once read, committed once entered** — it can jam recall, re-dial, or clean extraction until a condition is cleared, but it should be telegraphed as abnormal before entry unless the twist is explicitly "bad scan."
- **Generation framework** — raid lanes should expose configurable modifiers such as `normal`, `bad_read`, `locked`, `blind`, `double_site`, `fast_collapse`, `defender_heavy`, and `rich_signature`. Their probabilities belong in data/scripts, not prose.

### Why / rejected

- **Why ring-fixed, aperture-driven:** it keeps early progression on found Gate infrastructure while still giving a tier ladder — you drive the same found ring harder before any later Gate-creation question matters. Exceeding one ring means reassembling found segments at the current layer.
- **Rejected — "Builders made rings in many fixed sizes; tier up by claiming a bigger ring":** makes the aperture hardware-fixed, conflicting with the driven aperture and with the dome flexing as the throat widens.
- **Why one cosmology (FRONTIER-1):** the uncontrolled lane is the same phenomenon as the Gate, just un-propped ([[gate-physics|Gate Physics]]) — so there is no second travel system. Travel out, the push-your-luck timer, the contact surface, and the new-Gate pipeline are all the **Gate dialing the frontier**; the contest reuses the planet runtime-states instead of a bespoke instance type. One fewer system to balance, and the away-reserve replaces a separate collapse clock.
- **Rejected — "Rifts", a standalone uncontrolled-portal concept with its own extraction system** (Sealed/Open instances, Host/Scavenger, corpse-share drops as its own layer): a second cosmology, hand-balanced and duplicative. The contested frontier world delivers the same extraction loop on instancing we already have; the underdog-crashes-a-bigger-op fantasy stays **emergent** (a weak player can still walk into a contested world). Everything the concept did is now the Gate: uncontrolled openings are **misdials**, collapse timing is the un-propped throat's own physics.

- Tunables: gate tier, per-ring aperture cap, activation-cost bands (see [[#Economy|Economy]]).
- Open: dialing UX; spin-down / grace-window timings; the control-device component (the stealable "DHD-equivalent", name open).

## Mask energy

The survival / life-support layer, and the **combat resource** — powered by the Gate.
Replaces the ARK/Rust hunger-thirst grind with one number tied to the core fantasy. [current call]

- A **natural raider timer** (away from a sustaining field you're on a depleting budget) and an **asymmetric home-soil advantage** (topped up at home, on a clock in enemy territory).
- It _is_ the combat resource — **no separate health bar** (see [[#Combat|Combat]]).
- The player wears a **mask** powered by a field drawn from the Gate (framed as projected energy; exact physics open — a [[gate-physics|conceit]]).
- The mask is the gater's required interface: life support, HUD, scanner, claim key, sync/continuity buffer, field battery, and black box.
- The mask is the in-world UI: health, reserve, field strength, compass, scanner results, Gate diagnostics, claim progress, route data, comms, alerts, and sync status.
- **The sustaining field (MASK-2)** — each active Gate projects a bubble; inside it the mask tops up (effectively unlimited time on your own soil). It **weakens on a gradient** outward — the game's **soft world border**: a cost/risk curve, not a wall. The dangerous fringe is where the frontier lies.
- **Gate-proximate content (MASK-4)** — the field is also a content-shaping rule: valuable resources, dead houses, AI bases, claims, return paths, and raidable structures should cluster around Gate fields rather than scatter across a full planet. Worlds can imply huge scale, but the playable/generated chunk for a session lives near the Gate.

### The two-number split (MASK-3) — load-bearing

- **Field radius** — how far from your Gate you can roam/build; the **dev-expandable** lever (the Coalition "expands the field" as the studio grows the world).
- **Away reserve** — how long the mask lasts once you leave a field; the **raider clock** ( = the raid clock).
- These **must stay separate numbers.** If they're one stat, every "expand the field 10%" patch also lets raiders camp enemy soil 10% longer. Away reserve is **fixed by the attacker's home-Gate power** and **un-pumpable** by carried cores or beacons.

### Mask-at-zero (asymmetric by location)

- **On your own soil / inside a field:** soft failure — emergency recall to the Gate, no loss (a leash, not a punishment).
- **In hostile territory:** hard failure — you go down, lootable. The real risk that stops raiding from being a free teleport home.

- Tunables: drain curve, field-gradient shape, away-reserve-vs-Gate-power curve.
- **Open:** what the mask physically is; gradient vs. hard radius (leaning gradient); field radius Coalition-only vs. capped player upgrade; confirm asymmetric mask-at-zero; whether any survival meters exist beyond mask energy and how minimal they must be.

## Raiding

The headline fantasy: open a tunnel to a target, breach, take loot, withdraw — the
aggressor thrill without the defender tax. [decided core fantasy]

- **Directional bilateral tunnels (TUNNEL-1).** Opening a Gate is not shields-down to the galaxy — it's a **two-way tunnel to one planet**. The target can counter-attack through it, but **third parties cannot pile in** unless they open their own Gates. Each raid is a **duel/feud**, not a free-for-all (anti-zerg).
- **Finding is the gate.** You raid a home only by assembling current coordinate intel — never via a hub shortcut into an unknown home.
- **The raid clock = the attacker's away reserve** (SIEGE-1). Initiating a raid **locks the Gate open** until the clock runs out (so the defender can't slam the door — the offline-dodge fix). The clock is **fixed by the attacker's home-Gate power** at dial time, **un-pumpable**, and capped by the **~38-min** wormhole ceiling ([[gate-physics|physics]]). Raid length is a curve of your own base size.
- **The breach is a fight, not a wall.** An active Gate projects a dome that can't be walled in; past it you're in the defender's prepared fire-lanes.

### Two modes (kept separate)

- **Home raid** — direct Gate, bilateral feud; contained, opt-in, governed by presence + the clock. Can take build, stock, and the civilization itself once located — but **never the player's existence**.
- **Hub / frontier contest** — many-vs-many lives at hubs (capture) and on contested frontier worlds (extraction), which you choose to walk into. Guardrail: a hub may lower _travel cost_ to a doorstep, never the _target's defenses_.

### Raid value floor (anti-husk, LOOT-1)

- A breached target must be worth the trip. The floor is a **server-minted, tier-scaled bounty** (not a skim of the owner's stockpile), sized so yield ≥ the core cost to dial that tier — the inequality that kills dry holes. Minted from the visible Gate tier, so there's nothing to "spend before it's stolen."
- Also lootable: the coordinate DB (the map) on a breach, plus mask data / exposure samples from killed avatars. Exact hunting payload is open.

### Offline raiding

- Bounded, two ways only: a **located** home (rare) and an exposed [[#Holdings|holding]] (always contestable, no search). Neither costs your existence.

### Why / rejected

- **Why the attacker's home-Gate sets the clock** (not the defender's, not the gap): it's un-fakeable and rewards growth, and it produces the right asymmetry automatically — a small raider punching up gets a short surgical window, two whales get a long siege. It's safe because you can only raid equal-or-greater, so the defender is never the weaker party.
- **Rejected — topping up away reserve with carried cores/beacons:** lets a fuel-hoarder camp enemy soil indefinitely.
- **Rejected — a separate defender-set siege timer:** unneeded; the attacker's fixed clock already bounds the fight once the Gate locks open.

- Tunables: clock curve vs. Gate power, bounty size per tier, mass caps.
- **Open:** matchmaking / instancing of bilateral tunnels; transponder / no-caller-ID details; what peers can and can't trace of a raider's identity while the Coalition still knows every registered gater.

## Combat

Third-person, gear-and-positioning PvP — the model resolved enough to greybox. [tentative]

- "Real PvP combat" is a required ingredient of the first playable. Skill lives in **loadout and positioning**, not ability-twitch — which keeps the home-soil edge tunable as a _number_, not a reflex contest.
- **The mask _is_ the resource — no separate health bar.** On your own soil the mask tops up (full durability + home edge); on hostile soil your **away reserve is both raid clock and durability** — taking fire drains it. The fair-but-beatable home edge falls out of the field gradient, with no bolted-on buff.
- **The aperture is a two-way fire-lane.** Fire and momentum pass through a live Gate ([[gate-physics|physics]]): the defender shoots _into_ the tunnel; the raider lays covering fire _up_ it.
- **Muster cap (COMBAT-2).** The dome admits only **K** combat-effective defenders at the aperture lip, K scaling with the **attacker's throat power** (the dialer sizes the dome). A skilled solo can beat a _mustered_ defence **at the breach** — but only there; the rest hold the perimeter at range, and the attacker's away-reserve clock bounds grab-and-go.

### The dome

- An active Gate projects a tier-scaled protective **dome** — a field, sized by the power driving the throat, that lets matter cross the otherwise-lethal aperture safely. The Builders engineered it **on purpose: a Gate can never be walled in.**
- **Protects transit, not occupation** — shields you while crossing (no spawn-camping) and from the aperture's own energy; once you stand in it, combat is normal.
- **Destroys manually-built structures** in its radius (it senses _construction, not material_, so natural terrain is spared) — so you cannot entomb a Gate or pack a kill-box against it. Players are shielded/recalled, never killed by it.
- **Scales with power** — bigger throat → bigger dome → a bigger protected enemy beachhead on your soil. A Supergate sits in an open plaza ("impossible to hide"). **In a raid the attacker sizes the live dome**, so a defender can't shrink it to a pinhole.
- Grounded in the tidal shear at a flaring throat mouth; the protective field and the build-vs-natural discrimination are Builder **conceits** — see [[gate-physics|Gate Physics]].

### Two venues, one model

- The same combat serves the **home raid** (at the aperture, under the clock) and the **standalone arena** (hub capture, contested frontier worlds, space gates — quick in/out, no territory required).

### Tech spine (precursor-tech-hybrid)

- Combat verbs climb the **Standard → Heavy → Supergate** ladder: bow/melee + scavenged ballistics at the low-tech start (low-poly art as _canon, not budget_), scavenged Builder tech (mask, energy weapons, the driven dome) mid-game, fleet/ship combat at the top. Through-line: you fight on a Builder-tech budget you don't fully understand. [tentative]

### Why / rejected

- **Why the dome protects transit and destroys builds:** it closes the **doorstep kill-box** trap (the Rust/ARK "honeycomb the raid entrance" meta that would nullify raiding and revive the turtle) — a fair beachhead, not a cheese funnel — without being a persistent combat shield.
- **Why the attacker sizes the dome:** stops a defender shrinking their own dome to choke arrivals into a pinhole.
- **Rejected — a dome that protects the _defender's_ base** (a shield scaling with their power): rebuilds the turtle, the exact thing the dome and the driven aperture kill.

- Tunables: TTK curve, K(throat) values, gear tiers.
- Open: exact TTK, K-values, gear tiers; the second-to-second verbs (movement, abilities).

## Hub worlds

Neutral, **lawless commerce crossroads** — trade routing nodes you can capture for the tax. [tentative]

- Cheap-distance routing to reach distant partners affordably (vs. an expensive direct dial), and a capturable territorial prize for the conqueror — without being the home for open PvP (that's contested frontier worlds).
- A hub has **no private home Gate of its own**, so you **cannot turtle** there — it's always live and exposed. But the **designed PvP contest lives on contested frontier worlds**, not here; PvP at a hub is incidental (over cargo), not an arena.
- **Routing:** distant trade hops through hubs for far less power than one long jump (the in-world basis for cheap distance). Direct-dial is the private, expensive alternative.
- **Optionally capturable:** hold the relay/core and you can **tax** traffic, **throttle** (allies cheap, enemies expensive or gate-locked), and gain **intel** on who moves through. Capture-to-tax is the conqueror's take-and-hold prize — a **registry charter flip** (HOLD-1), defended by presence.
- **Relation to the other surfaces:** Safe Core = policed spawn/services/meeting; **hubs** = lawless routing crossroads; contested **frontier worlds** = the contest/extraction surface. Hot frontier for contest, Safe Core + hubs/jump stations for travel and optional barter.

- Tunables: toll rates, routing-cost reduction, capture rules.
- **Open:** whether long-distance dialing needs **jump / relay servers** (hop-routing) to stop one-jump reach being too strong. Hub capture-defence specifics.

## Trade, scarcity & meeting places

Trade is **emergent and in-person**, not a required economy pillar. PvE progression should
work without a trade-station layer: missing resources come mainly from exploration,
frontier worlds, dead houses, taming loops, biome variety, and crafting substitutions.
[current call]

- **No trade stations.** Do not add AI broker stations, station stock, escrow, pickup keys, contract boards, global seller directories, or reputation scores as the default trade solution.
- **Players can still trade by meeting.** If two players physically meet at the Safe Core, a hub, a jump station, a frontier camp, or another agreed point, they can barter, inspect goods, bring escorts, scam, ambush, or make private deals.
- **Jump stations are Gate-native meeting points.** They exist because that world's Gate has a relay/routing config: refuel, call-forward, lane-stabilize, or bridge sectors at lower cost than a direct long dial. They create traffic because travel needs them, not because the game placed a shop there.
- **The Gate is still the seam.** Goods move because players carry them through Gates. Discovering a jump station, safe route, or useful meeting point means discovering a useful coordinate in the Gate network.
- **Discovery stays in-game.** Useful routes and meeting places are valuable intel. Third-party markets can arrange meetings, but they cannot execute trades without in-game access, travel, cargo, timing, trust, and risk.
- **No global auction house.** No universal item search, remote buyout, or safe delivery. Trade should not replace exploration or frontier play with menu optimization.
- **Trade risk attaches to carried cargo and routes.** Moving goods creates physical exposure through cargo mass, aperture/open duration, repeated route use, hub/jump-station traffic, and public presence at Gate sites. Item price is not magic Gate heat; value only matters socially when players reveal, guard, advertise, or fight over it.
- **Home exposure is escalation, not the trade default.** A normal in-person trade can create route traces and weak leads through repeated behaviour, but it should not grant home raid access by itself.
- **Solo PvE must not require trade.** Resource asymmetry should make exploration and frontier routing valuable, but missing inputs need fallback paths: local substitutes, alternate recipes, dead-house loot, biome travel, or lower-efficiency local production.

### Why / rejected

- **Why no trade stations:** ARK-style PvE can work without formal trade; adding markets early risks menu optimization, courier chores, extra AI economy work, and a parallel system beside exploration.
- **Why physical trade remains:** players should be able to barter when they meet, but the game does not need to turn that into a full economy pillar.
- **Why jump stations stay:** jump stations are roads, not markets. They create natural contact by solving travel/routing, a Gate-native need.
- **Rejected — direct public home shops:** makes trade either home-suicide or invulnerable shopping, both bad.
- **Rejected — global auction house / universal search:** turns trade into menu optimization and erases local scarcity.
- **Rejected — formal contracts, escrow, pickup keys, and reputation scores:** too much market bureaucracy for the current direction; trust is personal, and reliable trade happens by physically meeting at safer places.
- **Rejected — AI trade stations as default:** solves meeting convenience by adding stock logic, station economy, market UI, and balance problems before the PvE loop proves it needs them.

- Tunables: cargo mass caps, jump-station routing discounts, hub tolls, fallback recipe efficiency, dead-house/resource loot balance.
- Open: whether formal trade infrastructure is needed later; how much hub/jump traffic contributes to heat ([[#Coordinates & obscurity|Coordinates & obscurity]]).

## Presence, respawn & XP

The in-world layer under presence, respawn, and sanctioned violence: you pilot a
**disposable avatar** and keep your existence — the body dies, you persist. **What the
avatar physically is, where it comes from, and how your mind connects is open — see
[[questions]], Avatar origin.** Only the origin-agnostic baseline here is decided. [decided; origin open]

- Sets the design's floor: civilizations and bodies are **mortal** but the **player is never deleted** — selection without extinction, made literal (the vehicle dies, you continue).
- The exact metaphysics are unknown in-world: transfer, projection, copying, succession, or something stranger. The Coalition certifies continuity; it does not explain it.

### Presence & respawn (LINK-1)

- You pilot a **disposable avatar** — genuinely present and **mortal**. What's fixed is that you can lose the body without losing yourself.
- **Only the avatar dies.** Death drops the body + carried gear (lootable); you respawn at an anchor. This is the in-world license for raiding, PvP, and conquest — killing a gater never deletes them.
- **Avatar roster:** develop **multiple avatars** (tuned for different worlds), **pilot one at a time**. One live body, never simultaneous presence.
- **Anchor = where you respawn.** Newcomers: the **Safe Core** hub (always available). Claim a Gate and build a forward facility → respawn there to hold the world.

### Recall

- **Recall to defend** = abandon your away-avatar and respawn at your anchor. Cost: the away-avatar + gear are **left behind, exposed** — a committed trade, not a free round-trip. On the frontier, recall is the emergency exit (you keep your existence, lose the run's loot).

### Two-layer progression (XP-1)

- **Mind-XP** — permanent, account-wide: knowledge + **banked adaptations**, earned by discovery. Part of your persistent self; **never lost**. _What you know how to be._
- **Avatar-XP** — per-avatar proficiency, earned by doing; **lost with the body on death**. _How developed this body is._
- **High-water-mark:** a fresh avatar **re-levels fast up to your peak**, slow beyond — so death costs gear + a temporary dip + position, never a knowledge wipe.

### Claiming, attunement & the tether

- A Gate you've **claimed** is your respawn anchor and **fully sustains** you (home soil). A foreign Gate sustains only partially → the away-clock. Beyond all Gate range, the avatar can't stay knit and fails. The **mask** is the avatar tech that draws this stabilizing field (see [[#Mask energy|Mask Energy]]).
- A fielded mask carries enough home/house imprint to return, sync, and recognize full sustain. This may become part of the exposure model, but the exact hunting mechanic is open.

### Why / rejected

- **Why selection without extinction:** it gives the brutal frontier its floor — your civilization is mortal but you are never deleted, so raiding and PvP stay brutal without permadeath. The body is really there and really dies; you persist.
- **Rejected — piloting multiple avatars at once (multibox):** sequential roster only, so presence stays a real constraint.

- Tunables: re-level curve, recall/abandon cost, away-clock length, respawn time.
- **Open:** disconnect / AFK / grace-window timings; exact respawn location chain when a forward facility is lost or when your home is wiped mid-run, likely falling back to Safe Core or another starter/operator anchor.

## Claiming

The ownership act: a living gater turns an unclaimed Gate into a home, holding, or house
asset. Claiming is a consciousness test disguised as property. [tentative]

- **Blood + mask + intent (CLAIM-1).** Blood proves living biological substrate; the mask proves recognized gater continuity; intent proves agency.
  - **Line:** a claim is written in blood and keyed by the mask.
- **The Gate registers claim patterns.** It does not understand human property law.
- **The Coalition registers legal ownership.** It maps claim patterns into houses, permissions, standing, tithe, and charter records.
- **Robots can assist, not claim.** Robots can scan, build, fight, carry, and repair, but the network tests embodied biological consciousness.
- **A house is a shared claim pattern.** A solo is a house of one; a group is several claimants bound to one legal owner.

### Why / rejected

- **Why blood:** claiming needs a concrete biological act, not just a UI button.
- **Why Coalition ownership:** the unavoidable global registry becomes lore instead of just backend.
- **Rejected — robot claimants:** would undercut the biological consciousness test and the avatar premise.

## Coordinates & obscurity

How hidden homes get reached. The primary loop is **Gate heat -> samples -> leads**:
Gate use leaves evidence, players collect damaged evidence, and enough matching evidence
can escalate into temporary raid access. [current call; tuning open]

- **Safety is coordinate obscurity, not closed-state immunity.**
  - **Why:** this gives homes real stakes without recreating routine offline loss.
- **Only your existence is unloseable.**
  - **Why:** civilization can be mortal while the player persists.

### Coordinate knowledge

- **Unknown (COORD-1)** — another player does not have enough current intel to dial you.
  - **Serves:** BUILDER-1, HOMESTEAD-1.
  - **Prevents:** WHALE-1.
- **Traced (COORD-2)** — another player has heat samples, route fragments, jump/hub logs, signatures, corpse data, or other clues.
  - **Serves:** RAIDER-1, EXPLORER-2.
- **Located (COORD-3)** — another player has enough current intel to dial you.
  - **Serves:** RAIDER-1, CONQUEROR-1.
  - **Risks:** DENY-OFFLINE-1.
- **Stale (COORD-4)** — old intel no longer completes a dial.
  - **Serves:** BUILDER-1, HOMESTEAD-1.
  - **Prevents:** PERMA-LEAK-1.

### Coordinates as intel

- **Coordinate database (COORD-5)** — a homebase record of known routes and coordinates.
  - The database is a raidable asset.
  - A bookmark removes discovery cost, never travel cost.
  - **Serves:** RAIDER-3, EXPLORER-2, TRADER-2.
- **Dial out (COORD-6)** — spend power to reach a random unclaimed frontier Gate.
  - Explorers do not need target intel to find new worlds.
  - Claimed homes are not in the random frontier pool.
  - **Serves:** EXPLORER-1, EXPLORER-4, HOMESTEAD-4.
- **Dial known (COORD-7)** — open a tunnel to a saved coordinate.
  - **Serves:** TRADER-2, RAIDER-1, CONQUEROR-1.

### Exposure requirements

- **Activity creates traces (EXPOSE-1)** — opening Gates, moving mass, fighting, claiming, trading, and repeated routing leave evidence.
  - **Serves:** RAIDER-1, EXPLORER-2.
  - **Prevents:** TURTLE-1.
  - **Risks:** TRADE-SUICIDE-1.
- **Quiet play leaks less (EXPOSE-2)** — closed, small, low-traffic homes are hard to locate.
  - **Serves:** BUILDER-1, HOMESTEAD-1.
  - **Prevents:** WHALE-1.
- **Big houses leak more (EXPOSE-3)** — more members, routes, masks, cargo, and claims create more traces.
  - **Serves:** RAIDER-1, CONQUEROR-3.
  - **Prevents:** megahouses becoming invisible.
- **Old home intel needs a recovery valve (EXPOSE-4)** — stale intel, address rotation, heat decay, relocation, or another mechanism must prevent one leak from dooming a home forever.
  - **Serves:** BUILDER-1, HOMESTEAD-1.
  - **Prevents:** PERMA-LEAK-1.
  - **Risks:** TURTLE-1 if too cheap.

### Gate heat, samples & leads (EXPOSE-5)

- **Gate heat is event evidence, not magic danger.** It is the gameplay read of coarse Gate events the server likely logs anyway: source/destination band, open duration, aperture size, transferred mass, hub/jump traffic, claims, combat near a Gate, structure damage, extraction, deaths, and repeated route use.
- **Heat is physical first.** Cargo mass, aperture, duration, repetition, and public Gate-site presence can create heat. Item price does **not** create Gate heat by itself; value matters only when players reveal, guard, advertise, or fight over it.
- **Samples are player-facing fragments.** Players do not read the backend ledger. They collect samples from Gate mouths, jump stations, hubs, dead houses, wrecks, camps, corpses, cargo routes, stolen logs, probes, and combat scars.
- **Leads escalate gradually.** One sample points to activity; matching samples point to a route, jump/hub point, staging Gate, house signature, or confidence band; only repeated/matching/recent evidence plus cost can become temporary raid access.
- **Reveal order:** activity first, route/meeting point second, identity third, home last. A normal trade or frontier run should expose the run before it exposes the true home.
- **Recency matters.** Heat decays, samples expire, logs go stale, and raid access is temporary. This is the recovery valve that stops one copied clue from dooming a home forever.
- **Avatar/mask data is supporting evidence.** A killed avatar or looted mask can provide a sample, signature, or route clue, but fielded avatars do not shed permanent home-address shards by default.
- **AI and player actors use the same path.** Dead houses, AI route traffic, automated defences, hubs, jump stations, and player houses all create/read heat through the same Gate event model where possible.

### Why / rejected

- **Why mortal, not safe:** a perfectly safe home makes the apex base unraidable and kills real stakes.
- **Why obscurity:** the tax was the certainty of loss, not the possibility; hidden coordinates remove the certainty without deleting raids.
- **Why heat/samples/leads:** it makes findability a hunt through recent activity, reuses logs the game already needs, and lets trade expose cargo/route/station presence without defaulting to home exposure.
- **Rejected — inviolable home:** removes home stakes and makes the frontier pointless to leave.
- **Rejected — presence-gated wipe:** a no-show abuses it; obscurity makes it unnecessary.
- **Rejected — permanent avatar-share as primary loop:** one corpse or copied address would recreate permanent coordinate doom and make trade/frontier play too brittle.
- **Rejected — raw backend radar:** players see samples, leads, and confidence, not a private debug transcript.

- Tunables: heat weights, decay curves, sample lifetime, lead confidence thresholds, raid-access cost/duration, station-log lifetime.
- **Open:** exact scan tools/UI; where return/recall lands if the home was wiped while you were away; house shared-ownership sub-rules for address rotation, stale-intel recovery, member churn, old access, route data, betrayal risk, and who may recall to shared home defense.

## Potential

The measure of an actor's strength and progression — one sticky number that drives
frontier visibility and the raid clock. [tentative]

- A single honest strength metric so near-peers meet (fair fights, peer-wipe selection) and the weak can't be trivially farmed.

### Model (POT-1)

- **One sticky high-water-mark per house**, aggregating:
  - **Gate capability** — highest Gate tier you can field/power (your reach ceiling).
  - **Footprint** — territory + holdings held.
  - **Tech depth** — how far up the tree you've unlocked.
  - **Roster** — active members/avatars (makes a house read bigger than a solo).
- **High-water-mark** — rises as you grow, **falls only on genuine dismantling**; no powering-down to dodge.
- **Account/house-wide** — closes smurfing; strength can't be hidden behind a small avatar.
- **Progression = Potential climbing.**

### What it drives

- **Frontier visibility** — a contested frontier world is visible/reachable to players ≤ the pusher's Potential, so a whale can't see (or farm) a small player's pushes. This is the anti-whale guardrail (`reach = exposure`); see [[#Gates|Gates]] FRONTIER-1.
- **Frontier/hub tiering** — near-peers meet by construction.
- **Raid clock** — the away-reserve keys off home-Gate power, a Potential component.
- **Not** home-raid eligibility — that's gated by **locating** the target through the exposure model.

### House is the ownership primitive (HOUSE-1)

- The unit that owns a home, holds a coordinate, and carries Potential is a **house**; a **solo player is a house of one** (no separate code path).
- Roster is a real component, so a big house reads bigger — and creates more traces through members, routes, masks, cargo, and claims. `reach = exposure` at the group level.
- Word convention: **house = the people, home = the place.** "House" never means a building.

### NPC houses — dead houses (HOUSE-2)

- Every ownership / defense / raid mechanic is written against a **house**, so an AI base is just a house whose owner is an NPC — a **dead house**: the registry row of a gater the frontier ate, base intact, defences running on automation (the same frozen-snapshot AI as [[#Holdings|Holdings]]). [current call]
- **Reuse is the point:** Potential rating (visibility/tiering), the dome, the aperture fight, the muster cap (AI defenders count toward K), the loot floor (the bounty is server-minted either way), and the attacker's away-reserve clock all apply unchanged — one code path for AI bases and player bases.
- Dead-house bases seed **procedural frontier worlds** ([[#Gates|FRONTIER-3]]): PvE raid content that trains the raid loop, keeps it alive at low population, and lets the greybox prove the raid fantasy without hundreds of players.
- **PvE-dodge guard:** if a dead house of equal tier yields player-tier loot at zero social risk, nobody raids players — dead-house loot sits **below** player loot at tier.

### Why / rejected

- **Why one sticky high-water-mark, account/house-wide:** it closes the two dodges at once — you can't power down to look weak, and you can't hide strength behind a small avatar (smurfing). Falling only on genuine dismantling means "raid longer = build more" feeds growth into capability.
- **Why the house is the primitive:** one ownership model, not two; and the big-house-is-leaky consequence (more members shedding the same coordinate) makes findability scale with size for free.
- **Why "house" (the word):** the ownership noun must be a plain, globally understood English word that scales solo → corp → space-scale bloc; the dynastic register fits the EVE-flavoured endgame. **Rejected — tribe:** ARK's noun (the IP line). **Rejected — charter:** legalistic/UK register, fails the global-English test; "charter" stays the Coalition permission word (charter standing, clearance). **Rejected — clan / crew / company:** Rust's word / doesn't own an empire / collides with in-fiction corporations.
- **Rejected — live power draw:** dodgeable by powering down.
- **Rejected — separate metrics** for reach / stamina / visibility: one number is simpler; split only if tuning forces it.
- **Rejected — solo homes + loose alliances:** no shared fortress; "wipe a house" would mean hunting each member's base, undercutting the ARK fantasy.
- **Rejected — hybrid (solo homes + house-only holdings):** keeps two ownership models; house-of-one unifies them.

- Tunables: the curve converting the four components into the number; component weights.
- **Open:** component weights and the climb curve; house shared-ownership sub-rules, including who may rotate/stabilize the house address, how leaving/kicking affects old access and route data, and how the muster cap counts a house vs. a solo.

## Holdings

Assets staked _outside_ your home sustaining field — the one place the design admits
**bounded offline raiding** without reviving the babysitting tax. [tentative]

- Serves three archetype gaps in one layer: the offline raider's _ambition_ (siphon exposed output, never the home), the builder's "build exposed for a payoff," and a forward-reach option the home field can't reach.
- A new object class: a forward node, claimed resource tap, or off-site cache. **Space-gate holdings** and **captured hubs** are the top-tier cases.
- It has **no private home Gate of its own**, so there is no quiet/closed home state to hide behind — the same reason space Gates can't be turtled. (The home itself is mortal-but-must-be-located; a holding is the **always-on, no-search** target.)

### The mechanic — flow-tap, no window

- **Always contestable** by an **uphill** raider (potential computed from the _holding's own value_, so a whale can't farm a newbie's node). **No scheduled vulnerability window.**
- **Offline defence is a frozen snapshot** of the automated defences you left — the attacker fights AI, not the absent owner.
- **Winning denies; it doesn't bleed.** Taking a holding flips it **dormant/neutral** (owner stops the trickle) and grants the attacker a **one-time, capped buffer** — **never an ongoing stream diverted to the attacker.** So there's no "a rival is paid by my node while I sleep" meter, and **no reclaim obligation**: you re-tap it next time you play, as normal play.
- **On-thesis:** the worst an offline player loses to this layer is one capped buffer of output they weren't collecting — never structures, stock, progress, or existence. That's "I missed a trickle," not the ARK/Rust offline-wipe.
- **Conqueror's take-and-hold** routes here and to hubs / space gates — capture is a **registry charter flip** defended by presence in an opt-in arena, not a decaying home-side claim.
- **Cost fit:** a holding is a DB row at rest; it spins a Contested instance only when an attacker is online and pays to dial it, so server load stays ∝ active raiders.

### Why / rejected

- **Why a flow-tap with a capped buffer, no window:** the bound is _structural_ — at most a buffer of uncollected output, never structures or progress — so it's not the ARK/Rust offline-wipe and creates no reclaim chore.
- **Rejected — a scheduled vulnerability window:** pulls players onto the network's clock and is exploitable by timezone capture, zerg broadcast, and wait-out.
- **Rejected — an ongoing flow diverted to the attacker:** reads as a decay meter bleeding to a rival; the babysitting tax reborn on offence.
- **Rejected — a home-side capturable with a hold-or-lose timer:** "reverts if not held by presence" is base-decay in costume; take-and-hold is served at hubs / space gates instead.

- Tunables: `buffer_cap`, dormancy / re-tap values.
- Open: whether a holding ever gets a present-defender fight (leaning no).

## Taming

_Stub — narrow scope decided (TAME-1), specifics open._

- **Narrow yes:** include **taming**; decline deep ARK-style breeding/genetics (a babysitting chore by construction — the exact pain Gaters is built to remove).
- **Source:** creatures come **only from frontier worlds** — a monster-spill rolls a `tameable` tag; world rank scales rarity. No new spawn system.
- **Taming is a frontier-clear objective, not a feed-and-wait timer** — subdue under the away-reserve leash; an event, not a maintenance state.
- **Lineage is a home-anchored record** (safe while unknown); the living animal is the **exposed wager** (costs a core; can die and be looted; the registry entry survives).
- **"Breeding" is shallow and lossless** — combine two registry entries for a blended/mutated re-roll; no timer.
- Open: tameable roll rate per frontier-world rank, blend odds, instantiation core-cost — or **defer the whole fork** for the greybox.

## Building

_Stub — the anti-decay rule is decided (BUILD-1); build depth is open (BUILD-2)._

- Building on your quiet planet-base is a loved pillar (ARK / Valheim). **No persistent base decay / upkeep** [current call] — the central thing being designed out.
- Defence is the **perimeter at range**, never doorstep cheese — the dome forbids walling the aperture.
- Safe while unknown; offline loss only if the home is **located** (rare), never from neglect.
- Open: build depth, materials, structure types, buildable defences, grid vs. free-form, anything analogous to taming.

## Economy

_Stub — drivers/sinks framed, numbers open._

- **Stagnation pressure** (ECON-1) — quiet bases plateau; growth needs imports — the engine that makes players open up. A peaceful homesteader still progresses to a **soft ceiling**, then plateaus (PROG-2) — a ceiling, not a leak, never an upkeep tax.
- **Power cores** — scarce, scavenged Gate fuel; the primary consumable behind activation.
- **Activation cost** (ECON-2) = `gate tier × distance band × tier-gap multiplier`. Bands: local / regional / cross-map ≈ power cell / partial core / full core.
- **Trade** (TRADE-1) — optional, physical, and in-person; players may barter when they meet at hubs, jump stations, the Safe Core, frontier camps, or agreed points. It is not the main answer to PvE scarcity.
- **Resource asymmetry** (REGION-1) — worlds differ so exploration, frontier routing, dead-house raids, taming, and substitutions matter. Trade can help socially, but the PvE loop must not depend on a market.
- **Sinks** (ECON-3) — hub tolls + jump-station routing fees + activation costs + the Coalition tithe.
- **Open:** concrete currencies, resource taxonomy, crafting inputs, faucet/sink balance; how hard stagnation pushes without becoming upkeep; whether long jumps need jump/relay servers.

## Progression

_Stub — drivers + the homesteader ceiling decided; tech tree open._

- Two in-world drivers gate one tier ladder: **rising charter clearance** and **more of the Gate network waking**. Ladder: surface worlds → larger/richer → orbital/space Gates → relay/hub Gates → megastructure/Supergates.
- **Gate tiers are driven, not built** — see [[#Gates|Gates]] and [[gate-physics|Gate Physics]].
- **No base decay** anywhere (BUILD-1).
- **Character progression = two-layer mind/avatar XP** — see [[#Presence, respawn & XP|Presence, respawn & XP]] (XP-1).
- **Quiet soft ceiling (PROG-2):** a mostly quiet homesteader progresses **forever up to a soft ceiling** (frontier tiers need opening), then plateaus — never decays, never punished. Safe because a never-leaver stays unknown.
- Open: the actual tech tree, research mechanics, gear tiers; the Supergate (PROG-1) endgame specifics.

## Player requirements

The playstyles Gaters expects, sorted by how the balance thesis treats each. Test every
proposal against this set: a decision that silently starves a served style, or revives a
denied one, is a blocking flag.

### Served

- **Raider**
  - **RAIDER-1 Find targets** — raiders need ways to locate valuable targets.
  - **RAIDER-2 Breach** — raids need a bounded fight, not wall math.
  - **RAIDER-3 Loot** — targets must hold value worth the risk.
  - **RAIDER-4 Extract** — raiders need a way home with stolen goods.
  - **RAIDER-5 Anonymity** — raiders need peer-facing anonymity until commitment.
- **Builder**
  - **BUILDER-1 Safe home** — builders need long-term homes without decay or chores.
  - **BUILDER-2 Build depth** — structures, defenses, production, and layout must matter.
  - **BUILDER-3 Fair defense** — defense happens around the perimeter, not doorstep cheese.
  - **BUILDER-4 Exposed building** — forward bases and holdings need upside worth the risk.
- **Explorer**
  - **EXPLORER-1 Unknown frontier** — explorers need unknown Gates/worlds to discover.
  - **EXPLORER-2 Valuable intel** — coordinates, routes, traces, and signatures must be useful loot.
  - **EXPLORER-3 Compounding maps** — early scouting should become late-game leverage.
  - **EXPLORER-4 Blind dialing** — random frontier discovery must work without target intel.
- **Conqueror**
  - **CONQUEROR-1 Holdings** — exposed assets must be worth taking.
  - **CONQUEROR-2 Capture** — conquest needs take-and-hold mechanics.
  - **CONQUEROR-3 Peer targets** — conquest should aim at peer-or-greater rivals.
  - **CONQUEROR-4 Mortal empire** — empires can fall; players persist.
- **Homesteader**
  - **HOMESTEAD-1 Quiet safety** — quiet players should be hard to locate.
  - **HOMESTEAD-2 No chores** — long-term building should not require upkeep babysitting.
  - **HOMESTEAD-3 Soft ceiling** — staying quiet should progress for a long time, then plateau.
  - **HOMESTEAD-4 Solo PvE** — frontier PvE should remain valuable without forcing PvP.
- **Tamer**
  - **TAME-1 Frontier creatures** — creatures should come from frontier worlds.
  - **TAME-2 Narrow depth** — taming exists; deep ARK-style breeding/upkeep does not dominate.

### Routed

- **Trader**
  - **TRADER-1 Resource asymmetry** — worlds need different outputs, but the primary answer is exploration/frontier play rather than a market.
  - **TRADER-2 In-person exchange** — trading is valid when players physically meet at hubs, jump stations, the Safe Core, frontier camps, or agreed points.
  - **TRADER-3 Cargo risk** — trade risks carried cargo and route exposure, not default home exposure.
  - **TRADER-4 No market pillar** — no trade stations, contracts, escrow, reputation scores, or global auction house by default.
- **PvP brawler**
  - **BRAWLER-1 Quick combat** — combat-focused players need fast fights.
  - **BRAWLER-2 Arena surface** — brawling belongs on contested frontier worlds, space Gates, or other always-live surfaces.
- **Clan / zerg**
  - **GROUP-1 Large objectives** — large groups need objectives worth coordinating around.
  - **GROUP-2 No punch-down** — large groups must not easily farm solos/newbies.
  - **GROUP-3 Group payoff** — group play needs rewards that justify organization overhead.

### Denied

- **Offline raider as default**
  - **DENY-OFFLINE-1 Rare offline raids** — homes can be raided if located, but offline raiding must not be routine.
  - **DENY-OFFLINE-2 No sleeping body** — the player's real body is not in-world as an offline target.
- **Megabase newbie farmer**
  - **DENY-WHALE-1 Obscurity wall** — strong players should not easily find small quiet homes.
  - **DENY-WHALE-2 Uphill visibility** — frontier visibility should keep whales away from low-power targets.
- **Turtle / babysitter**
  - **DENY-TURTLE-1 No permanent bunker meta** — staying quiet is safe but capped.
  - **DENY-TURTLE-2 No obligation** — players should never feel forced to log in to prevent decay/loss.

### Why

- **Why Served / Routed / Denied:** some fantasies are pillars, some are valid only on the right surface, and some recreate the ARK/Rust pain the design exists to remove.
- **Why IDs:** mechanics can list **Serves / Prevents / Risks / Test** instead of carrying long rationale every time.

## Validation — two questions, two scales

Two risks, de-risked differently. Don't conflate them.

- **Is the Gate-native raid site feasible? — greybox-answerable.** Can a generated Gate-proximate world chunk produce a valid raid: terrain, Gate, dome clearance, AI/dead-house base, defenders, loot, extraction, and save/reload? Build this **greybox proof** before art polish or galaxy-scale systems. Cheap, early, and _necessary but not sufficient_.
- **Will the population open up rather than turtle? — only shows at scale.** The real make-or-break, and a greybox **cannot** settle it: open-vs-bunker is an emergent **equilibrium** of a live economy — it needs hundreds of players, market depth, and social pressure before it stabilises. Five testers say nothing about where 500 land. Treat it **assume-and-commit**: reason from games that ran the experiment (Rust, EVE, Albion, Foxhole), model the economic pressure, and prove it only at a **scaled paid playtest**. The biggest risk is the one you can't cheaply prove.
- **Minimum viable slice:** one generated ground Gate site, one generated AI/dead-house base, valid dome clearance, pathable defenders, loot worth taking, extraction/recall, and persistence of the generated result. Closed/open, obscurity, coordinates, heat, economy, and real PvP can stay stubbed until the generated raid surface works. **Ground Gates only**; space Gates later.
- **Open:** whether players actually open their Gates remains a population-equilibrium question, not a solo greybox question.
- **Design for success enabling expansion, not requiring it** — a working small version is a complete thing, not a broken large one.

## Technical challenges — and the Gate mechanics that ease them

What is hard to _build_ about a game like this, and which gate lore/mechanic pays each bill.

> **The core insight.** The Gate is an **engineering device dressed as lore.** Most hard
> problems of survival-PvP-MMOs the Gate fiction _removes_ rather than solves. So the lens
> isn't "what's hard" — it's "what's hard _after_ the Gate already paid for most of it."
> The top risk is **not technical** — it's whether players will open their Gates
> (Validation, above). This section scopes the technical residue under that.

### Part A — problems the Gate fiction removes (bank these)

Each is a genre-standard hard problem the design has already traded away. Don't re-solve
them; just don't break the fiction that pays for them.

- **Seamless space-to-surface flight** → **Gate loads.** NMS's hardest engineering problem is sidestepped: you never fly planet-to-space continuously, you dial and load.
- **A persistent shared world to simulate 24/7** → **presence requirement + closed-by-default.** A Gate is open only while its owner is online and chose it. Offline = closed/quiet = little to simulate. No offline base sim, no decay tick, no sleeping-base economy. The single biggest server saving, and it's a _gameplay pillar_.
- **An authority transition between solo and contested play** → **server-authoritative always.** One authority owns each planet at all times, so there is no transition between authority models to reconcile or exploit. The cost cap comes from the presence requirement: only online players' planets are simulated, and quiet solo building — having no adversary — ticks cheaply.
- **World-size-proportional cost** → **three-state planet + seed-plus-deltas.** A planet is a DB row (**empty**) → a cheap server-side tick (**solo**) → a full spun-up instance (**contested**). Server cost ∝ online players, not world size.
- **Massive concurrent battles as the default** → **bilateral directional tunnels.** A raid is a two-party instance, not an N-body shared zone. Trivially shardable; no thundering-herd simulation in the common case.
- **Unbounded session / instance lifetime** → **away-reserve clock, ~38-min ceiling.** Contested instances are short-lived by fiction, so they're cheap to spin up and tear down and never leak.
- **Bounding the simulated volume** → **mask field + the dome.** Only the area around an _active_ Gate needs simulating because mechanics intentionally keep meaningful play near Gates; the field radius is the soft world border and the dome bounds the combat arena.
- **Infinite/contiguous procedural universe** → **discrete, seedable planets loaded one at a time.** Each planet is an isolated bounded chunk behind a `PlanetGenerator` abstraction.

### Part B — the technical residue, ranked, with the lever for each

What the Gate fiction does _not_ already pay for. Ranked by severity for a solo dev.

1. **Two worlds, live, through an open Gate — _highest_.** "A firefight at the aperture, not a loading screen" means rendering and simulating **two planets at once** through a portal — the Portal/Prey rendering problem, plus two physics scenes, plus netcode spanning them. Levers: **the dome bounds the portal** (you never render the whole far planet — only what's visible through the throat, and the dome caps that radius); **the aperture is the only shared volume** (both sides simulate their own planet; only the throat region is co-simulated); **graceful-degrade fallback** — the aperture degrades to a shared staging volume both sides load into; greybox can ship the cheap version first.
2. **Networked physics across an authority boundary — _high_.** "Momentum conserved through the Gate" means a physics object changes authority mid-flight — a classic netcode trap. Lever: the **aperture is the single transfer plane**, and the **mass cap** bounds how many such objects exist at once.
3. **Instance orchestration, cold-start, spin-up latency — _medium_.** Spinning up a contested instance the moment a raid commits, cheaply and fast. Lever: the **dial → tunnel-open sequence is a provisioning window** — the diegetic "dialing" animation covers instance cold-start; both endpoints known in advance; uphill-only shrinks the pool; the ~38-min cap makes instances short and self-terminating. (Greybox: a listen-server removes orchestration entirely.)
4. **The one irreducible shared state — the registry — _medium_.** Claims, coordinates, who-holds-what: a global, consistent, persistent dataset — the one component instancing can't remove; couples to raider anonymity. Lever: it's a **database problem, not a simulation problem**. The Coalition _is_ the natural home for centralized state (lore = the schema); the registry knows owners, but the raid tunnel need not expose the attacker until commit (transponder model). Keep the registry small (metadata only).
5. **The many-vs-many exception — hubs, server events — _medium, but cappable_.** Explicitly many-vs-many surfaces are where "instancing makes it cheap" breaks. Lever: they are **few, optional, and bounded** — hubs hard-capped in concurrency, uncontrolled lanes temporary by fiction (they collapse on a countdown). Keep them out of the greybox.
6. **Procedural generation breadth — _medium, already deferred_.** Lever: the **`PlanetGenerator` abstraction from day one** — greybox ships a trivial generator; richness lands later. Field radius bounds how much of each planet must be generated.
7. **The web export's real ceiling — _low-medium_.** A web build as a playtest funnel can't match native. Lever: scope it as a **demo slice** — a single planet, a single raid; a web client can host/join one bilateral instance without the full backend.
8. **The meta-risk — solo scope — _the real top risk_.** One person building portal rendering + orchestration + procgen + combat + netcode is enormous even after the Gate removes the worst. Lever: "design for success enabling expansion, not requiring it" — every item above has a named cheap path for the greybox.

### Part C — lore to lock _because it pays an engineering bill_

Fiction choices that double as architecture — each closes a technical hole, not just a story one.

- **"Dialing is a sequence, not an instant"** — the dial animation is the instance-provisioning window. Pays #3.
- **The dome radius as the render/sim bound** — also caps the portal frustum and dual-sim volume. Pays #1.
- **Gate-proximate content as a rule** — resources, AI bases, dead houses, claims, and objectives should be generated near Gate fields by default. Pays #6 and keeps local chunks sufficient for the prototype.
- **Mass cap / manifest on what crosses** — bounds physics objects transferring at the aperture. Pays #2.
- **The Coalition registry as the one sanctioned global DB** — centralize _all_ unavoidable shared state into the Coalition fiction; everything else stays instanced. Pays #4.
- **Uncontrolled lanes collapse on a countdown** — the auto-teardown that keeps the many-vs-many case bounded. Pays #5.

### What the greybox must prove (technical subset)

1. The **aperture fight** (#1/#2): does "firefight at the door" work at all? Ship the degraded shared-staging version first; defer true portal rendering.
2. The **on-demand instance** (#3): open → fight → tear down, end to end, once. A listen-server is the cheapest first version.

Everything else (procgen breadth, web export, hubs, space Gates) is deferred and stays out
of the first proof.

## Standing constraint — IP / originality

The Gate/wormhole _mechanic_ is genre-generic, but Gaters must avoid Stargate's
_expression_: no "Stargate," no Egyptian skin, no chevron/glyph set, no copied proper
nouns, no ascended-precursor lore, no named dialing device. Original names and story
throughout. (A practical design line, not legal advice.)
