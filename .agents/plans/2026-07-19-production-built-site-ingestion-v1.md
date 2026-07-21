# Production Built Site Ingestion v1 Plan

**Goal:** Consume RAID-2's generated Built Site JSON without fabricating missing physical
facts or running unsupported tactical simulations.

**Architecture:** Add a standard-library mapping inside the existing Raids harness. The
mapping preserves production recipe identity and physical facts, then a preflight returns
either `ready-for-scenario` or `insufficient-evidence`. Existing synthetic scenario
evaluation remains unchanged.

**Constraints:** No Unreal edits, no second evaluator, no new dependency, no inferred
clearance/visibility/blockers/slots. Requirements checked: Global none;
generated-content boundary and `RAID-1..6`; exceptions: none.

## Task 1 — Failing held-out ingestion test

- Load `research/settlements-bases-dungeons/generated-settlement-built-site-v1.json`.
- Require deterministic mapping of recipe versions, units, IDs, source references,
  coordinates, extents, connections, and zero-valued unknown facts.
- Require `insufficient-evidence` with causal findings and no policy matrix.
- Require unsupported units to fail explicitly.

## Task 2 — Minimal adapter and CLI

- Add production catalog validation and field mapping to `rift_raid_harness.py`.
- Add evidence preflight for slots, visibility, blockers, and traversal clearance.
- Add `--built-site-export <path>`; retain existing synthetic defaults and `--summary`.

## Task 3 — Handoff and verification

- Update the raid-machine setup and Raids status with the exact command and evidence.
- Run adapter tests, synthetic harness regression, scenario validation, shared-doc
  validation, JSON parsing, CLI summary, and touched-file hygiene.
- Do not commit.
