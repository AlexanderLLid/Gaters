# Intent-aware biome classification design

## Outcome

- Any coordinate can expose biome semantics derived from the same regional seed intent
  that shapes its terrain.
- Existing global-only callers retain their current behavior.
- The Built Site layer is not required.

## Call

- Add an intent-aware `FGatersBiomeField::Query` overload.
- Reuse `FGatersIntentTerrainField` to choose regional terrain and hydrology with its
  existing smooth influence.
- Keep biome classification inside `FGatersBiomeField`; do not create a second generator.
- Make content cells use the intent-aware overload they already have enough data to call.

## Contract

- Global and arrival-region samples remain identical to the existing query.
- A regional core uses its declared terrain and hydrology for height, water evidence,
  moisture, exposure, elevation, and biome key.
- Regional boundaries blend continuously rather than switching at an intent border.
- Dry regional intent does not inherit global water evidence at full regional influence.
- The query remains pure and owns no cells, sites, Actors, or assets.

## Verification

- Pure tests compare global-only and intent-aware arrival samples.
- Controlled regional intent proves local terrain/hydrology changes classification.
- Adjacent samples across a blend annulus prove bounded continuity.
- Content-cell tests prove recorded biome semantics consume the supplied intent.

## Excluded

- New biome names, final ecology, dressed assets, resource-loop guarantees, and water art.
