# Seed-Derived World Intent Implementation Plan

> **For agentic workers:** Execute inline with TDD. Do not branch, commit, or push; the
> human reviews the shared worktree. Steps use checkbox syntax for tracking.

**Goal:** A seed deterministically declares broad regional world intent without imposing
universal resource or biome guarantees.

**Architecture:** A pure recipe owns stable regional profiles and answers coordinate
queries. Generators consume it later; v1 does not change current terrain output. An
independent fidelity evaluator is a separate next slice after the intent contract is
verified.

**Tech Stack:** Unreal Engine 5.8 C++, existing automation framework.

## Global Constraints

- Empty, barren, wet, mountainous, sparse, and forestless intent must be valid.
- No Actors, assets, site-generation data, gameplay item names, yields, or full-world generation.
- Requirements checked: Global none recorded; generated content boundary; exceptions:
  none.

### Task 1: Pure seed-derived intent recipe

**Files:**

- Create: `Unreal/Prototype/Source/Prototype/Public/GatersWorldIntent.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersWorldIntent.cpp`
- Create: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersWorldIntentTests.cpp`

**Interfaces:**

- `FGatersWorldIntentRecipe::Generate(int32 Seed, float WorldSize)` returns a versioned
  global profile plus stable regional influences.
- `FGatersWorldIntentRecipe::At(FVector2D Point)` returns the responsible profile.
- `FGatersWorldRegionIntent` carries stable ID, center, radius, terrain/hydrology
  tendencies, and bounded vegetation/stone/landmark/travel-friction opportunities.

- [x] Write a failing test for deterministic identity, different-seed variation,
  coordinate stability, bounded profiles, and a valid sparse/forestless seed.
- [x] Run the focused build and verify RED because `GatersWorldIntent.h` is absent.
- [x] Implement the minimum hash-derived global profile and two regional influences.
- [x] Run `Gaters.Worldgen.WorldIntent` until GREEN.
- [x] Run full Gaters automation and registry/docs validators.

### Task 2: Content-cell intent consumption

**Files:**

- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersContentCellRecipe.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersContentCellRecipe.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersContentCellRecipeTests.cpp`

**Interfaces:**

- Content-cell recipe records intent version and responsible region ID.
- Effective vegetation/stone weights multiply declared intent by terrain-compatible biome
  opportunity; zero declared vegetation never forces a tree.

- [x] Add failing provenance and zero-vegetation suppression tests.
- [x] Bump the content-cell contract and implement the minimum intent adapter.
- [x] Run focused content-cell tests, held-out runtime seed, and full automation.

## Self-review

- The intent is authoritative input, not an evaluator judging its own output.
- No resource availability guarantee appears in the contract.
- Two regional influences are the minimum proof that one seed can describe different
  places without per-cell quotas.
- Generator consumption and fidelity evaluation remain separate reviewable slices.
