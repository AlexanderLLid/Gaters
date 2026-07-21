# Production tactical-role representative ensemble

> Execution snapshot. Current coordination remains in the Raids & Dungeons status and
> RAID-4 exchange; machine truth remains in `research/machines.json`.

**Goal:** Deterministically select a small set of physically distinct raid-role tuples
from a production Built Site Recipe and explain their headless outcomes without changing
the recipe or claiming runtime encounter placement.

**Boundary:** Extend the existing pure-Python Raids harness. Consume stable recipe IDs,
per-connection movement support, visibility, blockers, and neutral slots. Do not create
topology, infer a world edge from `outdoors`, modify shared contracts, or select final
runtime encounter placement.

**Requirements checked:** `BUILD-1`, `LOOT-1`, generated-content boundary, and
`RAID-1`–`RAID-8`; exceptions: none.

## Tasks

1. Record the human approval of RAID-4 option A.
2. Add failing tests proving deterministic output, unchanged source data, stable-ID role
   references, shallow/medium/deep/isolated coverage, causal failure explanations, and
   analysis-only gating while approach/scenario evidence is absent.
3. Add the smallest representative-selection and evaluation functions to
   `research/raids-dungeons/rift_raid_harness.py`; expose them through one CLI flag.
4. Update the Raids-owned generated-settlement evidence, machine setup, and status with
   the observed ensemble results.
5. Run focused unit/CLI checks, the existing scenario and resolver checks, shared-doc
   validation, registry validation, and scoped diff checks. No Unreal launch is needed.

