---
name: systems-schema
description: Reference for the Gaters systems page types and the data-driven content model for an open-world survival game. The agent reaches for this when creating or editing a systems page (system, item, resource, recipe, station, structure, creature, biome, status-effect, tech, formula).
---

# systems-schema

Content types (later, each maps to a C# data asset / ScriptableObject — no code yet):

- system - a whole mechanic area (gates, survival meters, building, combat).
- item - a usable object (tool, weapon, armor, consumable, placeable).
- resource - a raw or refined material; carries tags.
- recipe - ingredients (items/resources or tags) to an output item.
- station - a crafting or processing station.
- structure - a building piece or placeable.
- creature - an enemy, animal, or tameable.
- biome - a region type with spawns and hazards.
- status-effect - a buff, debuff, or condition.
- tech - a research node that unlocks recipes, items, structures.
- formula - an explicit calculation used by systems.

Rules:

- Document the model, not the numbers. Say where tunable values live (data file
  or ScriptableObject); never freeze balance numbers in prose.
- Split static data (Definition) from runtime state (Instance).
- Prefer material and recipe tags over rigid class trees.
- Code section: placeholder until the Unity project exists (then list C# classes/paths).
- Use MDA vocabulary: a mechanic is a rule, a dynamic is what emerges in play.
- Balance by default: each mechanic names the dev-tunable knob that keeps it healthy
  and the **design trap** it guards against (defined in [[pillars|Pillars]]). The
  bar is the aggressor fantasy without the **defender tax** — a dynamic that punishes
  an absent player has reinvented decay; flag it like a hard contradiction. Each
  system also names the layer it sits in and justifies any new knob against upstream
  knobs (see CLAUDE.md, Design order).

Use the matching template in docs/_templates/systems/.
