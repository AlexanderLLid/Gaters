# Village Growth v1

## Result

- Land settlements grow deterministically through stages `0`, `1`, and `2`.
- Growth is an append-only semantic patch: prior buildings, parcels, and coordinate-based
  path IDs remain unchanged.
- Four immutable growth fronts constrain later expansion without defining final art.
- Parcels declare support and access keys. V1 implements `support.ground` and
  `navigation.ground`; other habitats remain unimplemented adapters.
- An independent evaluator rejects skipped stages, altered sources, duplicate additions,
  disconnected entrances, missing parcels, and invalid expanded settlements.
- A pure settlement-to-recipe adapter emits parcel, growth-front, path, and building
  module nodes. Unreal materialization remains downstream and non-authoritative.

## Runtime evidence

- Seed `7`:
  - stage `0`: 6 buildings, 70 modules;
  - stage `1`: 10 buildings, 118 modules;
  - stage `2`: 16 buildings, 188 modules.
- Stage `2` also passed runtime generation on seeds `0`, `2`, `4`, and `53`, covering
  mountains/dry, archipelago/ocean, lowlands/dry, and mountains/lakes.
- Every run reported a valid settlement, all 16 assemblies valid, a valid schema-9
  recipe, zero compiler diagnostics, valid traversal, and valid performance evidence.
- Full Unreal automation: `66/66` Gaters tests passed.
- Registry validation: `PASS machines=68 waves=7`.

## Developer controls

- `Gaters.VillageStage 0`
- `Gaters.VillageStage 1`
- `Gaters.VillageStage 2`
- Headless: `-GatersVillageStage=N` or `RunEnvironmentSweep.ps1 -VillageStage N`.

## Honest boundary

- Implemented: deterministic land layout, stages, patches, independent evaluation,
  recipe integration, placeholder materialization, runtime selection, held-out runs.
- Not implemented: automatic background scheduling/publication, persistence of a grown
  stage in production saves, NPC/economic reasons for growth, PCG graph materialization,
  finished assets, perceptual validation, or tree/cliff/cave/water habitat adapters.
