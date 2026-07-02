# World

The Gaters world bible in one file. Mechanics live in [[systems]]; the deep physics in
[[gate-physics|Gate Physics]]; unresolved questions in [[questions]].

## Premise

- A long-dormant, galaxy-spanning network of ancient **Gates** suddenly switches on (see [[#Timeline & the Wake|The Wake]]).
- Each player controls a single **planet-base** sitting on one [[systems#Gates|Gate]]. The [[#United Gate Coalition|United Gate Coalition]] charters independent **gaters** to find, claim, and hold Gate-worlds for a tithe. _Gaters_ is the game's name and the in-world term for a player.
- Players are **human** — people who found a Gate and stepped through (multi-species / post-human / alien rejected as scope with no payoff this phase).
- The central conflict is a **gold rush over finite, un-manufacturable Gate infrastructure**. Reach and wealth require **opening your Gate and venturing out** — and venturing is what exposes you: acting is how your home is eventually **found** and made raidable. [current call]

## Design thesis — "reach equals exposure"

- You act on the world only as far as you've **opened** to it; you choose when and toward whom.
- Your home's safety is **obscurity — being unfound — not an inviolable wall**. A **sealed** planet-base is safe _while hidden_ but **mortal**: venture enough and you are eventually found and raidable (see [[systems#Coordinates & obscurity|Coordinates & Obscurity]]).
- "MAD in space" — mutual vulnerability breeds deterrence, alliances, and "cover my Gate while I trade" pacts.
- This is the spine of the whole design; the survival/PvP model is [[systems#Mask energy|Mask Energy]] + obscurity. **Why:** it delivers the aggressor fantasy (raiding, looting) without the defender tax (babysitting, decay, offline loss) — the genre's persistent-shared-world-plus-offline-vulnerability is the thing it rejects. [current call]

## What you can and can't lose

- **Never lost:** your **existence** — you pilot a disposable avatar and respawn on death; only avatars die (mechanics in [[systems#Presence, respawn & XP|Presence & Respawn]]; what the avatar is, is open — see [[questions]], Avatar origin).
- **Mortal:** territory, holdings, your built civilization, the home itself — losable in a raid once you're found. You can lose your empire, never yourself.

## World types

The place-types, kept high-level. They double as the unlock ladder (see [[systems#Progression|Progression]]). [tentative]

- **Home planet-base** — your claimed world on a single Gate; the seat of building and solo PvE. Safe **while unfound**, not merely while sealed.
- **Surface worlds** — the starting tier (only short-range Gates have woken); larger and richer worlds as you progress. Each world's seed rolls a **biome tag set**, so one world's output is another's missing input — the base trade driver (REGION-1).
- **Hub worlds** — neutral, **lawless commerce crossroads**: trade routing + tolls, **capturable for tax control**; the open PvP contest lives on contested frontier worlds, not here. See [[systems#Hub worlds|Hub Worlds]].
- **The Safe Core** — the Coalition-policed market and new-player spawn; safe commerce, always reachable.
- **The Frontier** — the unclaimed worlds you reach by **dialing out** ([[systems#Gates|Gates]], FRONTIER-1); the PvE/PvPvE contact surface where "reach equals exposure" rules. A frontier world is a database row until visited, a **solo PvE** tick when you're there alone, a **contested** instance when others arrive. Its biome tag set (REGION-1) plus monster-spill ([[systems#Taming|Taming]], TAME-1) and the husk bases of **dead houses** ([[systems#Potential|HOUSE-2]]) give it its flavour; a world can seat several Gates (FRONTIER-3); claim one to make it yours.
- **Later (wants to have)** — orbital / space Gates, hub/relay Gates, megastructure / Supergate sites: see [[questions]], Roadmap.
- Open: the network's true extent (one system / many / galaxies).

## The Builders

The precursor civilization that made the Gates — **unknown by design**. [current call]

- They built the Gate network and then vanished. **Nothing else is established.**
- Who they were and why they built it is hidden from everyone, including the Coalition.
- Their engineering encodes deliberate conceits we _do_ see: Gates **woke sealed**, **fail closed** without an operator, and project an un-wallable [[systems#Combat|dome]]. One principle — _the Gate guards what's behind it unless actively driven open_ — generates all three.
- Their ignorance is deliberate — the long-game **mystery engine**. The Wake and the rising instability point back to them; the hidden truth of _what_ woke the Gates is the central mystery. Candidate truths tracked in [[questions]]. [tentative]
- A standing hook: _why_ did the Builders forbid walling a Gate (the dome)? A Gate that can never be entombed can always be reached _through_ — left unanswered per Builders-unknown.
- **Do not invent** their history, names, appearance, or motives. Resolve only when the mystery payoff requires it.

## United Gate Coalition

The network's **clearinghouse** — not a government but an **offhand broker**. It doesn't run the Gates (the Builders' machine does) or rule the frontier; it lists which Gates can be reached, brokers charters to anyone who pays, and clips a cut. **Indifferent by construction** — it profits on transaction volume, not on who wins. [tentative]

### What it is

- Seated at the network's **central nexus** — a Builder megastructure of Gates that is the safe core, the Coalition's exchange floor, and the new-player spawn. Out in the network, Gates still dial **point-to-point** (a mesh), and lawless hubs sit in the frontier (a "mesh with a heart").
- When the Wake brought Earth's lane online, the Coalition — holding more network than it can claim alone — **brokered access for a tithe** to all comers on the same terms: solo prospectors, corporations, and houses alike ([[systems#Potential|HOUSE-1]]). It does not pick winners; it lists access and lets them compete.
- It **records charter standing** — a ledger entry, _not_ physical immunity: a found home is still raidable.
- It brokers **nothing past the frontier edge** — no listings, no standing, no cut; past that line "reach equals exposure" is the only law.

### Gate availability (the patch channel)

- **Demand-driven dialing (the frontier default):** a world is an empty database row until a player spends a core and supplies coordinates — dialing _is_ provisioning, so an instance exists only while someone pays to sustain it.
- **Exchange listings (the published core):** the Coalition **opens lanes for dialing** as the network stabilises and publishes them — a server-side allowlist the studio controls. It is the lever for staged rollout and content drops, delivered diegetically as **broker bulletins / new listings** ("broker as patch channel"). The Safe Core is always listed; new tiers ship as new listings.
- Invariant: listings govern reaching **others'/new** worlds, never **your own** home Gate — so the no-offline-loss-by-neglect guarantee is untouched.
- It **controls** its own **exchange floor** — the Safe Core, policed and safe for commerce. Its grip there is **total**; on the frontier it is **absent** (it brokers access, nothing more). That contrast — iron hub, lawless frontier — is the shape of its power, distinct from the lawless hub crossroads.

### Hands-off by design — can't hold it, doesn't want to

- **It can't.** The network is Builder tech the Coalition only half-decodes — too vast and alien to police or hold back. It can open lanes and clip a cut; it **cannot steer the frontier.** The selection, the frontier, what the network is _for_ — all the machine's, beyond its reach (it may not even see what the machine is doing).
- **It doesn't want to.** What the frontier yields — resources, energy, salvaged Builder tech — flows **back to Earth, which needs it.** Loosing thousands of self-interested gaters is the fastest way to extract that windfall, so hands-off is the **strategy, not a failure**: open the floodgates, skim the flow. _(How badly Earth needs it, and what "it" is, stays open — don't over-specify.)_ [tentative]
- This is the in-world **license for the competitive sandbox**: claiming, raiding, trading, sprawling are all permitted because the Coalition **gains from the churn and couldn't curb it anyway** — past its writ, "reach equals exposure" is the only law.
- **Two layers hold here too:** the honest surface motive — _get rich, feed Earth_ — is real; the hidden truth, that the frontier _is_ the gene-selection engine, is something the Coalition neither built nor sees. Patriotic gold rush on top, cosmic indifference underneath.
- Its few protective rules are **self-interest, not mercy**: uphill [[systems#Potential|frontier-visibility]] + home obscurity stop the strong from trivially farming the undeveloped — a productive frontier keeps yielding; a curb-stomped one doesn't.

### The body — a disposable avatar

- A gater pilots a **disposable avatar** and respawns on death; only the avatar dies (see [[systems#Presence, respawn & XP|Presence & Respawn]]). What the avatar physically is and how the mind connects is open — see [[questions]], Avatar origin.
- Because every avatar is **registered to a real person**, the system can always tie an avatar to its owner — so anonymity is purely **peer-facing**, not absolute.
- This gives the frontier its teeth without erasing the stock: **selection without extinction** — civilizations are mortal, players are not.

### Why / rejected

- **Network = mesh-with-a-heart**, chosen over pure hub-and-spoke — spokes would break the bilateral raid tunnel and the direct-dial-vs-hub economy.
- **Certified provisioning**, chosen over pure demand-dialing (gives the studio no rollout/capacity lever) and over per-Gate stability _windows_ (the strongest capacity lever, but it pulls players onto the network's schedule — eroding the no-obligation thesis — and double-books the Rift cosmology; held for frontier worlds later).
- **Hands-off broker (can't hold it, doesn't want to)**, chosen over the Coalition as a covert schemer breeding humanity — selection belongs to the Builders' machine. It half-understands the network (can't police it) and profits by piping the frontier's yield back to Earth (won't): PvP stays licensed without a sinister overseer, and the competition sits with the players (corps and groups).

### Open

- What peers can/can't trace of a raider's identity ([[questions]]).
- Named figures, politics, internal structure — none established (do not invent).

## Timeline & the Wake

Canonical order of events. A date conflict with this section is a hard contradiction.

- **Before** — the **Builders'** era, then an indeterminate **dormancy** of the sealed network. Nothing dated is established — do not invent.
- **Year zero — The Wake.** In-world **year x**: humans found a dormant Gate on Earth and learned to open it. Gameplay begins some years maybe after, in a land rush "that started five minutes ago." _Placeholder name_ (open). Why the Gate woke, how long it was dormant, what (if anything) is behind it — all open; do not invent. [tentative]
- **Over the game's life** — rising **misdial** frequency and harsher frontier pulls: the difficulty ramp and the visible symptom of the mystery (see [[systems#Gates|Gates]]). Not a fixed calendar; a trend. [tentative]

## The mystery

- The Coalition's "we flipped a dormant relay" is a **cover story**; something _else_ woke the network and is using it too. Candidate truths tracked in [[questions]]. [tentative]

## Themes & tone

- Frontier sci-fi gold-rush among the bones of a vanished precursor civilization, with an undercurrent of **cosmic unease**.
- Leaning **precursor-tech-hybrid** (start low-tech, climb by scavenging Builder tech); exact register open. [tentative]

## Not yet established (do not invent)

- Characters, species, cultures, religions, myths, languages — **none written**. These stay dormant until real content exists.

## Behind the curtain — the Builders' truth

**The designer's answer to the central mystery. Parked: do NOT surface it to players until a payoff earns it.** In-game, the Builders stay unknown; only the system's _function_ is observable. Recorded so we build consistently toward it.

### What players can observe (fair to make canon)

- The system **selects for the fittest gene**, keys to **DNA**, **discards vehicles**, and **propagates patterns**. It behaves like a galaxy-scale survival-of-the-fittest engine. This is visible and provable in-world.

### What stays the mystery (parked)

- **Bred for what, and for whom?** Who or what collects the "best gene" when the engine finds it? This is the question the game dangles, never answers early.

### The leading candidate truth [tentative — behind the curtain]

- **The Builders were dying.** Their own biology failed them — they could not save _themselves_.
- **So they built a successor-breeding machine.** Unable to persist as individuals, they aimed to cultivate a worthy **gene** to inherit what they couldn't keep. Selection without extinction is their design philosophy, not just a game thesis.
- **They seeded the galaxy with von Neumann probes** (self-replicating craft). The probes:
  - **Built and spread the Gate network** across the galaxy (why it's galaxy-spanning).
  - **Seeded living, breathable worlds** as arenas (why most Gate-worlds have life — and a living substrate avatars can be grown or sustained from).
  - **Set up selection to scale: planet-level first, then galaxy-wide** as the network wakes and connects. Early game = local competition; late game = the arenas wire together into galaxy-scale selection.
- **The Wake** = the machine (or its trigger) switching on the selection program — which is also why the network is unstable and Rifts proliferate.

### Why this candidate is strong

- It **retro-explains** a pile of decided mechanics with one stroke: the galaxy-spanning network, the living/breathable worlds, the gene-as-unit theme, the planet→galaxy scaling (which doubles as the studio's content/server-expansion lever and fits the existing "more of the network waking" progression driver).
- It needs **no benevolent caretaker** — the makers are indifferent/dead; the machine just runs. Fits "cosmic unease."
- Compatible with the open candidate truths (outside signal / tripwire / Coalition lie): the "best gene" could be cultivated _for_ whatever woke the network, or the Wake _is_ the tripwire that started selection. Keep these convergent, not contradictory.

### Discipline

- **Surface the function, park the purpose.** When/if a payoff arrives, reveal pieces of this. Until then, no NPC, page, or item states the Builders' motive.
