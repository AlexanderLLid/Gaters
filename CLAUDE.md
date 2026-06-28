# CLAUDE.md - Gaters

THIS IS A WOP FEEL FREE TO SUGGEST CHANGES TO ANYTHING ANYTIME. Nothing is set in stone.

This is the **parent repo** for the game Gaters. Right now it holds **design docs
only** (in `docs/`). The Unity game is added later as a sibling folder under this
root — there is no game code here yet.

`docs/` is an Obsidian vault: two linked wikis under one schema.

- `docs/lore/` - the world bible: canon, factions, places, events (World Anvil types).
- `docs/systems/` - the mechanics bible: how the game works (survival content model).

You (the agent) maintain the docs. The human curates sources and asks questions.
This file is the schema; read it at the start of every session.

Ponytail is active. Apply it to the docs: write the minimum page that does the
job, link instead of duplicating, no filler sections.

## Scope & priorities

Wiki mode is **active**: `docs/lore/` + `docs/systems/` are the living, maintained layer;
`docs/raw/design-overview.md` is now a **frozen founding source** (read, cite, don't edit in
place). On any conflict with the frozen overview, the concept pages win.

Scoped wiki: build pages for what's **decided or load-bearing**; stub or defer the rest. Don't
fabricate detail to fill a page.

- **In the wiki now:** the **Builders** and **Central Authority** (overview depth — frame what's
  decided, don't elaborate), and the **locked mechanics** the pivot settled (exposure states,
  mask/field, raid clock, bilateral tunnels, Rifts-as-extraction, consciousness-link,
  mortal-home/obscurity, Potential, holdings, hub worlds, combat).
- **In depth:** **gate physics** — the one place to go deep. Bind every claim to real physics
  and tag it GROUNDED / STRETCH / CONCEIT. See [[gate-physics|Gate Physics]].
- **Stubbed (link-out, not a full page):** still-open mechanics — taming, build depth, economy
  specifics, trade, progression numbers, the mystery's true answer, art identity. A short page
  pointing at the overview, expanded when it settles.
- **Deferred entirely (no page; capture in `docs/raw/`):** specific characters, weapons, items,
  recipes, creatures, named places/factions, species, cultures, religions, myths, languages.
- **Wanted later:** decided-but-deferred features (e.g. space gates) live in `docs/roadmap.md`.

## Follow recorded decisions

A page or proposal must conform to the decisions already recorded — link to
them, don't restate them. Each design/mechanics decision lives in its **concept page**
(a `## Why / rejected` section); the load-bearing one is the **balance thesis** — the
aggressor fantasy without the defender tax (see [[World Overview]]; the Four Design Traps
live in raw/design-overview.md).

**Nothing is a hard rule right now.** Every recorded decision — the concept-page rationale,
ADRs, the thesis, this schema — is provisional and open to change at this stage. A recorded
decision documents the current call and its _why_, not a lock. If a proposal conflicts with
one, **surface the conflict and the better reasoning and re-decide** — don't silently ignore
it, but don't treat it as immovable either. When the call changes, edit the page in place
(see Nothing is locked).

## Layers

1. `docs/raw/` - source material (design notes, transcripts, exports). Immutable.
   Read from it, never edit it. The ultimate source of truth.
2. `docs/lore/` and `docs/systems/` - the wiki pages you generate and maintain.
3. `CLAUDE.md` - this schema, co-evolved with the human.

## Page types

Every page starts with YAML frontmatter; `type` is required.

Lore (World Anvil set): character, location, organization, species, culture,
religion, myth, item, language, event, plus overview and timeline.

Systems (survival model): system, item, resource, recipe, station, structure,
creature, biome, status-effect, tech, formula, plus overview.

```yaml
---
type: organization
status: draft # draft | stable | needs-review
tags: []
sources: [] # provenance into docs/raw/
aliases: [] # add the Title-Case display name for multi-word pages
updated: 2026-06-27
---
```

Copy the matching file from `docs/_templates/`. Do not invent new section
structures per page.

## Conventions

