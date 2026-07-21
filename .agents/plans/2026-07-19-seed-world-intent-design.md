# Seed-derived world intent design

## Call

The seed defines what kind of world should exist. No universal resource, forest, or
arrival-area resource guarantee is imposed.

The numeric seed plus generator version deterministically derives a compact world-intent
recipe; detailed terrain and content remain lazily generated from that recipe.

## Production flow

`seed → world intent → terrain/biomes → streamed content → fidelity evaluator`

- World intent declares broad regional composition and neutral opportunity profiles.
- Terrain, biome, and content generators consume the same intent.
- Cells generate only when needed; the entire detailed world is never required upfront.
- Evaluation checks that generated samples match declared intent and remain deterministic.
- A barren, wet, mountainous, sparse, or forestless world passes when the intent says it
  should be that way.

## Minimum intent contract

- Seed and generator version.
- Stable region IDs and deterministic spatial influence.
- Intended terrain family, hydrology, biome tendency, vegetation opportunity, stone
  opportunity, landmark opportunity, and travel-friction tendency per region.
- No final assets, item names, resource yields, settlements, or tactical labels.

## Evaluator boundary

- Check deterministic reproduction of the intent recipe.
- Check generated coordinate samples against their responsible region/profile.
- Check region transitions for discontinuities not declared by the intent.
- Emit seed, region ID, coordinate, intended profile, observed sample, and failed rule.
- Never fail merely because a resource or biome is absent.

## Why

If this existed, we would no longer need to guess whether an unusual generated world is
interesting scarcity or a generator defect.

## Rejected

- Universal nearby wood/stone guarantee: erases seed identity and meaningful scarcity.
- Per-cell quotas: turns every region into the same averaged distribution.
- Full-world pre-generation: unnecessary because intent and sampled fidelity can be
  validated before detailed cells stream.

Requirements checked: Global none recorded; generated content boundary; exceptions:
none.
