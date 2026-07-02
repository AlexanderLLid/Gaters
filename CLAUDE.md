# CLAUDE.md - Gaters

THIS IS A WORK IN PROGRESS — suggest changes to anything, anytime. **Nothing is locked**: green-field (no users, no shipped versions), so when something changes, change it in place as if it had always been that way — no backward-compatibility, migration notes, deprecation shims, or "supersedes the old X" history.
Never branch, never open PRs — work off `main`, and don't commit or push unless asked (the human reviews local commits first).

This is the **parent repo** for the game Gaters. Right now it holds **design docs
only** (in `docs/`). The game is added later as a sibling folder under this
root.

`docs/` is an Obsidian vault: two linked wikis under one schema.

- `docs/lore/` - the world bible: canon, factions, places, events (World Anvil types).
- `docs/systems/` - the mechanics bible: how the game works (survival content model).

You maintain the docs; the human curates sources and asks questions. This file is the schema.

Ponytail is active — apply it to the docs: the minimum page that does the job, no filler.

## Scope & priorities

Wiki mode is **active**: `docs/lore/` + `docs/systems/` are the living, maintained wiki. Design
material that isn't a lore/systems page lives in focused docs at the `docs/` root — `pillars.md`
(cores, design traps, validation), `archetypes.md`, `gate-uses.md`,
`technical-challenges.md` — and architecture in `docs/adr/`.

Scoped wiki: build pages for what's **decided or load-bearing**; stub or defer the rest. Don't
fabricate detail to fill a page.

- **In depth:** **gate physics** — the one place to go deep. Bind every claim to real physics
  and tag it GROUNDED / STRETCH / CONCEIT. See [[gate-physics|Gate Physics]].
- **Stubbed (link-out, not a full page):** still-open mechanics — taming, build depth, economy
  specifics, trade, progression numbers, the mystery's true answer, art identity. A short page,
  expanded when it settles.
- **Deferred entirely (no page; capture in `docs/raw/`):** specific characters, weapons, items,
  recipes, creatures, named places/factions, species, cultures, religions, myths, languages.
- **Wanted later:** decided-but-deferred features (e.g. space gates) live in `docs/roadmap.md`.

## Design order

How mechanics and lore get built: fix foundations before details, and stop
minting parallel systems. It blends three known ideas — design backward from the
player experience (MDA), build the core loop first, decide hardest-to-reverse
things first. Shorthand: "core loop first".

Build in this order — each layer depends on the one above, and a late change high
up reworks everything below.

1. Experience [current call] - reach equals exposure; aggressor fantasy, no defender
   tax. See [[World Overview]], [[pillars|Pillars]].
2. Core loop - moment / session / long-term. Documented in [[Systems Overview]];
   link there, do not restate.
3. Primary verb + seam - open / raid, routed through the [[Gates|Gate]]. The Gate
   is the highest-blast object (everything routes through it); pin it as early as
   the loop allows.
4. Resources (the knobs) - [[mask-energy|mask energy]] (field radius + away
   reserve), power cores, [[potential|Potential]], [[coordinates|Coordinates & Obscurity]].
5. Conflict - [[raiding|Raiding]], [[combat|Combat]], [[hub-worlds|Hub Worlds]].
6. Counterforce - the stagnation [[economy|economy]], [[progression|Region/World tiers]], charter / tithe.
7. Content - [[taming|Taming]], [[building|Building]], items, biomes, creatures.

Rules:

- **Knob inheritance.** A layer spends the tunable knobs defined upstream. A new
  knob lower down must say why an existing upstream knob cannot carry it, and stay
  decoupled. Never merge two knobs that fail independently — like a shooter keeping
  weapon damage and fire-rate as separate dials, not one "DPS" number.
- **Occam / additions must earn it.** Default answer to "add a new mechanic,
  system, or page?" is **no** — each one multiplies what must be balanced,
  linked, kept contradiction-free, and later coded. First walk _up_ the layer
  tree, all the way to the fiction: can a higher knob be retuned — or a story
  premise carry it — instead of bolting on a new mechanic? A diegetic answer the
  world explains beats a hard game-block players resent. Only if nothing upstream
  can carry it does an addition earn consideration, and then it must name the
  existing mechanic that _cannot_ do the job (knob inheritance, generalized).
  Prefer retuning over minting. When in doubt, stub and defer.
  - _E.g._ Subnautica keeps you from swimming off the map with no invisible wall:
    crushing depth and leviathans make the edge self-policing. The fiction does
    the work a hard boundary would otherwise need.
- **Balance is orthogonal, not a final layer.** When you define a layer, set its
  structural ratios (the shape: how steep a curve is, which value dominates) but
  defer calibrated values (exact rates, costs) to playtest. Never freeze balance
  numbers in prose. Open-vs-turtle is assume-and-commit, settled at scale (see
  [[pillars|Pillars]] Validation).
