# AGENTS.md - Gaters Knowledge Wiki

This repo is an LLM-maintained knowledge base for the game Gaters, kept as an
Obsidian vault. It holds two linked wikis under one schema:

- lore/ - the world bible: canon, characters, factions, places, events. Page
  types follow the World Anvil category set.
- systems/ - the mechanics bible: systems, content, and how they map to code.
  Page types follow the open-world-survival content model.

You (the agent) maintain both wikis. The human curates sources and asks
questions. This file is the schema; read it at the start of every session.

Ponytail is active here. Apply it to the docs too: write the minimum page that
does the job, link instead of duplicating, no filler sections.

## Layers
1. raw/ - source material (design notes, transcripts, spec PDFs, data exports).
   Immutable. Read from it, never edit it. The ultimate source of truth.
2. lore/ and systems/ - the wiki pages you generate and maintain.
3. AGENTS.md - this schema, co-evolved with the human.

## Page types
Every page starts with YAML frontmatter; type is required.

Lore (World Anvil set): character, location, organization, species, culture,
religion, myth, item, language, event, plus overview and timeline.

Systems (survival model): system, item, resource, recipe, station, structure,
creature, biome, status-effect, tech, formula, plus overview.

Frontmatter:
```yaml
---
type: character
status: draft        # draft | stable | needs-review
tags: []
sources: []          # provenance into raw/
aliases: []
updated: 2026-06-27
---
```
Copy the matching file from _templates/lore/ or _templates/systems/. Do not
invent new section structures per page.

## Conventions
- Filenames: kebab-case matching the title. Content lives in folders by type
  (lore/characters/, systems/recipes/, ...); folders appear as pages are added.
- One subject per page. Cross-link with [[Wikilinks]]; a link with no page yet
  is a valid "write later" marker.
- Anchor claims to sources: cite inline as (src: raw/file.md) or quote a short
  line. Prefer anchoring over memory. This stops canon and mechanics from
  drifting over many edits.
- Lore and systems link both ways: an item's lore page points to its systems
  item page; a creature species points to its creature page.
- Numbers live in data, not prose. A systems page documents the model and says
  where the real values live (data file, ScriptableObject). Split static data
  (Definition) from runtime state (Instance); prefer tags over rigid class
  trees for materials and recipes.
- Code references: systems pages list the C# classes/paths that implement them
  under a Code section (e.g. CombatManager.cs, Assets/Scripts/Combat/).
- A myth page states whether it is true in canon or only believed.

## Operations (run via the skills in .claude/skills/)
- /setup-wiki - one-time, confirm conventions.
- /ingest <source> - read a raw source, write/update pages, update index.md and
  log.md, run a contradiction check on touched pages.
- /ask <question> - answer from the wiki with citations.
- /new-page <type> <title> - create a page from the right template.
- /lint - health check; scripts/lint.mjs runs the cheap deterministic checks.
- /grill-lore <page> - interview the human to flesh out a thin page.

## Contradiction handling
When new content conflicts with a page, classify and record it on the page:
- soft - tone or emphasis. Non-blocking. Flag and note.
- scope - true in different contexts. Non-blocking. Flag and explain each scope.
- hard - direct conflict (a character dies twice; a formula defined two ways; a
  date clash with the timeline). Blocking. Do not pick a winner; stop and ask.

Record inline:
```
> Contradiction severity: hard
> Status: unresolved - flagged for review
> Conflict: [[Source A]] says X; [[Page B]] says Y.
```
Never overwrite established canon or a design decision in passing.

## Index and log
index.md is the catalog of every page, grouped by section and type, each with a
one-line summary. Update on every ingest; read first on every query.

log.md is append-only with a parseable prefix:
```
## [2026-06-27] ingest | <source>
- created ...
- updated ...
```

## Notes
- This is a git repo of markdown and an Obsidian vault. Commits are the history.
- Keep this schema lean. Add a short rule when a mistake repeats, not an essay.
- Grounding: lore = World Anvil categories; systems = survival content model
  plus MDA vocabulary (a mechanic is a rule, a dynamic is what emerges).
