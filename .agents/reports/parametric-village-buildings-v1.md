# Parametric village buildings v1

## Evidence

- Settlement profile v1 deterministically records continuous height, footprint, and
  density biases. A held-out profile sweep contains both single-storey and tall villages
  rather than selecting fixed house templates.
- Building generator v1 expands each accepted anchor into stable foundation, floor,
  wall, doorway, and roof modules. Foundations extend below the sampled terrain anchor
  as a slope-tolerant skirt. The same input reproduces canonical module identity.
- Independent evaluator v1 rejects blocked anchors, duplicate module IDs, missing
  entrances, missing foundations or roofs, invalid transforms, and unsupported storeys.
- World Recipe schema 8 owns module transforms and semantic keys. Door modules expand
  into visible primitive frames; paths remain semantic and are no longer forced into the
  terrain surface.
- Runtime seeds `0`, `2`, `4`, `7`, and `53` each produced six valid assemblies with
  respectively `48`, `86`, `42`, `70`, and `56` modules. Every recipe and performance
  evaluation remained valid with eight native ISM batches and no per-module Actors.
- Full Unreal automation completed `60/60` tests successfully.
- Wide gallery captures for seeds `0`, `2`, and `7` contain the modular structures, but
  the village occupies too little of the frame and seed `7` trips the existing darkness
  guard. Visual quality is not promoted from those artifacts.

## Boundary

- This proves deterministic structural variety and replaceable primitive rendering, not
  finished art, natural-looking architecture, interiors, player placement UI, structural
  destruction, activity ports, or catalog asset swapping.
- The champion uses rectangular footprints, flat proxy roofs, a fixed six-role settlement
  slice, and terrain classification at the building anchor. Those are explicit next
  challengers rather than hidden claims.
