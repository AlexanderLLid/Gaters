# Terrain content cells

## Files

- `Unreal/Prototype/Source/Prototype/Public/GatersContentCellRecipe.h` — pure recipe contract.
- `Unreal/Prototype/Source/Prototype/Private/GatersContentCellRecipe.cpp` — deterministic bounded generator.
- `Unreal/Prototype/Source/Prototype/Private/Tests/GatersContentCellRecipeTests.cpp` — automation coverage.

## Interface

- `FGatersContentCellRecipe::Generate(Cell, CellSize, Environment, Semantics)` returns at most eight stable `ScatterTree` / `ScatterRock` placements.
- World identity is derived only from `Environment.Seed`; stable IDs include that seed, cell X/Y, and candidate index. Transforms use global terrain coordinates but remain strictly inside the requested cell.
- The generator hash-ranks sixteen deterministic opportunities before evaluation, avoiding scan-order spatial bias while using the existing materialized terrain height/normal functions and semantic water/slope thresholds.
- `Coverage` accounts for every opportunity as placed or rejected by a reserved footprint,
  water, steep terrain, or the placement budget.
- The recipe owns no Actors, assets, or world state.

## Tests and output

- TDD red: `Build.bat PrototypeEditor ...` failed on the intentionally missing `GatersContentCellRecipe.h`.
- Review TDD reds: the seedless call contract failed to compile against the redundant seed parameter; the full-land fixture then failed `placements span both X halves` under numeric scan order.
- Build after implementation: succeeded.
- Focused `Gaters.Worldgen.ContentCells`: 4 tests found, 4 succeeded.
- Full `Gaters`: 51 tests found, 51 succeeded.
- Covered identical-input determinism, single-source world identity, non-vacuous cross-cell/world identity separation, transform bounds, semantic keys/kinds, bounded budgets, exhaustive coverage accounting, distant dry-land placements, causal reserved/water/steep rejection evidence, and representative full-land coverage across both halves of each axis.

## Original lane boundary

- Candidate count (16) and placement cap (8) are intentionally small fixed recipe constants until runtime profiling or playtests justify exposing density tuning.
- This pure-generator lane does not own materialization. Runtime consumption is recorded
  separately in `.agents/reports/streamed-content-runtime.md`.

## Next integration hook

- When a terrain cell enters the desired streaming set, generate its `FGatersContentCellRecipe`, adapt each placement to an `FGatersRecipeNode`, and pass those nodes through the existing compiler/materializer path. Remove them when that cell leaves the desired set; keep generation itself actor-free.
