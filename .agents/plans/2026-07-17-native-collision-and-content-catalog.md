# Native Collision and Content Catalog Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Validate contract collision using Unreal's native mesh data, then let generators resolve accepted assets by semantic identity without owning import or loading infrastructure.

**Architecture:** Borrow Unreal Interchange, `UStaticMesh`, body setup, soft object references, and Asset Registry/Asset Manager infrastructure. Build only the missing Gaters rules: collision-policy acceptance and a tiny deterministic semantic catalog that admits independently linted meshes.

**Tech Stack:** Unreal Engine 5.8 C++, Unreal Automation Tests, native `UStaticMesh`, `UBodySetup`, and `TSoftObjectPtr`.

## Global Constraints

- Unreal remains authoritative for asset storage, collision representation, indexing, and loading.
- Gaters owns semantic keys, style/contract compatibility, acceptance diagnostics, and deterministic selection.
- World generators never consume source-tool paths or depend on Blender.
- No custom asset manager, UObject hierarchy, database, plugin, or generalized rule engine.
- Do not commit, branch, or push.

---

### Task 1: Native collision-policy intake

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersAssetIntakeTests.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersAssetIntake.cpp`

**Interfaces:**
- Consumes: `UStaticMesh::GetBodySetup()`, `FGatersAssetContract::Collision`.
- Produces: collision diagnostics through the existing `FGatersAssetIntake::ValidateStaticMesh` result.

- [x] **Step 1:** Change the imported foundation fixture to require simple collision and add deliberate `None` and `Complex` policy counterexamples.
- [x] **Step 2:** Build and run `Gaters.Content.AssetIntake`; require failure because non-`None` collision evidence is not implemented.
- [x] **Step 3:** Inspect native body setup simple primitives and collision trace mode; accept only the declared policy and emit observed-policy diagnostics.
- [x] **Step 4:** Build and rerun the focused test; require the matching fixture to pass and both counterexamples to fail for collision.

### Task 2: Minimum contracted content catalog

**Files:**
- Create: `Unreal/Prototype/Source/Prototype/Public/GatersContentCatalog.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersContentCatalog.cpp`
- Create: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersContentCatalogTests.cpp`
- Modify: `research/machines.json`

**Interfaces:**
- Consumes: `UStaticMesh`, `FGatersAssetContract`, and `FGatersAssetIntake::ValidateStaticMesh`.
- Produces: `FGatersContentCatalog::AddStaticMesh(...)` and `Find(ContentKey, StyleId)` returning a contract plus native soft mesh reference.

- [x] **Step 1:** Write tests proving a valid imported fixture can be found by semantic key/style, a rejected contract is never admitted, an unknown query returns no result, and duplicate semantic candidates resolve deterministically by highest contract version.
- [x] **Step 2:** Build and confirm failure names the absent catalog API.
- [x] **Step 3:** Implement one in-memory catalog using native `TSoftObjectPtr<UStaticMesh>`; admission calls the existing linter and selection compares contract versions.
- [x] **Step 4:** Build and run `Gaters.Content.Catalog`; require all acceptance, rejection, missing-query, and deterministic-selection cases to pass.
- [x] **Step 5:** Mark the catalog active in the machine registry while keeping collision/skeletal expansion explicit in its promotion evidence.

### Task 3: Fresh evidence

**Files:**
- Modify: this plan's checkboxes only.

**Interfaces:**
- Consumes: all prior task outputs.
- Produces: reproducible verification evidence without opening Unreal interactively.

- [x] **Step 1:** Run the headless importer twice and require zero ensures/import errors.
- [x] **Step 2:** Build `PrototypeEditor` and run all `Gaters.*` automation tests.
- [x] **Step 3:** Run machine-registry validation and `git diff --check`.
