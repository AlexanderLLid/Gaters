# Gaters — Comprehensive Design Overview (Raw Braindump)

> **What this is.** A single, deliberately exhaustive dump of everything established
> about the game **Gaters** so far. It is _not_ formatted as final wiki pages. Each
> concept gets its own named heading and a self-contained paragraph so another agent
> can lift it into its own page. Every claim is tagged:
> **[decided]** = firmly committed, **[tentative]** = current leaning / proposed,
> **[open]** = undecided / not yet established. Where two ideas conflict, the conflict
> is flagged rather than smoothed over. Recurring concepts use consistent names:
> **Gate**, **planet-base**, **hub world**, **sealed / port-open / gate-open**,
> **mask energy**, **siege timer**, **Rift**, **sustaining field**, **away reserve**,
> **coordinates**, **holding**.
>
> **Naming note.** The title is **Gaters** [decided, recent]. Earlier drafts and the
> request header used "Gateers"; that is superseded. A phonetic/searchability concern
> for "Gaters" (reads as "gators"; the "gay-ter" / "gaiters" homophones) was raised and
> is **unresolved** — see Open Questions. The in-world term for a player is **gater**.
>
> **Scope note.** Nothing in the Lore sections has been invented to fill gaps. Where a
> category (characters, species, religions, myths, languages) has no established content,
> it is listed explicitly as undecided rather than fabricated.

---

# PART I — LORE / WORLD

### Premise & Central Conflict

[decided] Gaters is a sci-fi multiplayer game in which each player controls a single
**planet-base** sitting on an ancient **Gate**. A long-dormant galaxy-spanning network of
Gates has suddenly switched on, and a **central authority** is deputizing independent
gaters (the players) to find, claim, exploit, and hold the worlds those Gates sit on.
The central conflict is a **gold rush over finite, un-manufacturable Gate infrastructure**:
reach and wealth require opening your Gate and venturing out — and venturing out is what exposes
you: acting is how your home is eventually **found** and made raidable and how you can
be attacked. The whole design exists to deliver the _aggressor_ fantasy (raiding, PvP, looting)
without the _defender_ tax (base-babysitting, decay, _certain_ offline loss, forced 24/7
presence) that the designer disliked in ARK and Rust — the tax was the **certainty** of loss,
which obscurity removes, not the possibility.

### Design Thesis ("Reach equals exposure")

[decided] The single organizing principle: **you can only act on the world as far as you
have opened yourself to it; you choose when and toward whom; and your home's safety is
**obscurity — being unfound — not an inviolable wall.** A **sealed** planet-base is safe
**while it stays hidden**, but it is **mortal**: act and venture enough and you are eventually found
and raidable. Acting (trading, raiding, exploring) requires opening and shedding a trail, which
creates risk. This is intended to be "MAD in
space": mutual vulnerability breeds deterrence, alliances, and "cover my Gate while I trade"
pacts emerge naturally.

### Origin Problem / Designer Intent

[decided] The project is a deliberate reaction to survival-PvP genre pain. **Loved** and to
be preserved: PvP combat, raiding/breaching, looting, high-stakes offense. **Hated** and to
be designed out: babysitting one's own base, upkeep/decay timers, getting raided while
offline, the "unpaid-labor" feeling of logging in to maintain rather than play, and being
forced online constantly to avoid loss. The diagnosis: persistent shared-world progress +
offline vulnerability mechanically rewards the highest-time-investment, most-aggressive
players and punishes everyone else, and "toxicity" and "base-babysitting" are two surfaces of
that one design choice.

### Setting & Tone

[tentative] Frontier sci-fi; a freshly-opened, lawless gold-rush among the bones of a
vanished precursor civilization. Tone leans toward dangerous-frontier-opportunity with an
undercurrent of cosmic unease (the network woke for a reason nobody understands). Aesthetic
touchstones below. Exact tone register (gritty vs. adventurous vs. eerie) is **[open]**.

### The Wake (the inciting event) — [WAKE-1: gate-signature detection]

[tentative] **Earth's Gate** was dormant and undetectable for so long nobody knew it was
there. Then the hub at the network's heart (the central nexus) brought **Earth's
lane** online **for reasons no one understands**, and the waking Gate radiated a detectable
**energy signature**. Humans traced it, found the Gate, and learned to open it — Stargate-style:
a found artifact, not a summons. Dialing through reaches the nexus (the safe core), where the
central authority charters the newcomers. This is the in-world "year zero." _Why_ the hub woke
Earth's lane is the central mystery, and the same activation **destabilizes** the network (so
Rifts proliferate over time). The name "the Wake" is a placeholder label for the event **[open]**.

### The Central Authority

