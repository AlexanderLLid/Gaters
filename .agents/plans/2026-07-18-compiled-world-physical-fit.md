# Compiled World Physical Fit Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:executing-plans`
> to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Evaluate generated placements with the exact selected asset contract and transform
that Unreal materializes.

**Architecture:** The world compiler copies the catalog-selected `FGatersAssetContract`
onto each compiled asset node. `FGatersPhysicalFitEvaluator` batch-evaluates those nodes
against the existing materialized terrain field, while `AGatersChunk::CompileWorld`
reports a summary and causal issues after every initial or streamed recompile. The
evaluator observes only; repair and rejection remain later promotion policy.

**Tech Stack:** Unreal Engine 5.8 C++, existing world compiler/catalog, pure physical-fit
evaluator, Unreal Automation Tests.

## Global Constraints

- Do not touch Blender files, generated art, terrain, or placement transforms.
- Preserve the catalog as the single selected-asset source.
- Add no new subsystem, actor, plugin, dependency, repair rule, or tuning setting.
- Evaluate terrain contacts only in this checkpoint; engine collision, navigation, and
  pairwise obstacle gathering remain separate evidence adapters.
- Do not commit, branch, or push.

---

### Task 1: Preserve selected mechanical facts

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersWorldCompilerTests.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersWorldCompiler.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersWorldCompiler.cpp`

**Interfaces:**
- Consumes: `FGatersCatalogAsset::Contract` selected by `FGatersContentCatalog`.
- Produces: `FGatersCompiledNode::AssetContract`, unset for semantic or unresolved nodes
  and set to the exact selected contract for compiled asset nodes.

- [x] **Step 1: Write the failing compiler assertion**

Assert that the compiled foundation node has an asset contract and that its asset ID,
content key, version, contacts, and bounds match the selected fixture contract.

- [x] **Step 2: Build and verify red**

Run the Unreal editor build. Expected: compilation fails because
`FGatersCompiledNode::AssetContract` does not exist.

- [x] **Step 3: Implement the minimum compiler copy**

Add `TOptional<FGatersAssetContract> AssetContract` to `FGatersCompiledNode` and assign
`Asset->Contract` only after the selected contract validates.

- [x] **Step 4: Build and run `Gaters.Runtime.WorldCompiler`**

Expected: the compiler test passes with the selected mechanical facts intact.

---

### Task 2: Evaluate actual compiled terrain contacts

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersPhysicalFitEvaluatorTests.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersPhysicalFitEvaluator.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersPhysicalFitEvaluator.cpp`

**Interfaces:**
- Consumes: `FGatersCompiledWorld`, `FGatersEnvironment`, pad radius, route target, and
  `FGatersPhysicalFitSettings`.
- Produces: one `FGatersPhysicalFitEvaluation` per compiled node carrying a selected
  asset contract; semantic nodes are ignored. Every issue carries both recipe and
  selected asset identity so runtime evidence names the failing contract.

- [x] **Step 1: Write the failing batch-evaluation test**

First assert an existing floating fixture records its selected asset ID. Create one
compiled contracted node aligned to sampled terrain and one semantic node.
Assert `EvaluateWorld` returns one valid evaluation. Raise the contracted node and assert
the same call returns `fit.contact.floating` naming its compiled recipe ID.

- [x] **Step 2: Build and verify red**

Expected: compilation fails because `FGatersPhysicalFitEvaluator::EvaluateWorld` does
not exist.

- [x] **Step 3: Implement the minimum pure loop**

For every compiled node with `AssetContract`, call existing `SampleTerrain` and
`Evaluate`; return the evaluations without spawning, tracing, repairing, or mutating.

- [x] **Step 4: Run `Gaters.Evaluation.PhysicalFit`**

Expected: all existing contact/clearance fixtures and the new compiled-world fixture pass.

---

### Task 3: Runtime evidence and promotion record

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersChunk.cpp`
- Modify: `research/machines.json`
- Modify: `.agents/reports/streamed-content-runtime.md`

**Interfaces:**
- Consumes: the valid compiled world produced by `AGatersChunk::CompileWorld`.
- Produces: `FIT evaluated=<n> valid=<n> issues=<n>` plus detailed log-only
  `FIT_FAIL` entries containing rule, recipe, asset, subject, and obstacle IDs.

- [x] **Step 1: Add runtime reporting through the pure evaluator**

After a valid recipe-matching compile, evaluate against the same route target used by
terrain materialization. Put only the summary on screen; send detailed failures to the
Unreal log so a bad generated world remains diagnosable without covering the viewport.

- [x] **Step 2: Verify the complete checkpoint**

Run the editor build, focused compiler and physical-fit filters in separate processes,
full `Gaters` automation, registry validation, `git diff --check`, and headless seed 7.
Expected: all automation passes and seed 7 emits compile, FIT, content-streaming, native
ISM, generated-rock, and performance evidence. Fit failures are evidence, not a failed
build, until a promotion policy is explicitly defined.

- [x] **Step 3: Record only verified capability**

Update the physical-fit and compiler verifier/champion text with the observed evidence.
Keep collision/navigation/attachment and automatic repair pending.

## Self-review

- The selected contract is copied once at the compiler boundary; no catalog re-query or
  parallel source of truth is introduced.
- The batch evaluator reuses the existing single-candidate policy instead of duplicating
  fit rules.
- This checkpoint cannot alter visuals or generated terrain.
- Runtime failures remain causal evidence and do not silently reject or move content.