- **Commit order, not waterfall.** Treat each layer as provisionally fixed, build
  down, revisit up when a lower layer exposes a flaw above.
- **Discovery exception.** Bottom-up prototyping to find the fun is research, not
  construction. Gaters is past discovery (the fantasy is [current call]), so construction
  runs top-down.

## Follow recorded decisions

A page or proposal must conform to the decisions already recorded — link, don't
restate (where they live: see Decisions). The load-bearing one is the **balance
thesis**: the aggressor fantasy without the defender tax (see [[World Overview]];
design traps in `docs/pillars.md`).

**Nothing is a hard rule right now.** Every recorded decision — the concept-page rationale,
ADRs, the thesis, this schema — is provisional and open to change at this stage. A recorded
decision documents the current call and its _why_, not a lock. If a proposal conflicts with
one, **surface the conflict and the better reasoning and re-decide** — don't silently ignore
it, but don't treat it as immovable either. When the call changes, edit the page in place.

## Layers

1. `docs/raw/` - source material (design notes, transcripts, exports). Immutable.
   Read from it, never edit it. The ultimate source of truth.
2. `docs/lore/` and `docs/systems/` - the wiki pages you generate and maintain.

## Page types

Page types, frontmatter, and how to start a new page live in `docs/_templates/README.md`.

## Conventions

- **Bullets are the default.** Write docs as bullet points (nested where there's
  hierarchy), not prose paragraphs. Reserve prose for a short framing line above a
  list. Don't use inline enumerations like `(1)… (2)… (3)…` inside a bullet; split
  them into sub-bullets. Applies to every doc and skill that writes docs.
- Filenames: kebab-case matching the title. Content lives in folders by type
  (`docs/lore/organizations/`, ...); folders appear as pages are added.
- One subject per page. Cross-link with `[[Wikilinks]]`. Link multi-word pages as
  `[[basename|Display]]` so the link resolves.
- Anchor claims to sources: cite inline as `(src: raw/file.md)`. Prefer anchoring
  over memory; it stops canon and mechanics from drifting over many edits.
- Numbers live in data, not prose: document the model and say where tunable values
  will live. Split static (Definition) from runtime (Instance); prefer tags over
  rigid class trees.
- **Code section:** leave as a placeholder until the game project exists. Once it
  does, systems pages list the classes/paths that implement them.
- **No graveyard comments.** When removing a mechanic, don't leave "was X, now removed"
  narration on the pages that referenced it — just state the current model. Don't reference,
  explain, or keep commented-out versions of old/deleted concepts (in docs or, later, code) —
  delete them outright. If it leaves something unresolved (e.g. a trap's counter is gone), flag
  it **open** and record the why/what-broke once in `docs/open-questions.md`, not repeated
  inline everywhere it's touched.

## Contradiction handling

New content conflicting with a page: classify (soft / scope / hard) and record it
inline; **hard** blocks — don't pick a winner, stop and ask. Full protocol: the
**contradiction-check** skill.

Undecided _design questions_ are not contradictions — track those in
`docs/open-questions.md`. Never overwrite established canon in passing.

## Decisions

Maintained via /domain-modeling. Decisions live next to what they govern:

- **Design / world / mechanics decisions** — folded into the **concept page** that owns the
  topic (a `## Why / rejected` section: the call, the _why_, what was rejected), cross-referenced
  to sibling pages. No separate design-decision register.
- **ADR** (`docs/adr/`) — architecture / technical decisions (the game code, the
  repo, tooling); standard ADR.
- **Mechanic IDs** — mechanics carry **section-scoped IDs** (e.g. `SIEGE-1`, `ECON-2`),
  tagged on the section header of the concept page that owns them. IDs are local to a section
  so adding one never renumbers another. No separate registry file — the tagged headers are the index.

Record a decision only when it is hard to reverse, surprising without context, and a real
trade-off — i.e. when a future agent could plausibly re-make it _differently_. Capture the
_why_ and what was rejected; never record just to preserve history. `docs/open-questions.md`
is the unresolved backlog; a settled item is folded into its concept page and its page tag
flips to `[current call]` (a working call, never a lock).

A decision being **actively worked** lives in one holding doc (a `*-options.md` at the `docs/`
root) plus an `open-questions.md` entry. On resolution it folds into its concept page (or an
ADR for technical) and the holding doc is deleted. Don't promote a half-formed idea to a
standalone concept page.

## Index

`docs/index.md` is the catalog of every page with a one-line summary. Update on
every ingest; read first on every query. Activity history lives in git — there is
no separate changelog.

## Notes

- Markdown + Obsidian vault under git. Commits are the history.
- Keep this schema lean. Add a short rule when a mistake repeats, not an essay.
