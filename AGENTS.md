# AGENTS.md - Gaters

THIS IS A WORK IN PROGRESS — suggest changes to anything, anytime. **Nothing is locked**:
green-field (no users, no shipped versions), so when something changes, change it in place
as if it had always been that way — no backward-compatibility, migration notes, or
"supersedes the old X" history.

Don't branch, commit, or push unless asked (the human reviews changes first); in a remote
session, use the branch you're given.

This is the **parent repo** for the game Gaters. It holds the design docs plus the early
Unreal prototype under `Unreal/Prototype/`; `docs/` remains an Obsidian vault.

**Greybox is active** — the standing discipline for all doc and design work
(repo skill copies live under `.agents/skills/` and `.claude/skills/`): the minimum canon
that answers the design question.
Ponytail applies to any code.

## The files

Five content files. One subject = one **section**, not one file; a section graduates to
its own file only when it's `[current call]` _and_ outgrows its parent (~150 lines).
The whole canon must stay readable in one sitting.

- `docs/raw/` — **immutable source material** (design notes, transcripts, exports) plus
  filed **research / options reports**. Read from it, never edit it. The ultimate source
  of truth. New sources get folded into the files below on request.
- `docs/world.md` — the world bible: premise, thesis, factions, places, events, the
  mystery, the behind-the-curtain truth.
- `docs/systems.md` — the mechanics bible: pillars, traps, core loop, every mechanic,
  archetypes, validation, technical challenges.
- `docs/gate-physics.md` — the one designated **deep** page: every claim bound to real
  physics, tagged GROUNDED / STRETCH / CONCEIT.
- `docs/questions.md` — the backlog: open questions (stable `#N` IDs, never reused),
  parked explorations, deferred wants, option banks.
- `docs/INSPIRATION.md` — the design compass (references, styles, wants, counters);
  not canon.

Scope rule: build out what's **decided or load-bearing**; stub the rest with `[open]` +
one line. No speculative content — no named characters, factions, items, creatures,
species, cultures, religions, myths, or languages until a mechanic needs one.

## Design order

Fix foundations before details; stop minting parallel systems. Build in this order — each
layer depends on the one above:

1. Experience — reach equals exposure; aggressor fantasy, no defender tax ([[world]]).
2. Core loop — moment / session / long-term ([[systems]], documented once).
3. Primary verb + seam — open / raid, routed through the Gate (the highest-blast object).
4. Resources (the knobs) — mask energy, power cores, Potential, coordinates.
5. Conflict — raiding, combat, hub worlds.
6. Counterforce — the stagnation economy, tiers, charter / tithe.
7. Content — taming, building, items, biomes, creatures.

- **Knob inheritance.** A layer spends the knobs defined upstream. A new knob must name
  the existing upstream knob that cannot carry it, and never merge two knobs that fail
  independently.
- **Additions must earn it** — the greybox ladder enforces this: retune upstream or answer
  with one line of fiction before minting a mechanic. When in doubt, stub and defer.
- **Prefer game-state mechanics.** When exploring a mechanic, first ask whether ordinary
  game data can carry it: ownership, logs, routes, claims, damage, cargo class, heat,
  presence, deaths, scans, or server-authoritative events. A new hidden meter or bespoke
  state needs a reason existing state cannot express it.
- **Balance is orthogonal.** Set structural ratios (shapes, which value dominates); defer
  calibrated values to playtest. Numbers never live in prose — name the tunables, say the
  values land in data.
- **Commit order, not waterfall.** Each layer is provisionally fixed; revisit up when a
  lower layer exposes a flaw above.

## Decisions & contradictions

- Decisions live **inline**, next to what they govern: a `### Why / rejected` block under
  the owning section (the call, the _why_, what was rejected). No separate register.
  Record one only when it's hard to reverse, surprising without context, and a real
  trade-off — otherwise skip.
- Mechanics carry **section-scoped IDs** (e.g. `SIEGE-1`, `ECON-2`) on the bullet or
  header that owns them; the tagged text is the index.
- Inline tags say how settled a claim is: **[current call]** (the call we'd make today +
  its why — never locked), **[tentative]** (leaning), **[open]** (unresolved, tracked in
  `questions.md`).
- **Never silently change a recorded call.** New content that directly contradicts one
  blocks: stop and ask, don't pick a winner. Undecided design questions are not
  contradictions — they go to `questions.md`.
- **No graveyard comments.** Removing something leaves the current model only — no
  "was X, now removed" narration, no commented-out old concepts. If a removal leaves
  something unresolved, flag it `[open]` and record the why once in `questions.md`.

## Conventions

- **Bullets are the default**; prose only as a short framing line above a list.
- **Naming: plain, globally understood English words.** A canonical term must be readable
  by a non-native player without a dictionary — no region-bound or legalistic register
  (charter, writ), no borrowed genre nouns (the IP line in [[systems]]). Test: does the
  word work in a sentence said by a 14-year-old in any country?
- Cross-link with heading wikilinks: `[[systems#Raiding|Raiding]]`, `[[world#The mystery]]`,
  `[[questions]]`, same-file `[[#Combat|Combat]]`.
- Anchor claims to sources: cite inline as `(src: raw/file.md)`. Prefer anchoring over
  memory.
- Split static data (Definition) from runtime state (Instance); prefer tags over rigid
  class trees.
- Generated content boundary: versioned recipes, contracts, and source artifacts are
  authoritative; Unreal assets and Actors are derived outputs behind adapters.

## Skills

- `.agents/skills/` is authoritative; `.claude/skills/` is a byte-identical discovery
  mirror. Update both together and run `research/Test-SharedAgentDocs.ps1`.
- **greybox** — the standing mode; runs on every doc/design task.
- **explore** — the session loop for working a question: options → grill → decide →
  fold in.
- **improving-workflows** — postmortem for a followed workflow that still failed;
  records shared evidence and proposes the smallest skill correction.

Keep this schema lean. Add a short rule when a mistake repeats, not an essay.

## Parallel workstreams

- For work in a long-lived chat, follow `.agents/workstreams/README.md`; it alone defines
  roles, ownership, startup, exchange, and closeout.
- Dated files under `.agents/plans/` and `.agents/reports/` are execution snapshots and
  evidence, not current status. Current capability status comes only from
  `research/machines.json`; current coordination comes only from `.agents/workstreams/`.

### Builder stance

For capability or implementation work, invoke `finding-magic-machines`; its skill owns
the method. A builder closes work only after independent evidence either promotes the
machine or records the falsified guarantee and next isolated experiment.

## Research registry

- `research/machines.json` is the single source of truth for magic-machine contracts,
  dependencies, status, verification, champions, and target waves. Reports and plans may
  cite machine IDs but never copy the graph.
- Run `research/Show-MachineRegistry.ps1 -Format Summary` for the current build waves,
  `-Format Mermaid` for an overview, and `research/Test-MachineRegistry.ps1` after edits.
- Machine changes follow `.agents/workstreams/README.md`: integrate the registry or raise
  an `INTEGRATE` exchange. Registry edits must pass
  `research/Test-MachineRegistry.ps1`.
