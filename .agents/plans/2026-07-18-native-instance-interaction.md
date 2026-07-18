# Native instance interaction

## Goal

Harvest and claim directly against Unreal ISM instances while preserving stable recipe IDs and diff replay, then remove per-item carrier actors.

## Boundary

- Unreal owns instance rendering, collision bodies, overlap events, and instance indices.
- Gaters owns the parallel stable-ID order and converts a touched batch/index into `chop:<id>` or `claim:<id>`.
- Ordered ISM removal/rebuild preserves the same stable-ID ordering.
- Claimed markers render in a non-interactive batch.

## Evidence gate

- Pure tests prove index-to-stable-ID resolution, removal, and claim-state transition.
- Native component tests prove collision is enabled only for interactive batches.
- Held-out runs report materializer v2, zero interaction carriers, preserved instance/triangle counts, lower actor counts, and no budget failures.
- Human visual play remains the final gate for touch feel and visible state transition.
