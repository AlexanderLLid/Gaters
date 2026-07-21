# Rift Raid Harness v0 Implementation Plan

**Goal:** Run deterministic raid evaluations against synthetic physical site recipes
before generated terrain and Built Site recipes are ready.

**Architecture:** Keep synthetic recipes as pure JSON shaped after
`FGatersBuiltSiteRecipe`. A Raids-owned Python adapter converts each recipe/scenario pair
into the existing Combat resolver's tactical arena, runs every baseline policy pairing,
then adds causal topology findings using stable recipe IDs. No Unreal Actor access, world
generation, or second site generator.

**Tech stack:** Python standard library, JSON, existing
`research/base-raid-lab/headless_raid_resolver.py`.

**Constraints:** No shared Unreal, canon, or machine-registry edits. Settlements, Bases & Dungeons retains
physical topology ownership. Numbers remain fixture data. Requirements checked:
`RIFT-1..6`, `ANCHOR-1..5`, `RAID-1..6`, `TRACE-1..6`, `RETURN-1..3`, `CLAIM-1..4`;
exceptions: none.

## Task 1: Contract fixtures and failing acceptance test

- Create `research/raids-dungeons/synthetic-built-sites-v1.json` with open settlement,
  multi-route fortress, single-door bunker, and sealed dungeon recipes.
- Create `research/raids-dungeons/test_rift_raid_harness.py` asserting deterministic
  matrices, version identity, wild/base coverage, and stable causal findings.
- Run the test and require failure because `rift_raid_harness` does not exist.

## Task 2: Minimal executable harness

- Create `research/raids-dungeons/rift_raid_harness.py`.
- Load and validate the site, scenario, and Combat catalogs.
- Compile neutral slots and physical graph facts into resolver arenas.
- Run all attacker/defender policy pairs and emit deterministic JSON evaluations.
- Diagnose unreachable objectives, one-door bunkers, direct-prize arrivals, and trivial
  extraction with stable recipe IDs.
- Run the focused test and existing resolver regression test.

## Task 3: One-command handoff

- Update the Raids machine setup and workstream status with the runnable command and
  current limits.
- Run scenario validation, harness tests, resolver tests, shared-doc validation, and
  touched-file hygiene checks.
- Do not commit; human review is required first.
