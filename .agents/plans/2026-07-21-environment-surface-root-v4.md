# Environment Surface Root v4 Implementation Plan

> **For agentic workers:** Execute inline with red-green verification. Do not create a
> branch, commit, add materials/assets, carve terrain, or edit active Settlements work.

**Goal:** Compose the verified Surface Condition Field into the authoritative Environment
Recipe so downstream systems query one accepted physical world root.

**Architecture:** Environment Recipe version 4 stores the versioned surface-condition
recipe after drainage composition and exposes one query adapter. Root validation first
validates the upstream physical layers, then independently recompiles and compares the
surface recipe. The Surface Condition compiler calls only the upstream validation seam,
preventing recursive full-root validation.

**Tech stack:** Unreal Engine 5.8 C++ and existing Unreal Automation.

## Global constraints

- `WORLD-1`: no surface condition is universally required; valid scarcity remains valid.
- Reuse `FGatersSurfaceConditionField`; do not duplicate its evidence or formulas.
- The root owns composition and provenance only, not rendering or content decisions.
- Preserve the current `129`-by-`129` accepted drainage evidence resolution.
- The three active Settlements-owned RED tests remain outside this machine.

### Task 1: RED root contract

- [x] Extend Environment Recipe tests to require a stored surface recipe, exact repeated
  composition, direct-source parity, query parity, held-out repeats, and causal rejection
  of corrupted surface provenance/settings.
- [x] Build and observe RED because Environment Recipe v3 has no surface member/query.

### Task 2: Minimum non-recursive composition

- [x] Add a narrow upstream provenance check for the terrain/climate/drainage facts the
  Surface Condition adapter consumes, avoiding recursive full-root validation.
- [x] Make full `Validate` recompile Surface Condition from its recorded settings and
  compare the exact recipe.
- [x] Compile Surface Condition last, store it, and expose `QuerySurfaceConditions`.
- [x] Bump Environment Recipe and compiler versions to `4`.

### Task 3: Evidence and truth

- [x] Run focused Environment Recipe and Surface Condition automation.
- [x] Run broad Worldgen and complete Gaters automation; report unrelated failures by
  exact owner and test name rather than claiming shared green.
- [x] Update `world.environment-recipe-compiler`, Primary status, and machine validators
  without claiming visual materials, terrain carving, resources, or runtime art.

Requirements checked: `WORLD-1`; exceptions: none.
