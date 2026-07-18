# Blender candidate representation proof

Status: **DONE_WITH_CONCERNS**

## Outcome

- The authoritative JSON brief, `SourceAssets/Blender/contracts/neutral-rock.json`, and
  authoritative generator produce a versioned, editable, reproducible derived `.blend`
  plus derived near/mid/far candidate meshes.
- Repeatable entry point:
  `& .\SourceAssets\Blender\Test-CandidateLod.ps1`
- Generated evidence is intentionally ignored under
  `SourceAssets/Blender/Derived/neutral-rock-v1/`.
- Blender used: **5.2.0 LTS**. Generator version: **1**.

## Evidence

| Role | Triangles | Blender bounds (meters) | OBJ bounds after `(x,y,z) -> (x,z,-y)` | Portable artifact |
|---|---:|---|---|---|
| near | 1280 | `[-1.2,-0.9,0]` to `[1.2,0.9,1.5]` | `[-1.2,0,-0.9]` to `[1.2,1.5,0.9]` | `neutral-rock-near-v1.obj` |
| mid | 320 | same | same | `neutral-rock-mid-v1.obj` |
| far | 80 | same | same | `neutral-rock-far-v1.obj` |

- All representations carry material meaning `neutral-stone-placeholder`.
- The manifest explicitly marks the JSON contract and generator authoritative and records
  their SHA-256 values; it marks the `.blend` derived and reproducible. It also records
  stable asset identity, Blender/generator versions, roles, OBJ hashes, triangle counts,
  Blender bounds, portable-file bounds, pivot, scale,
  material meaning, boundary policy, and individual validation results.
- The contract and manifest explicitly record `selectedStyle: null`; the fixture does not
  select or imply `gaters.clean-midpoly-painted` or any other final style.
- Two full headless builds produced byte-identical manifests.
- Before the second build, the test writes an unknown stale sentinel into the derived
  directory. The generator removes it by recreating the expected candidate directory.
  A separate sentinel proves an invalid contract is rejected before that cleanup occurs.
- Each exported OBJ was independently parsed for vertices, non-empty geometry, and
  triangle-only faces; OBJ hashes were stable across the repeated builds.
- The generator itself reopens the derived `.blend` headlessly before export,
  reacquires all three meshes by role, and validates 1280/320/80 triangles, shared
  origin/unit scale, and shared material meaning. Those checks and representation records
  are persisted under `blenderArtifact.validation` in the manifest.
- OBJ exports explicitly use forward `-Z`, up `Y`. The manifest records the resulting
  `(x,y,z) -> (x,z,-y)` mapping, both bounds spaces, and validation that every parsed OBJ
  bound equals the converted Blender-space bound.
- `neutral-rock-invalid.json` deliberately makes far detail equal near detail. The same
  runner rejects it with `representation detail must strictly decrease from near to far`.
- `preview-near.png` was rendered headlessly with Eevee. Its byte hash is deliberately not
  part of the deterministic manifest because repeated Eevee PNG bytes varied while the
  geometry evidence remained stable.

## Ownership boundary

- Blender near/mid/far meshes are **authored candidates only**. They are selected only if
  silhouette comparison demonstrates an advantage.
- Unreal automatic LOD, HLOD, and Nanite remain borrowed runtime/build options and the
  default comparison baseline; this proof does not replace or configure them.
- OBJ is used because the current Unreal Interchange intake path accepts OBJ, but that
  importer was not used as correctness evidence here.

## Concerns / next adapter work

- The current Unreal importer does not assign these three OBJ files to a Static Mesh LOD
  chain, set screen-size policy, choose Nanite/HLOD, or prove coordinate/unit conversion.
- OBJ material sidecars are emitted and Blender records shared semantic material meaning,
  but Unreal material binding still needs an explicit adapter contract.
- Existing Unreal import automation can be confused by stale generated assets and implicit
  settings, so eventual integration needs independent output cleanup, explicit import
  settings, and post-import geometry/unit assertions.
- No style is selected. The neutral geometry and placeholder material are pipeline
  fixtures only; final silhouette and surface art remain intentionally undecided.
