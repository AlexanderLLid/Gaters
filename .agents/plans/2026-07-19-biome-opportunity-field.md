# Biome opportunity field implementation plan

**Goal:** Convert verified biome facts into deterministic neutral content/travel
opportunities without generating assets, sites, or gameplay resources.

**Requirements checked:** Global none recorded; generated content boundary; exceptions:
none.

## Contract

- Input: `FGatersBiomeSample` only.
- Output: bounded vegetation, stone, landmark, and travel-friction weights.
- No seed, streamed-cell, Actor, asset, settlement, or final resource-name dependency.
- Water, slope, moisture, exposure, and elevation must influence separate weights.

## Evidence loop

- [x] RED: absent opportunity field fails compile.
- [x] GREEN: deterministic and bounded across held-out biome samples.
- [x] Counterexamples: water raises friction; moist flat land favors vegetation; exposed
  steep/high land favors stone; semantic profiles are non-vacuous.
- [x] Keep `world.biome-resource-generator` planned until opportunities drive placements
  and reachable resource loops pass.
