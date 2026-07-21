# World layer separation and biome field v1

## Outcome

- World & Terrain can run without the Built Site layer.
- Settlements, Bases & Dungeons contributes a pure optional recipe layer; missing/empty output is valid.
- A pure biome field answers coordinate queries without streamed cells, sites, Actors, or
  assets.

## Runtime evidence

- Command: `Gaters.BuiltSites <0|1>`; persisted across reloads.
- Headless argument: `-GatersBuiltSites=0|1`.
- Sweep argument: `RunEnvironmentSweep.ps1 -BuiltSites 0|1`.
- Seed `7`, world-only:
  - Recipe valid, `399` nodes, `0` stamped pieces.
  - Built Site counts all zero.
- Seed `7`, merged:
  - Recipe valid, `604` nodes.
  - `1` site, `6` buildings, `6` parcels, `25` path cells, `70` modules.
  - `99` older prototype base pieces remain; extraction is tracked by `SITE-4`.

## Visual evidence

- `Saved/EnvironmentGallery/seed-7-world-only-beauty.png`
- `Saved/EnvironmentGallery/seed-7-world-only-traversal.png`
- `Saved/EnvironmentGallery/seed-7-beauty.png`
- `Saved/EnvironmentGallery/seed-7-traversal.png`

## Automated evidence

- Build: `PrototypeEditor Win64 Development` succeeded.
- Full Unreal automation: `90/90` Gaters tests passed.
- `Gaters.Worldgen.Biomes`: `3/3` passed across seeds `0`, `2`, `4`, `7`, `53`,
  plus a controlled regional-intent fixture proving arrival compatibility, local dry
  override of globally wet terrain, intent provenance, and blend-annulus continuity.
- `Gaters.Worldgen.BiomeOpportunities`: pure bounded profile checks passed.
- `Gaters.Worldgen.WorldIntent`: `2/2` passed for deterministic regional declarations,
  different-seed variation, coordinate ownership, validation, and sparse intent.
- `Gaters.Worldgen.IntentTerrainField`: `2/2` passed for protected arrival identity,
  deterministic regional influence, declared terrain/hydrology consumption, and smooth
  region boundaries.
- `Gaters.Worldgen.Environment.SemanticFieldIntentAdapter`: explicit intent-aware
  materialized height and normal sampling passed.
- `Gaters.Worldgen.IntentTerrainFidelity`: `2/2` passed for matching observations,
  corrupted regional height, and missing regional evidence.
- `Gaters.Worldgen.RegionalWater`: `3/3` passed. Recipe tests cover deterministic dry
  suppression, lake footprints, bounded ocean extent, containment, and stable identities.
  Independent evaluator tests accept correct mixed lakes/ocean evidence and reject a
  missing surface, wrong water datum, and a dry-region leak; each surface must intersect
  sampled submerged terrain.
- `Gaters.Worldgen.ContentCells`: `6/6` passed after recipe v4 began recording intent and
  regional biome provenance, suppressing undeclared vegetation, and accepting truly
  barren cells.
- Machine registry: `PASS machines=74 waves=7`.
- Shared docs: `PASS shared-agent-docs skills=5 workstreams=8`.
- `git diff --check`: passed.

## Honest boundary

- `world.biome-field` is verified as a coordinate semantic query.
- Streamed terrain and biome classification consume regional terrain/hydrology tendencies,
  content cells consume the regional biome result, and regional water surfaces render.
  Regional water has independent physical-fit evaluation, but art quality, hard-edge
  leak visuals, and broad held-out profile sweeps remain before the water/terrain
  challengers can be promoted.
- `world.biome-resource-generator` remains planned: dressed resources, reachable loops,
  biome-driven density, and travel friction are not implemented.
- Biome keys are greybox semantics, not final ecology or art direction.
