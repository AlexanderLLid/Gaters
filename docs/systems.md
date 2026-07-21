# Systems

The Gaters mechanics bible in one file. The world it serves is [[world]]; the central
phenomenon's constraints live in [[rift-rules|Rift Rules]]. Numbers never live in
prose; values land in data at playtest.

Mechanic bullets use sparse **Serves / Prevents / Risks / Test** tags only when a
dependency is load-bearing.

## Shared requirements

This section is the single source of truth for cross-workstream product requirements.
Write each once as `SUBJECT-N [MUST|SHOULD] — observable outcome`; other files reference
the ID without repeating its meaning.

### Global

[open] No global requirements recorded.

### World generation

- **WORLD-1 [MUST] — Intentional world scarcity:** a seed may intentionally produce
  little connected walkable land or no optional sites, resources, vegetation, or
  landmarks; generators and evaluators distinguish declared scarcity from defects and
  never add optional content merely to satisfy a universal presence check.

### Scale

- **SCALE-1 [MUST] — One shared history:** all players resolve durable world state,
  claims, ownership, and consequences against one authoritative history; backend
  partitioning never creates conflicting or player-selected copies.
- **SCALE-2 [MUST] — Bounded shared geography:** worlds may place concurrent visitors in
  the same active geography, but every live simulation has an enforced player budget and
  no core mechanic requires mass attendance; cap values live in data.
- **SCALE-3 [MUST] — Capped home Anchors:** a shared world may host multiple guild-owned
  home Anchors, but home, resident, and active-player budgets are enforced; one claim
  grants no ownership over another guild's Anchor or territory.
- **SCALE-4 [MUST] — Persistent danger disconnect:** disconnecting during danger never
  removes or relocates the authoritative body; it persists through world sleep and server
  recovery until death or valid return, and reconnect resumes that body.
- **SCALE-5 [MUST] — Direct neighbour attack:** guilds sharing one world can reach and
  attack each other's home territories by ordinary travel without hostile-Rift access,
  including while owners are offline.

### Loot

- **LOOT-1 [MUST] — Carried depth reward:** collected loot remains carried and loseable
  until return through a valid Rift, while deeper expedition progress improves the
  available reward distribution.

### Art

- **ART-1 [MUST] — Recognizable style-independent presentation:** generated subjects
  clearly read as their intended type at the gameplay camera, with coherent silhouettes,
  identifiable major parts, and no accidental intersections, detached parts, or
  collapsed forms. Realism level, proportion language, shading, surface detail, and the
  final visual style remain open and replaceable.
- **ART-2 [MUST] — Characterful human variety:** human concepts and generated casts vary
  ancestry, skin tone, age, facial proportions, and asymmetry instead of defaulting to
  one conventionally attractive light-skinned face template. A chosen style may simplify
  or exaggerate consistently, but it must preserve individual variety and avoid offensive
  caricature.
- **ART-3 [MUST] — Isolated visual feasibility:** art-direction prototypes evaluate
  candidate visual languages using Art-owned source and evidence;
  they do not modify or count as evidence for character-family generation, rigging,
  skinning, animation, or runtime-character feasibility. Pipeline integration requires
  a separate Character Generation & Animation handoff.
- **ART-4 [MUST] — Reusable generation evidence:** every iterative art experiment
  preserves its versioned recipe and control deltas, tool and evaluator versions,
  source and derived artifact identities, evaluations, promotion or stop decision, and
  failure diagnosis so the candidate lineage can be replayed and mined when building
  generators.

### Built sites

- **BUILD-1 [MUST] — Per-connection movement support:** every built-site connection
  declares its supported movement modes; a site has no global movement type and may mix
  movement networks without changing recipe structure.

### Characters

- **CHAR-1 [MUST] — End-to-end generated humanoid:** one versioned mechanical recipe
  produces a controllable Unreal humanoid without manual body, rig, skinning, Physics
  Asset, IK, or animation setup. One repeatable run verifies locomotion, uneven-terrain
  contact, impact, fall, recovery, and return to locomotion through independent evidence.
- **CHAR-2 [MUST] — Native runtime authority:** versioned offline generation tools own
  reproducible body, rig, skinning, physical-intent, and source-motion generation;
  Unreal owns live movement, floor queries, IK, collision, physics, recovery,
  replication, and gameplay authority through native systems behind versioned adapters.
- **CHAR-3 [MUST] — Controlled family variation:** changes to dimensions, limb
  proportions, mass, collision, movement ranges, and physical strengths preserve required
  family bone roles and pass through the unchanged downstream pipeline. A change to limb
  count or skeletal roles requires a separately validated anatomy adapter.
