# Intent-aware biome classification implementation plan

**Goal:** Make biome semantics consume regional seed intent without depending on the Built Site layer.

**Architecture:** Add one overload to the existing pure biome field. It reuses the
intent-terrain adapter and existing classifier, while the old overload remains the global
compatibility path. Content cells route through the new overload.

**Tech stack:** Unreal Engine 5.8 C++ and existing automation.

## Constraints

- No new generator, Actor, asset, plugin, or Built Site layer dependency.
- Global and arrival behavior stays unchanged.
- Use the existing smooth regional influence.
- Requirements checked: Global none recorded; exceptions: none.

### Task 1: Intent-aware biome query

- [x] Add failing tests for arrival compatibility, regional profile consumption, dry
  override, and blend continuity.
- [x] Run focused biome automation and observe the missing-overload RED failure.
- [x] Add the minimum overload by reusing existing biome calculation and intent terrain.
- [x] Run focused biome automation until green.

### Task 2: Runtime consumer

- [x] Add a failing content-cell assertion proving supplied intent changes its biome fact.
- [x] Route content-cell biome queries through the intent-aware overload.
- [x] Run focused content-cell automation until green.

### Task 3: Evidence

- [x] Run the full Gaters suite and validators.
- [x] Update machine and workstream truth without claiming final biome ecology.
