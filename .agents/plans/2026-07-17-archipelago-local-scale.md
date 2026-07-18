# Archipelago Local-Scale Repair Plan

> **For agentic workers:** Execute inline; do not branch or commit unless the human asks.

**Goal:** Make runtime archipelago seeds contain readable islands and water channels inside the local 300 m gameplay window.

**Architecture:** Keep `FGatersEnvironment` as the pure terrain source. Replace its world-size-normalized archipelago radius with a small deterministic union of gameplay-scale island masses plus restrained noise; reuse the existing ocean surface, base-site selector, semantic field, and evaluator.

**Constraints:**

- Seed 2 is the held-out runtime archipelago case.
- Arrival terrain remains dry above the ocean plane.
- Local water coverage is meaningful but leaves buildable land.
- Preserve all non-archipelago families.
- Bump generator identity because seed output changes.
- Do not add biomes, erosion, island graphs, bridges, boats, or assets.

## Task 1: Encode the runtime failure

- [x] Add a seed-2 runtime-window test requiring local water, buildable land, and dry arrival clearance.
- [x] Run it against generator 7 and verify the expected zero-water failure.

## Task 2: Repair the archipelago formula

- [x] Generate one arrival island and two deterministic satellite masses at fixed gameplay scale.
- [x] Lower the ocean datum below the flattened arrival pad.
- [x] Run environment, base-site, semantic-field, and traversability tests.

## Task 3: Promote evidence

- [x] Bump `GatersGenVersion` to 8 and update the terrain-generator registry champion/challenge.
- [x] Capture seed 2 with metrics and paired gallery evidence.
- [x] Run all Gaters tests, registry validation, archive validation, and `git diff --check`.
