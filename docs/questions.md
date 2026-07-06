# Questions

The living backlog for **big cross-cutting forks**, parked explorations, deferred wants,
and option banks. System-local open questions live beside their owning mechanics in
[[systems]] or [[world]]. When an item settles, fold the call (its _why_ + what was
rejected) into the owning section as `Why / rejected`, flip that section's tag
`[tentative]→[current call]`, and delete the item here. Big decision-support reports
live in `raw/` and are pointed to from their item.

Items carry stable IDs (**#N**); IDs are never reused, so gaps are normal.

## Open questions

- **#4 Which Wake trigger is canon** (outside signal / tripwire / Coalition lie / unseeded-consciousness alert)? The behind-the-curtain direction is now **Builder consciousness-preservation research machine**; the specific Wake trigger stays open. ([[world#The mystery|The mystery]], [[world#Behind the curtain — the Builders' truth|behind the curtain]])
- **#5 Avatar origin** — what the avatar physically is, where the body comes from, how the mind connects. Full options report in `raw/avatar-origin-options.md`, but raw notes are not current truth. The **body-flavor pick is not made** — until it is, every doc uses only the origin-agnostic baseline (disposable avatar, respawn at an anchor, mask, two-layer XP, possible mask-derived exposure data). Touches [[systems#Presence, respawn & XP|Presence & Respawn]], [[systems#Mask energy|Mask Energy]], [[systems#Coordinates & obscurity|Coordinates]], [[world#United Gate Coalition|Coalition]].
- **#6 Theme/genre fork — commit the whole game to precursor-tech hybrid?** The leaning is adopted in the combat spine (COMBAT-1: bow/melee → scavenged Builder tech → fleets) and the tamer fork resolved narrow (TAME-1); committing the **whole** game (art identity #7, setting/tone register) is broader than the combat model and not yet signed off. Rejected alternatives: pure sci-fi (leaves art + combat open) and pure fantasy reskin (discards the [[gate-physics|Gate Physics]] depth and the space-scale endgame). Scope guards if taken: "easy to use" means _traverse_, not _master_ (dialing stays hard, maps onto the Standard→Heavy→Supergate ladder); fleet combat sits at the _top_ of the climb. Couples to #4, #7, #19.
- **#7 Final art identity** — NMS-lush vs. low-poly-stylized; two target identities, look deferred until after greybox. Touchstones and the leaning path (low-poly flat-shaded + strong post-processing) in `INSPIRATION.md`.
- **#17 Does the title "Gaters" survive** the phonetics ("gators" / "gaiters"), searchability, and harassment concerns? (Player term settled: gater.)
- **#18 Why did the Builders forbid walling a Gate?** The dome is a deliberate anti-enclosure feature — a Gate that can never be entombed can always be reached _through_. Mystery hook; rhymes with #4. Left unanswered per Builders-unknown.
- **#19 Is Earth the only species the Gates woke for?** _Parked; canonical answer-in-waiting._ The synchronized Wake plausibly lit **many** species' Gates at once — making the simultaneity itself evidence the threat is real, and scaling the natural-selection frame into three rings (player vs. player → humanity vs. other woken species → all vs. the threat). **Not** "rival Coalitions" — a Coalition is a human institution, contingent; rivals at scale are **alien polities that answered the Wake differently** (a hive, a sovereign, a machine intelligence, or no central body at all). Open fork: **do other species respawn, or really die?** — decides whether "only avatars die" is a human edge or a human weakness. If taken, soften "threat to Earth" → "threat to the woken worlds." Touches players-are-human ([[world#Premise|Premise]]), [[world#United Gate Coalition|Coalition]], #4. Roadmap-scale, not launch.
- **#20 Gate-DNA findability overhaul** — _explored 2026-06-29, **not adopted**; parked._ Explored replacing avatar-shares with a **two-factor lock** (dialing needs position _and_ DNA), re-key = mutating a Gate's config-genome, leak via goods + connecting, plus a gate-selection layer (probes breeding gates). _Survivor, kept:_ gate-config-as-genome explaining gate variety — folded tentatively into [[systems#Gates|Gates]]. _Blockers that stopped adoption:_ (1) the two-factor lock leaks position only when the victim dials a hunter-controlled gate, so an active trader can grow yet stay unraidable — **reopens the turtle-equilibrium trap**; (2) the gate-selection layer has **no cull** (nothing kills a bad-arena gate → drift, not selection), and uncontrolled misdials plausibly come from unstable/bad gates, which inverts it. Revisit only with fixes for both.
- **#21 Engine choice — Unity vs. Unreal (vs. Godot).** Undecided; full comparison and the tip-factors in `raw/engine-comparison.md`. Blockers to resolve first: art identity (#7), whether the agent pipeline holds up (needs a throwaway prototype), the base object-count budget. When chosen → record in [[systems#Technical challenges — and the Gate mechanics that ease them|Technical challenges]] with the why and what was rejected.
- **#25 Biological-filter Gate state — "gates tuned toward evolution"** — _explored 2026-07-06; promising, not adopted; parked._ The frame: the [[world#Behind the curtain — the Builders' truth|von Neumann probes]] tuned gate configs **blind** (see [[systems#Gate variation — the Builder "config"|Gate variation]]); a throat can **sort what crosses it**. The [[systems#Combat|dome]] already sorts _built vs. natural_; a **bio-filter** is its sibling — passes living matter, rejects inert/manufactured. Same [[gate-physics#The dome — protecting transit, clearing builds|conceit family]], no new physics debt.
  - _Not a gate **type** — a **state** a throat can be in._ Three sources: **(a) misfire** — instability flips a throat to bio-filtered for a while (reuses [[systems#Self-misdial — the uncontrolled lane (FRONTIER-2)|self-misdial]] / the Wake ramp); an emergent hazard, not a safety. **(b) deliberately engaged** — a controlled action puts a throat in bio-mode; on-thesis only as a **dialer-set or mutually-agreed** arena/denial lever, **never a target-set home defense** (that is the rejected defensive dome — TURTLE-1). **(c) permanently locked** — a rare handful stuck in bio-mode; candidate home for mystery-dense / Earth-gate-Wake ties. Most gates are never filtered.
  - _Why it dodges the turtle trap — the strip is **symmetric**._ A bio-throat disarms **both** sides to biological (the defender loses turrets/energy weapons/manufactured armor too), so it never hardens a breach — it changes the weapon tier for everyone and tends to **help the stripped underdog**. Danger case to avoid: a _target_ pre-setting their own gate to bio-mode as an ambush edge; keep activation with the dialer, the misfire, or mutual consent.
  - _Uses it opens:_ offensive **denial** (sabotage an enemy's active gate to bio-mode via the stealable control device → reinforcements arrive naked, cargo can't extract); a **naked-arena** duel condition (BRAWLER-2); a mid-run **lane twist** (a _dynamic_ `bio_filter` in the [[systems#Sensor readings & lane twists (FRONTIER-4)|FRONTIER-4]] enum). Governs **transit, not occupation** — gear already on the far side stays; the bite is extraction/reinforcement.
  - _Loot flips to what survives a filter:_ **Mind-XP adaptations, tamed lineage records, biological samples** — physical cargo can't extract, so the payoff is knowledge/creatures. Natural home for the "working with evolution" fantasy (adaptations earned under biosphere pressure) and a reason [[systems#Taming|taming]] exists (living defenders/loot for a living gate).
  - _Sharpest structural win:_ reframes "gate config variation" from vague (field size, stability) into a concrete, reusable axis — **admission rules at the throat** — riding on the transfer-manifest the aperture netcode already needs (tech bill #2): a predicate on the crossing manifest, toggled by state.
  - _Blockers before adopting:_ (1) **mask** — required tech; leaning: the mask is the machine-sanctioned exemption, so crossing a bio-throat strips gear/cargo but not your knit mask+body (also lets recall work — you already drop the away-avatar's gear on recall). (2) **mystery deflation** — keep it rare, surface function only, never purpose. (3) does **not** reopen #20 — no gate-breeding/cull; a runtime state, not a selection layer.
  - Touches [[systems#Gate variation — the Builder "config"|Gate variation]], [[systems#Self-misdial — the uncontrolled lane (FRONTIER-2)|Self-misdial]], [[systems#Taming|Taming]], [[systems#Presence, respawn & XP|Mind-XP]], [[systems#Mask energy|Mask]], #4, #5, #19.

## Wanted later (decided, deferred)

Things we've decided we want but are deferring. _Intended_, not undecided. When an item
comes into scope, build its section and strike it here.

- **Orbital / space Gates** — free-floating Gates with no planet beneath; cannot be turtled, so always-live PvP zones. (The greybox proves ground Gates first.)
- **Megastructure / Supergate sites** — rare precursor megastructures that pass fleets; server-event scale; the macro-expansion path between systems.
- **Space / fleet combat** — two-phase: fleet raids for orbital control, then infantry raids on the base. Ground/infantry combat ships first.
- **Multi-species frontier** — the late reveal of #19; launch stays Earth-intimate.

## Option bank — Gate uses & objects

A menu of things Gates can _do_, and the discrete objects the design implies. Mostly
**design options / deferred** — captured so they aren't lost or re-invented; integrate
into [[systems]] when one is adopted.

### Sharpest — leaning-in [tentative]

- **No caller ID + transponder codes** (USE-1): an incoming raid doesn't reveal _who_ until it commits; friendly trade requires an accepted code; spoofing codes is an infiltration play. The clean way to separate "open for business" from "open to attack."
- **Occupy-to-lock / jamming**: holding a Gate open (or keeping a line busy) locks a target out of the network — a denial/siege tactic that doesn't touch the safe-by-default home rule.
- **Buffer wipe of in-transit forces**: troops "in transit" sit in a vulnerable buffer; cutting the connection mid-raid wipes an in-flight strike force — a raid-commitment mechanic.
- **Stealable Gate control device**: damage or steal the console and a planet can receive but barely dial out — a "stranded, raidable, can't retaliate" state that is _not_ permanent base loss.
- **Gate-as-bomb (overload)**: overloading a Gate yields a planet-scale explosion — a rare, costly scorched-earth / siege-ender (the overdrive-collapse physics).

### Catalogued — not yet integrated [open]

- **Dial-into-a-star** (route a tunnel through a sun; exotic super-weapon behind disabled safeties) · **Safeties as a tech resource** (research/disable Builder safety protocols for weapon-grade uses) · **Comms through a closed Gate** (data/radio pass while the aperture is shut — diplomacy, ransom) · **Subspace relay / data network** (Gates carry data; control = information control) · **Recon probes** (scout before committing) · **Gate bridges** (chained call-forwarding — the cheap-distance basis for hubs) · **Harvested / relocatable Gates** (claim and move found orbital Gates) · **Reprogram-the-route** (a relay controller reroutes/traps traffic) · **Network self-update as a worm vector** · **Proximity priority conflicts** (a competing Gate suppresses a rival's) · **Time/exotic physics** (solar-flare time travel, black-hole dilation; the ~38-min cap already borrowed as the siege ceiling) · **EVE economy/access layer** (access lists, tolls, mass×distance fuel, a hard mass cap forcing big fleets onto Supergates, plantable beacons, area jammers, killable Gates).

### Implied objects [tentative, deferred]

- **Power core** — scavenged Gate fuel + the economy's hard currency; also portable mask fuel on your own soil. Does **not** extend the raid clock (away reserve is fixed by home-Gate power).
- **Gate control device** — the per-Gate console (power, dialing, targeting math); attackable, stealable (the "DHD-equivalent"; original name open).
- **Transponder / access codes** — distinguishes friendly trade from anonymous raiders; spoofable.
- **Gate segments** — pieces from which Supergates are reassembled and repaired.
- **Beacon** — a plantable landing site near a target (EVE cyno-beacon analog); does **not** extend away reserve — jump/landing point only.
- **Coordinate database** — the homebase log of discovered coordinates; a raidable asset (the map).
- **Jammer** — denies Gate-jumping across an area (EVE cyno-jammer analog).
- **Open:** weapons, armor, building materials, consumables, trade goods, artifacts.

### Resolved calls folded into systems

- **#2 Trade risk model** — resolved into [[systems#Trade, scarcity & meeting places|Trade, scarcity & meeting places]]: trade is optional, physical, and in-person; it risks carried cargo and route exposure before home exposure.
- **#24 Trade market shape** — resolved into [[systems#Trade, scarcity & meeting places|Trade, scarcity & meeting places]]: no trade stations, global auction house, formal contracts, escrow, pickup keys, or reputation score by default. PvE scarcity is solved mainly by exploration/frontier play, not markets.
- **#23 Exposure model** — resolved into [[systems#Coordinates & obscurity|Coordinates & obscurity]]: Gate heat / samples / leads is the primary home-location loop. Avatar/mask data is supporting evidence, not a permanent home-address shard.
