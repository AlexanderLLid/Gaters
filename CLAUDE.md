# CLAUDE.md - Gaters

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
Build only what this phase needs; defer the rest to `docs/raw/`.
- **Now, high-level:** the **Builders** (the precursor gate-makers) and the
  **Central Authority** (who runs the gates). Overview depth — frame what is
  decided; do not elaborate.
- **Now, in depth:** **gate physics** — the one place to go deep. Bind every
  claim to real physics and name the concept. See [[Gate Physics]].
- **Deferred (afterthought):** specific characters, weapons, items, recipes,
  creatures, detailed faction/world lore, and gameplay-systems detail. If a
  source mentions them, capture in `docs/raw/` — do **not** build pages yet.
- **Wanted later:** features we've decided we want but are deferring by phase (e.g.
  space gates) live in `docs/roadmap.md` — intended, not undecided.

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
status: draft        # draft | stable | needs-review
tags: []
sources: []          # provenance into docs/raw/
aliases: []          # add the Title-Case display name for multi-word pages
updated: 2026-06-27
---
```
Copy the matching file from `docs/_templates/`. Do not invent new section
structures per page.

## Conventions
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
- /domain-modeling - maintain the glossary (docs/CONTEXT.md) and record decisions (DDR/ADR).
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
Undecided *design questions* are not contradictions — track those in
`docs/open-questions.md`. Never overwrite established canon in passing.

## Decisions & glossary
Maintained via /domain-modeling. Two decision registers and one glossary:
- **DDR** (`docs/ddr/`) — design / world / mechanics decisions.
- **ADR** (`docs/adr/`) — architecture / technical decisions (the Unity code, the
  repo, tooling); standard ADR.
- **`docs/CONTEXT.md`** — the glossary (ubiquitous language): terms only, no lore
  or mechanics.

Numbered `0001-slug.md`, created lazily. Record a decision only when it is hard to
reverse, surprising without context, and a real trade-off — capture the *why* and
what was rejected. `docs/open-questions.md` is the unresolved backlog; a settled
item graduates to a numbered DDR/ADR and its page tag flips to `[decided]`.

## Nothing is locked
Green-field project: no users, no shipped versions, nothing locked in. Do not add
backward-compatibility, migration notes, deprecation shims, or "supersedes the old
X" history — when something changes, change it in place as if it had always been
that way. Only record a decision (DDR/ADR) when a future agent could plausibly
re-make it *differently*; never to preserve history for its own sake.

## Index
`docs/index.md` is the catalog of every page with a one-line summary. Update on
every ingest; read first on every query. Activity history lives in git — there is
no separate changelog.

## Notes
- Markdown + Obsidian vault under git. Commits are the history.
- Keep this schema lean. Add a short rule when a mistake repeats, not an essay.
