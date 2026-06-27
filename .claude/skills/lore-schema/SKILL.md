---
name: lore-schema
description: Reference for the Gaters lore page types and their fields, based on the World Anvil category set. The agent reaches for this when creating or editing a lore page (character, location, organization, species, culture, religion, myth, item, language, event).
---

# lore-schema

Lore page types and what each is for:
- character - a single named being, including deities. Named individuals only.
- location - a place: world, region, settlement, landmark, hub.
- organization - a group: faction, government, guild, order, pantheon.
- species - a kind of being (race, animal, plant, creature, construct); the
  whole kind, not an individual.
- culture - an ethnicity or culture.
- religion - a faith, deity, or pantheon; deities themselves are character pages.
- myth - a legend or in-world story; must state if it is true in canon.
- item - an object or artifact of significance.
- language - a spoken or written language.
- event - something that happened in canon; links to the timeline.

Rules:
- Named being maps to character. Whole kind maps to species. This split is
  deliberate.
- Cross-link to systems pages where an element appears in-game.
- Anchor canon to raw/ sources. See AGENTS.md for the full conventions.
Use the matching template in _templates/lore/.
