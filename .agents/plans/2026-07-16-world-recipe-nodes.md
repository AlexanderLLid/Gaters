# World Recipe Nodes Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:executing-plans`. Subagents are intentionally not used because repository instructions do not authorize delegation.

**Goal:** Give every generated place a minimal set of stable semantic nodes and reject structurally invalid recipes without loading a world.

**Architecture:** Extend the existing pure `FGatersWorldRecipe` value with only `Gate` and `BaseSite` nodes. Keep validation as a pure recipe method so command-line automation can find identity and structure failures before actor spawning; avoid a graph framework until relationships beyond these two nodes exist.

**Tech Stack:** Unreal Engine 5.8 C++, Unreal Automation Tests.

## Global Constraints

- Preserve terrain, base placement, random draw order, stable diffs, and `GatersGenVersion`.
- Bump only the recipe schema because canonical recipe output changes.
- Node identity is plain deterministic data; no UObject, DataAsset, PCG, JSON, or actor references.
- Do not add tags, relationships, asset contracts, or generic graph traversal before a consumer requires them.
- Do not branch, commit, or push; preserve unrelated human and Claude changes.

---

### Task 1: Stable Semantic Nodes

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersWorldRecipe.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersWorldRecipe.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersWorldRecipeTests.cpp`

**Interfaces:**
- Produces: `EGatersRecipeNodeKind`, `FGatersRecipeNode { Id, Kind, Location }`, `FGatersWorldRecipe::Nodes`, and `FindNode(const FString&)`.
- Consumes: existing generated Gate origin and explicit recipe base site.

- [ ] **Step 1: Write the failing node identity test**

  Assert that a valid generated recipe contains exactly `gate:0` at the origin and `base:0` at `BaseSite`; identical inputs yield identical node IDs/order; changing a node location changes canonical identity.

- [ ] **Step 2: Build to verify RED**

  Run the standard `PrototypeEditor Win64 Development` build.

  Expected: compile failure because `Nodes` and `FindNode` do not exist.

- [ ] **Step 3: Implement minimum nodes and schema 2 canonicalization**

  Add the Gate node unconditionally and BaseSite node only when placement succeeds. Append each node's ID, kind, and fixed-precision location to `CanonicalText()` in stored order. Set `SchemaVersion = 2`.

- [ ] **Step 4: Build and run `Gaters.Worldgen.WorldRecipe`**

  Expected: build succeeds and all focused recipe tests pass.

---

### Task 2: Pure Structural Validation

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersWorldRecipe.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersWorldRecipe.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersWorldRecipeTests.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersChunk.cpp`

**Interfaces:**
- Produces: `bool FGatersWorldRecipe::Validate(TArray<FString>& OutErrors) const`.
- Consumes: recipe scalar fields and nodes.

- [ ] **Step 1: Write failing validation tests**

  Assert that a generated recipe passes with no errors. Assert that duplicate IDs, a missing Gate, a mismatched BaseSite node, and non-finite coordinates each fail with a specific error fragment.

- [ ] **Step 2: Build to verify RED**

  Expected: compile failure because `Validate` does not exist.

- [ ] **Step 3: Implement minimum validation and runtime report**

  Validation clears output, checks schema/version basics, unique non-empty IDs, finite locations, exactly one Gate at the origin, and BaseSite node consistency. Extend the existing line to `RECIPE schema=<n> checksum=<hex> nodes=<n> valid=<yes|no>`; log validation errors only if present.

- [ ] **Step 4: Verify focused tests, all worldgen tests, and seed 7**

  Expected: build succeeds; all recipe/worldgen tests pass; seed 7 reports schema 2, two nodes, valid yes, and unchanged SITE/BASE/SCATTER/STAMP facts.

## Self-review

- The node shape contains only fields consumed by current identity and validation tests.
- Validation reports facts and does not introduce a score or gameplay policy.
- No generator behavior or persistent diff semantics change.
- Village, route, scatter, base-piece, and asset nodes remain separate future TDD slices.

