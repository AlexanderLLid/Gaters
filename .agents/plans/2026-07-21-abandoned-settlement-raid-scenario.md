# Abandoned Settlement Raid Scenario Implementation Plan

> **For agentic workers:** Execute inline with TDD. Do not branch, commit, or launch
> Unreal; repository rules override generic skill defaults.

**Goal:** Replace the provisional settlement analysis identity with one versioned
AI/abandoned-settlement loot-and-extract scenario while keeping results analysis-only
until a composed world approach and repaired site topology exist.

**Architecture:** Extend the existing Raids-owned JSON catalog and PowerShell validator.
The existing Python ensemble selects physical representatives, assigns evaluator-local
roles, and consumes the first compatible catalog scenario. It never writes tactical tags
to the Built Site Recipe.

**Tech Stack:** JSON, PowerShell, Python standard library/unittest.

## Global Constraints

- `BUILD-1`: respect per-connection movement support.
- `LOOT-1`: loot remains carried and loseable until valid extraction.
- `RAID-3`: world approach remains required; no internal arrival is promoted.
- Generated recipes remain authoritative; evaluator-local roles never modify them.
- Exact tuning remains in Combat fixtures, not scenario prose.

---

### Task 1: Catalog and validator contract

**Files:**
- Modify: `research/raids-dungeons/rift-raid-scenarios-v1.json`
- Modify: `research/raids-dungeons/Test-RiftRaidScenarios.ps1`

**Produces:** `abandoned-settlement-loot-v1`, using the existing natural/called
expedition access state, with loot/extraction roles and explicit world-approach,
site-network, fairness, and exploit requirements.

- [ ] Add failing validator assertions requiring one `settlement` scenario, access-state
  site-kind compatibility, carried extraction, and the two causal gates
  `world-approach-evidence-unknown` plus `site-network-disconnected-objective`.
- [ ] Run `Test-RiftRaidScenarios.ps1`; expect failure because no settlement scenario exists.
- [ ] Add `settlement` to the existing expedition access state and add the minimum
  `abandoned-settlement-loot-v1` scenario.
- [ ] Run the validator; expect four scenarios, two access states, exit `0`.

### Task 2: Ensemble consumes the compatible scenario

**Files:**
- Modify: `research/raids-dungeons/test_rift_raid_harness.py`
- Modify: `research/raids-dungeons/rift_raid_harness.py`

**Produces:** `evaluate_built_site_ensemble(...)` reports the catalogued settlement
scenario and no longer emits `no-compatible-scenario`; it remains `analysis-only` while
world approach or physical topology is invalid.

- [ ] Change the production ensemble test to require compatible scenario
  `abandoned-settlement-loot-v1`, that scenario as the evaluated identity, and absence of
  `no-compatible-scenario`.
- [ ] Run the focused test; expect failure because the catalog lacks that scenario or the
  harness still uses the provisional identity.
- [ ] Select the first stable-ID compatible catalog scenario, falling back to the current
  provisional scenario only when none exists.
- [ ] Run the focused and full Raids unittest suites; expect all tests to pass.

### Task 3: Evidence and workstream closeout

**Files:**
- Modify: `research/raids-dungeons/generated-settlement-frontier-v1.json`
- Modify: `research/raids-dungeons/rift-raid-machine-setup-v1.md`
- Modify: `.agents/workstreams/Raids & Dungeons.md`

- [ ] Update the held-out evidence to identify the catalogued settlement scenario while
  retaining `analysisOnly: true` and both causal blockers.
- [ ] Update the machine frontier and status with the exact verified state.
- [ ] Run Raids unittests, scenario validator, shared-doc validator, registry validator,
  JSON parsing, CLI summary, and scoped `git diff --check`.

