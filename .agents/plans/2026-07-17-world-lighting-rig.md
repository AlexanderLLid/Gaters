# World Lighting Rig Implementation Plan

> **For agentic workers:** Execute inline with `superpowers:executing-plans`; do not commit without the human asking.

**Goal:** Replace the inherited greybox lighting with a deterministic, readable native-Unreal rig shared by play and automated terrain captures.

**Phase:** Early evaluation infrastructure only. Final biome mood, weather, time-of-day, and cinematic lighting stay deferred until environment shapes, materials, and raid readability are proven.

**Architecture:** `UGatersTestSpawner` owns the temporary prototype world setup, so it will configure the existing native light actors once at world start. The rig uses Unreal's Directional Light, Sky Light, Sky Atmosphere, and Post Process actors; no custom renderer or lighting framework.

**Tech Stack:** Unreal Engine 5.8 C++, native lighting actors, PowerShell PNG regression check.

## Constraints

- Preserve generator v7 and all terrain geometry.
- Keep exposure fixed so different seeds remain visually comparable.
- Make shadowed terrain readable without flattening all slope definition.
- Touch the fewest existing files and add no dependency.

### Task 1: Encode visual failure

**Files:**
- Keep: `Unreal/Prototype/Scripts/Test-GalleryImage.ps1`

- [x] Verify seed 0 currently fails because lower-frame near-black coverage is above 1%.
- [x] Extend the gallery sweep to run this check for captured beauty images.

### Task 2: Configure the native lighting rig

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersTestSpawner.cpp`

- [x] Configure one directional sun, one skylight fill, and fixed post-process exposure at world start.
- [x] Reuse level actors; do not create a new actor class.
- [x] Log the effective rig once for headless evidence.

### Task 3: Verify real output

- [x] Build `PrototypeEditor Win64 Development`.
- [x] Capture seeds 0 and 53 with the normal lit renderer.
- [x] Require both beauty images to pass the dark-pixel and mean-luminance checks.
- [x] Run the complete `Gaters` automation suite and `git diff --check`.
- [x] Visually inspect both captures before claiming success.
