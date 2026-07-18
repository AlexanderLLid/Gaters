# Terrain Visual Evidence Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Every captured terrain seed produces comparable clean and diagnostic PNGs tied immutably to its metrics and recipe evidence.

**Architecture:** Reuse Unreal's offscreen screenshot request, the existing gallery camera, and `AGatersChunk::DrawTraversalDebug`. Extend the existing run writer with optional paired artifact parsing and SHA-256 evidence; no second terrain renderer or visual scoring model is introduced.

**Tech Stack:** Unreal Engine 5.8 C++, PowerShell, native PNG capture, SHA-256.

## Global Constraints

- Clean and diagnostic captures use the same seed, terrain, stream radius, resolution, and camera.
- A gallery run accepts either both captures or neither; a partial pair fails.
- Generated screenshots remain under `Saved/` and are evidence, not source assets.
- Existing non-gallery headless sweeps remain valid.
- Do not commit, branch, or push.

---

### Task 1: Paired artifact archive contract

**Files:**
- Modify: `Unreal/Prototype/Scripts/Test-EnvironmentRunArchive.ps1`
- Modify: `Unreal/Prototype/Scripts/Write-EnvironmentRun.ps1`

**Interfaces:**
- Consumes: optional log lines `gallery_beauty=<absolute PNG>` and `gallery_traversal=<absolute PNG>`.
- Produces: `artifacts.beauty` and `artifacts.traversal`, each containing absolute path and SHA-256.

- [x] **Step 1:** Add fake paired PNG files and gallery log lines to the archive test; assert paths and hashes are recorded, and a one-sided pair is rejected.
- [x] **Step 2:** Run `Test-EnvironmentRunArchive.ps1`; require failure because gallery artifacts are absent from the record.
- [x] **Step 3:** Parse the optional pair, reject missing/empty files or a partial pair, and record SHA-256 beside each absolute path.
- [x] **Step 4:** Rerun the archive test; require paired evidence acceptance and partial-pair rejection.

### Task 2: Same-camera clean and diagnostic capture

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersTestSpawner.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersTestSpawner.cpp`
- Modify: `Unreal/Prototype/Scripts/RunEnvironmentSweep.ps1`

**Interfaces:**
- Consumes: existing `-GatersGallery`, `PrepareGalleryCapture`, and `DrawTraversalDebug`.
- Produces: `seed-<seed>-beauty.png` followed by `seed-<seed>-traversal.png` from one world execution.

- [x] **Step 1:** Make the sweep delete only the two known per-seed output files before launch and require both non-empty PNGs afterward.
- [x] **Step 2:** Run one captured seed; require failure because the current spawner produces only `seed-<seed>.png`.
- [x] **Step 3:** Request the clean capture, then use a short timer to draw traversal evidence and request the diagnostic capture from the unchanged camera.
- [x] **Step 4:** Rerun one captured seed; require both files and archive hashes.

### Task 3: Fresh evidence

**Files:**
- Modify: `research/machines.json`
- Modify: this plan's checkboxes only.

**Interfaces:**
- Consumes: paired capture output and all existing Unreal tests.
- Produces: active visual-evaluation evidence without promoting subjective quality claims.

- [x] **Step 1:** Record the terrain gallery capability without marking `evaluation.world-visual` verified.
- [x] **Step 2:** Build `PrototypeEditor`, run all `Gaters.*` tests, run archive verification, validate the registry, and run `git diff --check`.
