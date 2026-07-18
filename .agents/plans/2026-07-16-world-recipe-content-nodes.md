# World Recipe Content Nodes Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:executing-plans`. Subagents are intentionally not used because repository instructions do not authorize delegation.

**Goal:** Make current scatter and build plots explicit stable recipe data before their Unreal actors are materialized.

**Architecture:** Extend the minimal node-kind enum with `ScatterTree`, `ScatterRock`, and `BuildPlot`. In the existing generation loops, create local-space recipe nodes first, then derive the current actor spawn position from the node; diffs continue to suppress materialization but never remove original recipe identity.

**Tech Stack:** Unreal Engine 5.8 C++, Unreal Automation Tests, command-line seed sweep.

## Global Constraints

- Preserve random draw order, scatter IDs/checksum, plot ordering, actor transforms, diff keys, and `GatersGenVersion`.
- Recipe nodes describe the generated original; harvested scatter and claimed plots remain nodes while diffs alter runtime state.
- Keep nodes local to the chunk; actor world location is a materializer concern.
- Do not refactor EBS base stamping in this slice.
- Do not branch, commit, or push; preserve unrelated changes.

---

### Task 1: Content Node Contract

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersWorldRecipe.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersWorldRecipeTests.cpp`

**Interfaces:**
- Produces: `EGatersRecipeNodeKind::ScatterTree`, `ScatterRock`, and `BuildPlot`.
- Consumes: existing `FGatersRecipeNode` canonicalization and validation.

- [ ] **Step 1: Write the failing content-kind test**

  Append one node of each new kind to a generated recipe. Assert stable lookup, canonical identity change, and successful structural validation.

- [ ] **Step 2: Build to verify RED**

  Expected: compile failure because the three node kinds do not exist.

- [ ] **Step 3: Add only the enum values**

  Existing canonicalization and validation already operate generically over kind and location.

- [ ] **Step 4: Build and run focused recipe tests**

  Expected: all recipe tests pass.

---

### Task 2: Recipe-first Scatter and Plot Materialization

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersChunk.cpp`

**Interfaces:**
- Consumes: `Recipe.Nodes` and new node kinds.
- Produces: stable IDs `scatter:<cell-index>` and `plot:<plot-index>` in local chunk space.

- [ ] **Step 1: Run a failing runtime expectation**

  Run seed 7 and assert the recipe has more nodes than the structural Gate/Base pair.

  Expected: failure because the current runtime reports `nodes=2`.

- [ ] **Step 2: Record scatter before diff/materialization**

  Create one local-space recipe node for every deterministic scatter candidate before checking `chop:<id>`. Spawn surviving actors at `GetActorLocation() + Node.Location`; preserve all stream draws and `SCATTER sum` arithmetic.

- [ ] **Step 3: Record plots before claim-marker materialization**

  Create `plot:<index>` from `PlotCenters[index] - GetActorLocation()` before reading claim state. Spawn the marker from the node-derived world location.

- [ ] **Step 4: Verify build, all worldgen tests, and seed 7 twice**

  Expected: both sweeps report the same recipe checksum and node count greater than two; SITE, BASE, SCATTER, STAMP, and diff behavior remain unchanged.

## Self-review

- Current loops remain the generators; this only captures their output before spawning.
- No new actor class, component, serialization system, or generic node API is introduced.
- EBS base pieces remain the next distinct seam.