[tentative] The network's established administrators, seated at the central nexus.
When Earth's Gate woke and human newcomers dialed in, the Authority — holding more network than
it can claim alone — **issued charters** to independent gaters (players) to claim and hold
Gate-worlds in exchange for a **cut/tithe**. It **holds the root of the network** (the nexus),
which is why it can dispense access — but it did **not** cause the Wake and does not know what
woke Earth's lane.
It **guarantees your charter standing** (no one strikes your claim off the registry while you hold
it — but this is _registry_ right, **not** physical immunity: a found home is raidable)
and **runs a safe core** (the policed market — including the coordinate/**share** trade —
plus the new-player spawn/tutorial zone), but its writ **stops at the frontier edge** — past that
line, reach equals exposure is the only law.

[decided] **Hidden motive.** The charters are not only profit: fearing the woken
network is a threat to Earth, the Authority uses its gaters as **crowdsourced reconnaissance** —
every world claimed and Builder ruin explored feeds its hunt for who built the Gates and why. The
gold rush is cover and engine at once; the Authority cannot survey the frontier alone, and its own
ignorance is exactly why it needs the players. **This is also the in-world license for the
competitive sandbox** — claiming, building, raiding, trading, and sprawling are all _sanctioned_,
because aggression maps the threat fastest **and** ruthless competition forges the strongest
humans. The Authority runs the frontier as deliberate capitalism / natural selection: the capable,
battle-hardened cadre that rises is what Earth will need against the threat. What the threat
actually is stays unknown even to the Authority — see The Mystery.

[decided, deepened] **Locked on natural selection.** The Authority is
**method-agnostic** — attack, tech, trade, build are all valid selection pressures, so **no
playstyle is "wrong"** (the in-world reason the sandbox serves every archetype equally). Its
protective rules are **selection-efficiency, not mercy:** uphill **Rift-visibility**
lets the weak punch up but keeps the strong from trivially farming the undeveloped, because
curb-stomping culls potential before it forges anything. The peaceful homesteader isn't punished —
they **self-select out** of the strong cadre and plateau (PROG-2). The body-on-Earth model (LINK-1)
is **what gives this teeth without erasing the stock: selection by culling, floored at the body** —
civilizations are **mortal and can be wiped**, so competition has real stakes, yet no
player is ever truly deleted (the body is on Earth). The Darwinism is the **revealed core, not the pitch:** the
gold-rush profit story is the cover (The Mystery), and players uncover that the Authority is forging
them.

Placeholder names: Directorate / Survey / Compact / Commission / Charter Office **[open]**.

### The Gates (the central object) — [GATE-1: the dome]

[decided] Gates are ring/aperture structures; **owning a planet means owning its Gate**, which
is _why_ bases are planets rather than an arbitrary rule. [decided] Gates **woke sealed** — the
network came online with every aperture shut, which is the in-world justification for
safe-by-default. [decided] Gates **cannot be manufactured by anyone alive**; they can only be
**found, claimed, and repaired**. Their finiteness is the root cause of all conflict. [decided]
The ring is fixed but the **aperture is driven** — how wide the throat opens is set by the exotic
matter you channel, not built, so "bigger Gate" never means fabricating one. [decided]
An active Gate projects a tier-scaled protective **dome** that destroys manual builds but spares
natural terrain, so it can never be walled in — a deliberate Builder feature that also protects
what comes through. See the Gate _system_ in Part II for mechanics. Placeholder in-world names for the
object: Rings / Apertures / Wayrings / Conduits **[open]**.

### The Gate Network

[tentative] The full web of Gates: **the Gate Network**. [tentative] More of the Gate
Network "waking" over time is one of the two
in-world drivers of content unlocks (the other being rising charter clearance). The network
periodically **self-updates** (Gates sync coordinates with neighbors) — a lore hook that
doubles as a propagating-cyber-weapon vector (see Gate Uses). **[open]:** the network's true
extent, topology, and whether it spans one system, many systems, or galaxies.

### Stable vs. Unstable Wormholes (the cosmology)

[tentative] Gates and Rifts are **two states of one physics**, not two unrelated systems. A
**Gate** is a _stable_ wormhole — Builder-engineered, persistent, dial-able, sealable, safe by
default, with two controllable endpoints (you and a dialed target). A **Rift** is an _unstable_
wormhole — spontaneous, uncontrollable, one end opening "somewhere," that **collapses on its own**
after a short life (this self-collapse is the in-world source of a Rift's countdown, not a
bolted-on timer). Rifts tend to tear open **near** Gates, because a stable wormhole marks a spot
where the fabric is already thin and active — so holding a Gate makes you a magnet for Rifts,
concentrating both opportunity and danger on Gate-holders. This unifies the cosmology and feeds
the mystery: the Wake didn't merely switch the network on, it **destabilized** it, so "Rifts are
increasing" reads at once as a difficulty ramp and as the Gate Network tearing while something reaches
through. See Rifts and "Rifts → New Gates" in Part II. **[open]:** whether Rifts can be stabilized
into new Gates, and whether players can ever influence where/when a Rift opens or it is pure RNG.

### The Builders (deliberately unknown)

[decided] Who built the Gates, and why, is **unknown to everyone** — the authority, the
players, and (for now) the design. This ignorance is an intentional, ownable choice (more
distinctive and more unsettling than a named precursor pantheon) and is treated as the
long-game mystery engine. **[open]:** literally everything about them, by design — they should
remain unnamed until/unless the mystery payoff requires otherwise.

### Power Cores (the fuel resource, lore framing)

[tentative] Gates run on rare **power cores** that are **scavenged, not manufactured**. Local
Gate jumps sip power; long jumps drain whole cores. Scarcity of cores is a primary economic
driver. (Mechanical framing in Economy / Part II.) Exact nature, rarity curve, and whether
cores are consumable or rechargeable are **[open]**.

### Regions & World Types — [REGION-1: resource/world asymmetry]

These double as the unlock ladder (see Progression). Each is a place-type that can become its
own page.

- **Home planet-base** [decided]: the player's own claimed world on a single Gate; safe when
  sealed; the seat of building, solo PvE, and defense.
- **Small surface worlds** [tentative]: the starting tier; only short-range Gates have woken,
  confining early play to one local cluster. The confinement is a _feature_ that explains
  starting scope.
- **Larger / richer worlds** [tentative]: more to build, defend, and take; escalation of stakes.
- **Orbital / space Gates ("space planets")** [tentative]: free-floating Gates with no planet
  beneath them. Crucially, they **cannot be turtled the way a sealed home planet-base can**, so
  they function as always-live PvP zones where lawless action concentrates.
- **Hub worlds** [tentative]: neutral, lawless crossroads (see Hub Worlds in Part II).
- **Megastructure / Supergate sites** [tentative]: rare precursor megastructures that pass
  fleets; server-event scale; the macro-expansion path between systems.
- **The Safe Core** [tentative]: the authority-policed market + spawn/tutorial region.
- **The Frontier** [tentative]: everything past the authority's writ, where exposure rules apply.

**Resource asymmetry — why goods move [tentative] — [REGION-1].** Each world's seed rolls a
**biome tag set** (e.g. `volcanic`, `cryo`, `irradiated`, `dense-core`) gating which raw resources
spawn there, so one world's output is another's missing input — the base trade driver, pure faucet
design on the safe side. At the high tiers asymmetry is made to **stick**: a rare **field-harmonic**
tag on some worlds means certain top-tier refinements only complete inside a matching world's
sustaining field, so the best goods **cannot be self-supplied by claiming a second planet** — they
always move through trade or raid. It rides the `PlanetGenerator` abstraction (asymmetry is data in
the seed, scaling like the planet count already does). Tunables: biome roll weights, recipe-crossing
depth, the second-claim cost (the self-supply throttle). Keep asymmetry gating _climbing faster /
reaching higher tiers_, never _functioning at all_ — the latter bleeds into the stagnation-vs-
obligation line (Conflicts).

### Factions / Organizations

- **The Central Authority** [tentative]: described above; the only firmly-conceived faction.
- **Chartered gaters (players)** [decided as a concept]: the player population, holding
  charters. Whether there are formal NPC factions, player alliances as first-class entities,
  or guild systems is **[open]**.
- **Pirates / raiders at hubs** [tentative]: emergent rather than a designed NPC faction;
  whether piracy is purely player-driven or has NPC presence is **[open]**.
- **"Zerg" clans** [tentative]: large organized groups; acknowledged as something the design
  funnels toward hub conflict and away from farming sleeping casuals. Not a formal faction.
- **[open]:** any named in-world factions, their politics, rivalries, leadership, or NPC powers.

### Key Characters

[open] **None established.** No named NPCs, authority figures, rival gaters, or quest-givers
exist yet. Do not invent. (If the mystery or onboarding later needs a face for the authority,
that is a future decision.)

### Species & Peoples

[open] **None established.** It is undecided whether players are all one species (e.g. humans),
whether multiple playable or NPC peoples exist, or whether the setting is post-human / alien.
Do not invent.

### Cultures

[open] **None established.**

### Religions

[open] **None established.** (Possible future hook: belief systems around the unknown Builders
or the meaning of the Wake — explicitly _not_ yet created.)

### Myths

[open] **None established.** (The Builders and the Wake are obvious future myth-fuel, but no
myths have been written.)

### Languages

[open] **None established.** No conlang, glyph set, or naming conventions decided. (Note: any
Gate "glyph/symbol" set must avoid copying Stargate's chevron/glyph expression — see IP note.)

### Notable Items / Objects

These are objects/items already implied by the design and can each become a page.

- **Power core** [tentative]: scavenged Gate fuel (above) and the economy's hard currency; also
  **portable mask fuel** for topping survival **on your own soil**. It does **not** extend the
  raid clock — away reserve is fixed by home-Gate power and un-pumpable.
- **Gate control device** [tentative]: the per-Gate console that supplies power, controls
  dialing, and does the targeting math; a discrete, attackable, _stealable_ component (the "DHD-equivalent," but must be given an original name and expression — **[open]** name).
- **Transponder / access codes** [tentative]: identification a Gate accepts to distinguish
  friendly trade traffic from anonymous raiders; spoofable (infiltration hook).
- **Gate segments** [tentative]: pieces from which megastructure/Supergates are assembled and
  repaired.
- **Beacon** [tentative]: a lightweight, plantable "landing site" near a target that permitted
  players can jump to, distinct from a full Gate (EVE cyno-beacon analog). It does **not** project
  a sustaining field that extends a raider's away reserve — the raid clock is un-pumpable; its role is the jump/landing point only.
- **Coordinate database** [tentative]: the per-homebase log of discovered **coordinates**
  (re-dialable addresses). A first-class, **raidable asset** — breaching a homebase can let an
  attacker steal the map (scouted planets and routes). See Coordinate Discovery.
- **Jammer** [tentative]: a structure that denies Gate-jumping across an area (EVE cyno-jammer analog).
- **[open]:** weapons, armor, building materials, consumables, trade goods, artifacts.

### Timeline / History

[tentative] **The Wake = year zero**; gameplay begins moments after, in a land rush "that
started five minutes ago." Before that: an indeterminate **dormancy** of the sealed network,
preceded by the **vanished Builders' era**. [decided] The design wants events to begin _now_
so all players start the rush together. **[open]:** any dated history, prior wars, the
authority's founding, how long dormancy lasted, or what (if anything) happened between the
Builders' disappearance and the Wake.

### The Mystery (cover story vs. truth)

[tentative] **Official story:** the authority found a dormant relay and switched the network
on — routine, profitable, under control. **Hidden truth (players slowly uncover):** the Gates
did _not_ wake on a timer or because the authority flipped a switch — **something woke them,
and that something is using the Gate Network too.** Three candidate truths, **not yet chosen
[open]:**

1. A signal from _outside_ the system — something is coming, and the Gates are how it gets in.
2. The network is a **tripwire** — lighting it told the original owners that someone is home.
3. The authority's "discovery" was a **lie** — it knew, and woke the Gates deliberately for
   undisclosed reasons.
   This is the intended late-game escalation and the reason the frontier keeps pushing outward.
   [tentative] Mechanically this couples to **Rifts** and the stable/unstable cosmology: the Wake
   **destabilized** the Gate Network, and rising Rift frequency/rank over the game's life is both the
   difficulty ramp and the visible symptom of whichever hidden truth becomes canon.

[decided] **Middle layer (the Authority's motive).** Between the official story and the
hidden truth sits the Authority's own fear: it suspects the network endangers Earth and charters
gaters to find out, profit as cover (see The Central Authority). This **colours the mystery
without choosing it** — it leans toward truth 3's "knew more than it let on," while 1 and 2 remain
the _nature_ of the threat. Crucially the Authority does not know the answer either; it is
investigating, not concealing a solution. This also gives players a narrative goal — explore,
build, and dig up Builder lore to uncover the threat — layered over the mechanical land rush.

### IP / Originality Constraint

[decided] The Gate/wormhole _mechanic_ is genre-generic and not ownable, but Gaters must avoid
Stargate's specific _expression_: no "Stargate," no Egyptian-mythology skin, no chevron/glyph
set, no copied proper nouns, no ascended-precursor lore, no named dialing device. Serial
numbers off; original names and story throughout. (Stated as a practical design line, not
legal advice.)

---

# PART II — SYSTEMS / MECHANICS

### Core Loop — Moment-to-Moment

[tentative] On your own **planet-base** you explore, gather, and build in a **solo PvE**
sandbox while **sealed** (safe). When you choose to act — trade or raid — you bring
your **Gate** to **port-open** or **gate-open**, accepting risk. Raiding: open a directional
tunnel to a target, push a strike force through, fight at and beyond the aperture under a
**mask energy** constraint, take loot, withdraw. Defending: if someone opens a raid tunnel to
your open Gate, a **siege timer** locks it open for a bounded fight you opted into. Exact
verbs of second-to-second play (movement, shooting, abilities) are **[open]**.

### Core Loop — Session

[tentative] Log in → your stuff is intact (no offline loss, no decay to repair) → choose
whether to stay sealed and build, open a **port** to trade, or open your **Gate** to raid /
visit a **hub world** → resolve contained, finite engagements → log off and reseal. The
explicit goal is "log in when you feel like it" with **no maintenance obligation**.

### Core Loop — Long-Term

[tentative] Grow your planet-base, accumulate power cores and loot, raise **charter clearance**,
and unlock higher **Region/World tiers** (small worlds → larger worlds → orbital/space Gates →
hub/relay Gates → megastructure Gates) as more of the **Gate Network** wakes — culminating in
fleet-scale Supergate play and, eventually, the mystery payoff. No persistent base to babysit
across this arc, by design.

### The Gate System

[decided] Each planet-base has a Gate. [decided] Travel is **one-way and directional**: the
_dialing_ side opens the tunnel and pays the power, so a raid is inherently the attacker
pushing through a tunnel they opened. [tentative] Momentum is conserved and **weapons fire
passes through** a live Gate (you can shoot up/down an open tunnel — breaching is a firefight
at the aperture, not a loading screen). [tentative] **Bigger aperture = bigger payload = more
power** (the basis of the gate-tier ladder). [tentative] Dialing requires **knowing the
target's coordinates** (coordinates are intel: discoverable, tradeable, stealable, hideable — a
planet whose coordinates nobody knows is effectively hidden; see Coordinate Discovery). Term
note: **coordinates** is the standardized term for what earlier drafts called the "address."
Specific dialing UX is **[open]**.

### Gate States: sealed / port-open / gate-open — [STATE-1: tiered exposure]

[tentative] Exposure is **tiered, not binary**:

- **Sealed** [tentative]: safe **while unfound**, isolated, stagnant — **not inviolable**: a home
  whose coordinate has been assembled is raidable. Incoming matter cannot reintegrate and
  the aperture is shut (no _cold_ breach through the throat); communication can still pass (see Comms), but no goods, energy, or people transit.
- **Port-open** [tentative]: can trade; pirates can hit your **cargo/convoys** but **cannot
  breach your vault/home**. Trading therefore risks goods, never your whole civilization.
- **Gate-open** [tentative]: can launch raids and is **fully breachable**.
  Each step out buys more reach at the cost of more risk. The three-tier model is the agreed
  shape; exact thresholds and naming are **[tentative]**.

### Presence Requirement — [PRES-1: presence / no offline raid]

[tentative] A Gate can only be **open (used to act) while the owner is
online and choosing it**; logging off normally **seals** it. Presence still gates _your own acting_
and stops a **cold** breach of a sealed throat — but it no longer makes you immune: a home whose
coordinate has been **found** can be raided, rarely, even offline. The babysitting tax is
killed not by an inviolable wall but by **obscurity** — offline loss is _uncertain and rare_ (skewed
to the recently-active); the tax was always the **certainty**, not the possibility.

[tentative] **In-world: the Gate fails closed (the "fail-safe Gate")** — PRES-1 as a consequence,
not a bolted-on rule:

- **Sealed is the resting state** — "Gates woke sealed" (GATE-1); open is the exception that takes
  an active operator. No operator → reverts to sealed.
- **Why it needs an operator / fails shut** = Builder design (a deliberate conceit, like the dome); the deeper _why_ feeds the Builders-mystery.
- **One Builder principle** — _the Gate guards what's behind it unless actively driven open_ —
  generates woke-sealed, seals-on-logoff, and the un-wallable dome at once.

[tentative] **No instant dodge** (closes the offline-dodge trap _before_ a raid lands):

- **A sealed Gate can't be _cold_-breached** — a sealed throat has no aperture to punch; but once
  your coordinate is **assembled** (via avatar-shares) an attacker can dial and open a raid
  on you. Being found is the threat, not just a watched window.
- **Sealing is a spin-down, not a switch** — closing the throat takes time, like opening it (the aperture is driven/channeled).
- **A committed inbound dial latches you** before the spin-down finishes → locked into the siege. Sealing covers the _cold_ breach; lasting safety is **being unfound**.

**[open]:** disconnect/AFK/grace-window timings; spin-down duration. (The consciousness-link layer
that makes presence diegetic — projected mind, body-on-Earth, respawn — is now decided: LINK-1.)

### Consciousness-Link & Avatar Identity — [LINK-1: body-on-Earth / avatar projection · XP-1: two-layer mind/avatar XP]

[tentative] The in-world layer under presence and respawn (resolves PRES-1's open
consciousness-link note).

**Body-on-Earth — [LINK-1].**

- A gater's **real body stays on Earth, in the Authority's custody**; what crosses a Gate is a
  **projected avatar** driven by the linked mind. Custody is a **condition of the charter** —
  accepted to become a gater (framed opt-in; functionally required).
- **Only avatars die.** Death drops the avatar + carried gear (lootable — LOOT-1); the mind
  re-anchors home. This is the **in-world license for sanctioned violence** (raiding, PvP, conquest): the Authority permits a brutal frontier because no one truly dies — **selection
  without extinction**.
- **Avatar roster:** develop **multiple avatars**, **pilot one at a time** (sequential; multibox declined). Home is their launch hub.
- **Recall to defend.** While away you can **instantly recall** your mind to your home Gate —
  **cost:** the away-avatar + gear are **left behind, exposed** (killable/lootable). The answer to
  "being online elsewhere doesn't protect home": recall does, at a price.

**Travel = exposure (open ≈ go somewhere) — extends TUNNEL-1.**

- Leaving through your own Gate makes it **active** (detectable — WAKE-1) and opens the **bilateral
  tunnel**: while out, home is reachable **only by your feud/destination partner through that one
  link**, never the whole network.
- Active **persists ~20 min after departure** — no instant seal behind you (mirrors the PRES-1 spin-down + commit-latch). Sealed-and-home is the only state with no _live_ tunnel — but "dark" now
  means **unfound**, not merely sealed.

**Two-layer progression — [XP-1]** (the character/skill progression PROG left open).

- **Mind-XP** — **permanent, account-wide**, earned by **discovery** (first-time unlocks: tech, recipes, skills). Lives with the body on Earth; **never lost**. _What you know how to do._
- **Avatar-XP** — **per-avatar proficiency**, earned by **doing**; **lost with the avatar on
  death**. _How good this body is at it._
- **High-water-mark "learn ability":** the mind stores your **strongest-ever proficiency per
  skill-line**; a fresh avatar **re-levels fast up to that mark**, slow beyond — so death costs
  **gear + a temporary dip + position**, never a knowledge wipe.
- vs. ARK: a new character there is a full grind reset; here the **mind keeps the ceiling**, only
  the avatar re-earns the climb.

### The Siege Timer — [SIEGE-1: raid clock / away reserve]

[tentative] Once a raid is initiated against an **open** Gate, the Gate is
**locked open** until the raid clock runs out, so the defender cannot instantly slam the door
(offline-dodge fix) and the fight still ends. The clock is the **attacker's away reserve**,
fixed by the **attacker's home Gate power** at dial time and **not extendable by carried
fuel** — the only way to raid longer is to grow your own base. Raid length is therefore a
**curve of Gate power**, not a single number, capped by the **~38-minute** max wormhole
duration.

### Directional Bilateral Tunnels (anti-zerg) — [TUNNEL-1: bilateral directional tunnels]

[tentative] Opening a Gate is **not** shields-down to the whole galaxy; it is a **jump tunnel
to one specific planet**. Raiding Planet A opens a **two-way** tunnel between you and A (A can
counter-attack through it), but **third parties cannot pile in** unless they also open their
own Gates to you. Each raid is a **duel/feud**, not a free-for-all. Trade routes work the same
way: two planets mutually open to each other, and that single link is the only vulnerable
surface.

### The Four Design Traps (and their solutions) — [REACH-1: uphill-only reach]

A consolidated, page-worthy set. Each trap is a failure mode the exposure model could fall into.

- **Turtle equilibrium** [decided problem]: if sealed = safe forever with no downside, everyone
  bunkers and the game dies. **Solution [tentative]:** the **economy forces exposure** — a
  sealed planet-base stagnates (finite stored resources, no growth, no tech progress); growth
  requires imports, imports require open routes, so ambition itself creates targets. **And sealing is
  no longer perfect safety** — the home is mortal and findable; a pure never-leaver stays
  _unfound_ (sheds no avatar-shares) but pays for it in permanent stagnation.
- **Trade-suicide** [decided problem]: if trading exposes your core like raiding does, every
  economic act invites a wipe. **Solution [tentative]:** the **tiered** sealed/port-open/gate-open
  model — trading at **port-open** only risks goods, not the vault.
- **Offline-dodge exploit** [decided problem]: if you can instantly seal mid-raid (or alt-F4),
  raiding is impossible; if you can't close, you're trapped online. **Solution [tentative]:**
  **presence requirement + committed siege timer** — opt-in, synchronous, finite, and you
  cannot be jumped while offline.
- **Zerg pile-on** [decided problem]: if opening exposes you to everyone, a dogpile returns
  power to whoever has the most bodies. **Solution [tentative]:** **directional bilateral
  tunnels** — raids are contained duels/feuds.
- **Whale dominance / new-player farming** [tentative]: if an established megabase can reach
  down and grief small or new players at will (the ARK mega-tribe problem), retention dies.
  **Solution [tentative]:** uphill-only now lives at **Rift visibility** — a whale
  can't see or enter a small player's quiet Rifts, so can't harvest the **shares** that locate their
  home; the small player is reached only if _they_ choose to crash up. Obscurity + uphill
  visibility replace the old home-dial wall ('s potential predicate is retired).
- **Doorstep kill-box** [decided problem]: if a defender can wall the aperture so raiders breach
  into a sealed box (the Rust/ARK "honeycomb the raid entrance" meta), raiding is nullified and
  the turtle returns. **Solution [decided]:** an active Gate projects a tier-scaled protective
  **dome** that destroys manual builds (not natural terrain), so it can never be walled in —
  fortify the perimeter at range, never the doorstep.

### Mask Energy & Survival — [MASK-1: mask survival layer]

[decided] **mask energy** is the survival/life-support layer, and the **Gate is its energy
source**: the player wears a life-support **mask** powered by a field drawn from the Gate (framed
as "magnetic"/projected energy — exact physics **[open]**). This creates a **natural raider timer**
(a raider away from a sustaining field is on a depleting budget and must be efficient) and an
**asymmetric home-soil advantage** (inside your own field you are topped up and strong; deep in
enemy territory you are on a clock). It replaces the ARK/Rust survival grind — tied to the core
Gate fantasy rather than a hunger/thirst chore. **Crucially, mask energy must be split into two
independent numbers** (see next section) or world-expansion will erode raids. **[open] (do not
invent specifics):** what the mask physically is, exact drain curve, recharge rules.

### The Sustaining Field (soft world border) — [MASK-2: sustaining field / soft border]

[tentative] Each active Gate projects a **sustaining field**. Inside it the mask tops up, giving
effectively unlimited time on your own soil. Moving outward, the field **weakens on a gradient**, so
the mask drains faster the farther you go. This is the game's **soft world border**: not a wall at
radius X, but a **cost curve** / risk gradient, so the dangerous fringe becomes a _place_, not an
edge. The fringe is exactly where **Rifts** naturally belong ("grab the loot and get back before the
mask dies"). It replaces a hard map boundary. **Dev/lore lever:** field radius is the
**authority-tunable expansion knob** — the studio grows the playable world over time by having the
authority "expand the field" (see Authority as Patch Channel). **Technical fit:** a field bubble per
_active_ Gate matches the lazy-load, cost-scales-with-online-players architecture — only the area
around online Gates needs simulating, so the soft border and the cheap-server design are the same
line. **[open]:** gradient vs. hard radius (leaning gradient); whether field radius is purely
authority-set or also a capped per-player upgrade.

### Mask: Field Radius vs. Away Reserve (the two-number split) — CRITICAL — [MASK-3: two-number split]

[tentative, load-bearing] Mask energy governs two things that pull in opposite directions, and they
**must be separate numbers**:

- **Field radius** — how far from your Gate you can roam/build; the soft border; the
  **dev-expandable** lever.
- **Away reserve** — how long the mask lasts once you leave a sustaining field (i.e. when raiding);
  the **raider timer**.
  If these are the _same_ stat, every "expand the field 10%" patch also lets raiders camp enemy soil
  10% longer, eroding raid tension with every content update. Keeping them separate lets the map
  expand for years **without touching the raid clock**. Away reserve is **fixed by the attacker's
  home Gate power** and **cannot be pumped** by carried cores or planted beacons — the
  only way to raid longer is to grow your own base.

### Mask-at-Zero Outcome (asymmetric by location)

[tentative] What happens when the mask empties is **asymmetric**, which solves two problems at once:

- **On your own soil / inside a field:** _soft_ failure — emergency recall to the Gate, no loss. A
  leash, not a punishment (preserves the anti-decay thesis).
- **In hostile territory during a raid:** _hard_ failure — you go down, vulnerable,
  finishable/lootable. The real risk that stops raiding from being a free teleport home.
  Alternative considered: uniform death everywhere (simpler, but closer to the ARK feeling being designed out). Asymmetric preferred. **[open]:** confirm; define the recall and downed states.

### Other Survival Meters

[open] **Undecided.** Whether Gaters has hunger, thirst, temperature, stamina, or health-regen
systems beyond **mask energy** is not established. The anti-"chore" intent suggests minimal
survival nagging, but nothing is decided.

### Building System — [BUILD-1: no decay/upkeep · BUILD-2: build depth (open)]

[tentative] Building on your **planet-base** is assumed to exist (building was a "loved" pillar
from ARK/Valheim) and happens in the safe **sealed** state. **Crucially, there is no persistent
base decay/upkeep [decided]** — that is the central thing being designed out. **[open]:**
build system depth, materials, structure types, whether defenses are buildable, grid vs.
free-form, and whether anything analogous to taming exists.

### Combat System — [COMBAT-1: combat model · COMBAT-2: muster cap]

[tentative] "Real PvP combat" is a required ingredient of the first playable. The model, resolved
enough to greybox:

- **Perspective & feel:** **third-person, gear-and-positioning** combat (Valheim-achievable solo),
  not ability-twitch. The skill ceiling lives in **loadout and positioning**, which keeps the
  home-soil edge tunable as a _number_, not a reflex contest.
- **The mask _is_ the combat resource — no separate health bar (MASK-1/3).** On your own soil the
  mask tops up (effectively full durability + home advantage); on hostile soil your **away reserve
  is both your raid clock and your effective durability** — taking fire drains it faster, so a
  raider's health and clock are one depleting number. The fair-but-beatable home edge falls
  straight out of the **field gradient** (MASK-2) the design already tunes — no bolted-on "home
  buff" stat.
- **The aperture is a two-way fire-lane.** Weapons fire and momentum pass through a live Gate
  (gate-physics): the defender shoots _into_ the tunnel before the raider commits; the raider lays
  covering fire _up_ it. The **dome** guarantees a fair landing (no spawn-camp), but
  stepping past it puts the raider in the defender's prepared fire-lanes and natural cover.
- **Muster cap — [COMBAT-2]** (resolves open-question on solo-vs-mustered; flips open knob 1): the dome admits only **K combat-effective defenders at the aperture lip**, K scaling with
  the **attacker's throat power** (the dialer sizes the dome). A skilled solo can beat a
  **mustered** defence _at the breach_ — but only there: the remaining defenders hold the perimeter
  at range, and the attacker's own away-reserve clock bounds whether they grab-and-go
  before it closes. It is an anti-zerg lever at the one chokepoint the raid is decided (rhymes with TUNNEL-1), **not** a promise the solo wins the open field.
- **Two venues, one model:** the same combat serves the **home raid** (at the aperture, under the clock) and the **standalone arena** (hubs, Open Rifts, space gates — quick in/out, no territory required).
- **Precursor-tech-hybrid spine [tentative]** (resolves the theme/genre fork): combat verbs climb
  the Standard→Heavy→Supergate ladder — bow/melee + scavenged ballistics at the low-tech start
  (makes low-poly art _canon, not budget_), scavenged Builder tech (the mask, energy weapons, the driven dome) mid-game, fleet/ship combat at the top (the deferred **two-phase** orbital→infantry structure). The through-line at every tier: you fight on a Builder-tech life-support budget you
  don't fully understand.

**[open]:** exact TTK curve, the K(throat) values, gear tiers (data, tuned later).

### Trade System — [TRADE-1: routes + market]

[tentative] Trade happens at **port-open**, risking **cargo/convoys** but not the vault. Distant
trade can be routed cheaply through **hub worlds** (paying a toll and accepting exposure) or
**dialed direct** (expensive, private). **Access lists** can permit specific partners; tolls
and the authority **tithe** are economic sinks. **[open]:** what is actually traded (goods,
resource types, currency), pricing/market mechanics, contracts, and whether trade is
player-to-player only or includes NPC markets in the safe core.

### Raiding System — Two Distinct Modes

[tentative] The design keeps **two raiding modes cleanly separate**:

- **Home raid** [tentative]: **direct Gate, bilateral feud** — contained, opt-in, two-way,
  governed by presence + siege timer. The babysitting fix and anti-zerg rule hold here.
- **Hub raid / hub warzone** [tentative]: **opt-in, chaotic, many-vs-many** — you choose to
  walk into it.
  **Guardrail [decided as a principle]:** a hub may lower your _travel cost_ to a target's
  doorstep but must **never** lower the _target's defenses_, and **finding is the gate** — you raid a
  home only by assembling its coordinate, never via a hub shortcut into an _unfound_ home.
  The destination always obeys the raid clock and the dome.

### Raid Value Floor (anti-husk) — [LOOT-1]

[tentative] A breached target must always be worth the trip, or raiding dies on dry holes (the
Raider's anti-husk need). The floor is a **server-minted, tier-scaled bounty** — _not_ a skim of
the owner's actual stockpile — guaranteed when a raid reaches the target's Gate control device, and
sized so the yield at any tier **≥ the core cost to dial that tier** (the single inequality that
mathematically kills the husk). Because it is minted from the defender's _Gate tier_ (visible in the
signature before you commit, WAKE-1), not their inventory, the owner has **nothing to "spend before
it's stolen"** for a _Rift_ run. (A **home raid** is different: it can take build,
stock, and the civilization itself, but **never the player's existence**.) Layered on top, also
non-existential: the **coordinate DB** stays stealable on a **breach** (COORD-1, the map as loot),
and every looted avatar yields a **coordinate-share** — the intel that lets a raider
assemble and later hit the owner's home. The
earlier "loot the accrued Authority tithe-debt" idea is **dropped** — it created a settle-early tax
and contradicts the Authority guaranteeing your charter. The owner logs back to an intact base
having lost the fuel of one fight, never their build, stock, or existence.

### Exposed Holdings & Offline Siphon — [HOLD-1]

[tentative] The one place the design admits **offline raiding — to a bounded degree** —
without reviving the babysitting tax. The home itself is now **mortal and findable** too
— no longer inviolable — but a holding is the _always-on, no-search-required_ target, contestable
without first assembling a coordinate. A holding is a new object class, the
**holding**: an asset planted _outside_ your home sustaining field — forward nodes, claimed resource
taps, off-site caches; **space-gate holdings** and **captured hubs** are the top-tier cases. A
holding has no woken-sealed Gate of its own, so there is **nothing to seal** — the same reason space
gates "cannot be turtled."

- **Always contestable, never on a schedule.** A holding is a **flow-tap** any **uphill** raider can
  dial (with potential computed from the _holding's own value_ so a whale can't farm a newbie's node). There is **no published vulnerability window** — that lever was rejected in (it pulls players onto the network's clock and is exploitable by timezone capture, zerg broadcast, and wait-out). Offline defence is a **frozen snapshot** of the automated defences the
  owner left — the attacker fights AI, not the absent owner.
- **Winning denies; it does not bleed.** Taking a holding flips it **dormant/neutral** (the owner stops getting the trickle) and grants the attacker a **one-time, capped buffer** grab — **never an
  ongoing stream diverted to the attacker.** This is the load-bearing choice: there is no "a rival
  is being paid by my node while I sleep" meter, so there is **no reclaim obligation** — the owner
  re-taps it next time they play, as normal play, not a chore. Bounded loss is _structural_: at most
  a buffer of output you weren't collecting, never structures or existence.
- **What it serves (three archetype gaps, one layer):** _offline raiding to some degree_ = siphon an
  absent player's **exposed output**, never fight them at it, never touch their home. The
  **Builder's** "build exposed for a payoff" = plant forward nodes for reach/output the home field
  can't reach. The **Conqueror's** "take **and** hold" routes to the **hubs and space gates** that
  are already always-live and outside the no-obligation line (HUB-1) — capture there is a **registry
  charter flip** defended by presence in an opt-in arena, **not** a decaying home-side claim (which would be BUILD-1 decay in disguise).
- **Lore:** the Authority warrants the world under your **own** Gate, not land-grabs past
  the frontier; a holding sits outside your sustaining field — mask-exposed on borrowed ground, the
  same physics that puts a deep raider on a clock.
- **Cost fit:** a holding is a DB row at rest; it spins a Contested instance only when an attacker is
  online and pays to dial it, so load stays ∝ active raiders, not the count of holdings.

**[open]:** `buffer_cap` and dormancy/re-tap values (data); whether a holding ever gets a
present-defender fight (leaning no — fights live at hubs / Open Rifts / space gates).

### Hub Worlds — [HUB-1: hub worlds]

[tentative] Neutral, **lawless commerce crossroads**: **trade routing nodes** for
reaching distant partners affordably (cheap-distance hop-routing vs. an expensive direct dial).
You **cannot turtle** at a hub (no woken-sealed Gate of its own), so it is always live and exposed —
but the **designed PvP contest now lives in Rifts**, not here; PvP at a hub is incidental
(over cargo), not an arena. **Optionally capturable [tentative]:** hold the relay/core and you can
**tax** traffic, **throttle** (allies cheap, enemies expensive or gate-locked), and gain **intel** on
who moves through — capture-to-tax is the Conqueror's take-and-hold prize (a registry charter flip,
HOLD-1). A hub is therefore a market _and_ a chokepoint. The old "hub as PvP brawl arena / accepted
zerg pile-on" framing is **retired** — that contest folds into Rifts and space gates.

### Rifts (spontaneous events — the dopamine engine) — [RIFT-1: rifts]

[tentative] **Rifts** are the _unstable_ half of the cosmology: spontaneous wormholes that open on
their own, can't be controlled, and **collapse on a countdown**. Design intent (inspired by Solo
Leveling's gates) is **opportunity, not invasion** — and this is the line that must not bend: a Rift
opens **next to** your Gate, as a _separate_ hole in your surroundings, **never through your sealed
Gate and never as a backdoor for enemy players into a sealed home.** A Rift opens onto **content**
(a PvE dungeon-pocket, a derelict, a rich anomaly, a monster spill) that you _choose_ to enter;
ignoring it costs nothing but the loot. Each Rift telegraphs a **rank + countdown** ("Rank-5 Rift
stabilizing, 18:42" — the countdown is the instability decaying). Random rank + scaling loot + time
limit = variable-ratio reward and anticipation (the "what do I get this time" pull); a rare top-rank
Rift is a server-wide event. **Healthy-vs-toxic line (the project's own thesis):** Rifts must make
players _excited to log in and see what's open_, never _obligated to log in or lose something_. The
moment an uncleared Rift punishes an offline player, it has reinvented decay. Two flavors, both
opt-in:

- **Sealed Rift (PvE)** [tentative]: solo/tribe-only, no outsiders; cleared under the
  **away-reserve leash**; baseline loot; **generates no shares**. The everyday loop and the answer to
  "what do I actually _do_ while building?"
- **Open Rift (PvPvE extraction)** [tentative]: the contact surface that makes hidden
  homes findable. Visible to players **≤ the opener's Potential** — bigger ops draw
  **Scavengers**; **one-way, return-to-home-only** (nobody can tail you home); **extract** (survive to the door, keep loot) vs. **recall** (emergency, drop the avatar); player corpses yield
  **coordinate-shares**. Better loot is the risk premium; the genre is extraction
  (Tarkov / Hunt).
  **Optional downside for ignoring a big Rift** [open]: a **PvE incursion** — monsters spill onto your
  surface and you fight them — **never** an auto-opened door for raiders, and **only while you are
  present** (same presence rule), so logging off can never cost your base. Name: **Rift** for the
  unstable hole, kept distinct from **Gate** (alt: "Conjunction," fitting the "connection made" phrasing). **[open]:** rank scale (E–S vs. 1–10), surface vs. orbit vs. both, the ignore-it
  downside, and how Rift loot relates to raid loot so neither trivializes the other.

### Rifts → New Gates (finite but replenishing)

[tentative, resolves a flagged conflict] If an unstable **Rift** can be **stabilized**, that is
where new Gates come from: you still never _manufacture_ a Gate, you **tame** a naturally-occurring
unstable wormhole into a permanent one. This reconciles "Gates are finite and unbuildable" with the
need for more Gates as the playerbase grows — **finite, but slowly replenishing, born from Rifts.**
**New sub-tension to resolve:** stabilization is _player-driven_ expansion, which rubs against
_authority-controlled_ expansion and Authority-as-Patch-Channel. Reconciliation options: gate
stabilization behind **authority tech / a license** (new Gates appear on the studio's schedule,
framed as the Directorate learning to stabilize the network), or simply keep stabilization **rare
and hard**. Needs a call (see Conflicts).

### Taming (narrow) — [TAME-1]

[tentative] A **narrow yes** on the tamer fork: include **taming**, decline deep ARK-style
**breeding/genetics**. Multi-generation raise-cycles and imprint timers _are_ a babysitting chore by
construction — the exact pain BUILD-1 remove — so the deep genome grind is rejected. The
narrow loop rides existing systems:

- **Source:** creatures come **only from Rifts** (RIFT-1) — a monster-spill rolls a `tameable` tag;
  Rift rank scales rarity/power on the variable-reward curve already there. No new spawn system.
- **Taming is a Rift-clear objective, not a feed-and-wait timer** — subdue/capture under the
  **away-reserve leash**, an _event_, not a maintenance state. Kills the babysitting shape at the root.
- **The genome is a home-anchored record** (the safe lineage) — a stat-block delta on your home
  planet, safe while the home stays **unfound**. One more delta in seed-plus-deltas, no new persistence.
- **The living animal is the exposed wager** — to use a creature off your soil you instantiate a
  living unit (costs a core, a sink); it can die and be looted, but the **registry entry survives**,
  so you lose the _deployment_, never the _lineage_. Reuses the cargo/raid-exposure model.
- **The surviving "breeding" is shallow and lossless** — combine two registry entries to roll a
  blended/mutated entry (a tag re-roll, a safe-soil crafting action, no timer). Depth = chasing rare
  Rift-rank tags and good rolls, not a husbandry calendar.

**[open]:** tameable roll rate per Rift rank, blend odds, instantiation core-cost; or **defer the
whole fork** (an honest lazy answer for the greybox — nothing breaks).

### Coordinate Discovery & the Coordinate Database — [COORD-1: coordinate DB · COORD-2: blind-dial]

[tentative] The concrete form of "coordinates are intel." **Coordinates are information, not
Gates.** You never build a new Gate; you **dial from the Gate you already own**, and what you save
is the **coordinate** — a re-dialable address logged in your homebase **coordinate database**.
"Locking into a planet to revisit later" means **re-dialing a saved bookmark**, not constructing a
second gate (so the finite/unbuildable rule holds). **Guardrail:** a bookmark removes the _discovery_
cost, never the _travel_ cost — re-dialing a known coordinate still burns fuel scaled by distance,
keeping the hub/distance economy intact. **The loop:** dial out or send a probe → reach a planet →
its coordinates log to your DB → some planets hold resources beyond your current tech, so the
bookmark is a **deferred target** → later, with the tech or tier, return to **claim** it (if still
empty) or extract what you couldn't before. Payoffs:

- **Discovery feeds PvP for free** [tentative]: a rich planet found early won't stay empty — either
  you claim it later or someone else does, turning your old bookmark into a **scouted raid target**
  you mapped before anyone lived there. Early exploration becomes a late-game hit list (rides the three-state model: bookmarked-empty = claim opportunity; bookmarked-occupied = raid).
- **The coordinate DB is raidable** [tentative]: breaching a homebase can let an attacker **steal
  the map** — scouted planets and routes — loot beyond materials, and a reason to protect what
  you've charted.
- **Coordinates are also assembled from avatar-shares** : every fielded avatar carries a
  share of its tribe's home coordinate; loot enough corpses (or **buy** the missing shares in the Safe Core market) to locate and raid a home. Coordinates are **epoch-bound and re-keyable** —
  re-keying voids collected shares (resolves the old "leaked forever" worry).
- **Blind exploration-dialing as a slot machine** [tentative]: re-dialing _known_ coordinates is
  precise, but dialing into the _unknown_ for a fuel cost yields a **random planet of random
  richness and danger** — the variable-ratio hook applied to exploration, and a natural fit for the
  instability cosmology (dialing blind into a half-formed connection).
  **[open]:** whether discovery is travel-only or also via probes / buying / stealing; whether blind
  exploration-dialing exists (leaning yes); whether the coordinate DB is stealable on a raid (leaning yes).

### Authority as Patch Channel (a principle worth locking)

[tentative → decide] The **central authority is the in-world voice for developer changes.** Field
expansions, new world tiers, new world types, even buffs and nerfs ship as **"Directorate
bulletins"**, so patch notes become lore. The world literally grows over the game's life (the same
unlock ladder) with a **diegetic reason**, and a balance change never has to read as a balance
change — it is always the authority adjusting the network. "Field harmonics improved 10%" is a
patch; "we don't know why the Rifts are increasing" is the plot. Worth locking early because it
shapes how every future update is written. (Flagged tentative only because it has not been formally
signed off; it is a strong leaning.)

### Gate Uses Catalogue (mechanic option space) — [USE-1: no caller ID / transponder codes]

A research-derived menu of things Gates can _do_, framed as design options. The five tagged
"sharpest" below are the current leaning for inclusion **[tentative]**; the rest are catalogued
**[open]** options not yet integrated. Each could be its own page.

**Sharpest / leaning-in [tentative]:**

- **No caller ID + transponder codes**: an incoming raid does not reveal _who_ it is until it
  commits; friendly trade requires an accepted code; spoofing codes is an infiltration play.
  The cleanest way to separate "open for business" from "open to attack."
- **Occupy-to-lock / jamming**: holding a Gate open (or keeping a line busy) can lock a target
  out of the network or refuse connections — a denial/siege tactic that doesn't touch the
  safe-by-default home rule.
- **Buffer wipe of in-transit forces**: troops "in transit" sit in a vulnerable buffer; cutting
  the connection mid-raid can wipe an in-flight strike force — a genuine raid-commitment
  mechanic.
- **Stealable Gate control device**: damage or steal a planet's control console and it can
  receive but barely dial out — a "stranded, raidable, can't retaliate" consequence state that
  is _not_ permanent base loss.
- **Gate-as-bomb (overload)**: overloading a Gate yields a planet-scale explosion — a rare,
  costly scorched-earth / siege-ender, server-event tier.

**Catalogued, not yet integrated [open]:**

- **Dial-into-a-star**: route a tunnel through a sun to sabotage a target system — an exotic
  super-weapon gated behind disabling safeties.
- **Safeties as a tech resource**: the Builders' safety protocols refuse dangerous jumps;
  researching/disabling them unlocks weapon-grade uses (risk/reward; manual dial bypasses safeties).
- **Comms through a sealed Gate**: radio/data pass even when the aperture is shut — diplomacy,
  threats, ransom, alliance pings without dropping exposure.
- **Subspace relay / data network**: Gates carry data; controlling Gates controls information.
- **Recon probes**: send an unmanned probe through to scout before committing a raid.
- **Gate bridges (chained call-forwarding)**: chains of Gates forward travelers hop-to-hop for
  far less power than one long jump — the in-world basis for the cheap-distance **hub**.
- **Harvested / relocatable Gates**: found (often orbital) Gates can be claimed and moved to
  build new routes (see Conflict with "Gates are unbuildable").
- **Reprogram-the-route**: whoever controls a relay/hub can reroute, misdirect, or trap traffic.
- **Network self-update as a worm vector**: the auto-sync between neighbor Gates can propagate a
  cyber-weapon across the Gate Network.
- **Proximity priority conflicts**: placing a competing Gate near a rival's can override/suppress
  theirs — territorial jamming by proximity.
- **Time/exotic physics**: solar-flare time travel; black-hole time dilation bleeding through an
  un-severable connection; the ~38-minute wormhole cap (already borrowed as the siege ceiling).
- **EVE-proven economy/access layer**: access lists (incl. asymmetric one-way), tolls, fuel
  scaling with mass × distance, a **hard mass cap** that forces the biggest fleets onto rare
  Supergates, plantable **beacons** as landing sites, system-wide **jammers** for area denial,
  and **Gates being killable / needing defense**.

### Progression & Tech — [PROG-1: supergate · PROG-2: sealed soft ceiling]

[tentative] Two in-world drivers gate the same tier ladder: **rising charter clearance** (you
prove yourself) and **more of the Gate Network waking**. Unlock ladder: small surface worlds →
larger/richer worlds → orbital/space Gates → relay/hub Gates → megastructure/Supergates.
[decided] **Gate tiers are driven, not built** : you never enlarge a Gate — the ring
is fixed and the **aperture** is a runtime variable set by exotic matter channeled, capped per
ring by Ford–Roman. **Standard** (strike team / cargo) → **Heavy** (vehicles, light armor, bulk)
= one ring fed harder; **Supergate** (fleets, capital ships, siege) = multiple **found segments**
reassembled into a megastructure, impossible to hide, drains a full core. Overdriving one ring
past its ceiling destabilises it into a Rift. [tentative] A tech path around **Gate safeties**
unlocks weapon-grade uses. [decided] **No base decay** anywhere in progression. **[open]:** the
actual tech tree, research mechanics, and gear tiers. (Character/skill progression model resolved —
two-layer mind/avatar XP, XP-1 /; curve values still open.)

[tentative] **Sealed soft ceiling — [PROG-2]** (resolves the homesteader fork). A peaceful
homesteader can progress **forever, fully sealed**, up to a **soft ceiling**, then plateaus — never
decays, never punished for not opening (BUILD-1). Their safety is **being unfound** (a never-leaver
sheds no avatar-shares), not an inviolable wall. Mechanism: one boolean per tech node,
`sealed-reachable`. Solo-Rift loot + own-biome supply climb the lower-to-mid ladder; the **frontier
tiers** (orbital/space, hub/relay, Supergate) flip to `false`, because their inputs are
field-harmonic goods you can't self-supply sealed (REGION-1) or world-types you must dial to.
Stagnation is thereby reframed as a **ceiling, not a leak** — hitting it reads as "I finished the
homestead game," never a bleeding meter (which would be the upkeep tax the stagnation-vs-obligation
conflict warns of). Opening stays meaningful because the frontier is real content gated behind it,
not a stat bump. The sealed player is also the **cheapest to host** (solo-occupied only, never a
Contested instance), so serving this style generously is an asset, not a cost.

### Economy — [ECON-1: stagnation · ECON-2: activation cost · ECON-3: tithe/tolls]

[tentative] Drivers and sinks:

- **Stagnation pressure** [tentative]: sealed planet-bases don't grow; growth needs imports —
  the engine that makes players open up. (See the critical tension in Conflicts: too much pressure recreates an obligation.)
- **Power cores** [tentative]: scarce, scavenged Gate fuel; the primary consumable behind all
  Gate activation.
- **Activation cost** [tentative]: proposed rule **activation cost = gate tier × distance band**.
  Distance bands: **local** (a trickle / power cell) → **regional** (partial core) → **cross-map**
  (a full, possibly unrecoverable core). A Supergate across the map is intended to be the single
  most expensive act in the game.
- **Direct-dial vs. hub routing** [tentative]: direct = high fuel, private, no third-party
  exposure; hub = low fuel, plus a toll, plus exposure in a live PvP crossroads.
- **Tolls** [tentative]: hub controllers tax traffic.
- **Authority tithe** [tentative]: the cut owed for your charter — a baseline sink.
- **Loot** [decided as a pillar]: raiding for goods is a core reward.
- **Fuel scales with mass × distance + hard mass cap** [tentative, EVE-inspired].
  **[open]:** concrete currencies, resource taxonomy, crafting inputs, sink/faucet balance, and
  whether there's a unified currency or barter.

### Decided / Proposed Formulas & Numbers (consolidated)

- **activation cost = gate tier × distance band × tier-gap multiplier** — [tentative]
  (the tier-gap term prices punching _up_; values **[open]**).
- **Distance bands**: local / regional / cross-map ≈ power cell / partial core / full core —
  [tentative].
- **Gate tiers & payloads**: Standard = strike team + cargo; Heavy = vehicles + light armor +
  bulk; Supergate = fleets + capital ships + siege — [tentative].
- **Raid clock**: a **curve of attacker home-Gate power**, capped at the ~38-min wormhole
  ceiling; un-pumpable — [tentative].
- **Mass cap on convenience Gates** forcing big fleets onto Supergates — [tentative].
- Everything else numeric (mask-energy drain rates, prices, TTK, gather rates) — **[open]**.

### Multiplayer / Networking Architecture

[decided] **Authority model: server-authoritative always** — one authority owns each planet at
all times, so there is no authority transition to reconcile or exploit. The cost cap comes from
the **presence requirement**, not from offloading simulation: a base is live only while its owner
is online — **with one bounded exception** : a home whose coordinate has been assembled can
be raided offline, spinning a Contested instance without the owner. This is **rare** by design (it
requires finding them first), so cost still scales ≈ with active players. Solo/sealed building has no adversary, so it **ticks cheaply**; full-rate simulation is
spent only on **contested** planets. [decided] **Three-state planet model:** **empty** (just a
database row) → **solo-occupied** (a cheap server-side tick) → **contested** (a full
server-authoritative instance spun up). [decided] **Seed-plus-deltas storage:** planets are
**lazy-loaded on demand**, so **server cost is proportional to online players**, not to world
size. [decided] **Gate-based instancing dramatically reduces infrastructure cost** vs. a
persistent shared world — a foundational architectural advantage. **[open]:** matchmaking and how
bilateral tunnels are instanced, hosting model.

### Tech Stack

[decided] **Unity with C#** (matches the designer's existing language comfort). [decided]
**Native-first Unity build, with a web export as a demo/playtest funnel.** [decided] A
**`PlanetGenerator` abstraction from day one** so procedural generation can be swapped in later
as a single-class change. [decided] The **Gate mechanic is the mask-energy source** (engine-level
tie between Gate energy, survivable range, the raider timer, and home-soil advantage). **[open]:**
netcode library/transport, hosting model, persistence backend specifics.

### Aesthetic / Art Direction

[tentative] Touchstones: **Minecraft**, **Valheim**, **No Man's Sky**. **NMS is the
_aspirational_ visual target**; the Gate structure deliberately **sidesteps NMS's hardest
engineering problem** (seamless space-to-surface flight is replaced by **Gate loads**).
**Valheim** represents the _achievable_ Unity aesthetic. [decided as a principle] Distinguish
**visual style (cheap, achievable solo)** from **engine tech of reference games (NMS's custom
engine and seamless flight — not achievable solo).** [tentative] Practical path to the NMS look
solo: **low-poly, flat-shaded 3D with strong post-processing.** [decided] **Art direction is
deferred until after greybox validation.** **[open]:** the final art identity (NMS-lush vs.
low-poly-stylized), palette, UI style.

### Validation Plan (greybox-before-art)

[decided] The biggest risk is **not technical** — it is whether **opt-in, presence-gated raiding
is actually fun**, i.e. **will players actually open their Gates?** Plan: build a **greybox proof
of concept** before any art. [tentative] **Minimum viable slice:** sealed/open states, a
bilateral raid tunnel, loot worth taking, economic pressure against permanent sealing, and real
PvP combat. [tentative] **Ground Gates only** for the initial concept proof; **space Gates**
(the two-phase fleet-then-infantry structure) come later. [decided as a principle] **Design for
success enabling expansion, not requiring it** — a working small version should be a complete
thing, not a broken large one.

---

# CONFLICTS / TENSIONS TO RESOLVE

Flagged rather than smoothed.

1. ~~**Raid-clock governance — siege timer vs. away reserve.**~~ Resolved: one
   clock — the attacker's **away reserve**, fixed by the attacker's **home Gate power** at dial time
   and **not** extendable by carried cores or beacons. The Gate stays locked open until that clock
   runs out (offline-dodge still closed), so no separate defender-set siege timer is needed. Raid
   length is a curve of Gate power capped at the ~38-min wormhole ceiling.
2. ~~**Siege-timer number.**~~ Resolved: not one value but a **curve of Gate
   power**, capped by the ~38-min wormhole ceiling; the mid-tier case serves as target session length.
3. **"Gates are unbuildable" vs. getting more Gates over time.** Reconcile **manufacturing** a Gate
   (forbidden) with the several ways the design now adds reach: **relocating found Gates /
   reassembling megastructures from found segments**, **stabilizing Rifts into new Gates**
   (finite-but-replenishing), and **saving coordinates** (re-dialable bookmarks, which are _information, not new Gates_ — this one is already resolved). State the umbrella rule: you never
   fabricate a Gate from nothing; you find, relocate, reassemble, or tame one, and you re-dial saved
   coordinates. **Open sub-tension:** Rift-stabilization is _player-driven_ expansion vs. the
   _authority-controlled_ expansion implied by Authority-as-Patch-Channel — see conflict 10.
   **Resolved (size axis):** — the ring is fixed, the aperture is driven; you exceed one
   ring only by reassembling found segments, so the umbrella rule (never fabricate) holds.
4. **Economic stagnation pressure vs. the "no obligation / log in when you want" promise.** This
   is the project's central balancing act: stagnation must be strong enough to push players to
   open up, but if it's too strong it becomes a _soft upkeep tax_ — recreating the very obligation
   the game exists to remove. Where is the line?
5. **Raider anonymity ("no caller ID") vs. the central registry / claim guarantee.** The authority
   knows who holds what and guarantees claims; raids are supposed to be anonymous until committed.
   Does the registry leak attacker identity, and how do these coexist?
6. ~~**Hub zerg problem.**~~ Resolved: hubs become **lawless commerce/routing**
   (capturable for tolls), and the open many-vs-many PvP contest moves to **Rifts** (visibility by Potential) **and space gates** — so the zerg/pile-on dynamic is no longer hosted _at the
   hub_. Hub PvP is now incidental (over cargo) or a capture objective, not a designed brawl arena.
7. **"Sealed = isolated/stagnant" vs. "comms pass through a sealed Gate."** Communication is meant
   to pass while sealed, but the economy treats sealed as cut off. Confirm the intended split
   (information yes; goods/energy/people no) so "isolated" isn't misread as total silence.
8. **NMS-aspirational visuals vs. low-poly-achievable-solo.** Two different target identities are
   both cited; the actual look is deferred and undecided, which leaves scope/feasibility ambiguous.
9. **Title "Gaters" — phonetics & searchability.** Reads as "gators"; homophones "gay-ters" /
   "gaiters"; SEO/searchability and community-harassment risks were raised and not resolved.
10. **Player-driven Rift-stabilization vs. authority-controlled (dev-paced) expansion.** Letting
    players stabilize Rifts into new Gates is satisfying but hands world-growth pacing to players,
    which conflicts with using the authority as the studio's patch/expansion channel. Decide:
    license/tech-gate stabilization (studio keeps the schedule) or accept player-paced Gate supply.
11. **Field radius and away reserve must stay decoupled.** The whole point of the two-number split:
    if a future "expand the field" patch ever also lengthens away reserve, world-expansion silently
    nerfs raid tension. A standing implementation rule, easy to violate by accident.

---

# BIGGEST OPEN DESIGN QUESTIONS

1. **Is the core fun real?** Will players actually open their Gates — i.e. does opt-in,
   presence-gated raiding produce the aggressor thrill without the defender tax? (The one greybox must answer.)
2. ~~**What governs the raid clock**~~ — decided : the attacker's away reserve,
   fixed by home-Gate power and un-pumpable; no separate siege timer.
3. **How hard should economic stagnation push** without becoming a disguised upkeep obligation?
4. **Which mystery truth** (outside signal / tripwire / authority lie) is canon?
5. ~~**What is the combat model**~~ — resolved (COMBAT-1/COMBAT-2): third-person
   gear-and-positioning; the mask _is_ the combat resource (away reserve = clock + durability);
   aperture as a two-way fire-lane; **muster cap** lets a solo win the breach (not the open field); precursor-tech-hybrid spine. Numbers (TTK, K-values) stay open.
6. **What is the final art identity** (NMS-lush vs. low-poly-stylized)?
7. **Are there survival meters beyond mask energy** — and how minimal should survival be?
8. **How do raider anonymity and a central claim-registry coexist?**
9. **Does the title "Gaters" survive** the phonetic/searchability/harassment concerns, and what
   is the in-world player term?
10. **Can Rifts be stabilized into Gates** — and is that player-driven or authority/tech-gated?
11. **Rift specifics:** rank scale (E–S vs. 1–10), spawn location (surface / orbit / both), the
    ignore-it downside (missable loot vs. PvE incursion), and Rift-loot vs. raid-loot balance.
12. **Mask/field specifics:** gradient vs. hard radius (leaning gradient); field radius as
    authority-only or a capped per-player upgrade; confirm the **asymmetric mask-at-zero** outcome.
13. **Lock "Authority as Patch Channel" as canon?** (Strong leaning; needs formal sign-off.)
14. ~~**Can coordinates go stale / be re-keyed?**~~ — resolved : coordinates are
    **epoch-bound and re-keyable**; re-keying voids collected avatar-shares. Re-key cost/cadence is
    the master knob for the online:offline raid ratio.

---

# PLAYER ARCHETYPES / EXPECTED PLAYSTYLES

The styles the game expects, drawn from ARK / Rust / Valheim and sorted by how the
balance thesis treats each. **Test every proposal against this set the same way it is
tested against the Four Design Traps and the balance thesis:** a decision that
silently starves a _served_ style, or revives a _denied_ one, is a blocking flag, not
a detail. Reference pull: **Rust** (raiding/offline-raiding, zergs, solo grinders,
road PvP, trader-bandits — the toxicity reacted to); **ARK** (taming/breeding,
mega-tribes farming newbies, elaborate bases, PvE homesteaders); **Valheim** (co-op,
building-for-joy with no decay, exploration, boss progression — its largest audience
never PvPs).

**Served — the loved pillars; each must have a satisfying standalone loop.**
The nested bullets are what the design _must_ provide for that loop to exist — check
every proposal against them. Each cites the **mechanic IDs** that deliver it (IDs are
tagged on the relevant section headers); **⚠ = gap** (no mechanic yet).

- **Raider** [decided — core fantasy]: open an uphill tunnel → breach into the protected
  **dome** → take loot / the coordinate DB / the control device → withdraw. The headline.
  - A way to **find targets** — active gates radiate a traceable signature (running = detectable, sealed = dark), plus stolen coordinate DBs, blind-dial, bought intel.
    (WAKE-1, COORD-1, COORD-2)
  - Targets that hold **real value** — not empty husks worth less than the away-reserve
    window. (LOOT-1)
  - A breach that's a **fight, not a wall** (dome + aperture combat) inside the clock.
    (GATE-1, SIEGE-1, COMBAT-1, COMBAT-2)
  - An **extraction path** home, and **anonymity** (raid without leaking your own coordinate). (TUNNEL-1, USE-1)
- **Builder** [tentative]: build freely on sealed soil; no decay; defence is the
  perimeter at range, never doorstep cheese.
  - **Safe sealed soil** — no decay; offline loss only if the home is **found** (rare),
    never from neglect. (BUILD-1, STATE-1)
  - **Build depth** worth the hours (structures, defences, production). (⚠ BUILD-2 open)
  - A reason _and_ a way to build **exposed** too (running/forward bases) that pays for
    the risk. (HOLD-1)
- **Explorer / scout** [tentative]: dial or probe unknown coordinates, map the
  network, blind-dial as a slot machine; early scouting becomes a late-game hit list.
  - **Unknown space** to probe/blind-dial, and **signatures to trace**. (COORD-2, WAKE-1)
  - **Coordinates as a valuable output** — tradeable, stealable intel (their "loot").
    (COORD-1)
  - **Compounding discovery** — early scouting becomes a hit-list others pay for. (COORD-1)
- **Trader / economist** [tentative]: port-open trade, hub routing, tolls, tithe;
  risks cargo, never the vault.
  - **Resource asymmetry** between worlds — a reason goods move. (REGION-1)
  - **Routes and a market** — hub routing, direct dial, mutual-registration partners /
    access lists. (HUB-1, TRADE-1, ECON-2)
  - **Risk on cargo, never the vault** (port-open). (STATE-1)
  - **Sinks** (tolls, tithe) that keep it flowing. (ECON-3)
- **Conqueror / warlord** [tentative]: the ARK "take a whole base"
  fantasy — take **and hold**, not just loot-and-leave.
  - **Exposed bases/territory worth capturing** (running outposts, forward fortresses)
    with real value, not husks. (HOLD-1, LOOT-1)
  - A way to **take _and_ hold** — capture transfers the asset; defending it is the game.
    Routed to the always-live capture targets (hubs for toll control, space gates) — capture is a
    registry charter flip held by presence, not a decaying home-side claim. (HOLD-1, HUB-1)
  - **Peer-or-greater targets only** (uphill) — conquest, not newbie-farming. (REACH-1)
  - **Takes the empire, never the _existence_** — conquest can now wipe a found home/civilization, but never the player (the body is on Earth).

**Redirected — valid, but funnelled to a designated arena, never onto sleeping homes:**

- **PvP brawler / roamer** [tentative]: combat for its own sake → **Open Rifts, space gates**
  (no longer hubs).
  - An **always-live arena** (Open/extraction Rifts, space gates) — no territory required to
    join.
  - **Combat fun on its own**, quick in/out. (COMBAT-1)
  - No need to own or defend anything to participate.
- **Clan / group / zerg** [tentative]: coordinated mass → **hub captures (toll control), big Rifts,
  and Supergate events**; cannot punch down (uphill Rift-visibility), so kept off
  solos and newbies.
  - **Large-scale objectives** that reward coordination (hub captures, Supergate events, top-rank Rifts). (HUB-1, PROG-1)
  - A hard **can't-punch-down** floor — kept off solos / newbies' existence. (REACH-1)
  - **Group-only payoffs** worth the org overhead. (PROG-1)

**Denied — the ARK/Rust pain the design exists to remove; scenarios must confirm each stays dead:**

- **Offline raider** [decided — reframed]: a sleeping player's **existence** stays denied (the body is on Earth). Their **home** is no longer immune — if its coordinate has been **found**
   it can be raided offline — but this is **rare** (you must find them; offline raids skew to the recently-active), not the ARK certainty. Their **exposed holdings** stay always-contestable
  without any search. (HOLD-1)
- **Megabase newbie-farmer** [decided — denied]: blocked by **obscurity + uphill Rift-visibility**
   — a whale can't see a small player's Rifts to harvest their shares (was 's home-dial wall, now relocated).
- **Turtle / babysitter** [decided — denied as a _dominant_ style]: sealing buys **obscurity, not
  immunity** (the home is mortal) and stays stagnant (Turtle equilibrium trap); the
  aperture cannot be cheese-fortified.

**Resolved forks (were undecided; now served at a defined scope):**

- **Tamer / breeder** [tentative — narrow yes]: include **taming** (Rift-sourced companions); decline deep ARK breeding/genetics (a babysitting chore by construction). A
  served style at the narrow scope. Needs:
  - **Creatures to chase** — Rift-rolled, rank-scaled; deep genetics declined. (TAME-1)
  - The **lineage/genome kept safe** (home-anchored registry), with living **stock as the
    exposed wager**. (TAME-1)
  - A breeding loop that doesn't become a babysitting chore — taming is an event,
    "breeding" a timer-free re-roll. (TAME-1, BUILD-1)
- **Peaceful homesteader** [tentative — served]: Valheim's largest audience never opens —
  build, explore, run solo PvE Rifts forever. **Resolved:** a permanently-sealed player is a
  first-class endgame, progressing to a **soft ceiling** (frontier tiers need opening), never
  punished for staying sealed. Needs:
  - A **permanent safe sealed loop** (build + Sealed Rifts) that **progresses up to a soft
    ceiling** — safe because a never-leaver stays **unfound** (sheds no shares), not by an
    inviolable wall. (PROG-2)
  - **Never punished** for not opening — opening stays optional, not mandatory. (PRES-1, BUILD-1)