- **CHAR-4 [MUST] — Replaceable zero-manual generation:** every promoted character
  generation stage consumes a versioned tool-neutral input and emits a versioned output,
  provenance receipt, and independent evaluation without manual modelling, repair,
  rigging, skinning, or animation work. Human visual acceptance may select or reject
  candidates; Blender, Houdini, and future tools remain replaceable derived adapters.
- **CHAR-5 [MUST] — Inspectable generated geometry:** every promoted character-geometry
  run emits labelled fixed-view previews rendered from its accepted artifact, with the
  run identity and capability limits visible; AI-imagined pictures never substitute for
  generated-geometry evidence.

## Design pillars

- **Cross-world reach equals exposure** — outward Rift activity creates evidence and
  possible cross-world attack routes.
- **One identity, contextual risk** — players do not switch servers or identities for
  PvE/PvP; Rift use creates cross-world exposure, while a shared home world carries
  persistent neighbour risk.
- **Bodies die, players persist** — bodies, gear, creatures, homes, holdings, and empires
  can be lost; the player cannot.
- **Cross-world safety is obscurity** — a quiet home is hard to target from another world;
  neighbours on the same world can reach it directly.
- **No upkeep tax** — homes do not decay or demand maintenance merely because owners are
  absent; absence does not prevent neighbour attack.
- **The Rift is the inter-world seam** — travel, exploration, trade, claiming, exposure,
  and attacks between worlds use temporary Rifts; same-world interaction uses ordinary
  geography.
- **Geography matters after arrival** — Anchors open a world; ordinary travel finds its
  settlements, resources, creatures, and other Anchors.
- **Multiple Anchors defeat one-door bunkering** — a claimed world can have several
  possible approaches; defense belongs to the settlement and surrounding terrain.
- **Fantasy first** — spells, Rifts, Anchors, creatures, and survival tools are not
  science fiction in disguise. [current call]

## Core loop [tentative]

- **Moment:** explore, gather, fight, tame, craft, and build; call or enter a Rift when
  seeking another world.
- **Session:** log in with no upkeep chore; choose a quiet build, expedition, hunt,
  trade, raid, or contested-world run; return before the Rift opportunity closes.
- **Long term:** grow the settlement, body, equipment, creatures, spellcraft, route map,
  and guild reach; greater reach creates more traces and rivals.

## First prototype priority — generated raid [current call]

Prove one generated fantasy world containing a valid arrival, raidable settlement,
defenders, loot, extraction, and persistence. Do not require markets, massive shared
worlds, live-service routing, or broad creature content.

- The generated terrain must not require Anchor-centered topology or a permanent
  Anchor-to-base corridor.
- Validate that each chosen arrival produces a pathable attack with reachable loot and
  extraction.
- AI/abandoned settlements provide the first repeatable targets.
- Exact Rift targeting, PvP matchmaking, and the population exposure equilibrium can
  remain stubbed.

## Failure modes

- **TURTLE-1 Turtle equilibrium** — quiet play cannot be optimal forever; growth needs
  outward exposure.
- **TRADE-SUICIDE-1 Trade suicide** — ordinary trade may expose cargo and routes, not
  automatically reveal a home to another world.
- **OFFLINE-DODGE-1 Offline dodge** — closing or logging out cannot cancel a committed
  raid.
- **ZERG-1 Zerg pile-on** — one trail cannot summon an unlimited force.
- **WHALE-1 Newbie farming** — strong guilds cannot reliably hunt downward across worlds;
  same-world participant caps and structure-damage rules remain required.
- **DOORSTEP-1 Doorstep kill-box** — fortifying one Anchor cannot make a world
  unreachable.
- **PERMA-LEAK-1 Permanent leak** — one copied clue cannot grant permanent cross-world
  access to a home.
- **Healthy/toxic line:** no passive decay or upkeep chore; direct same-world attack is
  intentional persistent risk.

## Rifts

Rifts are temporary magical openings between bounded worlds. [current call]

- **Natural Rifts (RIFT-1)** appear unpredictably and provide accidental, uncertain
  discovery. Describing that unpredictability as weather is [tentative], not a decided
  cause or rules model.
- **Called Rifts (RIFT-2)** are opened by trained casters. The spell, cost, targeting
  evidence, and degree of control are [open].
- **Temporary (RIFT-3)** — a Rift closes; expeditions and raids cannot stay connected
  indefinitely.
