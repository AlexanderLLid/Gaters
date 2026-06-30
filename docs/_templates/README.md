# Templates

Every wiki page starts from a template here and keeps its YAML frontmatter; `type` is required.

```yaml
---
type: organization
status: draft # draft | stable | needs-review
tags: []
sources: [] # provenance into docs/raw/
aliases: [] # add the Title-Case display name for multi-word pages
updated: YYYY-MM-DD
---
```

**Lore** (World Anvil set): character, location, organization, species, culture, religion, myth, item, language, event, plus overview and timeline.

**Systems** (survival model): system, item, resource, recipe, station, structure, creature, biome, status-effect, tech, formula, plus overview.

Copy the matching file from `lore/` or `systems/`. Templates exist for the in-use types (organization, location, event, system); for a deferred type, copy the closest sibling as a base. Don't invent new section structures per page.
