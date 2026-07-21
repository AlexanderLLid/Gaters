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
- `docs/rift-rules.md` — the one designated **deep** page: consistent magical Rift
  behaviour, gameplay consequences, and explicit open rules.
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
3. Primary verb + seam — call / enter / raid through temporary Rifts; Anchors shape
   arrival (the highest-blast objects).
4. Resources (the knobs) — Rift cost [open], Potential, traces, and route knowledge.
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
- **Resolve repository contradictions in place.** Existing text is not locked. When
  repository sources contradict and the intended winner is clear, any chat may update
  every affected current source so one coherent model remains. If the winner is unclear,
  ask one concrete question. Resolution is complete only when all affected authoritative
  sources agree; undecided design questions go to `questions.md`.
- **Make user conflicts concrete.** When a user instruction contradicts written material,
  ask before choosing: "`X` conflicts with `Y` because `Z`. Which should govern?" Name
  the actual statements and causal incompatibility, not merely that a contradiction
  exists. After the answer, update every affected current source in place.
- **No graveyard comments.** Removing something leaves the current model only — no
  "was X, now removed" narration, no commented-out old concepts. If a removal leaves
  something unresolved, flag it `[open]` and record the why once in `questions.md`.

## Conventions

- **Bullets are the default**; prose only as a short framing line above a list.
- **Naming: plain, globally understood English words.** A canonical term must be readable
  by a non-native player without a dictionary — no region-bound or legalistic register
  (charter, writ), no borrowed genre nouns (the IP line in [[systems]]). Test: does the
  word work in a sentence said by a 14-year-old in any country?
- **One concept, one canonical noun.** Use the same noun in UI, design docs, data schemas,
  and code identifiers. Namespaces, type prefixes, and localization may adapt syntax;
  they must not introduce a second semantic name for the same concept.
- Cross-link with heading wikilinks: `[[systems#Raiding|Raiding]]`, `[[world#Rifts and Anchors|Rifts and Anchors]]`,
  `[[questions]]`, same-file `[[#Combat|Combat]]`.
- Anchor claims to sources: cite inline as `(src: raw/file.md)`. Prefer anchoring over
  memory.
- Split static data (Definition) from runtime state (Instance); prefer tags over rigid
  class trees.
- **Requirements check:** after drafting a plan and before executing it, read
  `docs/systems.md#Shared requirements`. Check Global and every subject the plan creates,
  changes, or depends on. Update the plan until every applicable `MUST` is satisfied;
  record justification for each `SHOULD` exception. Handle request conflicts with the
  concrete question under [[#Decisions & contradictions]]. Complete the check with
  `Requirements checked: <IDs>; exceptions: <IDs or none>`. Requirement text lives only
  in the shared section; reference it by ID elsewhere. When the human states or changes
  a cross-workstream product requirement or acceptance criterion, add or update it there
  in the same task using that section's compact format. Propose agent-inferred
  requirements instead of silently adding them.
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

- **Interactive Unreal coordination.** `Unreal Runner` is the only task allowed to launch
  UnrealBuildTool, `UnrealEditor-Cmd`, commandlets, Automation tests, Unreal Editor, or
  Live Coding in the shared checkout. Other tasks send it
  `UNREAL RUN: <requester> | <exact command> | <purpose>` and continue non-Unreal work.
  When task messaging is available, send that line directly to `Unreal Runner`; human
  relay is only the fallback when task messaging is unavailable.
  The Runner reports the active requester, command, PID, and queue; it serializes
  agent-owned operations without asking the human. If the human Editor blocks a needed
  run, only the Runner asks once for it to be closed. No task closes, dismisses, attaches
  to, or terminates a process or dialog.

## Research registry

- `research/machines.json` is the single source of truth for magic-machine contracts,
  dependencies, status, verification, champions, and target waves. Reports and plans may
  cite machine IDs but never copy the graph.
- Run `research/Show-MachineRegistry.ps1 -Format Summary` for the current build waves,
  `-Format Mermaid` for an overview, and `research/Test-MachineRegistry.ps1` after edits.
- Machine changes follow `.agents/workstreams/README.md`: integrate the registry or raise
  an `INTEGRATE` exchange. Registry edits must pass
  `research/Test-MachineRegistry.ps1`.
