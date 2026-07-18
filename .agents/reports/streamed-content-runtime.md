# Streamed content runtime

## Outcome

- Terrain cells now own matching pure `FGatersContentCellRecipe` state.
- Recipe v2 owns the final deterministic tree/rock position, yaw, and scale; the visual
  materializer preserves that compiled transform instead of adding presentation-only
  variation.
- Loaded content recipes are adapted to ordinary world-recipe nodes, compiled through the
  existing content catalog, and materialized in the existing four native ISM batches.
- Leaving a terrain cell removes its content nodes; returning regenerates the same full
  IDs from seed, cell, and candidate identity.
- Harvest and claim diff construction now carries the complete recipe ID rather than a
  map-local integer, so cells and worlds cannot alias persistence entries.
- The original fixed 61x61 scatter pass is no longer used; resources come from the same
  bounded cells as terrain.

## Evidence

- Unreal editor build: succeeded.
- `Gaters.Worldgen.ContentCells`: 4/4 succeeded.
- `Gaters.Runtime.VisualMaterializer`: 1/1 succeeded, including a streamed content ID.
- The materializer test also proves its scatter transform exactly matches the compiled
  world node transform.
- Headless seed 7: nine terrain/content cells, 72 resource placements, valid matching
  compile, four native ISM batches, zero resource actors, and no performance failures.
- The runtime source crossed from cell `(0,0)` to `(-2,-4)` and back; both transitions
  unloaded and loaded nine cells and rematerialized 72 deterministic resources.

## Honest boundary

- v1 rebuilds the small loaded visual plan when the desired cell set changes. That is the
  simplest correct adapter for the current 3x3 set; profile larger radii and multiplayer
  sources before implementing incremental per-cell ISM mutation.
- Automation proves direct full-ID interaction mapping, but a live
  harvest -> save -> unload -> reload test is still pending; persistence is implemented,
  not yet promoted as verified lifecycle evidence.
- Placements are trees and rocks only. Biome composition, density curves, authored asset
  families, landmark rules, navigation reachability, and multiplayer authority remain
  separate later machines.
- Physical-fit consumes the same selected contract and transform that Unreal renders.
  Contacts now distinguish terrain support from unresolved piece attachment; attachment
  solving plus engine collision/navigation evidence remain pending.

## Compiled physical-fit observation

- The compiler now preserves the exact selected asset contract beside every compiled
  asset node, and physical-fit evaluates those contracts against the same materialized
  terrain field used by runtime generation.
- The compiler aligns tree and rock transforms so their selected contract's terrain
  contact lands on the recipe ground anchor after scale and rotation. The materializer
  preserves that transform exactly.
- Seed 7 evaluated 171 base/resource placements. Its repeatable initial and restored
  state recorded 105 valid placements, 66 issues, 137 evaluated terrain contacts, and 34
  explicitly pending attachment contacts. The previous 33 buried centered-pivot tree
  failures are gone.
- The remaining initial-state evidence is 53 stone-fence and 12 foundation contacts that
  are intentionally embedded by the current base stamp, plus one generated-rock
  slope-normal failure. This is diagnostic evidence, not a promotion: intended embed
  depth needs a contract representation, and attachment, collision, and navigation
  solving remain pending.
