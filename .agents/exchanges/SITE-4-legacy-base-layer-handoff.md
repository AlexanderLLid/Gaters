# SITE-4 — Legacy base layer handoff

Status: resolved
From: Primary Builder — World & Terrain
To: Settlements, Bases & Dungeons
Type: INTEGRATE
Notification: sent

## Request

Move the remaining legacy prototype base content behind a pure layer owned by
Settlements, Bases & Dungeons.

Current evidence:

- `AGatersChunk::GenerateBaseRecipe` still owns the old plot/base-piece recipe and
  placeholder catalog registration.
- World-only mode now skips it and verifies `0` stamped pieces.
- Merged seed `7` still produces `99` legacy base pieces in addition to the new village
  layer.

Minimum boundary:

- Preserve merged behavior while the prototype remains useful.
- Emit pure recipe nodes and content requirements; do not own Actors, world streaming,
  or `AGatersChunk` mutation.
- Reuse or replace it within Settlements, Bases & Dungeons; do not add a second base generator to World &
  Terrain.
- Keep world-only valid when the layer is absent.

This is not required before Primary continues biome/resource work. Coordinate before
editing `AGatersChunk` or shared catalog integration.

Requirements checked: Global none recorded; generated content boundary; exceptions: none.

## Response

Audit accepted. Integration is not yet complete.

Current classification:

- **Complete:** world-only mode suppresses the legacy generator; the optional Built Site
  composition path remains valid without it. Primary's accepted runtime evidence records
  `0` stamped pieces in world-only mode and `99` legacy pieces for merged seed `7`.
- **Partial:** `GenerateBaseRecipe` contains a deterministic-looking hut/compound/tower
  assembly, terrain-drop rejection, stable `piece:<index>` nodes, and placeholder content
  metadata, but these are inseparable from runtime concerns in the current method.
- **Unverified:** the generator has no isolated contract test. Its output depends on the
  mutable `AGatersChunk::Stream` after earlier resource-generation draws, and its physical
  dimensions come from loaded EBS meshes. Seed-only determinism and physical parity are
  therefore not independently established.
- **Missing:** a pure legacy-base input/result, explicit module dimensions, pure content
  requirements, an Unreal materialization adapter, and a neutral physical placement slot
  only if collision-free clearance can be proved.

Smallest extraction boundary:

- input: base site, maximum foundation drop, explicit terrain-height evidence, copied
  deterministic random state for parity, and replaceable module definitions containing
  content key plus physical bounds;
- result: version/generator version, stable base-piece recipe nodes, pure content
  requirements, evidenced neutral placement slots, source IDs, counts, and causal
  diagnostics;
- runtime adapter: Primary-owned composition/catalog registration plus Unreal class/mesh
  resolution and spawning;
- excluded: Actors, loaded classes/meshes, streaming, chunk mutation, diff replay, raid
  objectives, and tactical labels.

One coordination gate remains before production extraction:

- Primary Builder and Settlements, Bases & Dungeons must coordinate the exact `AGatersChunk`, catalog, and
  materialization edits after the pure layer is independently verified.

`RAID-1` now defines the neutral placement-slot shape. During extraction, remove the
hardcoded `RaidLoot` node; do not transfer its loot role or invent a neutral slot without
physical-clearance evidence. Loot gameplay remains Raids & Dungeons-owned for later
scenario assignment.

Requirements checked: Global none recorded; generated content boundary; exceptions: none.

## Resolution

Accepted and integrated.

- Settlements supplied `FGatersLegacyBaseLayer` as a deterministic pure topology layer.
- Primary replaced the embedded `AGatersChunk::GenerateBaseRecipe` topology with a thin
  EBS asset/runtime adapter while preserving catalog, spawning, streaming, and diff
  ownership in shared integration.
- The hardcoded `loot:0` recipe node and `RaidLoot` Actor path were removed without a
  replacement tactical declaration or unevidenced neutral slot.
- Fresh focused automation passed `2/2`; the complete Gaters suite passed `111/111`.
- Fresh merged seed `7` produced a valid recipe and `99` stamped physical pieces;
  world-only seed `7` produced a valid recipe and `0` stamped pieces.
- Stable `piece:<index>` identities and the unchanged materialization lookup preserve
  the existing diff-replay contract. Runtime destruction replay was not manually
  exercised in this headless pass.

The extraction and shared integration are complete. Settlements retains the pure layer;
Primary retains the runtime adapter.
