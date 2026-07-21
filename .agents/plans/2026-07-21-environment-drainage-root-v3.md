# Environment Drainage Root v3 Implementation Plan

> **For agentic workers:** Execute inline with red-green verification. Do not create a
> branch, commit, carve terrain, add rendering, or change downstream site ownership.

**Goal:** Make one Environment Recipe carry the accepted drainage graph, regional-water
fit, and neutral drainage-feature candidates as reproducible pure data.

**Architecture:** Environment Recipe v3 composes the already isolated machines in strict
dependency order: terrain/intent/climate -> drainage -> regional-water fit -> neutral
feature candidates. Validation independently rebuilds each derived layer from the root's
authoritative terrain and climate queries plus the settings recorded in its recipe. No
downstream system reconstructs or mutates these facts.

**Tech stack:** Unreal Engine 5.8 C++ and existing Unreal Automation.

## Global constraints

- `WORLD-1`: zero water surfaces or zero feature candidates remains valid declared
  scarcity.
- The root stores recipes and their settings, not Actors, assets, terrain carving, art,
  sites, encounters, or gameplay labels.
- Root composition begins with a bounded global evidence grid; exact resolution must pass
  the root-scale counterexample matrix. Hierarchical/local drainage is a later scale
  challenger, not hidden in this contract.
- Existing pure drainage, fit, and feature compilers remain independently callable.

### Task 1: RED root and provenance contract

- [x] Record water-fit datum tolerance and feature settings in their output recipes.
- [x] Add Environment Recipe v3 fields for drainage, water fit, and feature candidates.
- [x] Require exact same-seed repeatability and direct-source parity.
- [x] Require causal rejection of corrupted drainage, fit, and feature evidence.
- [x] Build and observe RED for missing root composition.

### Task 2: Minimum root composition

- [x] Compile bounded drainage from the authoritative root terrain/climate queries.
- [x] Fit declared regional water to that exact drainage recipe.
- [x] Compile neutral feature candidates from those accepted inputs.
- [x] Independently rebuild and compare all three derived layers during validation.
- [x] Keep empty water/features valid and preserve existing public query behavior.

### Task 3: Evidence and truth

- [x] Challenge seeds 11, 29, 47, 83, and 131 through the complete v3 root.
- [x] Run focused Environment/Drainage, Worldgen, and complete Gaters automation; retain
  the three unrelated active Settlements-owned RED failures instead of editing them.
- [x] Run registry/docs validators and scoped diff checks.
- [x] Update `world.environment-recipe-compiler`, `world.drainage-network`, and Primary
  status without claiming carving, rendering, streaming-scale drainage, or final art.

Requirements checked: `WORLD-1`; exceptions: none.
