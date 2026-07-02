# Systems

The Gaters mechanics bible in one file. The world it serves is [[world]]; the deep physics
in [[gate-physics|Gate Physics]]; unresolved questions in [[questions]]. Numbers never
live in prose — each mechanic names its tunables; values land in data at playtest.
Code references stay placeholders until the game project exists.

## Thesis & pillars

**Thesis — reach equals exposure** (see [[world]]). Aggressor fantasy, no defender tax.
The pillars are organized by **failure mode**, not by object — each is something that can
fail on its own and take the game down. Don't fold cores into the Gate just because they
are Gate states; nesting by object collapses the list and hides the distinct failures.

1. **The Gate** — the one object everything routes through.
   - Travel, mask-energy source, exposure surface, economy sink.
   - Finite and unbuildable: found / claimed / repaired / relocated / tamed.
2. **Binary exposure** — sealed / open.
   - Safe by default; risk is opt-in. How trade risk differs from raid risk is **open** ([[questions]] #2).
3. **Presence + siege timer** — the babysitting fix.
   - Open only while online and choosing it.
   - Raid commits → Gate locked open for a bounded fight.
4. **Mask energy — two decoupled numbers.**
   - Field radius = soft world border + studio expansion lever.
   - Away reserve = raider's clock on hostile soil.
   - Must stay separate or world-growth nerfs raids.
5. **Contained conflict.**
   - Bilateral directional tunnels (anti-zerg).
   - Asymmetric uphill-only reach (anti-whale — see [[#Potential|Potential]]).
6. **Stagnation economy** — the counterforce that makes players open up.
7. **Gate-based instancing** — what makes it buildable solo.
   - empty row → solo (cheap server tick) → contested (full server instance).
   - Server cost ∝ online players, not world size.
8. **The uncontrolled lane (misdials)** — the engagement engine: opportunity out, never invasion in (outbound only, active Gates only).

**Gate on all of it:** will players actually open their Gates? Make-or-break — and the one
thing a greybox **can't** settle: it's a population equilibrium, not a feel (see
[[#Validation — two questions, two scales|Validation]]).

## Core loop [tentative]

The cycle the thesis produces, at three scales. Documented here only — other sections
link here, they don't restate it.

- **Moment-to-moment** — sealed solo PvE: explore, gather, build. To act you open the Gate and accept risk → push a directional raid tunnel under a mask-energy leash, or defend under a siege timer you opted into.
- **Session** — log in (stuff intact, no decay) → stay sealed and build, or open to trade / raid / visit a hub world → resolve finite engagements → reseal and log off. No maintenance obligation.
- **Long-term** — grow the base, raise charter clearance, climb Region/World tiers and Potential as more of the network wakes → fleet-scale play and the mystery payoff. No base to babysit.

> Emergent multiplayer behavior (will players open up, will deterrence hold) is
> **assume-and-commit** — validated at playtest scale, not by greybox.

## Design traps (failure modes to avoid)

Each is a way the exposure model could collapse; the design's counter is named. Test
every proposal against these.

- **Turtle equilibrium** — if sealed = safe forever with no downside, everyone bunkers. _Counter:_ the economy forces exposure (sealed stagnates), and sealing buys obscurity, not immunity (the home is mortal).
- **Trade-suicide** — if trading exposes the vault like raiding, every economic act invites a wipe. _Counter:_ **open** — no mechanic currently stops this ([[questions]] #2).
- **Offline-dodge** — instant-seal mid-raid kills raiding; can't-close traps you online. _Counter:_ presence + the committed siege clock (opt-in, finite, no offline jump).
- **Zerg pile-on** — if opening exposes you to everyone, mass wins. _Counter:_ bilateral directional tunnels (contained duels).
- **Whale / newbie-farming** — a megabase griefing small players kills retention. _Counter:_ obscurity + uphill frontier-visibility (a whale can't see a small player's frontier pushes to harvest their shares).
- **Doorstep kill-box** — walling the aperture into a sealed box nullifies raiding. _Counter:_ the dome destroys manual builds (not terrain) — fortify the perimeter at range, never the doorstep.

Plus the healthy/toxic line: players should be _excited_ to log in, **never** _obligated_
on pain of loss — anything that punishes an absent player has reinvented decay.

## Gates

The central object: every planet-base sits on a Gate, and all reach flows through it. The
deep physics treatment is [[gate-physics|Gate Physics]]; this section is the game model.

- Owning a planet means **owning its Gate** — the reason bases are planets, not an arbitrary rule.
- Gate finiteness (found, never manufactured) is the root of all conflict; opening your Gate is how you reach the world _and_ how you become reachable.
- **Woke sealed; unmanufacturable.** The network came online with every aperture shut (safe-by-default), and Gates can only be **found, claimed, repaired** — never built (GATE-1).
- **Travel is one-way and directional.** The _dialing_ side opens the tunnel and pays the power, so a raid is inherently the attacker pushing through a tunnel they opened.
- **One ring props one live tunnel at a time (GATE-2).** A throat joins exactly two mouths ([[gate-physics|Gate Physics]]), so a Gate never holds multiple simultaneous connections — multi-party arrivals at one world are **sequential dials**, never held tunnels. Multi-tunnel exists only as multiple rings (Supergates, deferred).
- **Fire and momentum pass through** a live Gate — breaching is a firefight at the aperture, not a loading screen (see [[gate-physics|Gate Physics]], [[#Combat|Combat]]).
- **Dialing needs the target's coordinates** — a planet whose coordinate nobody has assembled is effectively hidden (see [[#Coordinates & obscurity|Coordinates & Obscurity]]).

### Tiers — driven, not built

- The **ring is fixed**; the **aperture** is a runtime variable set by how much exotic matter (power cores) you channel, capped per ring by the **Ford–Roman quantum inequality** ([[gate-physics|Gate Physics]]).
- **Standard → Heavy** = one ring fed lightly vs. hard (wider aperture, bigger payload, bigger dome, more cost).
- **Supergate** = past one ring's ceiling — combine multiple **found Builder segments** into a megastructure; recovered and repaired, never new-built.
- **Overdrive** one ring past its ceiling and the throat destabilises into an uncontrolled, self-collapsing lane (the gate-overload bomb).

### Gate variation — the Builder "config" [tentative — rough idea]

- **Loose idea, needs tuning — nothing depends on it.** Gates differ (field size, stability, what they dial, what grows there) maybe because each carries a small Builder-seeded **config** the [[world#Behind the curtain — the Builders' truth|von Neumann probes]] tuned **blind** — they couldn't predict a config's behaviour, so gates came out imperfectly/variously tuned. That imperfection is the variety.
- Just an explainer for _why gates aren't all the same_. It doesn't touch dialing/findability (still coordinates), it's **not** a selection system (explored and rejected — [[questions]] #20), and the _why_ behind the probes stays parked. Shape it properly if/when it matters.

### Presence (PRES-1)

- A Gate is **open only while the owner is online and choosing it**; logging off **seals** it (the Gate fails closed — a Builder conceit, like the dome). Sealing is a **spin-down**, not a switch. Sealed = no cold breach through the shut throat (comms still pass — [[questions]] #12).
- Presence stops a _cold_ breach, but no longer grants immunity: a **found** home can be raided, rarely, even offline. The babysitting tax died from **obscurity**, not a wall.
- **Offline tends _toward_ safety, not away from it** — sealing sheds no new shares and re-key voids those already taken, so logging off lowers your odds of being found, it doesn't raise them.

### Reaching the frontier — dial out (FRONTIER-1)

- **Two things you can dial:** a **claimed coordinate you've assembled** (a raid — needs shares), or **out into the unclaimed frontier** (expansion). You can't pick _which_ frontier world — dialing an unclaimed address lands you at a **random** one; **claimed homes are off the pool**, so no one is ever dropped onto occupied soil (preserves obscurity).
- **Frontier travel is one-way** — you can't return the way you came; you arrive, and the away-reserve clock is already ticking. Push your luck against it for loot, or **claim the far Gate** before it runs out.
- **The frontier world _is_ the contact surface** — there is no second travel phenomenon. It runs on three planet runtime-states: a **database row** when empty, a **cheap solo PvE tick** when you're there alone (build/loot in peace), a **server-authoritative contested instance** when others arrive (PvPvE). Persistent as a row, ephemeral as an instance → a bigger frontier costs no more to host until it's actually fought over.
- **Who can reach you there** scales with Potential and findability — small/quiet pushes draw few rivals; big ones are visible. Same obscurity economy as home, no separate rule.
- **Claiming the far Gate** (imprint) converts a frontier world into your soil and a forward respawn — this is how **new Gates** enter your holdings. Finite but slowly replenishing; pacing open ([[questions]] #9).
- **Creatures, biome variety, loot** come from the frontier world's tags — see [[world#World types|World Types]] (REGION-1) and [[#Taming|Taming]] (TAME-1).
- **A frontier world can seat multiple Gates (FRONTIER-3).** The seed rolls 1–N Gates per world; each can host a **dead-house base** ([[#Potential|HOUSE-2]]). Several houses reaching the same listed world arrive through **different Gates** (sequential dials — GATE-2 holds), which is the multi-party PvPvE mechanism: an event is just a multi-gate world getting listed (the broker patch channel, [[world#United Gate Coalition|Coalition]]). **Homes stay single-gate** — a second gate on claimed soil is an overland backdoor around obscurity; parked ([[questions]] #22).

### Self-misdial — the uncontrolled lane (FRONTIER-2)

- The network is unstable (the Wake). An **active** Gate channels energy through that frayed fabric and the strain occasionally **arcs open a lane by itself** — an un-propped throat ([[gate-physics|Gate Physics]]) onto a random frontier world. You didn't dial it; you didn't pay a core.
- **Outbound only — not a backdoor.** It opens a lane _out_; it never drops anyone onto your soil (homes stay off the random pool). You can always **manually close** it; left open, the un-propped throat collapses on its own countdown.
- **Only an active Gate misdials.** A sealed/offline Gate channels nothing, so it can't — "never while logged off" follows from the physics, not a bolted-on rule.
- It's the in-world face of rising instability: **more Gate activity → more misdials → harsher pulls** — the difficulty ramp and the visible symptom of the central mystery.

### Why / rejected

- **Why ring-fixed, aperture-driven:** it keeps the canon rule "Gates are found, never fabricated" intact while still giving a tier ladder — you drive the same found ring harder, you never build a bigger one. Exceeding one ring means reassembling found segments, so the rule holds.
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
- The player wears a life-support **mask** powered by a field drawn from the Gate (framed as projected energy; exact physics open — a [[gate-physics|conceit]]).
- **The sustaining field (MASK-2)** — each active Gate projects a bubble; inside it the mask tops up (effectively unlimited time on your own soil). It **weakens on a gradient** outward — the game's **soft world border**: a cost/risk curve, not a wall. The dangerous fringe is where the frontier lies.

### The two-number split (MASK-3) — load-bearing

- **Field radius** — how far from your Gate you can roam/build; the **dev-expandable** lever (the Coalition "expands the field" as the studio grows the world).
- **Away reserve** — how long the mask lasts once you leave a field; the **raider clock** ( = the raid clock).
- These **must stay separate numbers.** If they're one stat, every "expand the field 10%" patch also lets raiders camp enemy soil 10% longer. Away reserve is **fixed by the attacker's home-Gate power** and **un-pumpable** by carried cores or beacons.

### Mask-at-zero (asymmetric by location)

- **On your own soil / inside a field:** soft failure — emergency recall to the Gate, no loss (a leash, not a punishment).
- **In hostile territory:** hard failure — you go down, lootable. The real risk that stops raiding from being a free teleport home.

### Consumables — boosts, never meters (MASK-4)

- Food and the like are **optional buffs to body/PvE stats only** — never depletion meters (nothing punishes their absence), and **never a mask number** (reserve, drain, regen): the mask is also the health bar, so a mask-touching buff would pump the raid clock — the rejected fuel-hoarder camp ([[#Raiding|Raiding]]). [current call]

- Tunables: drain curve, field-gradient shape, away-reserve-vs-Gate-power curve.
- Open: what the mask physically is; gradient vs. hard radius (leaning gradient); field radius Coalition-only vs. a capped player upgrade.

## Raiding

The headline fantasy: open a tunnel to a target, breach, take loot, withdraw — the
aggressor thrill without the defender tax. [decided core fantasy]

- **Directional bilateral tunnels (TUNNEL-1).** Opening a Gate is not shields-down to the galaxy — it's a **two-way tunnel to one planet**. The target can counter-attack through it, but **third parties cannot pile in** unless they open their own Gates. Each raid is a **duel/feud**, not a free-for-all (anti-zerg).
- **Finding is the gate.** You raid a home only by assembling its coordinate — never via a hub shortcut into an unfound home.
- **The raid clock = the attacker's away reserve** (SIEGE-1). Initiating a raid **locks the Gate open** until the clock runs out (so the defender can't slam the door — the offline-dodge fix). The clock is **fixed by the attacker's home-Gate power** at dial time, **un-pumpable**, and capped by the **~38-min** wormhole ceiling ([[gate-physics|physics]]). Raid length is a curve of your own base size.
- **The breach is a fight, not a wall.** An active Gate projects a dome that can't be walled in; past it you're in the defender's prepared fire-lanes.

### Two modes (kept separate)

- **Home raid** — direct Gate, bilateral feud; contained, opt-in, governed by presence + the clock. Can take build, stock, and the civilization itself once found — but **never the player's existence**.
- **Hub / frontier contest** — many-vs-many lives at hubs (capture) and on contested frontier worlds (extraction), which you choose to walk into. Guardrail: a hub may lower _travel cost_ to a doorstep, never the _target's defenses_.

### Raid value floor (anti-husk, LOOT-1)

- A breached target must be worth the trip. The floor is a **server-minted, tier-scaled bounty** (not a skim of the owner's stockpile), sized so yield ≥ the core cost to dial that tier — the inequality that kills dry holes. Minted from the visible Gate tier, so there's nothing to "spend before it's stolen."
- Also lootable: the coordinate DB (the map) on a breach, and a **coordinate-share** from every killed avatar.

### Offline raiding

- Bounded, two ways only: a **found** home (rare) and an exposed [[#Holdings|holding]] (always contestable, no search). Neither costs your existence.

### Why / rejected

- **Why the attacker's home-Gate sets the clock** (not the defender's, not the gap): it's un-fakeable and rewards growth, and it produces the right asymmetry automatically — a small raider punching up gets a short surgical window, two whales get a long siege. It's safe because you can only raid equal-or-greater, so the defender is never the weaker party.
- **Rejected — topping up away reserve with carried cores/beacons:** lets a fuel-hoarder camp enemy soil indefinitely.
- **Rejected — a separate defender-set siege timer:** unneeded; the attacker's fixed clock already bounds the fight once the Gate locks open.

- Tunables: clock curve vs. Gate power, bounty size per tier, mass caps.
- Open: matchmaking / instancing of bilateral tunnels; transponder / no-caller-ID details (USE-1).

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
- A hub has **no woken-sealed Gate of its own**, so you **cannot turtle** there — it's always live and exposed. But the **designed PvP contest lives on contested frontier worlds**, not here; PvP at a hub is incidental (over cargo), not an arena.
- **Routing:** distant trade hops through hubs for far less power than one long jump (the in-world basis for cheap distance). Direct-dial is the private, expensive alternative.
- **Optionally capturable:** hold the relay/core and you can **tax** traffic, **throttle** (allies cheap, enemies expensive or gate-locked), and gain **intel** on who moves through. Capture-to-tax is the conqueror's take-and-hold prize — a **registry charter flip** (HOLD-1), defended by presence.
- **Relation to the other surfaces:** Safe Core = policed safe commerce; **hubs** = lawless commerce; contested **frontier worlds** = the contest/extraction surface. Hot frontier for contest, Safe Core + hubs for commerce.

- Tunables: toll rates, routing-cost reduction, capture rules.
- Open: whether long-distance dialing needs **jump / relay servers** (hop-routing) to stop one-jump reach being too strong (parked — [[questions]] #16). Hub capture-defence specifics.

## Presence, respawn & XP

The in-world layer under presence, respawn, and sanctioned violence: you pilot a
**disposable avatar** and keep your existence — the body dies, you persist. **What the
avatar physically is, where it comes from, and how your mind connects is open — see
[[questions]], Avatar origin.** Only the origin-agnostic baseline here is decided. [decided; origin open]

- Sets the design's floor: civilizations and bodies are **mortal** but the **player is never deleted** — selection without extinction, made literal (the vehicle dies, you continue).

### Presence & respawn (LINK-1)

- You pilot a **disposable avatar** — genuinely present and **mortal**. What's fixed is that you can lose the body without losing yourself.
- **Only the avatar dies.** Death drops the body + carried gear (lootable); you respawn at an anchor. This is the in-world license for raiding, PvP, and conquest — killing a gater never deletes them.
- **Avatar roster:** develop **multiple avatars** (tuned for different worlds), **pilot one at a time**. One live body, never simultaneous presence.
- **Anchor = where you respawn.** Newcomers: the **Safe Core** hub (always available; arrival flow in [[#First hour — the earned mask|First hour]]). Claim a Gate and build a forward facility → respawn there to hold the world.

### Recall

- **Recall to defend** = abandon your away-avatar and respawn at your anchor. Cost: the away-avatar + gear are **left behind, exposed** — a committed trade, not a free round-trip. On the frontier, recall is the emergency exit (you keep your existence, lose the run's loot).

### Two-layer progression (XP-1)

- **Mind-XP** — permanent, account-wide: knowledge + **banked adaptations**, earned by discovery. Part of your persistent self; **never lost**. _What you know how to be._
- **Avatar-XP** — per-avatar proficiency, earned by doing; **lost with the body on death**. _How developed this body is._
- **High-water-mark:** a fresh avatar **re-levels fast up to your peak**, slow beyond — so death costs gear + a temporary dip + position, never a knowledge wipe.

### Claiming, attunement & the tether

- A Gate you've **claimed** is your respawn anchor and **fully sustains** you (home soil). A foreign Gate sustains only partially → the away-clock. Beyond all Gate range, the avatar can't stay knit and fails. The **mask** is the avatar tech that draws this stabilizing field (see [[#Mask energy|Mask Energy]]).

### Why / rejected

- **Why selection without extinction:** it gives the brutal frontier its floor — your civilization is mortal but you are never deleted, so raiding and PvP stay brutal without permadeath. The body is really there and really dies; you persist.
- **Rejected — piloting multiple avatars at once (multibox):** sequential roster only, so presence stays a real constraint.

- Tunables: re-level curve, recall/abandon cost, away-clock length, respawn time.
- Open: disconnect / AFK / grace-window timings; exact respawn location chain when a forward facility is lost (fall back to Safe Core).

## Coordinates & obscurity

How hidden homes get found — the mechanism behind the **mortal home**: protection is
obscurity (a re-keyable share-trail), not invulnerability. [tentative]

- Gives real, ARK-style stakes back (a home _can_ be wiped) **without** the defender tax. The tax came from the **certainty** of offline loss; hidden coordinates remove the certainty, not the possibility.
- Only your **existence** is unloseable; your **civilization is mortal**.

### Coordinates as intel (COORD-1)

- You never build a new Gate; you **dial from the one you own** and save the **coordinate** — a re-dialable address in your homebase **coordinate database**.
- The DB is a **raidable asset** — a breach can steal the **map** (scouted planets and routes).
- A bookmark removes the _discovery_ cost, never the _travel_ cost (re-dialing still burns distance-scaled fuel).
- **Blind exploration-dialing** (dial the unknown for a fuel cost → a random planet of random richness/danger) is the variable-ratio hook for explorers. [leaning yes]

### Found by avatar-shares (SHARE-1)

- A home coordinate is a **geohash**; **every fielded avatar carries a share** of it.
- **Loot a dead avatar → get its share.** Below a threshold = a coarse region (hunt/scout, can't dial); at/above threshold = the exact dialable coordinate. Shares are **tradeable intel** — buy the last one you need in the Safe Core.
- **Epoch-bound, re-keyable.** Re-keying your address **voids every share already collected**. Cheap/automatic while sealed at home. **Re-key cost/cadence is the master knob** for the online:offline raid ratio.
- **Anti-dodge:** every fielded avatar carries a _current-epoch_ share (no clean decoy avatars); shares die on re-key (no infinite banking).

### The odds — what "safe" promises (player-facing)

The player-facing read of SHARE-1; the mechanism is above, not restated.

- "Safe" is never a wall — it's a **high probability of staying unfound**. A sealed Gate can't be cold-breached; a home whose coordinate someone has assembled is raidable, rarely even while you're offline.
- The odds move with **how you play, not a toggle**:
  - **Quiet, small, mostly sealed** → odds strongly in your favor: few avatars fielded = few shares shed; low Potential keeps you off big hunters' reach.
  - **Loud, large, always out** → odds against you: many avatars leak shares fast; high Potential makes you visible and reachable; big houses are findable by construction (endgame siege targets).
- **Offline tends _toward_ safety, not away from it.** Sealing (fail-closed) sheds no new shares while you're away, and re-key — cheap/automatic while sealed — voids shares a hunter already collected. Quiet-and-safe last week ≈ still safe after a week away; loud-and-found before logging off is not. How strongly offline favors you is the **re-key cadence** (the master knob, SHARE-1).
- `reach = exposure` read as probability: the small are nearly unraidable, the big are worth the siege.

### Consequence — uphill relocates

- Home raids are gated by **finding** (assembling shares), not a power wall. Anti-whale-farm protection moves to frontier-visibility ([[#Potential|Potential]]).
- Ownership is the **house**: one shared coordinate, every member sheds shares of it — so big houses are inherently **leaky** (endgame siege targets) and solos stay stealthy. `reach = exposure` at the group level.

### Why / rejected

- **Why mortal, not safe:** a perfectly-safe home makes the apex base un-raidable — inverts `reach = exposure` where it should bite, kills real stakes. Obscurity restores them without the tax (the tax was the _certainty_ of loss, not the possibility).
- **Rejected — inviolable home:** removes home stakes, makes the frontier pointless to leave.
- **Rejected — presence-gated wipe** (home falls only to a present defender): a no-show abuses it; obscurity makes it unneeded.
- **Rejected — permanent coordinates:** a leak is forever; re-key is the recovery valve.

- Tunables: threshold _k_, share decay, re-key cost/cadence, reveal flavor (prefix-narrowing default vs. triangulation).
- Open: where "return home" lands if the home was wiped while you were away ([[questions]] #13); house shared-ownership sub-rules (#14).

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
- **Not** home-raid eligibility — that's gated by **finding** (shares).

### House is the ownership primitive (HOUSE-1)

- The unit that owns a home, holds a coordinate, and carries Potential is a **house**; a **solo player is a house of one** (no separate code path).
- Roster is a real component, so a big house reads bigger — and **sheds more shares of the same coordinate**, so it's inherently findable. `reach = exposure` at the group level.
- Word convention: **house = the people, home = the place.** "House" never means a building.

### NPC houses — dead houses (HOUSE-2)

- Every ownership / defense / raid mechanic is written against a **house**, so an AI base is just a house whose owner is an NPC — a **dead house**: the registry row of a gater the frontier ate, base intact, defences running on automation (the same frozen-snapshot AI as [[#Holdings|Holdings]]). [current call]
- **Reuse is the point:** Potential rating (visibility/tiering), the dome, the aperture fight, the muster cap (AI defenders count toward K), the loot floor (the bounty is server-minted either way), and the attacker's away-reserve clock all apply unchanged — one code path for AI bases and player bases.
- Dead-house bases seed **procedural frontier worlds** ([[#Gates|FRONTIER-3]]): PvE raid content that trains the raid loop, keeps it alive at low population, and lets the greybox prove the raid fantasy without hundreds of players.
- **PvE-dodge guard:** if a dead house of equal tier yields player-tier loot at zero social risk, nobody raids players — dead-house loot sits **below** player loot at tier (knob lands with the frontier-vs-raid loot balance, [[questions]] #10).

### Why / rejected

- **Why one sticky high-water-mark, account/house-wide:** it closes the two dodges at once — you can't power down to look weak, and you can't hide strength behind a small avatar (smurfing). Falling only on genuine dismantling means "raid longer = build more" feeds growth into capability.
- **Why the house is the primitive:** one ownership model, not two; and the big-house-is-leaky consequence (more members shedding the same coordinate) makes findability scale with size for free.
- **Why "house" (the word):** the ownership noun must be a plain, globally understood English word that scales solo → corp → space-scale bloc; the dynastic register fits the EVE-flavoured endgame. **Rejected — tribe:** ARK's noun (the IP line). **Rejected — charter:** legalistic/UK register, fails the global-English test; "charter" stays the Coalition permission word (charter standing, clearance). **Rejected — clan / crew / company:** Rust's word / doesn't own an empire / collides with in-fiction corporations.
- **Rejected — live power draw:** dodgeable by powering down.
- **Rejected — separate metrics** for reach / stamina / visibility: one number is simpler; split only if tuning forces it.
- **Rejected — solo homes + loose alliances:** no shared fortress; "wipe a house" would mean hunting each member's base, undercutting the ARK fantasy.
- **Rejected — hybrid (solo homes + house-only holdings):** keeps two ownership models; house-of-one unifies them.

- Tunables: the curve converting the four components into the number; component weights.
- Open: component weights and the climb curve; house shared-ownership sub-rules ([[questions]] #14).

## Holdings

Assets staked _outside_ your home sustaining field — the one place the design admits
**bounded offline raiding** without reviving the babysitting tax. [tentative]

- Serves three archetype gaps in one layer: the offline raider's _ambition_ (siphon exposed output, never the home), the builder's "build exposed for a payoff," and a forward-reach option the home field can't reach.
- A new object class: a forward node, claimed resource tap, or off-site cache. **Space-gate holdings** and **captured hubs** are the top-tier cases.
- It has **no woken-sealed Gate of its own**, so there's **nothing to seal** — the same reason space gates can't be turtled. (The home itself is mortal-but-must-be-found; a holding is the **always-on, no-search** target.)

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
- **Lineage is a home-anchored record** (safe while unfound); the living animal is the **exposed wager** (costs a core; can die and be looted; the registry entry survives).
- **"Breeding" is shallow and lossless** — combine two registry entries for a blended/mutated re-roll; no timer.
- Open: tameable roll rate per frontier-world rank, blend odds, instantiation core-cost — or **defer the whole fork** for the greybox.

## Building

_Stub — the anti-decay rule is decided (BUILD-1); build depth is open (BUILD-2)._

- Building on your sealed planet-base is a loved pillar (ARK / Valheim). **No persistent base decay / upkeep** [current call] — the central thing being designed out.
- Defence is the **perimeter at range**, never doorstep cheese — the dome forbids walling the aperture.
- Safe on sealed soil; offline loss only if the home is **found** (rare), never from neglect.
- Open: build depth, materials, structure types, buildable defences, grid vs. free-form, anything analogous to taming.

## Economy

_Stub — drivers/sinks framed, numbers open._

- **Stagnation pressure** (ECON-1) — sealed bases don't grow; growth needs imports — the engine that makes players open up. A peaceful homesteader still progresses to a **soft ceiling**, then plateaus (PROG-2) — a ceiling, not a leak, never an upkeep tax.
- **Power cores** — scarce, scavenged Gate fuel; the primary consumable behind activation.
- **Activation cost** (ECON-2) = `gate tier × distance band × tier-gap multiplier`. Bands: local / regional / cross-map ≈ power cell / partial core / full core.
- **Trade** (TRADE-1) — routed cheaply through hubs (toll + exposure) or dialed direct (private, expensive). Driven by **biome resource asymmetry** (REGION-1). ⚠ How much a trade run risks vs. a raid is open ([[questions]] #2).
- **Sinks** (ECON-3) — hub tolls + the Coalition tithe.
- Open: concrete currencies, resource taxonomy, crafting inputs, faucet/sink balance; how hard stagnation pushes without becoming upkeep; whether long jumps need jump/relay servers.

## Progression

_Stub — drivers + the homesteader ceiling decided; tech tree open._

- Two in-world drivers gate one tier ladder: **rising charter clearance** and **more of the Gate network waking**. Ladder: surface worlds → larger/richer → orbital/space Gates → relay/hub Gates → megastructure/Supergates.
- **Gate tiers are driven, not built** — see [[#Gates|Gates]] and [[gate-physics|Gate Physics]].
- **No base decay** anywhere (BUILD-1).
- **Character progression = two-layer mind/avatar XP** — see [[#Presence, respawn & XP|Presence, respawn & XP]] (XP-1).
- **Sealed soft ceiling (PROG-2):** a fully-sealed homesteader progresses **forever up to a soft ceiling** (frontier tiers need opening), then plateaus — never decays, never punished. Safe because a never-leaver stays unfound.
- Open: the actual tech tree, research mechanics, gear tiers; the Supergate (PROG-1) endgame specifics.

## First hour — the earned mask

_Stub — the shape is set (BOOT-1); intro-flow details are content, deferred._

- **BOOT-1** [tentative]: a newcomer gets a **loaner mask on a small fixed reserve** — the away-clock number for a gater with no home Gate yet ([[#Mask energy|MASK-3]]). The first Gate is claimed through a Coalition-**listed** starter lane ([[world#United Gate Coalition|the patch channel]]): reach and imprint a dormant Gate before the reserve dies — the real claim verb ([[#Gates|FRONTIER-1]]), taught by doing it.
- Survival pressure exists **only here** and retires at the first claim — after it, no meter (consumables are boosts, MASK-4).
- Open: intro flow and failure handling; starter-band curation; loaner reserve length; the post-claim agenda.

## Player archetypes

The playstyles Gaters expects, sorted by how the balance thesis treats each. **Test every
proposal against this set** the same way it's tested against the design traps: a decision
that silently starves a _served_ style, or revives a _denied_ one, is a blocking flag.

Reference pull: **Rust** (raiding/offline-raiding, zergs, solo grinders, road PvP,
trader-bandits — the toxicity reacted to); **ARK** (taming/breeding, mega-tribes farming
newbies, elaborate bases, PvE homesteaders); **Valheim** (co-op, building-for-joy with no
decay, exploration, boss progression — its largest audience never PvPs).

Mechanic IDs point to the section that owns the mechanic; **⚠ = gap** (no mechanic yet).

### Served — the loved pillars; each needs a satisfying standalone loop

- **Raider** [decided — core fantasy]: open an uphill tunnel → breach the protected **dome** → take loot / the coordinate DB / the control device → withdraw. The headline.
  - **Find targets** — active gates radiate a traceable signature (running = detectable, sealed = dark), plus stolen coordinate DBs, blind-dial, bought intel. (COORD-1/2)
  - Targets that hold **real value** — not husks worth less than the away-reserve window. (LOOT-1)
  - A breach that's a **fight, not a wall** (dome + aperture combat) inside the clock. (GATE-1, SIEGE-1, COMBAT-1/2)
  - An **extraction path** home, and **anonymity** (raid without leaking your own coordinate). (TUNNEL-1, USE-1)
- **Builder** [tentative]: build freely on sealed soil; no decay; defence is the perimeter at range, never doorstep cheese.
  - **Safe sealed soil** — no decay; offline loss only if the home is **found** (rare), never from neglect. (BUILD-1)
  - **Build depth** worth the hours (structures, defences, production). (⚠ BUILD-2 open)
  - A reason _and_ a way to build **exposed** too (forward bases) that pays for the risk. (HOLD-1)
- **Explorer / scout** [tentative]: dial or probe unknown coordinates, map the network, blind-dial as a slot machine; early scouting becomes a late-game hit list.
  - **Unknown space** to probe/blind-dial, and **signatures to trace**. (COORD-2)
  - **Coordinates as a valuable output** — tradeable, stealable intel (their "loot"). (COORD-1)
  - **Compounding discovery** — early scouting becomes a hit-list others pay for. (COORD-1)
- **Trader / economist** [tentative]: hub routing, tolls, tithe; how much a trade run risks vs. a raid is **open** ([[questions]] #2).
  - **Resource asymmetry** between worlds — a reason goods move. (REGION-1)
  - **Routes and a market** — hub routing, direct dial, access lists. (HUB-1, TRADE-1/ECON-2)
  - **Risk on cargo vs. the vault** — ⚠ open.
  - **Sinks** (tolls, tithe) that keep it flowing. (ECON-3)
- **Conqueror / warlord** [tentative]: the ARK "take a whole base" fantasy — take **and hold**, not just loot-and-leave.
  - **Exposed bases/territory worth capturing** with real value, not husks. (HOLD-1, LOOT-1)
  - A way to **take _and_ hold** — capture transfers the asset; defending it is the game. Routed to always-live capture targets (hubs for toll control, space gates) — a registry charter flip held by presence, not a decaying home-side claim. (HOLD-1, HUB-1)
  - **Peer-or-greater targets only** (uphill) — conquest, not newbie-farming. (REACH-1)
  - **Takes the empire, never the _existence_** — conquest can wipe a found home/civilization, but never the player. (LINK-1)

### Redirected — valid, but funnelled to a designated arena, never onto sleeping homes

- **PvP brawler / roamer** [tentative]: combat for its own sake → contested frontier worlds, space gates (no longer hubs).
  - An **always-live arena** — no territory required to join.
  - **Combat fun on its own**, quick in/out. (COMBAT-1)
- **Clan / group / zerg** [tentative]: coordinated mass → **hub captures (toll control), big frontier pushes, Supergate events**; cannot punch down (uphill frontier-visibility), so kept off solos and newbies.
  - **Large-scale objectives** that reward coordination. (HUB-1, PROG-1)
  - A hard **can't-punch-down** floor — kept off solos / newbies' existence. (REACH-1)
  - **Group-only payoffs** worth the org overhead. (PROG-1)

### Denied — the ARK/Rust pain the design exists to remove; scenarios must confirm each stays dead

- **Offline raider** [decided — reframed]: a sleeping player's **existence** stays denied (the body is on Earth). Their **home** is no longer immune — a **found** coordinate can be raided offline — but this is **rare** (you must find them; offline raids skew to the recently-active), not the ARK certainty. Their exposed holdings stay always-contestable without any search.
- **Megabase newbie-farmer** [decided — denied]: blocked by **obscurity + uphill frontier-visibility** — a whale can't see a small player's frontier pushes to harvest their shares.
- **Turtle / babysitter** [decided — denied as a _dominant_ style]: sealing buys **obscurity, not immunity** (the home is mortal) and stays stagnant; the aperture cannot be cheese-fortified.

### Resolved forks — were undecided; now served at a defined scope

- **Tamer / breeder** [tentative — narrow yes]: include taming (frontier-sourced companions); decline deep ARK breeding/genetics (a babysitting chore by construction).
- **Peaceful homesteader** [tentative — served]: Valheim's largest audience never opens — build, explore, run solo PvE on the frontier forever, progressing to a **soft ceiling** (PROG-2), never punished for staying sealed. Safe because a never-leaver stays **unfound**, not by an inviolable wall.

## Validation — two questions, two scales

Two risks, de-risked differently. Don't conflate them.

- **Is the loop fun? — greybox-answerable.** Do the sealed/open state, the bilateral tunnel, the siege clock, and real PvP combat feel good moment-to-moment? Build a **greybox proof** before any art. Cheap, early, and _necessary but not sufficient_.
- **Will the population open up rather than turtle? — only shows at scale.** The real make-or-break, and a greybox **cannot** settle it: open-vs-bunker is an emergent **equilibrium** of a live economy — it needs hundreds of players, market depth, and social pressure before it stabilises. Five testers say nothing about where 500 land. Treat it **assume-and-commit**: reason from games that ran the experiment (Rust, EVE, Albion, Foxhole), model the economic pressure, and prove it only at a **scaled paid playtest**. The biggest risk is the one you can't cheaply prove.
- **Minimum viable slice:** sealed/open states, a bilateral raid tunnel, loot worth taking, economic pressure against permanent sealing, real PvP combat. **Ground Gates only**; space Gates later.
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
- **A persistent shared world to simulate 24/7** → **presence requirement + sealed-by-default.** A Gate is open only while its owner is online and chose it. Offline = sealed = nothing to simulate. No offline base sim, no decay tick, no sleeping-base economy. The single biggest server saving, and it's a _gameplay pillar_.
- **An authority transition between solo and contested play** → **server-authoritative always.** One authority owns each planet at all times, so there is no transition between authority models to reconcile or exploit. The cost cap comes from the presence requirement: only online players' planets are simulated, and solo/sealed building — having no adversary — ticks cheaply.
- **World-size-proportional cost** → **three-state planet + seed-plus-deltas.** A planet is a DB row (**empty**) → a cheap server-side tick (**solo**) → a full spun-up instance (**contested**). Server cost ∝ online players, not world size.
- **Massive concurrent battles as the default** → **bilateral directional tunnels.** A raid is a two-party instance, not an N-body shared zone. Trivially shardable; no thundering-herd simulation in the common case.
- **Unbounded session / instance lifetime** → **away-reserve clock, ~38-min ceiling.** Contested instances are short-lived by fiction, so they're cheap to spin up and tear down and never leak.
- **Bounding the simulated volume** → **mask field + the dome.** Only the area around an _active_ Gate needs simulating; the field radius is the soft world border and the dome bounds the combat arena.
- **Infinite/contiguous procedural universe** → **discrete, seedable planets loaded one at a time.** Each planet is an isolated bounded chunk behind a `PlanetGenerator` abstraction.

### Part B — the technical residue, ranked, with the lever for each

What the Gate fiction does _not_ already pay for. Ranked by severity for a solo dev.

1. **Two worlds, live, through an open Gate — _highest_.** "A firefight at the aperture, not a loading screen" means rendering and simulating **two planets at once** through a portal — the Portal/Prey rendering problem, plus two physics scenes, plus netcode spanning them. Levers: **the dome bounds the portal** (you never render the whole far planet — only what's visible through the throat, and the dome caps that radius); **the aperture is the only shared volume** (both sides simulate their own planet; only the throat region is co-simulated); **graceful-degrade fallback** — the aperture degrades to a shared staging volume both sides load into; greybox can ship the cheap version first.
2. **Networked physics across an authority boundary — _high_.** "Momentum conserved through the Gate" means a physics object changes authority mid-flight — a classic netcode trap. Lever: the **aperture is the single transfer plane**, and the **mass cap** bounds how many such objects exist at once.
3. **Instance orchestration, cold-start, spin-up latency — _medium_.** Spinning up a contested instance the moment a raid commits, cheaply and fast. Lever: the **dial → tunnel-open sequence is a provisioning window** — the diegetic "dialing" animation covers instance cold-start; both endpoints known in advance; uphill-only shrinks the pool; the ~38-min cap makes instances short and self-terminating. (Greybox: a listen-server removes orchestration entirely.)
4. **The one irreducible shared state — the registry — _medium_.** Claims, coordinates, who-holds-what: a global, consistent, persistent dataset — the one component instancing can't remove; couples to raider anonymity ([[questions]] #8). Lever: it's a **database problem, not a simulation problem**. The Coalition _is_ the natural home for centralized state (lore = the schema); the registry knows owners, but the raid tunnel need not expose the attacker until commit (transponder model). Keep the registry small (metadata only).
5. **The many-vs-many exception — hubs, server events — _medium, but cappable_.** Explicitly many-vs-many surfaces are where "instancing makes it cheap" breaks. Lever: they are **few, optional, and bounded** — hubs hard-capped in concurrency, uncontrolled lanes temporary by fiction (they collapse on a countdown). Keep them out of the greybox.
6. **Procedural generation breadth — _medium, already deferred_.** Lever: the **`PlanetGenerator` abstraction from day one** — greybox ships a trivial generator; richness lands later. Field radius bounds how much of each planet must be generated.
7. **The web export's real ceiling — _low-medium_.** A web build as a playtest funnel can't match native. Lever: scope it as a **demo slice** — a single planet, a single raid; a web client can host/join one bilateral instance without the full backend.
8. **The meta-risk — solo scope — _the real top risk_.** One person building portal rendering + orchestration + procgen + combat + netcode is enormous even after the Gate removes the worst. Lever: "design for success enabling expansion, not requiring it" — every item above has a named cheap path for the greybox.

### Part C — lore to lock _because it pays an engineering bill_

Fiction choices that double as architecture — each closes a technical hole, not just a story one.

- **"Dialing is a sequence, not an instant"** — the dial animation is the instance-provisioning window. Pays #3.
- **The dome radius as the render/sim bound** — also caps the portal frustum and dual-sim volume. Pays #1.
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
