# Static Mesh Intake Linter Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Independently compare an imported static mesh against its Asset Contract before any catalog or world generator can use it.

**Architecture:** Add one pure validation adapter from `UStaticMesh` measurements to `FGatersAssetContract`. It validates the contract first, then compares measured bounds/pivot and required sockets; collision inspection waits for a fixture that explicitly imports collision geometry.

**Tech Stack:** Unreal Engine 5.8 C++, Unreal Automation Tests.

## Global Constraints

- The linter is independent from the Python importer and must not trust importer reports.
- No catalog, UObject hierarchy, generic rule engine, or character support in this checkpoint.
- The generated fixture must be reproducible from `wood-foundation.obj` before tests run.
- Do not commit, branch, or push.

---

### Task 1: Imported static-mesh fixture tests

**Files:**
- Create: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersAssetIntakeTests.cpp`

**Interfaces:**
- Consumes: `/Game/Gaters/Generated/Fixtures/SM_WoodFoundation` and `FGatersAssetContract`.
- Produces: failing tests for the absent `FGatersAssetIntake::ValidateStaticMesh` API.

- [x] **Step 1:** Add one matching contract plus wrong-pivot and missing-socket counterexamples.
- [x] **Step 2:** Build and confirm failure names the missing intake header.

### Task 2: Minimum independent linter

**Files:**
- Create: `Unreal/Prototype/Source/Prototype/Public/GatersAssetIntake.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersAssetIntake.cpp`

**Interfaces:**
- Produces: `FGatersAssetIntake::ValidateStaticMesh(const UStaticMesh&, const FGatersAssetContract&, TArray<FString>&, float)`.

- [x] **Step 1:** Validate the contract, static render class, measured bounds center/extent, and required socket names.
- [x] **Step 2:** Build and run `Gaters.Content.AssetIntake`; require valid import acceptance and intended counterexample rejection.

### Task 3: Evidence

**Files:**
- Modify: `research/machines.json`

- [x] **Step 1:** Keep the linter active until collision and skeletal fixtures exist; advance current focus only to its next missing fixture.
- [x] **Step 2:** Run importer verification, build, all `Gaters.*` tests, registry validation, and `git diff --check`.
