# Drainage-to-Regional-Water Fit v1 Implementation Plan

> **For agentic workers:** Execute inline with strict red-green verification. Do not
> create a branch, commit, or change rendering.

**Goal:** Prove whether each declared regional water surface is physically associated
with the deterministic drainage graph, while preserving both inputs unchanged.

**Architecture:** Extend `FGatersDrainageNetwork` with one pure fit operation. It maps
regional-water footprints onto drainage cells, records basin, terminal, channel, inlet,
outlet, accumulation, and terrain-intersection evidence, and emits causal issues for
invalid provenance or unsupported surfaces. It does not generate water, modify terrain,
force flow, carve, render, or compose into the environment root.

**Tech stack:** Unreal Engine 5.8 C++, existing `FGatersDrainageRecipe`, existing
`FGatersRegionalWaterRecipe`, Unreal Automation tests.

## Global Constraints

- `WORLD-1`: declared absence remains valid; an empty regional-water recipe is a valid
  fit result.
- Regional water remains the declaration authority; drainage remains physical evidence.
- No new generator, Actor, asset, material, terrain carving, or root version.
- Preserve stable IDs and exact repeated-input output.

### Task 1: Failing fit contract

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersDrainageNetwork.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersDrainageNetworkTests.cpp`

**Interfaces:**
- Consumes: `FGatersDrainageRecipe`, `FGatersRegionalWaterRecipe`, datum tolerance.
- Produces: `FGatersDrainageWaterFitResult` with per-surface physical associations and
  causal `FGatersDrainageIssue` diagnostics.

- [x] Declare the wished-for pure fit result and `FitRegionalWater` entry point.
- [x] Add a synthetic downstream-water test covering determinism, cell coverage,
  submerged terrain, basins, terminals, channel contact, inlet/outlet flow, and maximum
  accumulation.
- [x] Add counterexamples for invalid water version, duplicate identity, out-of-grid
  coverage, unsupported terrain datum, invalid drainage provenance, and valid empty
  scarcity.
- [x] Build and run the focused test to observe the expected RED failure caused by the
  unimplemented fit operation.

### Task 2: Minimum pure fit implementation

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersDrainageNetwork.cpp`

**Interfaces:**
- Consumes: immutable recipes from Task 1.
- Produces: deterministic associations without altering either recipe.

- [x] Validate only consumed drainage and regional-water provenance.
- [x] Use exact circle-to-grid-cell overlap against the existing drainage cell size.
- [x] Record unique cell, basin, terminal, channel, inlet, outlet, accumulation, and
  submerged-terrain evidence per surface.
- [x] Emit one causal rule family for provenance, identity, coverage, and terrain support.
- [x] Run the focused tests to GREEN.

### Task 3: Held-out root evidence and closure

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersDrainageNetworkTests.cpp`
- Modify: `research/machines.json`
- Modify: `.agents/workstreams/Primary Builder — World & Terrain.md`

**Interfaces:**
- Consumes: real compiled Environment Recipes and their independently built drainage.
- Produces: evidence that names the first unsupported seed/surface rather than repairing
  it inside the evaluator.

- [x] Add real-root repeated-seed fit coverage across dry and wet regional declarations.
- [x] Run focused Drainage, Regional Water, Climate, and Environment Recipe automation.
- [x] Run the complete Gaters suite and repository validators.
- [x] Update machine truth with the exact supported guarantee or falsified case; promote
  only if the held-out fit gate is satisfied.

Requirements checked: `WORLD-1`; exceptions: none.
