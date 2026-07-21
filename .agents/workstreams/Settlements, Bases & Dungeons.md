# Settlements, Bases & Dungeons

## Current objective

- Feed physical-fit evidence back into coarse settlement planning, then continue the
  visual-plausibility wave without adding tactical meaning or runtime growth.

## Owned outputs

- Parcels, paths, districts, modular buildings, villages, outposts, bases, settlement
  growth, and physical dungeon layouts.
- Deterministic site recipes plus structural, physical-fit, placement, assembly, and
  settlement-plausibility evaluation owned by those recipes.

## Boundaries

- Primary Builder — World & Terrain owns terrain, biomes, water, vegetation, resources,
  seeds, streaming, candidate-site discovery, environmental constraints, and shared
  world integration.
- Raids & Dungeons owns encounters, objectives, attackers, defenders, traps, loot, and
  tactical simulation/evaluation.
- Settlements, Bases & Dungeons consumes terrain through explicit pure-data queries and emits deterministic
  semantic recipes without depending on spawned Unreal Actors or final art.

## Evidence

- [`Village growth v1`](../reports/village-growth-v1.md) records the transferred land
  village, parcel, path, building-assembly, growth, evaluator, and recipe evidence.
- [`Parametric village buildings v1`](../reports/parametric-village-buildings-v1.md)
  records reusable modular massing and placeholder materialization.
- [`Settlement vertical slice v1`](../reports/settlement-vertical-slice-v1.md) records the
  earlier generator/evaluator and terrain-site integration evidence.
- `GatersBuiltSiteLayer` provides the pure optional recipe-node layer; its three focused
  automation tests verify empty mode, exact legacy-node parity, determinism, and causal
  rejection.
- `GatersBuiltSiteRecipe` provides the versioned physical graph consumed without Actors;
  focused recipe and settlement-adapter automation passes 4/4, with 92/92 full `Gaters`
  tests passing.
- `GatersBuiltSiteRecipeJson` and `GatersBuiltSiteExportCommandlet` export generated
  physical recipes as deterministic UTF-8 JSON without Actors or map loading. The held-out
  seed-73 settlement repeats at SHA-256
  `33dff742822d7ebca94436487bdaee11226f5a88039331c0612b0767b51a0904`;
  focused Built Sites automation passes 7/7 and the fresh complete suite passes 103/103.
- `FGatersLegacyBaseLayer` now preserves the existing deterministic hut, compound, and
  tower topology behind a version-1 pure boundary using explicit replaceable module
  contracts and a terrain-height query. It emits stable physical pieces and used content
  requirements without assets, Actors, loot, tactical facts, or invented slots. Focused
  legacy-base automation passes 2/2, all Built Sites automation passes 9/9, and the fresh
  complete Gaters suite passes 111/111.
- Primary's runtime adapter now consumes the pure legacy-base layer. Fresh merged seed
  `7` preserves `99` physical pieces, world-only preserves `0`, and the embedded topology
  and hardcoded generated loot path are removed.
- Access-aware settlement generation now faces facades toward selected entrance cells,
  prevents building/path cell conflicts, compiles physical doorway thresholds, rejects
  blocked required links causally, and requires every indoor neutral slot to reach the
  site network. The deterministic seed-73 artifact exports `171` spaces, `320` directed
  connections, `320` visibility facts, `150` blockers, and `129` neutral slots at
  SHA-256 `d9b6e6bc565884c33d8d1fc7206c00c09c176566a43364dbf868853019c01d6e`.
  All `10` indoor slots are reachable, Raids preflight remains `ready-for-scenario`,
  focused automation passes `12/12`, and the complete `Gaters` suite passes `146/146`.

## Exchanges

- [`SITE-7 — Settlement access-fit registry integration`](../exchanges/SITE-7-settlement-access-fit-registry-integration.md)
  — open; asks Primary to record RAID-5 as a falsified settlement-generator guarantee
  and strengthen its existing promotion gate without creating or promoting a machine.

- [`RAID-5 — Disconnected generated interiors`](../exchanges/RAID-5-disconnected-generated-interiors.md)
  — resolved; Raids independently verified the deterministic artifact, `10/10` reachable
  indoor objectives, zero disconnected findings, causal regression coverage, and its
  fresh `10/10` owned tests.

- [`WORLD-5 — Optional village scarcity counterexample`](../exchanges/WORLD-5-optional-village-scarcity.md)
  — answered; the Built Site layer already accepts absence, while the Primary-owned
  site-route planner must consume explicit required IDs and record optional omissions
  without weakening required sites or routes.

- [`SITE-1 — Settlements, Bases & Dungeons ownership`](../exchanges/SITE-1-settlements-bases-dungeons-ownership.md)
  — resolved; Settlements, Bases & Dungeons and Raids & Dungeons are separate workstreams.
- [`SITE-2 — Village system handoff`](../exchanges/SITE-2-village-system-handoff.md)
  — resolved; ownership accepted after fresh source, automation, and runtime audit.
- [`WORLD-1 — Optional Built Site layer`](../exchanges/WORLD-1-optional-built-site-layer.md)
  — resolved; pure layer result is verified and integrated by Primary Builder.
- [`SITE-4 — Legacy base layer handoff`](../exchanges/SITE-4-legacy-base-layer-handoff.md)
  — resolved; pure topology and shared runtime integration are complete.
- [`SITE-5 — Pure legacy base layer integration`](../exchanges/SITE-5-pure-legacy-base-layer-integration.md)
  — resolved; Primary integrated the adapter with focused, full-suite, merged-parity,
  and world-only evidence while preserving ownership boundaries.
- [`RAID-1 — Built Site Recipe tactical contract`](../exchanges/RAID-1-built-site-recipe-contract.md)
  — resolved; versioned physical facts, neutral slots, and the production C++ seam are
  accepted by Raids & Dungeons.
- [`RAID-2 — Built Site Recipe JSON export`](../exchanges/RAID-2-built-site-recipe-json-export.md)
  — resolved; Raids independently verified the deterministic export and accepted the
  held-out settlement as its first ingestion and missing-evidence challenge.
- [`RAID-3 — Generated site tactical evidence`](../exchanges/RAID-3-generated-site-tactical-evidence.md)
  — answered; the deterministic seed-73 artifact proves movement clearance, neutral
  slots, visibility, blockers, and causal evidence coverage without tactical roles.
- [`WORLD-3 — Built Site blocker and clearance failure`](../exchanges/WORLD-3-built-site-blocker-clearance-failure.md)
  — resolved; rotated-capsule false positives and genuinely blocked coarse path edges
  are resolved with focused `9/9` and independent full-suite `114/114` evidence.
- [`SITE-6 — Settlement physical-evidence registry integration`](../exchanges/SITE-6-settlement-physical-evidence-registry-integration.md)
  — resolved; Primary recorded only the verified settlement-generator evidence and next
  full-footprint gate without promoting or expanding the machine graph.
- [`SITE-3 — Tactical ownership boundary`](../exchanges/SITE-3-tactical-ownership-boundary.md)
  — resolved; Settlements, Bases & Dungeons emits physical facts and neutral placement slots only.
