# Climate Field Held-Out Verification Plan

**Goal:** Challenge Climate Field v1 with new climate relationships and multiple real
seeded terrain fields before promotion or environment-root integration.

**Architecture:** Extend the pure automation fixture with a data-driven challenge matrix.
Do not add an evaluator abstraction unless the matrix exposes repeated production use.

- [x] Add polar/tropical, alpine/lowland, arid/maritime, continental/maritime,
  windward/leeward, and freeze-thaw comparison cases.
- [x] Add a multi-seed real-terrain sweep for determinism, bounds, and accepted-height
  identity.
- [x] Run the new test before implementation changes and record any falsified invariant.
- [x] Make the smallest physical-signal correction required by the counterexample.
- [x] Run focused dependency tests and the complete Gaters suite.
- [x] Promote or retain `world.climate-field` based on evidence, then update current
  workstream truth.

Requirements checked: `WORLD-1`; exceptions: none.