- **One inter-world phenomenon (RIFT-4)** — natural exploration, deliberate travel, and
  hostile entry between worlds use Rifts rather than parallel travel systems; movement
  within one world remains ordinary.
- **World provisioning (RIFT-5)** — the casting/opening sequence provides time to load or
  start the destination world.
- **No menu (RIFT-6)** — players discover worlds and Anchors through play; a route map is
  earned knowledge, not a server list.

### Targeting [open]

- Decide whether blind casting can reach claimed worlds.
- Decide what evidence lets a caster bias toward a known world.
- Decide whether the caster chooses a world, a region, an Anchor, or only improves odds.
- Persistent map knowledge must not become permanent hostile access by itself.

### Connection and limits [open]

- Simultaneous Rifts per Anchor and world.
- Directionality, what crosses, carried mass, casting interruption, and collapse.
- Return travel, recall, and what happens when a Rift closes; carried loot is secured
  only by returning through a valid Rift (LOOT-1).

## Anchors

Ancient magical stones or ruins that attract and stabilize Rifts. [current call]

- **Several per world (ANCHOR-1)** — Anchors may be far apart and create different
  approaches, routes, and travel needs.
- **Same rules after claim (ANCHOR-2)** — claiming records ownership and return; it does
  not disable, privatize, or reconfigure Anchors.
- **Arrival focus (ANCHOR-3)** — incoming Rifts tend toward Anchors. Exact selection and
  safe-arrival rules are [open].
- **Respawn (ANCHOR-4)** — a claimed Anchor is a return point after death; exact fantasy
  explanation is [open].
- **Physical discovery (ANCHOR-5)** — players must reach an Anchor in the world to learn
  it; no remote directory reveals every site.

## World travel

- A world is a bounded persistent record when empty and an active simulation when
  occupied.
- The universe keeps one shared history. A live world may provide shared geography for
  its concurrent visitors, subject to its player budget (SCALE-1, SCALE-2).
- Players enter through a Rift near an Anchor, then travel physically across the world.
- Land, sea, and flying creatures may carry local travel; uninterrupted cross-server
  travel is not required by this design.
- Discovering another Anchor expands the known route map. Reuse rules remain [open].
- A claimed world stays governed by the same natural Rift and Anchor rules as an
  unclaimed world.

## Raiding

The cross-world headline fantasy: hunt a valuable world, open a temporary way in, find
the settlement, breach, take loot, and escape. Neighbouring guilds on the same world can
also attack by ordinary travel. [current call]

- **Finding earns attack (RAID-1)** — a cross-world home raid requires current evidence;
  no global home list. Same-world neighbours need no trace or hostile Rift.
- **Bounded raid (RAID-2)** — a committed hostile Rift survives defender shutdown and
  closes on a fixed raid clock.
- **World approach (RAID-3)** — arrival does not place attackers inside the vault; they
  traverse and breach ordinary defenses.
- **No single door (RAID-4)** — another Anchor may become the attack route, so the
  defender cannot solve every raid by fortifying one mouth.
- **Worth attacking (RAID-5)** — a breached target must pay at least the structural cost
  of reaching it; exact loot floor remains tuning.
- **Rare cross-world offline raids (RAID-6)** — possible after successful hunting, never
  granted merely because a player was absent.
- **Carried extraction (RAID-7)** — loot enters inventory when collected but remains
  loseable until the carrier returns through a valid Rift; deeper progress offers a
  better reward distribution. [current call]
- **Direct neighbour attack (RAID-8)** — guilds sharing a world can travel to and attack
  each other's home territories without opening a hostile Rift, including while owners
  are offline. [current call]

### Open raid rules

- Arrival-Anchor selection and warning.
- Attacker/defender caps, simultaneous hostile Rifts, and anti-zerg enforcement.
- Arrival protection or no-build clearance.
- Exact cross-world raid clock, identity reveal, and offline timing; same-world raid caps
  and structure-damage rules.

## Combat

- Gear, bodies, creatures, terrain, preparation, and positioning drive fights.
- Defense is perimeter and settlement design, not spawn killing at one portal.
- Ranged and melee survival combat forms the baseline; spells and creature combat expand
  it. Exact verbs and tuning are [open].
- Home raids and contested frontier encounters reuse the same combat model.

## Spellcraft and classes

- **Approved-trait generation (SPELL-1).** Every live spell, whether standard or derived
  from player behaviour, resolves to a versioned recipe composed only from approved rule
  traits. [current call]
- **Lab expansion (SPELL-2).** Offline research may propose new rule traits, but a trait
  cannot enter live recipes until an independent verifier passes it and an explicit
  promotion approves it. [current call]
