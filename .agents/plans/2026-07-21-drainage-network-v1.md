# Drainage Network v1 Implementation Plan

**Goal:** Convert accepted terrain and precipitation evidence into one deterministic
downhill flow graph that later systems can use for rivers, basins, wetlands, deltas, and
waterfall candidates.

**Architecture:** Sample a bounded regular grid, choose only strictly downhill D8 edges,
accumulate precipitation in descending-height order, and expose stable cells/segments plus
terminal-basin identity. The first version is pure data and does not carve or render water.

- [x] Add failing synthetic tests for a sloped catchment, a closed basin, wet-versus-dry
  accumulation, waterfall-drop evidence, determinism, and invalid/non-finite input.
- [x] Add a failing real-root test over multiple seeds for exact repeat, bounds, stable
  IDs, no uphill edges, and accepted terrain/climate consumption.
- [x] Implement the minimum versioned grid recipe and build result.
- [x] Run focused Drainage, Climate, Environment Recipe, Landform, and Water tests.
- [x] Run the complete Gaters suite and repository validators.
- [x] Mark the machine active or verified based on evidence; do not claim carving,
  rendered rivers, lakes, wetlands, deltas, or beaches.

Requirements checked: `WORLD-1`; exceptions: none.
