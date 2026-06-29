# Open Questions & Conflicts

The project's living decision backlog. Unlike the contradiction discipline (which
handles canon/mechanics conflicts _between pages_), this tracks **undecided design
questions** — things not yet settled.

Resolve an item → fold the decision (its _why_ + what you rejected) into the relevant
**concept page** (a `## Why / rejected` section), flip that page's tag
`[tentative]→[decided]`, and strike it here. Use /domain-modeling. (Technical / code choices
still get an **ADR** in `docs/adr/`.)

Decided so far (folded into the concept pages — see each page's _Why / rejected_): the game is
named after its players, reach-equals-exposure, players-are-human ([[World Overview]]); Coalition
as patch channel + certified provisioning, the mandate, mesh-with-a-heart topology
([[united-gate-coalition|United Gate Coalition]]); the gate dome ([[combat|Combat]]); aperture driven, not
built ([[Gates]]); the raid clock ([[raiding|Raiding]]); asymmetric uphill reach + Potential + tribe
ownership ([[potential|Potential]]); exposed holdings ([[holdings|Holdings]]); consciousness-link /
avatars / two-layer XP ([[consciousness-link|Consciousness Link]]); mortal home / obscurity /
avatar-shares ([[coordinates|Coordinates & Obscurity]]); Rifts as the PvPvE extraction layer
([[rifts|Rifts]]).

> **Canon note.** The home is **mortal**, protected by **obscurity** (a re-keyable avatar-share
> trail), not a wall; only your _existence_ (body-on-Earth) is unloseable. See
> [[coordinates|Coordinates & Obscurity]] and [[consciousness-link|Consciousness Link]].

## Conflicts / tensions to resolve

1. ~~**Raid-clock governance**~~ — resolved: one clock, the attacker's away reserve, fixed by the attacker's home Gate power and **not** extendable by carried fuel. ([[raiding|Raiding]], [[mask-energy|Mask Energy]])
2. ~~**Siege-timer number**~~ — resolved: not one value but a curve of Gate power, capped by the ~38-min wormhole ceiling. ([[raiding|Raiding]])
3. **"Gates unbuildable" vs. more Gates over time** — reconcile relocate/reassemble/stabilize-Rifts/saved-coordinates under one umbrella rule. ([[Gates]], [[rifts|Rifts]]) _Size axis resolved: ring fixed, aperture driven — no fabrication. Rift-stabilization pacing still open (see 10)._
4. **Stagnation pressure vs. "no obligation" promise** — strong enough to push players out, not so strong it's a disguised upkeep tax. ([[economy|Economy]]) _Homesteader sub-case resolved (PROG-2): sealed play hits a soft ceiling and plateaus — a ceiling, not a leak — so the never-opener is never taxed (see [[progression|Progression]]). The general faucet/sink line still open._
5. **Raider anonymity vs. the central registry** — "no caller ID" vs. the [[united-gate-coalition|United Gate Coalition]] knowing who holds what. _Narrowed:_ the Coalition **holds your body** (see [[consciousness-link|Consciousness Link]]), so it always knows your identity — anonymity is purely **peer-facing** (other players don't see caller ID), never Coalition-facing. The open part is only what peers can/can't trace.
6. ~~**Hub zerg problem**~~ — resolved: hubs become **lawless commerce/routing** (capturable for tolls), the open PvP contest moves to **Rifts** (visibility by Potential) **and space gates**, so the many-vs-many chaos is no longer hosted _at the hub_. ([[hub-worlds|Hub Worlds]], [[rifts|Rifts]])
7. **"Sealed = isolated" vs. "comms pass through a sealed Gate"** — confirm the split (info yes; goods/energy/people no).
8. **NMS-aspirational visuals vs. low-poly-achievable-solo** — two target identities; look is deferred.
9. **Title "Gaters"** — phonetics ("gators" / "gaiters"), searchability, harassment risk. (Player term settled: gater.)
10. **Player-driven Rift-stabilization vs. Coalition-paced expansion** — who controls world-growth pacing? ([[rifts|Rifts]], [[united-gate-coalition|United Gate Coalition]])
11. **Field radius and away reserve must stay decoupled** — standing rule; an "expand the field" patch must never lengthen the raid clock. ([[mask-energy|Mask Energy]])
12. ~~**Power-potential computation**~~ — resolved: potential is a sticky high-water-mark of footprint (not live draw), and the uphill _tier-gap_ cost is a separate term from distance. ([[potential|Potential]])
13. ~~**Solo vs. a mustered home defence**~~ — resolved via the **muster cap (COMBAT-2)**: the dome admits only K combat-effective defenders at the lip (K scales with attacker throat power), so a skilled solo can win the _breach_ against a muster — but not the open field, where the perimeter and the away-reserve clock still rule. ([[combat|Combat]])
14. **Long-distance dialing may be too strong** — reaching very far in a single jump might be OP; a likely mitigation is **jump / relay servers** (hop-routing through intermediary nodes, cf. the "Gate bridges" catalogue item) rather than cheap direct long jumps. _Parked — revisit later._ ([[economy|Economy]], [[hub-worlds|Hub Worlds]])

## Biggest open design questions

1. **Is the core fun real?** Will players actually open their Gates? (The greybox must answer.)
2. ~~**What governs the raid clock**~~ — decided: the attacker's away reserve, fixed by home-Gate power, un-pumpable. ([[raiding|Raiding]])
3. **How hard should stagnation push** without becoming upkeep?
4. **Which mystery truth** is canon (outside signal / tripwire / Coalition lie)? ([[the-wake|The Wake]]) (Coalition's motive now decided — threat-to-Earth reconnaissance, see [[united-gate-coalition|United Gate Coalition]] — which colours but does **not** pick the truth; the three candidates stay open.)
5. ~~**Combat model**~~ — resolved (COMBAT-1/COMBAT-2): third-person gear-and-positioning; the mask _is_ the combat resource (away reserve = clock + durability); aperture as a two-way fire-lane; muster cap for solo-vs-mustered; precursor-tech-hybrid spine. Numbers (TTK, K-values) stay open as data. ([[combat|Combat]])
6. **Final art identity** (NMS-lush vs. low-poly-stylized).
7. **Survival meters beyond mask energy** — any, and how minimal?
8. **Raider anonymity + central registry** coexistence.
9. **Does "Gaters" survive** the searchability/phonetics concern? (Player term settled: gater.)
10. **Can Rifts be stabilized into Gates** — player-driven or Coalition/tech-gated? ([[rifts|Rifts]])
11. **Rift specifics** — rank scale (E–S vs. 1–10), spawn location, ignore-it downside, Rift-loot vs. raid-loot balance. ([[rifts|Rifts]])
12. **Mask/field specifics** — gradient vs. hard radius; field radius Coalition-only vs. capped upgrade; confirm asymmetric mask-at-zero. ([[mask-energy|Mask Energy]])
13. ~~**Lock "Coalition as Patch Channel" as canon?**~~ — decided: yes; Gate availability = Coalition-certified provisioning. ([[united-gate-coalition|United Gate Coalition]])
14. **Why did the Builders forbid walling a Gate?** The dome (see [[combat|Combat]]) is a deliberate anti-enclosure feature — a Gate that can never be entombed can always be reached _through_. Mystery hook; rhymes with #4. Left unanswered per Builders-unknown.
15. **Is Earth the only species the Gates woke for?** _Parked; canonical answer-in-waiting._ The
    synchronized Wake plausibly lit **many** species' Gates at once — making the simultaneity itself
    evidence the threat is real, and scaling the natural-selection frame into three rings (player vs.
    player → humanity vs. other woken species → all vs. the threat — see [[united-gate-coalition|United Gate Coalition]]). **Correction:** _not_
    "rival Coalitions" — a Coalition is a **human** institution, contingent; alien evolution would
    not converge on it. Rivals at scale are **alien polities that answered the Wake differently** (a
    hive, a sovereign, a machine intelligence, or no central body at all). Open fork: **do other
    species hold bodies / respawn (see [[consciousness-link|Consciousness Link]]), or do they really die?** — this decides whether
    "only avatars die" is a human edge or a human weakness. If taken, soften the "threat to
    Earth" framing → "threat to the woken worlds." Touches players-are-human ([[World Overview]]), [[united-gate-coalition|United Gate Coalition]], [[consciousness-link|Consciousness Link]], the Species section,
    and deepens #4. Roadmap, not launch.
16. ~~**Sealed progression ceiling**~~ — resolved (PROG-2): a peaceful homesteader progresses _forever_ fully sealed up to a **soft ceiling** (one `sealed-reachable` boolean per tech node; frontier tiers flip to `false`), then plateaus — never decays, never punished. Stagnation as a ceiling, not a leak. The homesteader's **safety** is now **being unfound** — a pure-PvE never-leaver sheds no avatar-shares, so stays unfound; the plateau still holds. ([[progression|Progression]], [[coordinates|Coordinates & Obscurity]], [[rifts|Rifts]])
17. **Theme/genre fork — sci-fi, fantasy, or precursor-tech hybrid?** _Leaning now adopted in the combat spine (COMBAT-1): bow/melee → scavenged Builder tech → fleets, with the tamer fork resolved narrow (TAME-1). The fork stays listed because committing the **whole** game to precursor-tech-hybrid as canon (art identity #6, setting/tone register) is broader than the combat model and not yet formally signed off._ Should players wield creatures/bows/melee rather than (or alongside) the sci-fi frame? Three paths, by what each licenses:
    - **Pure sci-fi** (status quo) — keeps everything; leaves #5, #6 and the tamer fork open.
    - **Pure fantasy reskin** — cheaper art, broader familiarity, but discards the [[gate-physics|Gate Physics]] depth CLAUDE.md says to go deep on (the stable/unstable-wormhole cosmology, ~38-min siege ceiling, Ford–Roman aperture cap are all borrowed physics that arrive pre-balanced), and breaks the space-scale endgame (fleets, Supergates).
    - **Precursor-tech hybrid (leaning)** — Gate stays sci-fi Builder tech; **players start low-tech and climb by scavenging Builder tech**. "Lower life uses Gates to become greater" becomes the progression spine. Licenses: low-poly art as _canon not budget_ (resolves #6); the climb _is_ the tech tree + combat model (bow/melee → scavenged tech → fleets; resolves #5); creatures/taming fork and the already-Solo-Leveling-coded [[rifts|Rifts]] (monster dungeons, surface incursions); moves further from Stargate (the decided IP line). **Scope guards:** "easy to use" must mean _traverse_ (step through an open Gate), not _master_ (dialing/aperture/control device stay hard — maps onto the Standard→Heavy→Supergate ladder); fleet-scale combat sits at the _top_ of the climb, not beside the bow start (fine — space Gates already deferred). Pressures #15 and the Setting & Tone register (currently `[open]`). Couples to players-are-human ([[World Overview]]), #5, #6, #15, the tamer/breeder open fork.
18. ~~**Can coordinates go stale / be re-keyed?**~~ — resolved: yes. Coordinates are **epoch-bound and re-keyable**; re-keying invalidates collected avatar-shares. Re-key cost/cadence is the master knob for the online:offline raid ratio. ([[coordinates|Coordinates & Obscurity]])
19. ~~**Solo vs. tribe as the unit of ownership / Potential**~~ — resolved: the **tribe** is the ownership/Potential primitive; a solo player is a **tribe of one**. One shared coordinate; Potential is the tribe's; every member sheds shares of it (so big tribes are inherently findable). ([[potential|Potential]])
20. **Return-home when the home was wiped mid-Rift** — Rift exit is one-way to your own home; if that home was destroyed while you were inside, where do you land? Likely a starter/operator anchor — undefined. ([[rifts|Rifts]], [[coordinates|Coordinates & Obscurity]])
21. **Tribe shared-ownership sub-rules** — the tribe owns one home/coordinate; the sharing mechanics are open: **who may re-key** the shared coordinate (any member vs. leader-only) and what happens if they're offline while the tribe is being found; **member churn** — does leaving/being kicked force a re-key, and can a defector sell the coordinate; **defense** — who may recall to the shared home and how the muster cap (COMBAT-2) counts a tribe vs. a solo. ([[potential|Potential]], [[coordinates|Coordinates & Obscurity]], [[consciousness-link|Consciousness Link]])
22. **Gate-DNA findability overhaul** — _explored 2026-06-29, **not adopted**; parked._ A session explored replacing avatar-shares with a **two-factor lock** (dialing a gate needs both its **position** _and_ its **DNA**), **re-key = mutating a Gate's config-genome**, leak via goods (DNA) + connecting (position), plus a **gate-selection** layer (probes breeding gates). _Survivor, kept:_ gate-config-as-genome explaining gate variety — folded tentatively into [[gates|Gates]]. _Blockers that stopped adoption:_ **(1)** the two-factor lock leaks position only when the victim dials a hunter-controlled gate, so an active trader can grow yet stay unraidable — **reopens the turtle-equilibrium trap**; **(2)** the gate-selection layer has **no cull** (nothing kills a bad-arena gate → drift, not selection), and Rifts plausibly come from _unstable/bad_ gates, which inverts it. Revisit only with fixes for both. Touches [[coordinates|Coordinates & Obscurity]], [[rifts|Rifts]], [[potential|Potential]], [[the-wake|The Wake]].