- **Bullets are the default.** Write docs as bullet points (nested where there's
  hierarchy), not prose paragraphs — it's far easier to read and scan. Reserve prose
  for a short framing/intro line above a list. Avoid inline enumerations like
  `(1)… (2)… (3)…` inside a bullet; split them into sub-bullets. Applies to every doc
  and every skill that writes docs.
- Filenames: kebab-case matching the title. Content lives in folders by type
  (`docs/lore/organizations/`, ...); folders appear as pages are added.
- One subject per page. Cross-link with `[[Wikilinks]]`. Link multi-word pages as
  `[[basename|Display]]` so the link resolves and the lint orphan check sees it.
- Anchor claims to sources: cite inline as `(src: raw/file.md)`. Prefer anchoring
  over memory; it stops canon and mechanics from drifting over many edits.
- Numbers live in data, not prose: document the model and say where tunable values
  will live. Split static (Definition) from runtime (Instance); prefer tags over
  rigid class trees.
- **Code section:** leave as a placeholder until the Unity project exists. Once it
  does, systems pages list the C# classes/paths that implement them.
- A myth page states whether it is true in canon or only believed.

## Operations (skills in `.claude/skills/`)

- /ingest <source> - read a raw source, write/update pages, update index,
  run a contradiction check on touched pages.
- /ask <question> - answer from the wiki with citations.
- /new-page <type> <title> - create a page from the right template.
- /lint - health check; `scripts/lint.mjs` runs the deterministic checks.
- /grill-lore <page> - interview the human to flesh out a thin page.
- /domain-modeling - maintain the glossary (docs/CONTEXT.md) and record decisions (in the concept pages; ADR for technical).
- /edit-article - revise a page section by section for clarity and dependency order.
- /setup-wiki - one-time, confirm conventions.

## Contradiction handling

When new content conflicts with a page, classify and record it on the page:

- soft - tone or emphasis. Non-blocking. Flag and note.
- scope - true in different contexts. Non-blocking. Flag and explain each scope.
- hard - direct conflict (a date clash with the timeline; a fact two ways).
  Blocking. Do not pick a winner; stop and ask.

Record inline, and it surfaces in /lint until resolved:

```
> Contradiction severity: hard
> Status: unresolved - flagged for review
> Conflict: [[Source A]] says X; [[Page B]] says Y.
```

Undecided _design questions_ are not contradictions — track those in
`docs/open-questions.md`. Never overwrite established canon in passing.

## Decisions & glossary

Maintained via /domain-modeling. Decisions live next to what they govern, plus one glossary:

- **Design / world / mechanics decisions** — folded into the **concept page** that owns the
  topic (a `## Why / rejected` section: the call, the _why_, what was rejected), cross-referenced
  to sibling pages. No separate design-decision register.
- **ADR** (`docs/adr/`) — architecture / technical decisions (the Unity code, the
  repo, tooling); standard ADR.
- **`docs/CONTEXT.md`** — the glossary (ubiquitous language): terms only, no lore
  or mechanics.
- **Mechanic IDs** — mechanics carry **section-scoped IDs** (e.g. `SIEGE-1`, `ECON-2`),
  tagged on their section header in `raw/design-overview.md`. IDs are local to a section
  so adding one never renumbers another. No separate registry file — the tagged headers are the index.

Record a decision only when it is hard to reverse, surprising without context, and a real
trade-off — capture the _why_ and what was rejected. `docs/open-questions.md` is the unresolved
backlog; a settled item is folded into its concept page and its page tag flips to `[decided]`.

## Nothing is locked

Green-field project: no users, no shipped versions, nothing locked in. Do not add
backward-compatibility, migration notes, deprecation shims, or "supersedes the old
X" history — when something changes, change it in place as if it had always been
that way. Only record a decision (in a concept page, or an ADR for technical) when a future
agent could plausibly re-make it _differently_; never to preserve history for its own sake.

## Index

`docs/index.md` is the catalog of every page with a one-line summary. Update on
every ingest; read first on every query. Activity history lives in git — there is
no separate changelog.

## Notes

- Markdown + Obsidian vault under git. Commits are the history.
- **Git: never branch. Commit straight to `main` and push immediately** — every change goes directly to `main`.
- Keep this schema lean. Add a short rule when a mistake repeats, not an essay.
