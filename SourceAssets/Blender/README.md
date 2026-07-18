# Blender sources

- The JSON asset brief and generator script are authoritative.
- The `.blend` is a versioned, editable, reproducible derived artifact.
- Keep one generated asset or tightly coupled asset family per `.blend` file.
- Unreal imports and other generated outputs are derived artifacts; do not store them here.

## Candidate representation proof

Run `./Test-CandidateLod.ps1` from PowerShell. It invokes Blender headlessly twice,
checks deterministic manifests, and proves that a deliberately mismatched representation
contract is rejected. Generated `.blend`, OBJ, manifest, and preview evidence is written
under ignored `Derived/`.

Each representation also exports a derived FBX using Blender `-Y` forward / `Z` up.
The OBJ remains the independently parsed portable geometry evidence; the FBX is only the
native Unreal LOD-import adapter artifact. Blender's FBX bytes contain nondeterministic
export metadata, so the manifest says so instead of publishing a misleading stable hash;
Unreal independently measures the imported triangles and bounds.

The near/mid/far meshes are authored candidates, not a runtime LOD policy. Unreal's
automatic LOD, HLOD, and Nanite remain borrowed build/runtime options; prefer them unless
an authored candidate demonstrably preserves the required silhouette better.

The fixture contract explicitly records `selectedStyle: null`; it is pipeline evidence,
not a style selection.

## Neutral motion proof

Run `./Test-MotionFixture.ps1` from PowerShell. A style-neutral JSON brief generates a
tiny armature, fully weighted primitive mesh, one-second root-motion clip, contact
markers, derived `.blend`, and FBX. The harness builds twice, clears stale output,
reopens the `.blend`, and compares hierarchy, timing, root samples, weights, and markers.

Run `../../Unreal/Prototype/Scripts/Test-MotionFixtureImport.ps1` for the full round trip.
Unreal imports a native Skeletal Mesh, Skeleton, and Anim Sequence twice and compares
required hierarchy, scale, duration, key count, and root path. FBX does not carry Blender
timeline markers into Unreal notifies; the import report preserves that gap explicitly.
