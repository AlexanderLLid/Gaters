# Unreal Static Mesh Importer Checkpoint Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Prove that one external source mesh imports into Unreal headlessly, reproducibly, and with machine-readable provenance before world generators consume it.

**Architecture:** Use a tiny ASCII OBJ as the portable source fixture and Unreal's native `AssetImportTask` through the already-enabled Python plugin. A PowerShell verifier invokes the importer twice, requires the same object identity and source fingerprint, and preserves a JSON report; the generated `.uasset` is a derived cache, never the source of truth.

**Tech Stack:** Unreal Engine 5.8 Python API, PowerShell, OBJ, JSON.

## Global Constraints

- Do not require Blender on this workstation; Blender will later export into the same source-artifact boundary.
- Do not add Unreal runtime dependencies or expose source paths to world generators.
- Import must be unattended, replace-in-place, and idempotent at the contract level.
- Do not commit, branch, or push.

---

### Task 1: Failing importer verifier

**Files:**
- Create: `Unreal/Prototype/Scripts/Test-ContractAssetImport.ps1`

**Interfaces:**
- Consumes: `ImportContractAsset.py`, the external fixture, and UnrealEditor-Cmd.
- Produces: a nonzero failure until the importer and fixture exist.

- [x] **Step 1:** Add the verifier that runs two imports and compares their JSON evidence.
- [x] **Step 2:** Run it and confirm failure names the missing importer.

### Task 2: Minimal native import adapter

**Files:**
- Create: `research/fixtures/assets/wood-foundation.obj`
- Create: `Unreal/Prototype/Scripts/ImportContractAsset.py`
- Generated: `Unreal/Prototype/Content/Gaters/Generated/Fixtures/SM_WoodFoundation.uasset`
- Generated: `Unreal/Prototype/Saved/AssetImport/wood-foundation.json`

**Interfaces:**
- Consumes: environment variables `GATERS_IMPORT_SOURCE`, `GATERS_IMPORT_DESTINATION`, `GATERS_IMPORT_NAME`, and `GATERS_IMPORT_REPORT`.
- Produces: one imported `StaticMesh` object path plus source SHA-256, importer version, and import identity in JSON.

- [x] **Step 1:** Add the centered foundation OBJ fixture in canonical contract coordinates.
- [x] **Step 2:** Implement the minimum `AssetImportTask` adapter with explicit failure diagnostics.
- [x] **Step 3:** Run the verifier; require two contract-equivalent reports and one generated asset.

### Task 3: Evidence and next focus

**Files:**
- Modify: `research/machines.json`

**Interfaces:**
- Consumes: importer verifier output and generated report.
- Produces: importer status based on evidence; the Asset Contract remains active until the intake linter measures the imported mesh.

- [x] **Step 1:** Record importer evidence without promoting beyond what the fixture proves.
- [x] **Step 2:** Run registry validation, importer verification, all `Gaters.*` tests, and `git diff --check`.
