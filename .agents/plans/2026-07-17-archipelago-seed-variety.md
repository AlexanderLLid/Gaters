# Archipelago Seed Variety Plan

> **For agentic workers:** Execute inline; do not branch or commit unless the human asks.

**Goal:** Make archipelago seeds vary island count, scale, and arrangement without weakening gameplay guarantees.

**Architecture:** Keep the generator as one pure height function. Derive 1–3 satellite masses and their parameters directly from the existing seed and combine them with the guaranteed arrival island; validate topology through sampled runtime coastline signatures rather than exposing generator internals.

**Constraints:**

- Every archipelago retains one dry arrival island.
- Runtime local water, buildable land, base placement, and traversal remain valid.
- No new random state, recipe schema, biome system, island graph, bridges, boats, or assets.
- Bump generator identity only after held-out tests pass.

## Task 1: Reject identical archipelago topology

- [x] Add a runtime multi-seed test that records sampled coastline signatures and requires meaningful diversity.
- [x] Verify generator 8 fails the new scale/count diversity contract.

## Task 2: Derive island masses from seed

- [x] Replace two fixed satellites with 1–3 seed-derived satellites.
- [x] Vary satellite distance, angle, inner radius, and outer radius within bounded ranges.
- [x] Preserve the arrival island and focused seed-2 guarantees.

## Task 3: Promote generator 9

- [x] Run environment, navigation, recipe, and full Gaters automation.
- [x] Capture multiple archipelago seeds and inspect paired evidence.
- [x] Promote generator 9 in the registry and validate archive, gallery, registry, and diff gates.
