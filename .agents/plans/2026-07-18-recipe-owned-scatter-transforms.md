# Recipe-owned scatter transforms implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:executing-plans` to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make streamed content recipes own the exact tree/rock transforms Unreal renders.

**Architecture:** Move the existing stable-ID-derived yaw and scale calculation from `FGatersVisualMaterializer::Plan` into `FGatersContentCellRecipe::Generate`. The materializer continues selecting native ISM batches and catalog meshes, but copies compiled transforms without mutation.

**Tech stack:** Unreal Engine 5.8 C++, Unreal Automation Tests, native `FTransform`, `FCrc`, and ISM materialization.

## Global constraints

- Do not touch Blender or generated art.
- Preserve current rendered scatter scale/yaw values exactly.
- Add no new class, dependency, setting, migration, or compatibility path.
- Do not change claim-marker presentation in this checkpoint.
- Use test-first red/green verification and do not commit.

---

### Task 1: Move scatter presentation transforms into recipe data

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersContentCellRecipeTests.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersVisualMaterializerTests.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersContentCellRecipe.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersVisualMaterializer.cpp`

**Interfaces:**
- Consumes: `FGatersContentCellPlacement::Transform` and `FGatersCompiledNode::Transform`.
- Produces: content-cell recipe version 2 with final scatter yaw/scale; materializer plans whose scatter transform equals the compiled node transform.

- [x] **Step 1: Write the failing tests**

Change the content-cell version expectation to `2`, assert every placement scale is the existing tree or rock base scale multiplied by a deterministic `0.85..1.15` variation, and assert the visual plan preserves a compiled tree transform exactly.

- [x] **Step 2: Run focused tests and verify red**

Run the Unreal build, then invoke `Automation RunTests Gaters.Worldgen.ContentCells;Quit`
and `Automation RunTests Gaters.Runtime.VisualMaterializer;Quit` in separate Unreal
processes. UE 5.8 does not accept a second semicolon-separated `Automation RunTests`
request in the same `ExecCmds` value.

Expected: content-cell version/scale assertions and materializer transform-preservation assertion fail because v1 leaves scale at identity and the materializer mutates it.

- [x] **Step 3: Implement the minimum move**

In `FGatersContentCellRecipe::Generate`, derive the existing visual hash from `Placement.Id`, set the exact existing yaw, and set `(0.8,0.8,5.0)` for trees or `(1.8,1.4,0.8)` for rocks multiplied by the existing variation. Set recipe version to `2`. Remove only the matching hash/yaw/scale mutation from `FGatersVisualMaterializer::Plan`.

- [x] **Step 4: Verify green**

Run the build, focused tests, full `Gaters` automation, machine-registry validation, and headless seed 7. Expected: all tests pass; seed 7 retains 72 streamed placements, four native ISM batches, generated-native-LOD catalog selection, and no performance issues.

- [x] **Step 5: Record evidence**

Update `research/machines.json` and the streamed-content report only if the verified ownership boundary or champion changed. Keep physical-fit v1 status honest; this checkpoint enables later runtime fit evaluation but does not claim it.
