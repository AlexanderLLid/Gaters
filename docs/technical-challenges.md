# Technical Challenges — and the Gate Mechanics That Ease Them

What is hard to _build_ about a game like this, and which gate lore/mechanic pays each
bill. A focusing doc, like [pillars](pillars.md); full design lives in
[design-overview](raw/design-overview.md) and the concept pages.

> **The core insight.** The Gate is an **engineering device dressed as lore.** Almost
> every architectural decision in [design-overview](raw/design-overview.md) (Multiplayer /
> Networking, Tech Stack) is a hard problem of survival-PvP-MMOs that the Gate fiction
> _removes_ rather than solves. So the lens isn't "what's hard" — it's "what's hard _after_
> the Gate already paid for most of it." The design's own [validation plan](raw/design-overview.md)
> says the top risk is **not technical** — it's whether players will open their Gates. This
> doc scopes the technical residue under that.

---

## Part A — problems the Gate fiction removes (bank these)

Each is a genre-standard hard problem the design has already traded away. Don't re-solve
them; just don't break the fiction that pays for them.

- **Seamless space-to-surface flight** → **Gate loads.** NMS's hardest engineering problem
  is sidestepped: you never fly planet-to-space continuously, you dial and load (src:
  raw/design-overview.md, Aesthetic).
- **A persistent shared world to simulate 24/7** → **presence requirement + sealed-by-default.**
  A Gate is open only while its owner is online and chose it. Offline = sealed = nothing to
  simulate. No offline base sim, no decay tick, no sleeping-base economy. The single biggest
  server saving, and it's a _gameplay pillar_ ([[World Overview|reach equals exposure]], Presence
  Requirement).
- **An authority transition between solo and contested play** → **server-authoritative always.**
  One authority owns each planet at all times, so there is no transition between authority models
  to reconcile or exploit. The cost cap comes from the presence requirement: only online players'
  planets are simulated, and solo/sealed building — having no adversary — ticks cheaply (src:
  raw/design-overview.md, Multiplayer).
- **World-size-proportional cost** → **three-state planet + seed-plus-deltas.** A planet is a DB
  row (**empty**) → a cheap server-side tick (**solo**) → a full spun-up instance
  (**contested**). Server cost ∝ online players, not world size (src: raw/design-overview.md,
  Multiplayer).
- **Massive concurrent battles as the default** → **bilateral directional tunnels.** A raid is a
  two-party instance, not an N-body shared zone. Trivially shardable; no thundering-herd
  simulation in the common case (Directional Bilateral Tunnels, anti-zerg).
- **Unbounded session / instance lifetime** → **away-reserve clock, ~38-min ceiling.** Contested
  instances are short-lived by fiction, so they're cheap to spin up and tear down and never leak
  ([[raiding|Raiding]], [[Gate Physics]] §Time).
- **Bounding the simulated volume** → **mask field + the dome.** Only the area around an _active_
  Gate needs simulating; the field radius is the soft world border and the dome bounds the combat
  arena (src: raw/design-overview.md, Sustaining Field; [[combat|the dome]]).
- **Infinite/contiguous procedural universe** → **discrete, seedable planets loaded one at a
  time.** Each planet is an isolated bounded chunk behind a `PlanetGenerator` abstraction (src:
  raw/design-overview.md, Tech Stack).

---

## Part B — the technical residue, ranked, with the lever for each

What the Gate fiction does _not_ already pay for. Ranked by severity for a **solo Unity/C# dev**.

### 1. Two worlds, live, through an open Gate — _highest_

- **The problem.** The design wants breaching to be "a firefight at the aperture, not a loading
  screen": weapons fire and momentum **pass through** a live Gate (src: raw/design-overview.md,
  The Gate System; [[Gate Physics]] §What passes through). That means rendering and simulating
  **two planets at once** through a portal — the Portal/Prey rendering problem, plus two physics
  scenes, plus netcode spanning them. Genuinely hard in Unity solo.
- **The lever:**
  - **The dome bounds the portal** ([[combat|the dome]]). You never render the whole
    far planet — only what's visible through the throat, and the dome caps that radius. Small
    aperture = small portal frustum = bounded second-scene render.
  - **The aperture is the only shared volume.** Both sides simulate their own planet; only the
    throat region is co-simulated. Scope the expensive dual-sim to a sphere around the aperture.
  - **Graceful-degrade fallback:** if true portal rendering is too much for the greybox, the
    aperture degrades to a shared staging volume both sides load into — keeps the "fight at the
    door" feel without full two-world rendering. Greybox can ship the cheap version first.

### 2. Networked physics across an authority boundary — _high (subset of #1, but distinct)_

- **The problem.** "Momentum conserved through the Gate" means a physics object leaves one
  planet's authority and enters the other's mid-flight. Ownership transfer of live physics
  objects across a tunnel is a classic netcode trap (jitter, double-ownership, desync).
- **The lever:** the **aperture is the single transfer plane** — ownership transfers at one
  well-defined surface, not anywhere. The **mass cap** (Gate Uses / EVE layer) bounds how many
  such objects exist at once. Treat the throat as the one transfer point and most of the generic
  difficulty collapses.

### 3. Instance orchestration, cold-start, spin-up latency — _medium_

- **The problem.** Spinning up a contested instance the moment a raid commits, then tearing it
  down — on demand, cheaply, fast enough that the player doesn't feel a stall. Real ops work for
  one person.
