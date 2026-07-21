# Parametric Blender Face Proof Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Produce one editable Blender face that plausibly belongs to the approved painted mid-poly direction while preserving named facial parameters for later generation.

**Architecture:** Borrow MPFB's maintained human topology and morph targets, adapt one versioned JSON face recipe into those parameters, and add only the minimum Blender material/look-development pass needed for the art test. The generated `.blend` and fixed renders are derived evidence; the recipe and adapter are authoritative.

**Tech Stack:** Blender 5.2 LTS, MPFB 2.x Blender extension, Blender Python API, PowerShell verifier, JSON recipe.

## Global Constraints

- `ART-1`: grounded mature presentation; no childlike, cartoon, toy, cel-shaded, or faceted result.
- `ART-2`: one characterful adult face with plausible ancestry, age, proportions, and natural asymmetry; no generic attractive light-skinned default.
- `ART-3`: this is isolated Art Direction evidence and contains no rig, animation, Unreal, or character-family pipeline claim.
- Use stable topology and named parameters so later faces can change features without rebuilding topology.
- Preserve the previous result only as ignored derived output until the new verifier replaces it; do not use it as visual evidence.
- Do not commit, branch, or push.

---

### Task 1: Prove the parametric foundation exists

**Files:**
- Modify: `research/character-style-exploration/blender-face-proof/Test-FaceProof.ps1`
- Derived dependency: Blender user extension `mpfb`

**Interfaces:**
- Consumes: Blender extension repository configured as `blender_org`.
- Produces: a verifier that fails until `mpfb` is enabled and exposes a human basemesh operator/API.

- [ ] Replace the old geometry-floor check with a preflight that opens Blender factory settings plus user extensions and asserts `mpfb` loads.
- [ ] Run the verifier and observe RED because MPFB is not installed.
- [ ] Install and enable MPFB with Blender's native extension command: `blender -c extension install -s -e mpfb`.
- [ ] Re-run only the MPFB preflight and require a clean PASS before writing generation code.

### Task 2: Define the face recipe contract

**Files:**
- Modify: `research/character-style-exploration/blender-face-proof/face-brief.json`
- Modify: `research/character-style-exploration/blender-face-proof/Test-FaceProof.ps1`

**Interfaces:**
- Consumes: `schemaVersion`, `proofId`, `styleId`, adult age, skin palette, named facial targets, asymmetry targets, render views.
- Produces: strict JSON validation and expected parameter names for the Blender adapter.

- [ ] Encode one mature dark-skinned face with a long crooked nose, strong jaw, restrained cheek asymmetry, unequal eyelids, unequal ears, and close-cropped greying hair.
- [ ] Add verifier assertions for the approved proof/style IDs, adult age floor, nonzero asymmetry, and the fixed `front`, `three-quarter`, `profile` views.
- [ ] Run the verifier and observe RED because the MPFB recipe adapter and new artifacts do not exist.

### Task 3: Generate the editable MPFB head

**Files:**
- Replace: `research/character-style-exploration/blender-face-proof/generate_face.py`
- Derived: `research/character-style-exploration/blender-face-proof/Derived/face-v2/face-proof.blend`
- Derived: `research/character-style-exploration/blender-face-proof/Derived/face-v2/manifest.json`

**Interfaces:**
- Consumes: `face-brief.json` and MPFB's bundled basemesh/morph data.
- Produces: `FaceProofHead`, stable topology, named recipe/morph metadata, and no armature/actions.

- [ ] Use MPFB's own human creation service/operator rather than hand-authored primitives.
- [ ] Apply the recipe through MPFB targets; keep the resulting topology unchanged.
- [ ] Add only named asymmetry shape keys when MPFB has no equivalent bilateral target.
- [ ] Delete or hide body geometry below the shoulders only after the facial targets are applied; do not alter head topology.
- [ ] Save, reopen, and emit a manifest containing MPFB version, topology counts, recipe hash, applied target values, shape keys, and absence of armature/actions.
- [ ] Run the verifier until the structural contract passes.

### Task 4: Match the approved painted-mid-poly face direction

**Files:**
- Modify: `research/character-style-exploration/blender-face-proof/generate_face.py`
- Derived: `research/character-style-exploration/blender-face-proof/Derived/face-v2/front.png`
- Derived: `research/character-style-exploration/blender-face-proof/Derived/face-v2/three-quarter.png`
- Derived: `research/character-style-exploration/blender-face-proof/Derived/face-v2/profile.png`

**Interfaces:**
- Consumes: the unchanged parametric head and the approved concept-A reference.
- Produces: restrained painted skin/eyes/hair materials, neutral studio light, and fixed 768px comparison renders.

- [ ] Create one skin material with broad hand-painted value variation and high roughness; avoid pore-level photorealism and procedural noise that reads as marble.
- [ ] Create anatomically scaled eyes and close-cropped hair/brows using MPFB geometry or surface-bound Blender geometry; no floating curves or primitive hair cap.
- [ ] Frame and render the three fixed views with a portrait lens and identical lighting.
- [ ] Inspect all views against `ART-1` and the approved A reference; reject immediately for detached features, toy proportions, generic face, or profile failure.
- [ ] Make only evidence-driven corrective edits, preserving named controls and topology.

### Task 5: Verify and report the real feasibility result

**Files:**
- Modify: `.agents/workstreams/Art Direction.md`
- Modify: `.agents/workflow-feedback.md` only if the workflow or machine fails.

**Interfaces:**
- Consumes: reopened `.blend`, manifest, three renders, and human visual acceptance.
- Produces: a truthful Art Direction feasibility result; no Character Generation & Animation claim.

- [ ] Run `Test-FaceProof.ps1`, `research/Test-SharedAgentDocs.ps1`, and `git diff --check` fresh.
- [ ] Open the verified `.blend` visibly in Blender.
- [ ] Present all three renders and state the largest remaining mismatch before asking for human acceptance.
- [ ] Record success only if the human accepts that the face belongs to concept A; otherwise record the diagnostic failure and next isolated correction.

Requirements checked: `ART-1`, `ART-2`, `ART-3`; exceptions: none. `CHAR-1`, `CHAR-2`, and `CHAR-3` are excluded by `ART-3`.
