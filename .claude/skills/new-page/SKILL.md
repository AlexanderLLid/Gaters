---
name: new-page
description: Create a new wiki page from the right template. Use when the user asks to add or create a character, location, organization, item, system, recipe, creature, biome, or any other page type.
---

# new-page

Input: a type and a title.

1. Pick the template: docs/_templates/lore/<type>.md or docs/_templates/systems/<type>.md.
   Valid types are listed in CLAUDE.md.
2. Copy it to the content folder for that type (e.g. docs/lore/characters/,
   docs/systems/recipes/) with a kebab-case filename matching the title.
3. Fill the frontmatter (type, status: draft, updated, any aliases) and the
   title. Leave sections as prompts unless the human gives content.
4. Add wikilinks to obviously related existing pages.
5. Add the page to docs/index.md.
