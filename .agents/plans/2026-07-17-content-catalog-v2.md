# Contracted Content Catalog v2 Implementation Plan

> **For agentic workers:** Execute inline with TDD. Do not branch, commit, or push unless the human asks.

**Goal:** Resolve semantic recipe keys to compatible placeholder or finished assets with explainable rejection and deterministic replacement.

**Architecture:** The catalog keeps accepted contracts and optional mesh references. A pure query filters by style, collision, render class, bounds, and required ports, records stable rejection diagnostics, then deterministically selects the highest compatible version. Withdrawal removes an artifact and immediately exposes the next compatible candidate.

**Tech Stack:** Unreal Engine 5.8 C++, asset contracts/intake, soft mesh references, Unreal automation tests.

## Global Constraints

- Production-tool paths never enter queries.
- Placeholder contracts pass the same mechanical contract validation as art assets.
- Existing static-mesh intake remains the only path for cataloging a real mesh.
- Selection is deterministic by contract version then asset ID.
- Do not commit or branch.

### Task 1: Query diagnostics and capabilities

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersContentCatalog.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersContentCatalog.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersContentCatalogTests.cpp`

- [x] Add failing fixtures for missing keys and incompatible collision, render class, bounds, and required ports.
- [x] Implement typed query/result diagnostics and deterministic compatible selection.
- [x] Preserve the existing simple `Find` adapter.

### Task 2: Placeholder replacement and withdrawal

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersContentCatalog.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersContentCatalog.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersContentCatalogTests.cpp`

- [x] Add failing fixtures for a valid contract-only placeholder, art replacement, rejected invalid placeholder, and champion withdrawal fallback.
- [x] Implement contract-only registration and artifact withdrawal.
- [x] Run focused and complete Gaters suites.

### Task 3: Evidence

**Files:**
- Modify: `research/machines.json`
- Modify: `.agents/plans/2026-07-17-content-catalog-v2.md`

- [x] Record catalog v2 as champion while keeping it active behind its asset-intake-linter prerequisite.
- [x] Run registry, archive, gallery, and diff checks.
- [x] Select the asset-intake linter; verifying it will unlock catalog promotion and the world compiler.
