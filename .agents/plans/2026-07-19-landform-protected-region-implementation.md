# Landform Protected Region Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use
> `superpowers:executing-plans` to implement this plan task-by-task. Steps use checkbox
> syntax for tracking. Do not branch, commit, or push unless the human asks.

**Goal:** Preserve seed-derived playable Arrival/build terrain while allowing extreme
landform processes everywhere outside smooth protected regions.

**Architecture:** The pure landform recipe gains generic protected circles and scales
the total process delta from zero inside each inner radius to full outside its outer
radius. Primary runtime composition derives those circles from the accepted Arrival
geometry and a champion-terrain build candidate, then independently regenerates the
World Recipe against the challenger.

**Tech Stack:** Unreal Engine 5.8 C++, Unreal Automation Tests, existing environment
sweep/archive scripts.

## Global Constraints

- No village, population, resource, climate, drainage, art, or encounter guarantee.
- No global cap on physical signals and no seed-specific branch.
- Current terrain remains champion until held-out evidence passes.
- Protected geometry is pure data and has no Actor or asset dependency.

---

### Task 1: Pure protected-region field contract

**Files:**

- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersLandformProcessField.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersLandformProcessField.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersLandformProcessFieldTests.cpp`

**Interfaces:**

- Add `FGatersLandformProtectedRegion { Id, Center, InnerRadius, OuterRadius }`.
- Extend `FGatersLandformProcessField::Compile` with an optional protected-region array.
- Copy valid regions into recipe version `2`.
- Expose `ProcessInfluence` on each query sample: `0` means exact base preservation and
  `1` means full requested process response.

- [x] Add a failing automation test proving exact inner preservation, smooth transition,
  full outside response, deterministic compilation, causal ID/geometry rejection, and
  protected seed-`131` build-candidate rediscovery through `FGatersWorldRecipe::Generate`.
- [x] Build and verify RED on the missing protected-region API.
- [x] Implement the minimum validation and `min` smooth influence across regions.
- [x] Scale every contribution by the final process influence and run
  `Gaters.Worldgen.LandformProcesses`.

### Task 2: Primary runtime composition

**Files:**

- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersChunk.cpp`

**Interfaces:**

- Before attaching processes, query the champion terrain with the existing
  `FindBaseSite(MinBaseDistance, MaxBaseDistance, BaseFootprintRadius,
  MaxFoundationDrop)` contract.
- Always protect the Arrival footprint; protect the champion build footprint only when
  the champion supplies one.
- Do not inject the champion result into World Recipe generation.

- [x] Add the two generic protected circles in `RollSite` and retain normal downstream
  generation.
- [x] Build and run focused environment, landform, route, and recipe automation.

### Task 3: Held-out evidence and machine truth

**Files:**

- Modify: `research/machines.json`
- Modify: `.agents/workstreams/Primary Builder — World & Terrain.md`

- [x] Rerun glacial seed `131` world-only and require a valid base site, Arrival escape,
  valid route plan, finite terrain, and valid performance evidence.
- [x] Rerun all twenty held-out seed/brief combinations into a new immutable JSONL
  archive.
- [x] Capture updated seed `131` baseline/glacial galleries and validate both pairs.
- [x] Run the complete `Gaters` automation suite.
- [x] Keep the challenger active unless every promotion condition passes; record any new
  counterexample as the next isolated experiment.
- [x] Run machine/shared-doc validators and scoped `git diff --check`.

Requirements checked: generated-content boundary and the approved protected-playable-
region design. Exceptions: no commit because `AGENTS.md` requires explicit human
authorization.