- **Universal core with optional traits (SPELL-3).** Every spell recipe declares its
  action, target, cost, limits, and counters through one shared structure; typed traits
  such as a magical source, creature form, or movement capability attach only when the
  spell uses them. [current call]
- **Actions before roles (SPELL-4).** The initial action vocabulary is harm, protect,
  restore, strengthen, weaken, control, sense, hide, move, create, change, and transfer.
  Familiar roles and classes are derived from action combinations and optional traits;
  they are not a second power-granting taxonomy. [current call]
- An LLM may interpret observed play and propose candidates; it cannot approve, grant, or
  install a mechanic.
- [open] Define the optional trait vocabulary, composition rules, evaluators, offer rules,
  graph visibility, and class reading under [[questions#Open questions|question #33]].

### Why / rejected

- The live/lab boundary permits the spell system to expand without making an untested
  player-specific rule authoritative. Hand-authoring every complete spell was rejected
  as the only path because it cannot support behaviour-derived personal adaptations.
- Direct live rule invention was rejected because its interactions, counters, save
  behaviour, and multiplayer meaning cannot be validated before the player receives it.
- A separate recipe structure for every magic domain was rejected because hybrid spells
  would require bespoke translation; one rigid structure with every possible field was
  rejected because most spells would carry irrelevant fields.
- A separate fixed role system was rejected because it would duplicate recipe actions
  and drift from what abilities actually do; labels such as tank remain derived views.

## Presence, death, and return

- **Bodies die; players persist (RETURN-1).** Death drops the current body and carried
  gear; the player returns at a claimed Anchor or fallback start.
- **One active body (RETURN-2).** No simultaneous multibody presence.
- **Recall costs the run (RETURN-3).** Emergency return abandons the away body, gear, and
  unextracted loot.
- **Danger-disconnected body persists (RETURN-4).** Disconnecting during danger never
  removes or relocates the active body. Reconnecting resumes it; an empty sleeping world
  stores it and restores it when that world next becomes active. [current call]
- Permanent knowledge survives death; body-bound development may not. Exact progression
  split and fiction are [open].

## Claiming

- **Home Anchor claim (CLAIM-1).** A living player claims an unowned Anchor and its
  defined local building territory.
- **Guild ownership (CLAIM-2).** A guild owns each home Anchor it claims; a solo player
  is a guild of one.
- **Shared home world (CLAIM-3).** Several guilds may own separate home Anchors on one
  world; claiming one grants no ownership over the others. [current call]
- **No transit immunity (CLAIM-4).** Claiming does not change Rift or Anchor behaviour.
- **Bounded homes (CLAIM-5).** Home Anchors, residents, and active players obey world
  budgets; exact caps and full-world behaviour are [open].
- Claiming grants local building rights, return, permissions, and the ownership record.
  The recognizing institution or ritual is [open].

## Traces and obscurity

The primary cross-world hunt is **activity → evidence → temporary hostile access**.
[current call]

- **Unknown (TRACE-1)** — another guild lacks enough current evidence to bias a Rift
  toward the home.
- **Traced (TRACE-2)** — activity, routes, witnesses, combat, cargo, deaths, or stolen
  records point toward the guild or world.
- **Located (TRACE-3)** — evidence grants a bounded chance or window to attempt hostile
  entry; exact targeting is [open].
- **Stale (TRACE-4)** — old evidence no longer completes the attempt.
- **Activity leaks (TRACE-5)** — more Rifts, Anchors, members, cargo, claims, and combat
  create more evidence.
- **Reveal order (TRACE-6)** — activity first, route second, identity third, home last.
- **Neighbour knowledge (TRACE-7)** — guilds sharing one world may discover and attack
  homes by ordinary travel; trace access is unnecessary.
- Geography and discovered Anchor locations may remain known while current hostile access
  expires.

### Why / rejected

- **Why activity evidence:** it reuses ordinary server events, makes cross-world hunting
  active, and lets quiet homes recover cross-world obscurity.
- **Rejected — permanent cross-world address key:** one copied key would create permanent
  access from every world.
- **Rejected — raw backend radar:** players find physical clues and uncertain leads, not
  a debug ledger.

- Tunables: evidence weights, decay, lead confidence, access duration, and route cost.

## Potential

A sticky guild-strength rating used to match exposure and prevent downward farming.
[tentative]

- Aggregates footprint, progression, roster, and Rift capability.
- Drives which frontier activity and hostile opportunities can connect.
- A big guild creates more traces and reads as a bigger target.
- Cross-world home-raid eligibility still requires locating the target; Potential alone
  is not an address. Same-world neighbours need neither.
- Exact in-world presentation and weights are [open].

## Holdings

- Holdings are exposed forward assets: resource sites, caches, camps, or claimed public
  objectives outside the home territory.
- They are easier to find and contest than homes.
- Losing one denies its output and grants bounded loot; it never creates an ongoing
  offline drain or reclaim obligation.
- Use holdings for take-and-hold conflict rather than making home ownership decay.

## Taming

- Include taming as a core fantasy. [current call]
- Taming is an active objective, not a feed-and-wait maintenance timer.
- Creatures provide labour, travel, hauling, combat, or mounts.
- A creature taken into danger can die and be looted; long-term unlocks need not be erased
  with every death.
- Deep breeding/genetics remains rejected unless a later prototype proves it fun without
  babysitting.

## Building

- Building a long-term fantasy settlement is a core pillar.
- **No persistent base decay or upkeep.** [current call]
- Safe from cross-world attack while hard to find; same-world neighbours can attack by
  ordinary travel, including while owners are offline.
- Defenses protect the settlement and approaches. They cannot rely on sealing one Anchor.
- Built sites follow `BUILD-1`: movement support belongs to each connection, so one
  settlement may contain different movement networks. [current call]
- Exact build depth, materials, structure types, and defense pieces are [open].

### Why / rejected

- Per-connection movement keeps recipes and physical evaluators usable for land, mixed,
  surface-water, and submerged sites without requiring those generators now.
- Rejected: one settlement-wide movement type, because mixed sites such as paths joining
  docks would require parallel recipes or special cases.
- Deferred: water-settlement generation until the ground loop works; the current land
  planner becomes one replaceable implementation of the shared contract.

## Economy and progression

- Quiet worlds reach a soft progression ceiling; further growth needs exploration,
  dangerous Rifts, rare creatures, raids, or optional trade.
- Worlds differ in resources and threats so routes and exploration matter.
- Calling and sustaining Rifts requires a cost; the resource and cost shape are [open].
- Trade is optional, physical, and in person. It risks carried cargo and cross-world route
  evidence, not automatic home exposure to outsiders.
- Progression climbs through survival tools, equipment, bodies, creatures, settlements,
  and Rift spellcraft. Exact tiers are [open].

## Player requirements

- **Raider:** find, approach, breach, loot, extract, and avoid revealing too much.
- **Builder:** keep a persistent home without upkeep; gain meaningful perimeter defense.
- **Explorer:** find unknown worlds, Anchors, routes, creatures, and valuable evidence.
- **Tamer:** earn creatures through active play and risk them deliberately.
- **Homesteader:** progress quietly to a ceiling without decay or compulsory trade.
- **Trader:** meet physically, move cargo, choose partners, and risk routes before home.
- **Group:** share an owned home Anchor, permissions, return point, and defense while
  other guilds may hold separate homes on the same world.

## Validation and technical constraints

- **First proof:** repeatedly generate a playable raid from arrival through extraction;
  reject blocked paths, unreachable loot, invalid defender navigation, and excessive
  object cost.
- **Population proof:** whether players venture out rather than turtle requires a scaled
  economy test; a small greybox cannot settle it.
- **Bounded worlds:** each world remains an isolated seed-plus-changes record loaded on
  demand. Do not require seamless planet-scale simulation.
- **Rift load:** casting and opening hide instance provisioning; the destination is known
  before transfer resolves.
- **Temporary engagement:** Rift collapse and the raid clock bound contested instance
  lifetime.
- **Persistent local conflict:** same-world neighbour attacks need no Rift lifetime;
  world and raid budgets bound participation instead.
- **Registry:** home-Anchor claims, local guild ownership, Potential, and current access
  are shared database state; no replacement faction is needed to justify the schema yet.
- **Online model (SCALE-1–5):** separate simulations resolve against one durable history;
  shared geography is bounded, and disconnected bodies persist as stored world state
  without requiring an empty world to keep simulating.
- **Generated-content boundary:** versioned recipes and source data are authoritative;
  terrain generation must not depend on permanent Anchor-centered topology.
- **Deferred:** seamless multi-region worlds, broad naval simulation, flying traversal,
  live-service scale, and true portal rendering.

## Standing constraint — originality

- Borrow survival-craft, escalating dungeon, taming, and player-conflict structures;
  never copy names, factions, art, creatures, interfaces, or a visible level-granting
  "System" from the reference works.
- Rifts are worlds to discover and contest, not curated theme-park activities selected
  from a menu.