- **The lever:** the **dial → tunnel-open sequence is a provisioning window** — the diegetic
  "dialing" animation is cover for instance cold-start. Both endpoints are known in advance
  (directional tunnel), uphill-only ([[potential|Potential]]) shrinks the matchmaking
  pool, and the ~38-min cap means instances are short and self-terminating. Standard container
  orchestration fits; the fiction hides the seams. (For the greybox, a listen-server — one side
  hosts the bilateral instance — removes orchestration entirely; see the morning checklist.)

### 4. The one irreducible shared state — the registry — _medium_

- **The problem.** Claims, coordinates, who-holds-what, the [[Central Authority]] guarantee: a
  **global, consistent, persistent** dataset. It's the one component instancing can't remove, and
  it couples to raider anonymity (Conflict #5 — "no caller ID" vs. a registry that knows everyone).
- **The lever:** it's a **database problem, not a simulation problem** — rows and queries, not
  60Hz physics. The Authority _is_ the natural home for centralized state (lore = the schema).
  Anonymity is solvable in the same layer: the registry knows owners, but the _raid tunnel_ need
  not expose the attacker's coordinate until commit (transponder model). Keep the registry small
  (metadata only); never let simulation state into it.

### 5. The many-vs-many exception — hubs, Open Rifts, server events — _medium, but cappable_

- **The problem.** [[Hub Worlds]], Open Rifts, and server-wide top-rank Rifts are _explicitly_
  many-vs-many (src: raw/design-overview.md). This is where "instancing makes it cheap" breaks —
  the expensive concurrent-simulation case the rest of the design avoids. Hub _balance_ is also
  flagged unsolved (Conflict #6).
- **The lever:** these are **few, optional, and bounded.** Hubs can be hard-capped in concurrency;
  Open Rifts are **temporary by fiction** (they collapse on a countdown) so they tear down on their
  own and never become permanent shared worlds. Scope them as the deliberate, budgeted exception —
  and keep them out of the greybox (deferred by the roadmap).

### 6. Procedural generation breadth — _medium, already deferred_

- **The problem.** Many varied planets, reproducible from seed. Content breadth is a time sink for
  a solo dev even with good tooling.
- **The lever:** the **`PlanetGenerator` abstraction from day one** (src: raw/design-overview.md,
  Tech Stack) makes this a single-class swap — greybox ships a trivial generator; richness lands
  later without touching the rest. Field radius bounds how much of each planet must be generated.

### 7. The web export's real ceiling — _low-medium_

- **The problem.** The plan is native-first with a **web export as a playtest funnel** (src:
  raw/design-overview.md, Tech Stack). Unity WebGL can't match native on threading, memory, and
  networking transport — the funnel build can't be the full game.
- **The lever:** scope the web build as a **demo slice**, not parity — a single planet, a single
  raid, no heavy orchestration. The instancing model helps: a web client can host/join one
  bilateral instance without the full backend.

### 8. The meta-risk — solo scope — _the real top risk_

- One person building portal rendering + on-demand orchestration + procedural gen + combat +
  netcode is enormous, _even after_ the Gate removes the worst of it.
- **The lever is the design's own discipline:** "design for success enabling expansion, not
  requiring it" — a working small version is a complete thing, not a broken large one (src:
  raw/design-overview.md, Validation Plan). Every item above has a **named cheap path** for the
  greybox; the residue is real but each piece has a smaller first version.

---

## Part C — lore to lock _because it pays an engineering bill_

Fiction choices that double as architecture. Worth deciding early — each closes a technical hole,
not just a story one. Surface in the concept pages when confirmed.

- **"Dialing is a sequence, not an instant"** — the dial animation is the instance-provisioning
  window. Pays #3 (cold-start).
- **The dome radius as the render/sim bound** — already the [[combat|dome]]; note
  explicitly that it also caps the portal frustum and dual-sim volume (#1).
- **Mass cap / manifest on what crosses** — already catalogued (Gate Uses, EVE layer). Bounds the
  count of physics objects transferring at the aperture (#2).
- **The Authority registry as the one sanctioned global DB** — centralize _all_ unavoidable shared
  state into the Authority fiction; everything else stays instanced. Pays #4.
- **Open Rifts collapse on a countdown** — already canon (src: raw/design-overview.md, Rifts);
  note it doubles as the auto-teardown that keeps the many-vs-many case bounded (#5).

---

## What the greybox must prove (technical subset)

The [validation plan](raw/design-overview.md) names the _fun_ question (will players open their
Gates). The technical questions it should also answer, cheaply, in order:

1. The **aperture fight** (#1/#2): does "firefight at the door" work at all? Ship the degraded
   shared-staging version first; defer true portal rendering.
2. The **on-demand instance** (#3): open → fight → tear down, end to end, once. A listen-server is
   the cheapest first version.

Everything else (procgen breadth, web export, hubs, space Gates) is deferred by the roadmap and
should stay out of the first proof.

---

## See also

- [pillars](pillars.md) — the cores that must work (this doc is their build-cost shadow).
- [design-overview](raw/design-overview.md) — Multiplayer / Networking, Tech Stack, Validation Plan.
- [open-questions](open-questions.md) — Conflicts #5 (anonymity vs. registry), #6 (hub zerg).
- [[Gate Physics]] — what passes through, the dome, aperture-driven sizing.
- Concepts: [[combat|the dome]], [[raiding|the raid clock]] (clock ceiling),
  [[potential|Potential]] (matchmaking pool).
