# Terrain and Water Material Pass Implementation Plan

> **For agentic workers:** Execute inline with `superpowers:executing-plans`; do not branch or commit unless the user asks.

**Goal:** Replace flat plastic terrain colours and marker-blue water with readable stylized terrain bands and native Unreal water across streamed seeds.

**Architecture:** Keep terrain geometry unchanged. A pure palette classifier maps terrain family, height, and triangle slope to three material slots; every streamed cell applies matching dynamic colour instances. Water uses Unreal's packaged water material with the current deterministic surface footprints and falls back to the existing project material if unavailable.

**Tech Stack:** Unreal Engine 5.8 C++, Dynamic Mesh material IDs, Unreal Automation Tests, offscreen gallery capture.

## Global Constraints

- Preserve terrain determinism, collision, streaming seams, and generator version.
- Use only native/project materials already installed; create no binary material assets unattended.
- Keep the classifier pure and independently testable.
- Treat gallery captures as perceptual evidence, not automatic proof of quality.

---

### Task 1: Terrain palette contract

**Files:**
- Create: `Unreal/Prototype/Source/Prototype/Public/GatersTerrainPalette.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersTerrainPalette.cpp`
- Create: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersTerrainPaletteTests.cpp`

- [x] Add a failing test for low shoreline, ordinary ground, steep rock, and high mountain rock classification.
- [x] Build and confirm failure because `FGatersTerrainPalette` does not exist.
- [x] Implement three stable slots and terrain-family colour triples.
- [x] Run the focused palette test.

### Task 2: Streamed terrain material bands

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersTerrainCell.cpp`

- [x] Enable the dynamic-mesh material-ID attribute after displacement.
- [x] Classify each triangle from average height and geometric normal Z.
- [x] Create three BasicShapeMaterial dynamic instances from the environment palette and bind slots 0–2.
- [x] Build Development and run terrain seam, palette, and environment tests.

### Task 3: Native water and gallery evaluation

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersChunk.cpp`

- [x] Load `/Engine/EngineMaterials/WaterMaterial.WaterMaterial` for generated water, retaining `MI_Claimed` as fallback.
- [x] Run all `Gaters.Worldgen` tests.
- [x] Capture seeds 0, 2, 4, and 7 with gallery radius 4 (plus seed 53 lake regression).
- [x] Inspect all PNGs for terrain coverage, band readability, water visibility, seams, and material failures.
