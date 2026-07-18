# Blender -> Unreal Candidate LOD Implementation Plan

> **For agentic workers:** execute inline with TDD; do not commit because the human reviews the shared worktree first.

**Goal:** Prove that one Blender-authored near/mid/far candidate can become one validated native Unreal Static Mesh LOD chain and optionally replace the runtime rock placeholder.

**Architecture:** The authoritative JSON + Blender generator remain the asset source. Blender adds one FBX per already-validated representation. A focused Unreal Python adapter validates the manifest before mutation, imports the near FBX as LOD0, uses `StaticMeshEditorSubsystem.import_lod` for mid/far, and records measured Unreal evidence. Runtime catalog registration attempts the generated asset and retains the existing placeholder when it is absent.

**Tech Stack:** Blender 5.2 Python, FBX, Unreal 5.8 Python editor scripting, native `UStaticMesh` LODs, PowerShell harness, existing Gaters catalog/materializer.

## Global constraints

- JSON contract and generator are authoritative; `.blend`, FBX, `.uasset`, reports, and previews are derived.
- `selectedStyle` remains `null`; this is pipeline evidence, not the final art direction.
- Unreal owns LOD switching. Gaters owns identity, validation, and evidence only.
- No new dependency, runtime LOD selector, Nanite rule, HLOD rule, or general asset-family abstraction.
- Generated Unreal assets remain under ignored `/Game/Gaters/Generated/`.

---

### Task 1: Export validated FBX representations

**Files:**
- Modify: `SourceAssets/Blender/Test-CandidateLod.ps1`
- Modify: `SourceAssets/Blender/generate_candidate_lod.py`
- Modify: `SourceAssets/Blender/README.md`

**Interface:** Each `manifest.json` representation gains `unrealFbx.file`, explicit axes, and `deterministicBytes: false`. The FBX contains exactly the selected representation at meter-authored scale; deterministic OBJ evidence and measured Unreal results validate geometry because Blender FBX transport bytes contain changing metadata.

- [ ] Add harness assertions requiring one nonempty FBX and hash record per near/mid/far representation.
- [ ] Run `SourceAssets/Blender/Test-CandidateLod.ps1`; verify RED because `unrealFbx` is absent.
- [ ] Add the smallest selected-object FBX export beside the existing OBJ evidence and record its hash.
- [ ] Re-run the harness; verify deterministic manifests, 1280/320/80 OBJ evidence, valid FBX artifacts, stale-output cleanup, and invalid-contract preservation.

### Task 2: Import one native Unreal LOD chain

**Files:**
- Create: `Unreal/Prototype/Scripts/Test-CandidateLodImport.ps1`
- Create: `Unreal/Prototype/Scripts/ImportCandidateLod.py`
- Output: `Unreal/Prototype/Content/Gaters/Generated/Candidates/SM_NeutralRock.uasset`
- Output: `Unreal/Prototype/Saved/AssetImport/neutral-rock-lod.json`

**Interface:** `ImportCandidateLod.py` consumes `GATERS_CANDIDATE_MANIFEST`, `GATERS_IMPORT_DESTINATION`, `GATERS_IMPORT_NAME`, and `GATERS_IMPORT_REPORT`; it produces one saved `StaticMesh` and JSON evidence containing source manifest hash, object path, LOD count, measured triangles, bounds centimeters, screen sizes, and engine version.

- [ ] Write the PowerShell harness first. It builds Blender evidence, runs the missing importer twice, requires exact LOD triangles `1280/320/80`, requires bounds `240x180x150 cm` within tolerance, compares stable report fields, and checks the generated `.uasset`.
- [ ] Run the harness; verify RED because `ImportCandidateLod.py` does not exist.
- [ ] Implement manifest validation before mutation, automated near-FBX import, `remove_lods`, sequential `import_lod` calls for LOD1/LOD2, measured `get_num_lods` / `get_num_triangles`, bounds validation, save, and sorted JSON report.
- [ ] Re-run twice; verify GREEN with one asset, three native LODs, exact decreasing triangles, converted centimeter bounds, and stable evidence.

### Task 3: Optional runtime catalog adoption

**Files:**
- Modify test first: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersContentCatalogTests.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersChunk.cpp`
- Modify: `research/machines.json`
- Create: `.agents/reports/blender-unreal-candidate-lod.md`

**Interface:** At runtime, `/Game/Gaters/Generated/Candidates/SM_NeutralRock` registers as `environment.rock` when loadable and contract-valid. If missing, the existing style-neutral placeholder contract is registered instead.

- [ ] Add a catalog test proving a real compatible mesh wins over a placeholder for the same semantic key; run focused automation and verify RED if replacement precedence is missing.
- [ ] Implement only the required catalog/runtime registration behavior; keep trees and all missing-art behavior unchanged.
- [ ] Run the focused catalog/materializer tests, full `Gaters` automation, candidate import harness, registry validator, and one headless seed-7 runtime.
- [ ] Record whether the runtime compiler selected the generated mesh and whether performance stayed within the existing budget. Keep the environment asset factory `planned`; one neutral rock is not a family.

## Self-review

- Scope covers one static candidate only; characters, skeletal LODs, materials, animation, Nanite, and HLOD remain separate machines.
- All mutation occurs after manifest/file validation.
- Imported triangle and centimeter bounds evidence comes from Unreal, not merely from the Blender manifest.
- No source-of-truth conflict: the generated `.uasset` can be deleted and reproduced.
