# Questions

The living backlog: undecided design questions, parked explorations, deferred wants, and
option banks. When an item settles, fold the call (its _why_ + what was rejected) into the
owning section of [[world]] / [[systems]] as `Why / rejected`, flip that section's tag
`[tentative]→[current call]`, and delete the item here. Big decision-support reports live
in `raw/` and are pointed to from their item.

Items carry stable IDs (**#N**); IDs are never reused, so gaps are normal.

## Open questions

- **#1 Is the core fun real, and will players open their Gates?** Two questions, two scales: the greybox proves the loop is _fun_; whether the population actually _opens up_ is an equilibrium that only resolves at playtest scale, not greybox — **assume-and-commit** ([[systems#Validation — two questions, two scales|Validation]]).
- **#2 Trade risk model — tiered exposure removed.** Gates were sealed / port-open / gate-open: port-open let trade risk cargo without exposing the vault, and was the named counter to the **trade-suicide** trap and the basis of the Trader archetype. Exposure is now **binary — sealed / open**; nothing currently stops a trade run from carrying the same breach risk as a raid. Needs either a new mechanic or a decision that trade is meant to carry full risk. Touches [[systems#Design traps (failure modes to avoid)|traps]], [[systems#Economy|Economy]] (TRADE-1), the Trader archetype.
- **#3 How hard should stagnation push** without becoming upkeep? Strong enough to push players out, not so strong it's a disguised upkeep tax. The homesteader sub-case is resolved (PROG-2: a soft ceiling, not a leak); the general faucet/sink line is open. ([[systems#Economy|Economy]])
- **#4 Which mystery truth is canon** (outside signal / tripwire / Coalition lie)? The Coalition's motive is decided (threat-to-Earth reconnaissance) — it colours but does **not** pick the truth; the three candidates stay open. ([[world#The mystery|The mystery]], [[world#Behind the curtain — the Builders' truth|behind the curtain]])
- **#5 Avatar origin** — what the avatar physically is, where the body comes from, how the mind connects. Full options report + recommendation in `raw/avatar-origin-options.md` (4 archetypes: printed/cloned, seeded-evolved organism, mask-kernel, creature/symbiote; 6 invariants treated as settled). The **body-flavor pick is not made** — until it is, every doc uses only the origin-agnostic baseline (disposable avatar, respawn at an anchor, mask, coordinate-shares, two-layer XP). Touches [[systems#Presence, respawn & XP|Presence & Respawn]], [[systems#Mask energy|Mask Energy]], [[systems#Coordinates & obscurity|Coordinates]], [[world#United Gate Coalition|Coalition]].
- **#6 Theme/genre fork — commit the whole game to precursor-tech hybrid?** The leaning is adopted in the combat spine (COMBAT-1: bow/melee → scavenged Builder tech → fleets) and the tamer fork resolved narrow (TAME-1); committing the **whole** game (art identity #7, setting/tone register) is broader than the combat model and not yet signed off. Rejected alternatives: pure sci-fi (leaves art + combat open) and pure fantasy reskin (discards the [[gate-physics|Gate Physics]] depth and the space-scale endgame). Scope guards if taken: "easy to use" means _traverse_, not _master_ (dialing stays hard, maps onto the Standard→Heavy→Supergate ladder); fleet combat sits at the _top_ of the climb. Couples to #4, #7, #19.
- **#7 Final art identity** — NMS-lush vs. low-poly-stylized; two target identities, look deferred until after greybox. Touchstones and the leaning path (low-poly flat-shaded + strong post-processing) in `INSPIRATION.md`.
- **#8 Raider anonymity vs. the central registry.** The Coalition **registers every gater**, so it always knows your identity — anonymity is purely **peer-facing** (other players don't see caller ID), never Coalition-facing. The open part is only what peers can/can't trace. ([[world#United Gate Coalition|Coalition]], [[systems#Raiding|Raiding]])
- **#9 Player-driven frontier-claiming vs. Coalition-paced expansion** — who controls world-growth pacing? Claimed frontier Gates are finite but slowly replenishing; the pacing knob is unassigned. ([[systems#Gates|Gates]] FRONTIER-1, [[world#United Gate Coalition|Coalition]])
- **#10 Frontier-world specifics** — rank scale (E–S vs. 1–10), frontier-loot vs. raid-loot balance, and the self-misdial rate/trigger (FRONTIER-2). ([[systems#Gates|Gates]], [[world#World types|World Types]])
- **#11 Mask/field specifics** — gradient vs. hard radius (leaning gradient); field radius Coalition-only vs. capped player upgrade; confirm asymmetric mask-at-zero. ([[systems#Mask energy|Mask Energy]])
- **#12 "Sealed = isolated" vs. "comms pass through a sealed Gate"** — confirm the split (info yes; goods/energy/people no). ([[systems#Gates|Gates]] PRES-1)
- **#13 Return-home when the home was wiped mid-run** — frontier travel is one-way out (recall is the way back); if your home was destroyed while you were out, where does recall land you? Likely a starter/operator anchor — undefined. ([[systems#Gates|Gates]], [[systems#Presence, respawn & XP|Recall]])
- **#14 Tribe shared-ownership sub-rules** — who may re-key the shared coordinate (any member vs. leader-only) and what happens if they're offline while the tribe is being found; member churn (does leaving/being kicked force a re-key; can a defector sell the coordinate); defense (who may recall to the shared home; how the muster cap counts a tribe vs. a solo). ([[systems#Potential|Potential]] TRIBE-1, [[systems#Coordinates & obscurity|Coordinates]])
- **#15 Survival meters beyond mask energy** — any, and how minimal? ([[systems#Mask energy|Mask Energy]])
- **#16 Long-distance dialing may be too strong** — reaching very far in one jump might be OP; likely mitigation is **jump / relay servers** (hop-routing through intermediary nodes, cf. Gate bridges in the option bank) rather than cheap direct long jumps. _Parked — revisit later._ ([[systems#Economy|Economy]], [[systems#Hub worlds|Hub Worlds]])
- **#17 Does the title "Gaters" survive** the phonetics ("gators" / "gaiters"), searchability, and harassment concerns? (Player term settled: gater.)
- **#18 Why did the Builders forbid walling a Gate?** The dome is a deliberate anti-enclosure feature — a Gate that can never be entombed can always be reached _through_. Mystery hook; rhymes with #4. Left unanswered per Builders-unknown.
- **#19 Is Earth the only species the Gates woke for?** _Parked; canonical answer-in-waiting._ The synchronized Wake plausibly lit **many** species' Gates at once — making the simultaneity itself evidence the threat is real, and scaling the natural-selection frame into three rings (player vs. player → humanity vs. other woken species → all vs. the threat). **Not** "rival Coalitions" — a Coalition is a human institution, contingent; rivals at scale are **alien polities that answered the Wake differently** (a hive, a sovereign, a machine intelligence, or no central body at all). Open fork: **do other species respawn, or really die?** — decides whether "only avatars die" is a human edge or a human weakness. If taken, soften "threat to Earth" → "threat to the woken worlds." Touches players-are-human ([[world#Premise|Premise]]), [[world#United Gate Coalition|Coalition]], #4. Roadmap-scale, not launch.
- **#20 Gate-DNA findability overhaul** — _explored 2026-06-29, **not adopted**; parked._ Explored replacing avatar-shares with a **two-factor lock** (dialing needs position _and_ DNA), re-key = mutating a Gate's config-genome, leak via goods + connecting, plus a gate-selection layer (probes breeding gates). _Survivor, kept:_ gate-config-as-genome explaining gate variety — folded tentatively into [[systems#Gates|Gates]]. _Blockers that stopped adoption:_ (1) the two-factor lock leaks position only when the victim dials a hunter-controlled gate, so an active trader can grow yet stay unraidable — **reopens the turtle-equilibrium trap**; (2) the gate-selection layer has **no cull** (nothing kills a bad-arena gate → drift, not selection), and uncontrolled Rifts plausibly come from unstable/bad gates, which inverts it. Revisit only with fixes for both.
- **#21 Engine choice — Unity vs. Unreal (vs. Godot).** Undecided; full comparison and the tip-factors in `raw/engine-comparison.md`. Blockers to resolve first: art identity (#7), whether the agent pipeline holds up (needs a throwaway prototype), the base object-count budget. When chosen → record in [[systems#Technical challenges — and the Gate mechanics that ease them|Technical challenges]] with the why and what was rejected.

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
- **Gate-as-bomb (overload)**: overloading a Gate yields a planet-scale explosion — a rare, costly scorched-earth / siege-ender (the overdrive-to-Rift physics).

### Catalogued — not yet integrated [open]

- **Dial-into-a-star** (route a tunnel through a sun; exotic super-weapon behind disabled safeties) · **Safeties as a tech resource** (research/disable Builder safety protocols for weapon-grade uses) · **Comms through a sealed Gate** (data/radio pass while the aperture is shut — diplomacy, ransom) · **Subspace relay / data network** (Gates carry data; control = information control) · **Recon probes** (scout before committing) · **Gate bridges** (chained call-forwarding — the cheap-distance basis for hubs) · **Harvested / relocatable Gates** (claim and move found orbital Gates) · **Reprogram-the-route** (a relay controller reroutes/traps traffic) · **Network self-update as a worm vector** · **Proximity priority conflicts** (a competing Gate suppresses a rival's) · **Time/exotic physics** (solar-flare time travel, black-hole dilation; the ~38-min cap already borrowed as the siege ceiling) · **EVE economy/access layer** (access lists, tolls, mass×distance fuel, a hard mass cap forcing big fleets onto Supergates, plantable beacons, area jammers, killable Gates).

### Implied objects [tentative, deferred]

- **Power core** — scavenged Gate fuel + the economy's hard currency; also portable mask fuel on your own soil. Does **not** extend the raid clock (away reserve is fixed by home-Gate power).
- **Gate control device** — the per-Gate console (power, dialing, targeting math); attackable, stealable (the "DHD-equivalent"; original name open).
- **Transponder / access codes** — distinguishes friendly trade from anonymous raiders; spoofable.
- **Gate segments** — pieces from which Supergates are reassembled and repaired.
- **Beacon** — a plantable landing site near a target (EVE cyno-beacon analog); does **not** extend away reserve — jump/landing point only.
- **Coordinate database** — the homebase log of discovered coordinates; a raidable asset (the map).
- **Jammer** — denies Gate-jumping across an area (EVE cyno-jammer analog).
- **Open:** weapons, armor, building materials, consumables, trade goods, artifacts.
