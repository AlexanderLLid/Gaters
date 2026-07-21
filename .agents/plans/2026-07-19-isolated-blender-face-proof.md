# Isolated Blender Face Proof Implementation Plan

> **For agentic workers:** Execute inline with test-first checkpoints. Do not delegate,
> branch, commit, or modify Character Generation artifacts.

**Goal:** Produce one editable Blender head and fixed-view renders that test whether the
selected painted mid-poly face language can be authored directly in Blender.

**Architecture:** A compact JSON look brief feeds one Blender Python generator. Blender
native primitives, voxel remesh, materials, camera, and lights create an isolated head;
a PowerShell verifier rebuilds it and validates the saved source, semantic manifest, and
three renders. The artifact is visual evidence for `content.style-contract`, not evidence
for `content.character-body-factory`.

**Tech stack:** Blender 5.2 Python API, PowerShell, JSON, PNG.

## Global constraints

- `ART-1`: mature, plausible, restrained non-photorealistic, smooth readable forms.
- `ART-2`: mature dark-skinned face with age, distinctive proportions, and natural
  asymmetry instead of a generic glamour face.
- `ART-3`: all source and evidence stay under
  `research/character-style-exploration/blender-face-proof/`; no character-generation,
  rigging, animation, physics, or runtime artifact is touched or inferred.
- No added dependency, rig, expression system, topology claim, or Unreal import.

---

### Task 1: Define and prove the missing face contract

**Files:**

- Create: `research/character-style-exploration/blender-face-proof/face-brief.json`
- Create: `research/character-style-exploration/blender-face-proof/Test-FaceProof.ps1`

**Produces:** A verifier requiring `face-proof.blend`, `manifest.json`, and
`front.png`, `three-quarter.png`, `profile.png`; the manifest must report one visual
proof, three views, skin/eye/hair/cloth materials, smooth head geometry, explicit
asymmetry landmarks, and no armature or animation.

- [ ] Write the brief and verifier before the generator exists.
- [ ] Run `./Test-FaceProof.ps1`.
- [ ] Verify RED: fail with `Missing required path: ...generate_face.py`.

### Task 2: Build the smallest native Blender face

**Files:**

- Create: `research/character-style-exploration/blender-face-proof/generate_face.py`

**Consumes:** `face-brief.json` schema version 1.

**Produces:** One isolated `.blend`, semantic manifest, and three fixed-camera renders.

- [ ] Clear the default scene and validate the brief.
- [ ] Build a smooth adult head from native ellipsoid masses joined by voxel remesh.
- [ ] Add broad crooked nose, strong jaw, unequal ears, asymmetrical eyes/brows, lips,
  close-cropped grey hair, neck, and simple blue cloth collar.
- [ ] Add matte painted materials, neutral backdrop, three-point lighting, and fixed
  front/three-quarter/profile cameras.
- [ ] Save and reopen the `.blend`; write semantic evidence only after reopen checks pass.
- [ ] Run `./Test-FaceProof.ps1`.
- [ ] Verify GREEN: every artifact and semantic contract check passes.

### Task 3: Evaluate the visual proof without promoting character capability

**Files:**

- Modify: `.agents/workstreams/Art Direction.md`

**Produces:** A labeled visual comparison and truthful status.

- [ ] Inspect all three renders against the selected A reference.
- [ ] Record whether the proof supports painted-mid-poly face feasibility or falsifies it,
  naming the largest visible gap.
- [ ] Run `Test-SharedAgentDocs.ps1` and `git diff --check`.
- [ ] Open the verified `.blend` visibly only if no Blender process is already running.

