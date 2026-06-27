# Gaters Knowledge Wiki

An LLM-maintained knowledge base for Gaters, kept as an Obsidian vault in git.
Two linked parts:

- lore/ - the world bible (World Anvil categories: character, location,
  organization, species, culture, religion, myth, item, language, event).
- systems/ - the mechanics bible (survival content model: system, item,
  resource, recipe, station, structure, creature, biome, status-effect, tech,
  formula).

The agent does the writing and upkeep; you curate sources and ask questions.
AGENTS.md is the operating manual it follows. CONTEXT.md is the shared glossary.

## Use it
1. Open this folder in Obsidian (graph view, wikilinks, Dataview).
2. Run Claude Code from the repo root; it reads AGENTS.md and loads the skills
   in .claude/skills/.
3. Drop sources into raw/ and run /ingest. The agent writes pages, updates
   index.md, and logs the work.
4. Ask questions with /ask. Run /lint now and then.

## Layout
- AGENTS.md, CLAUDE.md, CONTEXT.md, index.md, log.md
- lore/ and systems/ - content, folder per type
- _templates/ - page templates per type
- raw/ - immutable sources, never edited
- .claude/skills/ - the agent behaviors
- scripts/lint.mjs - deterministic checks (CI or pre-commit)

## Conventions
Every page has YAML frontmatter with a required type, cross-links with
[[Wikilinks]], and anchors claims to raw/. Balance numbers live in data, not
prose. See AGENTS.md for the full set.
