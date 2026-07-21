# World Generation Brief

The world-generation contract for the prototype. This is not a status page;
`research/machines.json` owns machine contracts, dependencies, evidence, and current
waves.

## Outcome

A seed produces a reproducible playable world recipe containing terrain, water, semantic
ground, routes, streamed content, resources, settlements, bases, populations, and stable
identity for later diffs. Unreal materializes only the bounded cells currently needed.

## Required boundaries

- Generation produces pure versioned data before Actors or assets exist.
- Terrain identity is independent of the current streaming window.
- Arrival is a semantic site, not a required Anchor-centered landform or permanent
  arrival-to-base corridor ([[questions]] #28).
- Terrain exposes shared height, slope, water, walkability, buildability, and route
  queries; downstream generators do not rederive them.
- Content recipes use semantic keys; the catalog chooses placeholders or finished assets.
- Persistent changes reference stable recipe IDs instead of serializing generated levels.
- Every generator has an independent evaluator and held-out seeds.
- Visual, tactical, structural, and performance quality remain separate evidence rather
  than one opaque score.

## Capability sequence

`world.terrain-generator` and `world.recipe` feed semantic terrain, navigation, route
planning, bounded content recipes, cell streaming, biome/resources, settlements, bases,
populations, and finally `runtime.playable-world-assembler`.

The exact graph and status live only in `research/machines.json`. Use:

```powershell
./research/Show-MachineRegistry.ps1 -Format Summary
./research/Show-MachineRegistry.ps1 -Format Mermaid
./research/Test-MachineRegistry.ps1
```

## Visual test

`Unreal/Prototype/Scripts/RunEnvironmentSweep.ps1 -CaptureGallery` exercises real seeds
through the Unreal map and records comparable images. Numeric checks reject known hard
failures; galleries expose shapes and composition the metrics do not yet understand.
