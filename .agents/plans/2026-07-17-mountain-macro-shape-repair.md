# Mountain Macro-Shape Repair Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace repeated noisy mountain ridges with broad massifs, connected valleys, and useful plateaus while retaining mountain-scale relief.

**Architecture:** Retune only the mountain branch of `FGatersEnvironment::HeightAt`; all hydrology, semantic classification, streaming, recipes, and materialization continue consuming the same height function. Use seed 0 as the newly held-out dry-mountain failure and preserve seed 53 as the mountain-lake regression.

**Tech Stack:** Unreal Engine 5.8 C++, deterministic Perlin fractals, Unreal Automation Tests, paired gallery capture.

## Global Constraints

- Do not add erosion simulation, terrain stamps, splines, or new generator classes.
- Preserve deterministic seed identity and the four existing environment families.
- Mountain relief remains materially greater than lowland relief.
- Runtime seed 0 gains connected buildable terrain without flattening the whole family.
- Seed 53 keeps local lakes, valid arrival escape, and a reachable base.
- Do not commit, branch, or push.

---

### Task 1: Held-out dry-mountain failure

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersEnvironmentTests.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersTerrainNavigationTests.cpp`

**Interfaces:**
- Consumes: runtime-sized seed 0 through the existing evaluator, semantic field, and traversability evaluator.
- Produces: regression requirements for relief, roughness, buildable fraction, reachable fraction, and component count.

- [x] **Step 1:** Require seed 0's local 30 km window to retain at least 2500 cm relief, stay below 400 cm mean neighbor step, and expose at least 8% buildable samples.
- [x] **Step 2:** Require its runtime semantic field to reach at least 75% of walkable terrain with no more than 48 components while retaining arrival escape and base reachability.
- [x] **Step 3:** Build and run the two focused tests; require failures matching the recorded 1% buildable, 565 cm roughness, 60% reachability, and 116-component evidence.

### Task 2: Minimum mountain formula retune

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersEnvironment.cpp`

**Interfaces:**
- Consumes: existing `Fractal`, seed-derived rotation, phase, and noise offset.
- Produces: the same `HeightAt` API with lower-frequency massifs, subordinate ridges, and broad valley floors.

- [x] **Step 1:** Lower the massif and ridge frequencies, reduce ridge amplitude/detail, and use a broad noise mask so valleys remain outside the steep massif cores.
- [x] **Step 2:** Run the focused metric and navigation tests; tune only existing formula constants until seed 0 and seed 53 both satisfy their independent checks.
- [x] **Step 3:** Run environment variety and base-site sweeps to ensure all families and 128 base-site seeds remain valid.

### Task 3: Visual and full-suite evidence

**Files:**
- Modify: `research/machines.json`
- Modify: this plan's checkboxes only.

**Interfaces:**
- Consumes: the repaired generator and paired gallery system.
- Produces: before/after comparable seed-0 and seed-53 evidence plus fresh automation results.

- [x] **Step 1:** Increment the terrain generator champion/version evidence because seed output changes in place.
- [x] **Step 2:** Capture seed 0 and seed 53 clean/traversal pairs and record their metric vectors.
- [x] **Step 3:** Build, run all `Gaters.*` tests, validate the run archive and machine registry, and run `git diff --check`.
