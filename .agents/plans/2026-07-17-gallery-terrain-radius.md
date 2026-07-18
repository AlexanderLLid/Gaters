# Gallery Terrain Radius Implementation Plan

> **For agentic workers:** Execute inline with `superpowers:executing-plans`; do not branch or commit unless the user asks.

**Goal:** Make remote gallery screenshots load terrain across the camera framing while preserving the normal gameplay streaming budget.

**Architecture:** `AGatersChunk` owns an editor-visible gallery radius and a public one-shot preparation method. `UGatersTestSpawner` invokes it only for `-GatersGallery`, optionally overriding the radius from `-GatersGalleryRadius=N`, before requesting the screenshot.

**Tech Stack:** Unreal Engine 5.8 C++, Unreal Automation Tests, existing offscreen environment sweep.

## Global Constraints

- Normal play remains `TerrainLoadRadius = 1`.
- Gallery loading is synchronous and gallery-only.
- Pawn-driven streaming must not unload gallery cells before capture.
- No new dependencies or camera system.

---

### Task 1: Gallery-only terrain radius

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersChunk.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersChunk.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersEnvironmentTests.cpp`

- [x] Add a failing defaults test proving the gallery radius exceeds the gameplay radius.
- [x] Build and confirm failure because `GalleryTerrainLoadRadius` is absent.
- [x] Add `GalleryTerrainLoadRadius = 4` and `PrepareGalleryCapture(int32 OverrideRadius = 0)`.
- [x] Let the existing refresh path accept a radius override and suspend pawn refresh after gallery preparation.
- [x] Run the focused defaults test.

### Task 2: Capture integration and visual proof

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersTestSpawner.cpp`
- Modify: `Unreal/Prototype/Scripts/RunEnvironmentSweep.ps1`

- [x] Before requesting a gallery screenshot, locate the chunk and call `PrepareGalleryCapture`.
- [x] Parse `-GatersGalleryRadius=N`; expose the same value as `RunEnvironmentSweep.ps1 -GalleryRadius`.
- [x] Log the effective radius and active terrain-cell count for evidence.
- [x] Build Development and run all `Gaters.Worldgen` tests.
- [x] Capture seed 53 offscreen and visually confirm terrain fills the camera framing instead of showing floating water/assets.
