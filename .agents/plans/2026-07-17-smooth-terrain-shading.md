# Smooth Terrain Shading Implementation Plan

> **For agentic workers:** Execute inline; do not branch or commit unless the user asks.

**Goal:** Remove whole-triangle color borders and split-normal shading seams without changing generated geometry or adding authored assets.

**Architecture:** Extend the pure terrain palette with a continuous color evaluator. Each terrain vertex samples the deterministic global height field to derive a cross-cell-consistent normal and color, then the streamed mesh renders once with a minimal generated project vertex-color material.

**Tech Stack:** Unreal Engine 5.8 C++, Dynamic Mesh normal/color overlays, Unreal Automation Tests.

## Global Constraints

- Preserve terrain geometry, collision, streaming, generator version, and seed determinism.
- Use one generated default-lit project material with no texture dependencies or emissive output.
- Keep visual classification pure and independently testable.
- Do not increase triangle count.

---

### Task 1: Continuous palette contract

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersTerrainPalette.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersTerrainPalette.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersTerrainPaletteTests.cpp`

- [x] Add failing tests that colors blend continuously around slope and shoreline transitions.
- [x] Implement a pure `BlendColor` evaluator using smooth thresholds and existing palettes.
- [x] Run the focused palette test.

### Task 2: Cross-cell vertex shading

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersTerrainCell.cpp`

- [x] Replace preserved split normals with per-vertex normals derived from global height samples.
- [x] Populate the dynamic mesh color overlay with `BlendColor` results.
- [x] Replace three BasicShapeMaterial slots with one default-lit vertex-color material.
- [x] Build Development and run all `Gaters.Worldgen` tests.

### Task 3: Visual regression

- [x] Capture seeds 53, 7, 2, and 4 at gallery radius 4.
- [x] Compare material transitions, dark seams, cell boundaries, water visibility, and capture cost.
- [x] Retune only the pure thresholds if the evidence requires it (no correctness retune needed).
