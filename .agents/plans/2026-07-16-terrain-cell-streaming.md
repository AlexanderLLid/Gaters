# Terrain Cell Streaming Implementation Plan

> **For agentic workers:** Execute inline with `superpowers:executing-plans`; do not branch or commit unless the user asks.

**Goal:** Replace the single generated terrain mesh with deterministic terrain cells that load around the local player and unload outside a bounded radius.

**Architecture:** A pure `FGatersWorldCellStreaming` function converts world-local positions into a stable, bounded set of cell coordinates. `AGatersTerrainCell` owns one generated mesh and samples the existing environment function in global coordinates so neighboring edges match. `AGatersChunk` remains the prototype coordinator and refreshes the active cells only when the player changes cell.

**Tech Stack:** Unreal Engine 5.8 C++, Dynamic Mesh, Unreal Automation Tests.

## Global Constraints

- Keep the current Gate-to-base route outside the streaming contract; Rift versus permanent Gate remains open.
- Keep village, scatter, and base generation near the origin in this slice.
- Use one world seed and global sample coordinates; cells must not invent per-cell terrain seeds.
- Keep a bounded 3x3 active terrain set by default.
- Do not convert the level to World Partition in this slice.

---

### Task 1: Deterministic cell-set contract

**Files:**
- Create: `Unreal/Prototype/Source/Prototype/Public/GatersWorldCellStreaming.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersWorldCellStreaming.cpp`
- Create: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersWorldCellStreamingTests.cpp`

**Interfaces:**
- Produces: `CellAt(FVector2D, float)`, `CellCenter(FIntPoint, float)`, and `DesiredCells(TArray<FVector2D>, float, int32, float)`.

- [ ] Write automation tests for negative-coordinate flooring, stable 3x3 ordering, multi-source deduplication, and world-edge clipping.
- [ ] Build and confirm the new test target fails because the contract does not exist.
- [ ] Implement only the pure coordinate/set functions.
- [ ] Build and run `Gaters.Worldgen.Streaming.Cells`; confirm it passes.

### Task 2: One seamless generated terrain cell

**Files:**
- Create: `Unreal/Prototype/Source/Prototype/Public/GatersTerrainCell.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersTerrainCell.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersWorldCellStreamingTests.cpp`

**Interfaces:**
- Consumes: the existing `FGatersEnvironment` and `FGatersTerrainSemanticField::MaterializedHeightAt`.
- Produces: `AGatersTerrainCell::Configure(...)` and `Build()`.

- [ ] Add a failing seam test proving adjacent cells request identical global coordinates along their shared edge.
- [ ] Implement a minimal Dynamic Mesh actor that generates one rectangle, displaces vertices using global-local sample positions, recomputes normals, and enables collision.
- [ ] Run `Gaters.Worldgen.Streaming`; confirm the pure seam contract passes.

### Task 3: Player-centered runtime lifecycle

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersChunk.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersChunk.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersEnvironmentTests.cpp`

**Interfaces:**
- Consumes: `FGatersWorldCellStreaming::DesiredCells` and `AGatersTerrainCell`.
- Produces: a 3x3 active cell set refreshed when the player crosses a cell boundary; `STREAM` runtime report.

- [ ] Change the defaults test first to require `WorldSize`, `TerrainCellSize`, `TerrainCellResolution`, and `TerrainLoadRadius`.
- [ ] Build and confirm the defaults test fails.
- [ ] Add the four tunables and an active-cell map to `AGatersChunk`.
- [ ] Replace `BuildGround()` with initial cell creation, and refresh the set from `Tick()` only after a source-cell change.
- [ ] Keep the coordinator mesh empty; retain the current water plane and origin content.
- [ ] Build and run all `Gaters.Worldgen` tests.

### Task 4: Record the unresolved Rift foundation change

**Files:**
- Modify: `docs/questions.md`

- [ ] Add one `[open]` question: permanent Gate versus temporary Rift/anchor.
- [ ] State that new map generation and streaming must not require Gate-centered topology or a permanent Gate-to-base corridor while the question is open.
- [ ] Do not rewrite current Gate canon until the contradiction is decided.

### Task 5: Playable verification

**Files:**
- No source files unless verification finds a defect.

- [ ] Close Unreal Editor, build `PrototypeEditor Win64 Development`, and run `Gaters.Worldgen` headlessly.
- [ ] Launch the Terrain Lab level, confirm nine cells initially exist, walk across a 100 m boundary, and confirm the `STREAM` report shows a bounded active count with entered/exited cells.
- [ ] Inspect seams, collision, water, and origin base/scatter placement.

