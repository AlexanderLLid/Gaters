# World Recipe Base Nodes Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:executing-plans`. Subagents are intentionally not used because repository instructions do not authorize delegation.

**Goal:** Capture every deterministic EBS stamp row and the raid loot point as engine-independent recipe nodes before diff replay and actor spawning.

**Architecture:** Extend recipe nodes only with transform and `ContentKey`, fields now required by base materialization. Stamp rows receive semantic keys such as `wood.foundation`, `stone.wall`, and `door`; recipe nodes never store Unreal class or asset paths. Existing EBS classes/meshes remain the current adapter while spawn transforms are consumed from recipe nodes.

**Tech Stack:** Unreal Engine 5.8 C++, Unreal Automation Tests, command-line seed sweep.

## Global Constraints

- Preserve EBS class/mesh selection, row order, piece IDs, transform values, diff keys, loot position, and random draws.
- Keep all recipe transforms in chunk-local space.
- Bump recipe schema only; do not change `GatersGenVersion`.
- `ContentKey` is a semantic catalog key, never an Unreal object path.
- Do not build the future asset catalog/resolver in this slice.
- Do not branch, commit, or push; preserve unrelated changes.

---

### Task 1: Transform and Content Contract

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersWorldRecipe.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersWorldRecipe.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersWorldRecipeTests.cpp`

**Interfaces:**
- Produces: `BasePiece` and `RaidLoot` kinds; node `Rotation`, `Scale`, and `ContentKey`; schema 3 canonicalization.
- Consumes: existing generic node lookup and structural validation.

- [ ] **Step 1: Write failing transform/content tests**

  Add a BasePiece node and assert transform/content changes affect checksum. Assert validation rejects a BasePiece with empty `ContentKey`, non-finite rotation, or non-positive scale and accepts a well-formed one.

- [ ] **Step 2: Build to verify RED**

  Expected: compile failure because the new fields and kinds do not exist.

- [ ] **Step 3: Implement minimum contract**

  Add fields with zero/one defaults, append them in fixed order to canonical text, and extend generic validation with finite transform, positive scale, and BasePiece content-key checks.

- [ ] **Step 4: Build and run focused recipe tests**

  Expected: all recipe tests pass.

---

### Task 2: Recipe-first EBS Rows and Loot

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersChunk.cpp`

**Interfaces:**
- Consumes: deterministic `FStampRow` data and `Recipe.Nodes`.
- Produces: `piece:<row-index>` BasePiece nodes and `loot:0` RaidLoot node.

- [ ] **Step 1: Run failing runtime expectation**

  Assert seed 7 node count exceeds its current scatter/plot count and includes base pieces indirectly through `nodes >= scatter + plots + stamp + structural`.

- [ ] **Step 2: Add semantic content keys to stamp rows**

  Assign tier/piece keys (`wood.foundation`, `metal.window`, `door`, and equivalent) while constructing rows. Do not derive keys from Unreal object paths.

- [ ] **Step 3: Record and consume BasePiece nodes**

  Before checking `piece:<id>`, append the node with row-local transform and key. Construct the spawn transform from node fields; continue resolving class/mesh through the existing row adapter.

- [ ] **Step 4: Record and consume RaidLoot node**

  Append `loot:0` before spawning the target point and spawn it from the node location.

- [ ] **Step 5: Verify build, all worldgen tests, and duplicate seed sweeps**

  Expected: identical expanded recipe checksums/counts; unchanged BASE/SCATTER/STAMP output; valid recipe; no diff or generator-version change.

## Self-review

- Transform/content fields exist because the current base consumer requires them.
- Semantic keys keep EBS behind an adapter boundary.
- No catalog, resolver interface, inheritance tree, or data asset is introduced prematurely.

