# Blender to Unreal native LOD checkpoint

## Outcome

- One style-neutral rock contract and Blender generator produce near, mid, and far
  candidate representations.
- Derived FBX transports import headlessly as one Unreal Static Mesh with native
  LOD0, LOD1, and LOD2. Unreal owns screen-size switching.
- The runtime environment-content adapter promotes the generated mesh only after the
  existing intake linter accepts it; the engine sphere remains the optional fallback.
- This is one pipeline fixture, not an environment asset family and not a selected art
  style. `content.environment-asset-factory` therefore remains planned.

## Evidence

- Blender 5.2 LTS harness: repeated manifest builds match; invalid detail ordering is
  rejected without deleting prior valid output.
- Source/portable triangles: `1280 / 320 / 80`.
- FBX bytes are explicitly marked nondeterministic because Blender exporter metadata
  changes; deterministic geometry evidence remains in the hashed OBJ artifacts.
- Unreal 5.8 import harness: repeated imports produce
  `/Game/Gaters/Generated/Candidates/SM_NeutralRock` with three native LODs.
- Unreal-measured triangles: `1280 / 320 / 80`.
- Unreal-measured size: `240 x 180 x 150 cm` after explicit scene-unit conversion.
- Focused environment-content automation accepts the generated mesh and selects contract
  version 2 over the version 1 placeholder; trees remain placeholder-safe.
- Unreal editor build succeeds and the complete `Gaters` suite passes `52/52`.
- Headless seed 7 reports `CATALOG key=environment.rock source=generated-native-lod`,
  72 streamed content placements, four valid native ISM batches, and zero performance
  issues at the current budget.

## Ownership boundary

- JSON contract and Blender generator are authoritative source inputs.
- `.blend`, OBJ, FBX, preview, Unreal `.uasset`, and import report are derived evidence.
- Gaters owns stable identity, semantic keys, contracts, intake, and selection.
- Blender owns source geometry construction; Unreal owns Static Mesh LOD runtime behavior,
  collision generation, rendering, and instancing.
- Nanite, HLOD, automatic reduction, final materials, and final LOD thresholds remain
  deliberately unevaluated rather than reimplemented.
