# Drainage Feature Candidates v1 Implementation Plan

> **For agentic workers:** Execute inline with strict red-green verification. Do not
> create a branch, commit, carve terrain, or add rendering.

**Goal:** Compile deterministic neutral river-system, lake, wetland, delta, and waterfall
candidates from the accepted drainage graph and regional-water fit.

**Architecture:** Extend `FGatersDrainageNetwork` with one pure candidate compiler over
the existing drainage recipe, regional-water recipe, and fit result. Candidates preserve
their source cells, segments, basins, and water-surface IDs. They declare physical feature
kind only; they do not choose art, gameplay, sites, encounters, or terrain modification.

**Tech stack:** Unreal Engine 5.8 C++ and existing Unreal Automation.

## Global Constraints

- `WORLD-1`: zero candidates of any or every optional type is valid.
- No named-terrain-family checks and no universal feature-presence assertion.
- No second drainage graph, water generator, Actor, asset, material, carving, or root
  version.
- Stable identity derives only from accepted source IDs and basin/cell identity.

### Task 1: Failing pure candidate contract

- [x] Declare versioned feature kind, candidate, settings, recipe, and compile result in
  `GatersDrainageNetwork.h`.
- [x] Add one synthetic fixture containing a channel system, terminal lake, wet low-drop
  basin, ocean mouth, and supported waterfall.
- [x] Require exact determinism, stable unique IDs, and complete source provenance.
- [x] Add valid dry/empty scarcity plus corrupted drainage, water, and fit provenance.
- [x] Build and observe RED only because the candidate compiler is absent.

### Task 2: Minimum compiler

- [x] Group channel segments by basin into neutral river-system candidates.
- [x] Convert fitted lake surfaces into lake candidates without inventing absent lakes.
- [x] Group qualifying wet low-drop cells by basin into wetland candidates.
- [x] Emit deltas only for channel-supported ocean mouths.
- [x] Preserve existing waterfall segment evidence as waterfall candidates.
- [x] Run the focused Drainage Network suite to GREEN.

### Task 3: Held-out evidence and truth

- [x] Challenge five real compiled roots at the proven 65-by-65 fit resolution.
- [x] Prove repeated output, unique IDs, valid source references, and optional absence.
- [x] Run Worldgen and complete Gaters automation.
- [x] Run registry/docs validation and diff checks.
- [x] Update `world.drainage-network` and Primary status without claiming carving,
  rendering, or final river/lake/wetland/delta geometry.

Requirements checked: `WORLD-1`; exceptions: none.
