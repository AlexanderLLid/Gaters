# Settlement Access Fit Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Ensure every generated ground-floor indoor neutral slot has a clear deterministic route from the settlement path network.

**Architecture:** Keep the settlement generator terrain-query-only and make building access causal before publication. The generator faces each facade toward its selected entrance and rejects path/building cell conflicts; the recipe adapter compiles stable doorway thresholds and rejects blocked required links or any unreachable indoor slot instead of deleting connections.

**Tech Stack:** Unreal Engine 5.8 C++, Automation Tests, deterministic Built Site Recipe JSON, Python standard-library raid harness.

## Global Constraints

- Generated recipes remain authoritative; Unreal Actors and assets are derived adapters.
- `BUILD-1`: every connection declares its supported movement modes.
- No tactical roles, raid suitability, runtime growth, terrain mutation, or global movement type.
- All Unreal build, commandlet, and automation invocations go through Unreal Runner.

---

### Task 1: Prove the disconnected-slot failure

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersSettlementSiteRecipeAdapterTests.cpp`

**Interfaces:**
- Consumes: `FGatersSettlementSiteRecipeAdapter::Compile` and stored directed connections.
- Produces: a regression assertion requiring every indoor placement slot to be reachable from a `path-centerline` space.

- [ ] **Step 1: Add the graph assertion**

Add a test helper that seeds reachability from every `path-centerline` space, follows directed connections to a fixed point, and returns false when a placement slot whose referenced space is tagged `indoors` is not reachable. Assert it for the seed-73 stage-1 recipe.

- [ ] **Step 2: Verify RED through Unreal Runner**

Run the focused adapter test and require the new assertion to fail on the current eight unexplained indoor slots.

### Task 2: Make coarse access causal in settlement planning

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersSettlementGenerator.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersSettlementTests.cpp`

**Interfaces:**
- Consumes: `FGatersTerrainPath`, accepted building cells, accepted path cells, and `EntranceCell`.
- Produces: an entrance-facing `FGatersSettlementBuilding::Yaw` and plans where no accepted building cell overlaps a path and no new path crosses an accepted building cell.

- [ ] **Step 1: Add planner assertions**

For generated settlements, assert that each building local +X direction points toward `EntranceCell`; assert building cells are absent from `PathCells`; assert each building-to-center path excludes every other building cell.

- [ ] **Step 2: Implement the minimum planner correction**

During candidate selection, reject a candidate whose building cell is already a path cell or whose path after index zero crosses an accepted building cell. Set yaw from `CellLocation(Field, EntranceCell) - Building.Location`, not from the settlement center.

### Task 3: Compile and independently enforce required doorway access

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersSettlementSiteRecipeAdapter.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersSettlementSiteRecipeAdapterTests.cpp`

**Interfaces:**
- Consumes: building opening transform, width, headroom, ground usable space, movement settings, and generated blockers.
- Produces: stable exterior/interior threshold spaces, complete required connections, causal blocked-link diagnostics, and all-indoor-slot reachability rejection.

- [ ] **Step 1: Add causal counterexamples**

Add one fixture where a required doorway connection is blocked and assert compilation fails with the stable connection and blocker IDs. Add one fixture with an extra disconnected indoor slot and assert the reachability gate identifies its stable slot/space ID.

- [ ] **Step 2: Add threshold compilation**

For each opening, derive outward local +X from its transform. Add non-slot spaces `<building>:threshold:exterior` and `<building>:threshold:interior` at opening-center height, beyond wall thickness and clearance radius. Connect entrance to exterior, exterior to opening, opening to interior, and interior to the ground usable space using the existing movement-mode-aware helpers.

- [ ] **Step 3: Replace deletion with rejection**

Return the first intersecting generated blocker for each connection. Emit `site.connection.blocked` with connection, blocker, building/source IDs and fail compilation; never remove the connection or visibility fact.

- [ ] **Step 4: Require every indoor slot reachable**

Compute directed reachability from all `path-centerline` spaces. Fail with `site.slot.disconnected` naming each indoor placement slot and referenced space that is not reachable.

### Task 4: Verify, export, and close RAID-5 evidence

**Files:**
- Modify: `research/settlements-bases-dungeons/generated-settlement-built-site-v1.json`
- Modify: `.agents/exchanges/RAID-5-disconnected-generated-interiors.md`
- Modify: `.agents/workstreams/Settlements, Bases & Dungeons.md`

**Interfaces:**
- Consumes: corrected deterministic recipe export and Raids-owned headless harness.
- Produces: reproducible artifact hash, focused/full-suite evidence, and an updated recipient Response ready for requester resolution.

- [ ] **Step 1: Verify GREEN through Unreal Runner**

Build `PrototypeEditor Win64 Development`, run `Gaters.Worldgen.Settlement+Gaters.BuiltSites.SettlementAdapter+Gaters.BuiltSites.JsonExport.GeneratedSettlement`, then run complete `Gaters` automation.

- [ ] **Step 2: Regenerate twice through Unreal Runner**

Run `GatersBuiltSiteExport` with seed `73`, stage `1`, and the authoritative artifact path twice. Require byte-identical SHA-256.

- [ ] **Step 3: Run non-Unreal acceptance checks**

Run the Raids production preflight/harness against the artifact. Require `ready-for-scenario`, zero unexplained disconnected indoor slots, and existing causal missing-evidence tests to pass.

- [ ] **Step 4: Record evidence and notify Raids**

Update RAID-5 Response and the Settlements status with the exact causal fix, counts, hash, commands, and focused/full results. Keep packet status `answered`; Raids owns Resolution.

Requirements checked: generated-content boundary and `BUILD-1`; exceptions: none.
