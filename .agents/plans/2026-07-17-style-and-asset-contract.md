# Style and Asset Contract Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Record the approved clean-mid-poly/painted-warmth target and add one testable Unreal-independent asset contract that later Blender, importer, catalog, and world machines can share.

**Architecture:** Keep the visual call as one portable JSON artifact plus a short design-compass entry. Implement the mechanical asset boundary as a plain C++ value with deterministic validation; it owns no Unreal asset paths or Actors. Defer JSON import, catalog lookup, and character-specific anatomy until their machines become current.

**Tech Stack:** Unreal Engine 5.8 C++, Unreal Automation Tests, JSON contract data, PowerShell registry validation.

## Global Constraints

- Recipes, contracts, and source artifacts are authoritative; Unreal assets and Actors are derived outputs.
- The visual target is clean mid-poly structure with hand-painted warmth, not photorealism or visibly faceted low-poly terrain.
- Do not add dependencies, Blueprint classes, UObject hierarchies, or an importer in this checkpoint.
- Do not commit, branch, or push.

---

### Task 1: Record the approved contracts

**Files:**
- Modify: `AGENTS.md`
- Modify: `docs/INSPIRATION.md`
- Modify: `docs/questions.md`
- Create: `research/style-contract.json`

**Interfaces:**
- Consumes: approved visual comparison A plus the warmth and shape language of C.
- Produces: `gaters.clean-midpoly-painted` style identity version 1 and the Unreal adapter boundary.

- [x] **Step 1:** Replace the deferred art fork with the approved visual call and remove settled question `#7`.
- [x] **Step 2:** Add the generated-content boundary to `AGENTS.md`.
- [x] **Step 3:** Add the portable style contract and validate it with `ConvertFrom-Json`.

### Task 2: Add the mechanical Asset Contract test-first

**Files:**
- Create: `Unreal/Prototype/Source/Prototype/Public/GatersAssetContract.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersAssetContract.cpp`
- Create: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersAssetContractTests.cpp`

**Interfaces:**
- Consumes: semantic content keys and `gaters.clean-midpoly-painted` style identity.
- Produces: `FGatersAssetContract::Validate(TArray<FString>&)` covering identity, scale, orientation, bounds, contacts, clearance, collision, ports, render class, and persistence.

- [x] **Step 1:** Write fixtures for one valid ground asset and deliberately broken fields.
- [x] **Step 2:** Build and confirm the new test fails because `GatersAssetContract.h` is absent.
- [x] **Step 3:** Add the smallest plain C++ contract and deterministic diagnostics.
- [x] **Step 4:** Build and run `Gaters.Content.AssetContract`; require all fixtures to pass.

### Task 3: Promote from evidence

**Files:**
- Modify: `research/machines.json`

**Interfaces:**
- Consumes: focused automation log and portable style contract.
- Produces: updated machine statuses/champions and next current focus.

- [x] **Step 1:** Mark the style contract active with its selected candidate; keep it unverified until counterexample galleries exist.
- [x] **Step 2:** Keep the asset contract active until real imported pivot, contact, socket, and skeletal fixtures complete its challenge set.
- [x] **Step 3:** Run the registry validator, all `Gaters.Worldgen` plus `Gaters.Content` automation, and `git diff --check`.
