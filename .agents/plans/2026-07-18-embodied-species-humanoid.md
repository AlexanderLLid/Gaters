# Embodied Species Humanoid Implementation Plan

> **For agentic workers:** Execute inline in the Art Direction task. Do not delegate.

**Goal:** Build an isolated proof where a versioned humanoid brief plus an impact event
produces a simple weighted Blender humanoid, generated skeleton, generated reaction
motion, FBX transport, and validation manifest without manual DCC edits.

**Architecture:** A pure-Python reaction synthesizer converts event inputs and a seed
into a bounded motion recipe. A Blender adapter compiles body proportions plus that
recipe into derived artifacts. Tests prove per-event reproducibility, cross-event
variation, bounded motion, complete weights, hierarchy, timing, and clean rebuilds.

**Tech Stack:** Python standard library, Blender 5.2 Python API, PowerShell.

## Global Constraints

- Side project only: `research/embodied-species-lab/`.
- Do not modify Unreal production code or `research/machines.json`.
- Do not branch, commit, or push.
- Art quality is explicitly out of scope; rigid weighted primitives are sufficient.
- Authoritative inputs are JSON briefs and generator source; `.blend`, FBX, recipes,
  and manifests are derived.
- The same event and seed must produce the same semantic recipe.
- Different event seeds may produce different valid reactions.

---

### Task 1: Reaction synthesizer

**Files:**
- Create: `research/embodied-species-lab/tests/test_reaction.py`
- Create: `research/embodied-species-lab/reaction.py`

**Interface:**
- `synthesize_reaction(event: dict, seed: int, fps: int) -> dict`
- Output contains ordered keyframes with root translation and named bone rotations.

- [ ] Write failing `unittest` cases proving identical input equality, seed variation,
  bounded root displacement, ordered frames, and neutral recovery.
- [ ] Run `python -m unittest discover research/embodied-species-lab/tests -v` and verify
  failure because `reaction.py` does not exist.
- [ ] Implement the minimum seeded reaction recipe using `random.Random(seed)`.
- [ ] Re-run the unit tests and verify they pass.

### Task 2: Blender humanoid compiler

**Files:**
- Create: `research/embodied-species-lab/species/humanoid.json`
- Create: `research/embodied-species-lab/generate_humanoid.py`
- Create: `research/embodied-species-lab/Build-Humanoid.ps1`

**Interface:**
- Consumes the humanoid JSON brief and `synthesize_reaction`.
- Produces `humanoid.blend`, `humanoid.fbx`, `reaction.json`, and `manifest.json` under
  `research/embodied-species-lab/Derived/humanoid-v1/`.

- [ ] Add a minimal versioned brief with body dimensions, impact event, seed, frame rate,
  and artifact policy.
- [ ] Add an integration assertion to the harness before the Blender generator exists.
- [ ] Run the harness and verify it fails because `generate_humanoid.py` is missing.
- [ ] Generate a parent-first humanoid armature from body dimensions.
- [ ] Generate one rigidly weighted box segment per visible body part.
- [ ] Apply the synthesized reaction to root, pelvis, spine, head, arms, and legs.
- [ ] Reopen the `.blend` and validate hierarchy, one armature, one mesh, complete
  weights, action assignment, timing, and recovery.
- [ ] Export FBX and write the semantic manifest.

### Task 3: Repeatable black-box verification

**Files:**
- Create: `research/embodied-species-lab/Test-HumanoidMachine.ps1`
- Create: `research/embodied-species-lab/README.md`
- Create: `research/embodied-species-lab/.gitignore`

**Interface:**
- `./Test-HumanoidMachine.ps1` is the one-command verifier.

- [ ] Run unit tests from the harness.
- [ ] Build twice from a clean derived directory.
- [ ] Insert a stale sentinel between builds and prove the second build removes it.
- [ ] Compare semantic manifests byte-for-byte across repeated builds.
- [ ] Assert required bones, complete validation, reaction recovery, non-empty Blender
  and FBX artifacts, and honest FBX byte nondeterminism.
- [ ] Document input, output, command, current guarantees, and explicit limitations.
- [ ] Run the full verifier fresh and inspect its complete output.

### Task 4: Workstream closeout

**Files:**
- Modify: `.agents/workstreams/art.md`

- [ ] Record the isolated objective, evidence command, artifact path, and explicit lack
  of shared integration.
- [ ] Re-scan exchanges and leave registry integration unrequested until the proof
  produces evidence worth integrating.

