# Built Site Recipe v1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use
> `superpowers:executing-plans` to implement this plan task-by-task. Steps use checkbox
> (`- [ ]`) syntax for tracking. Work inline in the shared tree; do not branch, commit,
> stage, or push.

**Goal:** Provide one versioned pure-data physical site recipe and adapt the accepted
land-village generator to it without inventing raid semantics or depending on Actors.

**Architecture:** Add a small physical graph contract with validation and canonical
identity. A separate village adapter maps existing building envelopes and coordinate-
stable path cells into evidenced centerlines, building envelopes, and directed topology.
The optional Built Site layer returns the semantic recipe beside its unchanged World
Recipe nodes; clearance-dependent facts remain absent until a source proves them.

**Tech Stack:** Unreal Engine 5.8 C++, existing pure settlement/building contracts,
Unreal automation.

## Global Constraints

- Pure data only: no Actors, assets, catalog lookups, streaming state, world mutation,
  raid objectives, loot roles, tactical labels, or difficulty declarations.
- Keep the current settlement generator, building generator, and World Recipe-node output
  unchanged.
- Empty blocker and visibility arrays are valid when the source generator has no evidence;
  the adapter must not fabricate geometry facts.
- Runtime ownership context is external instance state, not generated Built Site Recipe
  definition data.
- Requirements checked: Global none recorded; First prototype priority; Raiding `RAID-1`
  through `RAID-6`; Building; generated content boundary. Exceptions: none.

---

### Task 1: Pure physical graph contract and validator

**Files:**

- Create: `Unreal/Prototype/Source/Prototype/Public/GatersBuiltSiteRecipe.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersBuiltSiteRecipe.cpp`
- Test: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersBuiltSiteRecipeTests.cpp`

**Interfaces:**

- `EGatersBuiltSiteKind`: `Settlement`, `Outpost`, `Base`, `Fortress`, `Dungeon`.
- `FGatersBuiltSiteSpace`: stable ID, axis-aligned center/extent, physical tags, source IDs.
- `FGatersBuiltSiteConnection`: stable directed endpoints, width/headroom/step/jump facts,
  blocker IDs, physical tags, source IDs.
- `FGatersBuiltSiteVisibility`: stable directed endpoints, distance/height facts, blocker
  IDs, physical tags, source IDs.
- `FGatersBuiltSiteBlocker`: stable ID, center/extent, physical tags, source IDs.
- `FGatersBuiltSitePlacementSlot`: stable ID, containing space, location, clearance
  radius/height, physical tags, source IDs. It has no scenario role.
- `FGatersBuiltSiteIssue`: rule ID, subject ID, message.
- `FGatersBuiltSiteRecipe`: contract/site/generator versions, seed, site ID/kind, site
  area, element arrays, `FindSpace`, `CanonicalText`, `Checksum`, and `Validate`.

- [x] **Step 1: Write failing contract and counterexample tests**

Require one valid settlement fixture to preserve canonical text/checksum, accept empty
visibility/blocker arrays, and expose neutral slots without tactical labels. Require
invalid fixtures to emit stable issues for duplicate IDs, missing references, negative
dimensions, out-of-space slots, and missing provenance.

- [x] **Step 2: Run the focused tests and verify RED**

Run the Unreal build. Expected: compilation fails because `GatersBuiltSiteRecipe.h` does
not exist.

- [x] **Step 3: Implement the minimum types and validator**

Use arrays and strings already provided by `CoreMinimal`; add no reflection, inheritance,
factory, dependency, or speculative site subtype. Validate cross-element references and
physical dimensions. Canonicalize in stored array order so deterministic generators retain
their authored topology order.

- [x] **Step 4: Run `Gaters.BuiltSites.Recipe` and verify GREEN**

Expected: contract and counterexample tests pass.

### Task 2: Village-to-site-recipe adapter

**Files:**

- Create: `Unreal/Prototype/Source/Prototype/Public/GatersSettlementSiteRecipeAdapter.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersSettlementSiteRecipeAdapter.cpp`
- Test: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersSettlementSiteRecipeAdapterTests.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersBuiltSiteLayer.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersBuiltSiteLayer.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersBuiltSiteLayerTests.cpp`

**Interfaces:**

- `FGatersSettlementSiteRecipeCompilation`: `bCompiled`, recipe, diagnostics.
- `FGatersSettlementSiteRecipeAdapter::Compile(Field, Seed, Plan)`.
- `FGatersBuiltSiteLayerResult::SiteRecipes` contains the accepted settlement recipe.

- [x] **Step 1: Write failing adapter tests**

Require fixed inputs to reproduce the same recipe/checksum; all path cells become outdoor
centerlines with coordinate-stable IDs; orthogonally adjacent path cells receive both
directed connections; and each valid building receives one roofed assembly envelope with
its semantic role separated from physical tags. Verify the adapter invents no clearance
or placement slot, every element cites source IDs, growth preserves every prior fact, and
visibility/blockers remain empty.

- [x] **Step 2: Run `Gaters.BuiltSites.SettlementAdapter` and verify RED**

Expected: compilation fails because the adapter does not exist.

- [x] **Step 3: Implement the minimum adapter**

Reuse settlement evaluation, building generation/evaluation, coordinate-stable path IDs,
and module transforms. Derive each building envelope from its existing module transforms;
do not duplicate building dimension constants. Reject invalid source plans with causal
diagnostics, then run the independent Built Site Recipe validator.

- [x] **Step 4: Attach the semantic recipe to the optional layer**

Keep existing World Recipe nodes byte-for-byte equivalent. Add the accepted recipe only
after settlement and site-recipe validation pass.

- [x] **Step 5: Run focused tests and all `Gaters` automation**

Expected: both new filters pass, existing Built Site Layer parity remains green, and the
full suite has zero failures.

### Task 3: Contract handoff and evidence

**Files:**

- Modify: `.agents/exchanges/RAID-1-built-site-recipe-contract.md`
- Modify: `.agents/exchanges/SITE-4-legacy-base-layer-handoff.md`
- Modify: `.agents/workstreams/Settlements, Bases & Dungeons.md`

- [x] **Step 1: Record the exact verified v1 boundary**

Answer `RAID-1` with the physical graph types, explicit omissions, test evidence, and
consumer boundary. Record that the legacy `RaidLoot` node is removed rather than
transferred when SITE-4 integration occurs.

- [x] **Step 2: Run repository validators**

Run `research/Test-MachineRegistry.ps1`, `research/Test-SharedAgentDocs.ps1`, trailing-
whitespace checks, the incremental Unreal build, both focused filters, and all `Gaters`
automation.

- [x] **Step 3: Notify Raids & Dungeons and Primary Builder**

Send authoritative exchange paths and exact integration responsibilities. Do not edit
`AGatersChunk`, the shared catalog, or `research/machines.json` in this task.
