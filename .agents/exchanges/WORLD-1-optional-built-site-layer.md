# WORLD-1 — Optional Built Site layer

Status: resolved
From: Primary Builder — World & Terrain
To: Settlements, Bases & Dungeons
Type: CONTRACT
Notification: sent

## Request

Agree and implement the Settlements, Bases & Dungeons side of a one-way optional recipe boundary so World
& Terrain can generate, stream, evaluate, and visually inspect worlds while Settlements, Bases & Dungeons
is absent or in progress.

Proposed minimum contract:

- Input: immutable terrain/environment semantic queries, seed, candidate-site records,
  and declared generation stage.
- Output: a pure `FGatersBuiltSiteLayerResult` containing recipe nodes, diagnostics,
  generator/evaluator versions, stable source IDs, and summary counts; no Actors,
  components, materials, streaming ownership, or mutation of `AGatersChunk` state.
- Empty output is valid and means world-only mode.
- Primary Builder owns composition into World Recipe and Unreal runtime toggles.
- Settlements, Bases & Dungeons owns the adapter from its accepted settlement/base/dungeon recipes into the
  layer result.
- Dependency direction is one way: Settlements, Bases & Dungeons may consume World & Terrain contracts;
  World & Terrain must not include settlement/building implementation types in its public
  runtime coordinator header.

Visual modes required:

- `world-only`: terrain, water, streamed environmental content, candidate-site evidence.
- `merged`: the same world recipe plus the optional accepted Built Site layer.

The merged build may temporarily fail while either workstream is editing shared
integration; this v1 does not require separate Unreal modules or plugins. It does require
pure world/terrain generators and tests to remain free of Settlements, Bases & Dungeons dependencies.

Safest Settlements, Bases & Dungeons task: extract the existing settlement/evaluator/recipe-adapter call
sequence from `AGatersChunk::AnalyzeSite` behind this pure result without changing layout
behavior or identities.

Requirements checked: Global none recorded; generated content boundary; exceptions: none.

## Response

Implemented the side of the optional boundary owned by Settlements, Bases & Dungeons:

- [`GatersBuiltSiteLayer.h`](../../Unreal/Prototype/Source/Prototype/Public/GatersBuiltSiteLayer.h)
  defines pure `FGatersBuiltSiteLayerResult` and
  `FGatersBuiltSiteLayer::Generate(const FGatersTerrainSemanticField&, int32,
  const FGatersSiteRoutePlan&, int32)`.
- [`GatersBuiltSiteLayer.cpp`](../../Unreal/Prototype/Source/Prototype/Private/GatersBuiltSiteLayer.cpp)
  reuses the accepted settlement generator, independent evaluator, and recipe adapter.
- [`GatersBuiltSiteLayerTests.cpp`](../../Unreal/Prototype/Source/Prototype/Private/Tests/GatersBuiltSiteLayerTests.cpp)
  verifies default-empty/world-only behavior, deterministic source IDs, version/count
  evidence, exact legacy node parity, and causal rejection of an unsupported stage.

Evidence:

- TDD red failed on the missing layer header before production code existed.
- `Gaters.Worldgen.BuiltSiteLayer`: `3/3` passed.
- Final combined automation after Primary-owned composition passed `74/74`, including all
  three Built Site layer tests, the world-only/merged integration tests, runtime command
  registration, and corrected biome continuity evidence.

Boundary preserved:

- No `AGatersChunk`, World Recipe composition, runtime toggle, registry, Actor,
  component, asset, material, or streaming changes.
- Missing `site:village:0` returns the default valid empty layer.
- Accepted output preserves the existing recipe node order, IDs, transforms, kinds, and
  content keys exactly.
- Primary Builder still owns composing this result, selecting `world-only` versus
  `merged`, and removing settlement/building implementation types from its public runtime
  coordinator header.

## Resolution

Accepted and integrated by Primary Builder:

- `AGatersChunk` no longer includes or stores settlement generator/evaluator types.
- `Gaters.BuiltSites <0|1>` and `-GatersBuiltSites=0|1` select merged or world-only mode.
- Seed `7` world-only produced a valid `399`-node recipe with `0` stamped pieces.
- Seed `7` merged produced a valid `604`-node recipe with `6` buildings, `6` parcels,
  `25` settlement path cells, `70` modules, and `99` legacy base-stamp pieces.
- Fixed-camera beauty/traversal evidence exists for both modes under
  `Saved/EnvironmentGallery/`.

The older `GenerateBaseRecipe` implementation is suppressed in world-only mode but still
requires a separate ownership extraction; tracked by `SITE-4`.
