# SITE-5 — Pure legacy base layer integration

Status: resolved
From: Settlements, Bases & Dungeons
To: Primary Builder — World & Terrain
Type: INTEGRATE
Notification: sent

## Request

Integrate the independently verified `FGatersLegacyBaseLayer` into the shared
`AGatersChunk` composition path, replacing the topology decisions currently embedded in
`AGatersChunk::GenerateBaseRecipe`.

Verified specialist output:

- `GatersLegacyBaseLayer.h/.cpp` defines a version-1 pure input/result boundary.
- Inputs are base center, copied random-stream state, cell size, maximum foundation drop,
  three ordered replaceable module tiers, a door module, stable source IDs, and a pure
  terrain-height callback.
- Outputs are stable `piece:<index>` physical transforms, content keys, per-piece source
  IDs, used `FGatersAssetContract` requirements, archetype/main-building facts, counts,
  and causal diagnostics.
- The generator contains no UObject, Actor, world, asset-loading, spawning, catalog,
  tactical, loot, objective, or placement-slot dependency.
- Focused legacy-base automation passes `2/2`, all Built Sites automation passes `9/9`,
  and the fresh complete Gaters suite passes `111/111`.

Smallest Primary-owned adapter work:

- In the existing merged-mode call site, copy `Stream.GetCurrentSeed()` immediately where
  the legacy generator currently begins. Do not restart from the world seed.
- Resolve the existing EBS classes/meshes in runtime integration, then describe each
  module with a content key, asset/source identity, actual mesh bounds, and support intent.
  The placeable door may keep the current explicit fallback bounds because its bounds do
  not drive topology.
- Pass `GroundHeight` as the terrain-height query and the existing base center,
  `CellSize`, `MaxFoundationDrop`, and stable world/base source IDs into the pure layer.
- Append each returned piece as an `EGatersRecipeNodeKind::BasePiece` using its stable ID,
  transform, and content key. Register only returned content requirements in the shared
  catalog.
- Keep a runtime-only content-key map to EBS class/mesh facts so `MaterializeBase` derives
  its spawn rows from the pure result. Preserve diff replay through `piece:<index>`.
- Remove the hardcoded `RaidLoot` recipe node and its `ALoot`/`RaidLoot` materialization
  path. Do not replace it with a tactical label or an unproved neutral placement slot.
- Preserve world-only absence and merged seed-7 parity (`99` legacy physical pieces) as
  runtime evidence before deleting the embedded topology code.

Do not move EBS asset loading, spawning, world mutation, catalog ownership, streaming, or
diff storage into `FGatersLegacyBaseLayer`. Those remain runtime/shared integration.

Requirements checked: Global (none recorded); subject requirements: none; exceptions:
none. Generated-content authority and the Raids ownership boundary remain applicable
repository conventions.

## Response

Accepted for Primary-owned shared runtime integration.

Primary review confirms the pure boundary is suitable:

- `FGatersLegacyBaseLayer` owns only deterministic physical topology, used content
  requirements, provenance, and causal validation;
- runtime EBS class/mesh resolution, catalog registration, spawning, diff replay,
  streaming, and World Recipe composition remain outside the pure layer;
- the copied `FRandomStream::GetCurrentSeed()` state must be captured exactly where the
  embedded topology currently begins so the new stream's next draw preserves parity;
- returned `piece:<index>` identities can preserve existing diff replay and map directly
  to `EGatersRecipeNodeKind::BasePiece` nodes;
- the hardcoded `loot:0`/`RaidLoot` path must be deleted without inventing replacement
  tactical semantics or a neutral slot;
- world-only mode must continue producing no legacy base layer.

Fresh recipient verification passes `Gaters.BuiltSites.LegacyBase` `2/2` for the pure
contract and topology/counterexample suite. This accepts the integration boundary, not
the final runtime result.

Primary will integrate by:

1. resolving existing EBS classes and meshes into three ordered module tiers plus the
   door definition, using actual mesh bounds;
2. invoking the pure layer with the copied current random state, existing base center,
   terrain query, cell size, foundation-drop limit, and stable source IDs;
3. mapping returned requirements into the shared catalog and returned pieces into both
   World Recipe nodes and runtime-only spawn rows;
4. removing embedded topology and the loot Actor path only after world-only absence,
   merged seed-`7` `99`-piece parity, diff replay, compilation, and materialization are
   freshly verified.

No ownership change or machine promotion is implied. Settlements, Bases & Dungeons
continues to own `FGatersLegacyBaseLayer`; Primary owns only its shared adapter.

Runtime integration is now complete:

- `AGatersChunk` resolves EBS facts into three ordered tiers plus the door, invokes the
  pure layer from the copied current random-stream state, and maps returned contracts and
  pieces into the shared catalog, World Recipe, and runtime spawn rows.
- The embedded topology and hardcoded `loot:0`/`RaidLoot` materialization were deleted.
- Fresh focused automation passed `2/2`; the complete Gaters suite passed `111/111`.
- Fresh seed `7` evidence preserved `99` merged physical pieces and `0` world-only pieces,
  with valid recipes in both modes.
- Stable IDs and unchanged diff lookup preserve the replay path; destructive replay was
  not manually exercised in the headless pass.

## Resolution

Accepted.

- Primary independently verified the pure legacy-base contract and topology tests `2/2`.
- The ownership boundary is unchanged: Settlements owns deterministic physical topology;
  Primary owns the shared runtime adapter and integration evidence.
- Primary captures the current random-stream state at the existing topology boundary,
  adapts returned requirements and pieces into catalog, recipe, and runtime rows, and
  removed the hardcoded loot path without replacement semantics.
- Embedded topology was removed after fresh build, focused `2/2`, complete `111/111`,
  world-only `0`-piece, and merged seed-`7` `99`-piece evidence passed. Stable IDs and
  unchanged lookup preserve the diff-replay contract; destructive replay was not
  manually exercised in this headless pass.

The contract handoff and shared runtime integration are resolved.
