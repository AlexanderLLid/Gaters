# Landform Protected Region Design

## Outcome

- Extreme landform briefs may reshape most of a world.
- They must not erase the seed-derived playable Arrival/build refuge.
- The terrain-family generator remains champion until held-out evidence passes.

## Contract

- A landform recipe accepts zero or more generic protected regions: stable ID, center,
  inner radius, and outer radius.
- Inside the inner radius, the challenger preserves the accepted base height exactly.
- Between radii, all landform-process height changes fade in smoothly.
- Outside the outer radius, the requested physical processes apply normally.
- The pure field knows only protected geometry. It does not know about Actors, bases,
  villages, Rifts, encounters, or assets.

## Runtime composition

- Primary-owned world composition queries the champion terrain for its existing valid
  build candidate before attaching challenger processes.
- It supplies two protected regions when evidence exists:
  - the Arrival area around the origin;
  - the champion build candidate plus its evaluated footprint.
- The normal World Recipe generator then evaluates the protected challenger terrain. It
  must independently rediscover a valid build site; the old result is not injected.
- If the champion has no valid build candidate, no fake site is invented. Existing
  diagnostics remain authoritative.

## Validation

- Reject empty/duplicate IDs, non-finite geometry, non-positive inner radii, or outer
  radii smaller than inner radii.
- Unit evidence proves exact inner preservation, smooth transition, unchanged output
  outside the protected region, and causal malformed-region rejection.
- Runtime evidence reruns held-out glacial seed `131` and the full four-fixture/five-seed
  sweep.
- Promotion still fails if any previously valid champion site becomes invalid, Arrival
  escape fails, output is non-finite/discontinuous, or runtime budgets fail.

## Scope

- No climate, drainage, biome, settlement, character, or art changes.
- No global cap on water, ice, volcanism, or relief.
- No reroll loop and no hardcoded seed exceptions.

## Why / rejected

- **Call:** protect explicit playable geometry while leaving the remaining world
  physically expressive.
- **Why:** this fixes the observed loss of seed `131`'s valid site without forcing every
  glacial or ocean world toward the same water/buildability ratios.
- **Rejected:** global hydrology clamps reduce variety; rerolling hides causal failures
  and weakens seed reproducibility.
