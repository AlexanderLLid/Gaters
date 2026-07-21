# Unanchored Recovery Implementation Plan

> **For agentic workers:** Execute inline in the Art Direction task. Do not delegate.

**Goal:** Generate an active humanoid that takes one bounded corrective step after an
impact and exports the simulated recovery as a skeleton animation.

**Architecture:** A pure-Python planner converts an event and stance into a foot-target
schedule. A Blender adapter reuses the current body, joint, simulation, bake, reopen,
export, and render functions while replacing passive feet with active feet controlled by
generated ground-contact targets. The current anchored build remains unchanged.

**Tech Stack:** Python standard library, Blender 5.2 Python API, PowerShell.

## Global Constraints

- Side project only: `research/embodied-species-lab/`.
- Do not modify Unreal, `research/machines.json`, branches, commits, or remotes.
- Reuse the current humanoid and physical-profile generators.
- One recovery step, flat ground, and placeholder art only.
- Preserve `Test-PhysicalHumanoid.ps1` as the anchored champion.
- No humanoid rigid-body part may be `PASSIVE` in the challenger.

---

### Task 1: Recovery planner

**Files:**
- Create: `research/embodied-species-lab/recovery.py`
- Create: `research/embodied-species-lab/tests/test_recovery.py`

**Interfaces:**
- Consumes: impact event dictionary and stance dictionary.
- Produces: `plan_recovery(event: dict, stance: dict, fps: int = 30) -> dict`.

- [x] Write failing tests proving weak impacts hold stance, strong impacts step in the
  horizontal force direction, left/right impacts select the matching foot, step length
  stays bounded, frames are ordered, and identical inputs are equal.

```python
plan = plan_recovery(event, {"foot_l": [0.16, 0.0, 0.05], "foot_r": [-0.16, 0.0, 0.05]})
assert plan["mode"] == "step"
assert 0.18 <= plan["stepLengthMeters"] <= 0.40
assert plan["frames"] == {"release": 8, "apex": 16, "plant": 24, "settled": 91}
```

- [x] Run `python -m unittest tests/test_recovery.py -v`; expect import failure.
- [x] Implement the minimum deterministic planner: normalize the horizontal event
  direction, hold below the strength threshold, otherwise select the foot on the force
  side and place its target along the force direction with a bounded lift arc.
- [x] Re-run the test and expect all planner cases to pass.

### Task 2: Active-body profile

**Files:**
- Modify: `research/embodied-species-lab/physical_profile.py`
- Modify: `research/embodied-species-lab/tests/test_physical_profile.py`

**Interfaces:**
- Change: `generate_physical_profile(body: dict, anchored_feet: bool = True) -> dict`.
- The default must preserve the champion; `anchored_feet=False` makes all parts active.

- [x] Add a failing test that requests `anchored_feet=False` and asserts no part has
  `anchored=True` while total mass and anatomical joint limits remain unchanged.
- [x] Run the focused tests and verify the new keyword is rejected.
- [x] Add the keyword and derive the foot `anchored` values from it.
- [x] Run all unit tests and both existing humanoid verifiers.

### Task 3: Blender recovery compiler

**Files:**
- Create: `research/embodied-species-lab/generate_recovery_humanoid.py`
- Create: `research/embodied-species-lab/Build-RecoveryHumanoid.ps1`
- Create: `research/embodied-species-lab/species/recovery-cases.json`

**Interfaces:**
- CLI consumes `--brief`, `--cases`, `--case-id`, and `--output`.
- Output per case: Blend, FBX, physical profile, recovery plan, four PNG diagnostics,
  and a semantic manifest.

- [x] Add four cases: left, right, forward, and backward impacts at recovery strength.
- [x] Add a failing build invocation before the compiler exists.
- [x] Reuse current armature, mesh, proxy, joint, impact, simulation, bake, reopen,
  export, and preview helpers; do not copy their implementations.
- [x] Generate a passive ground plane but create every humanoid proxy as `ACTIVE`.
- [x] Create one animated passive contact target per foot. Generate a spring constraint
  from each active foot to its target; the selected target follows the planner's
  release/apex/plant path while the other remains planted.
- [x] Generate a kinematic pelvis target over the final support center and connect it
  with bounded linear and angular springs so the active torso can regain balance.
- [x] Simulate through the settled frame, bake proxy transforms to
  `A_GeneratedRecovery_<case-id>`, reopen, export, and render rest/impact/step/settled.
- [x] Record active-part count, step foot, target, actual foot travel, lowest body point,
  final motion, final center-of-mass projection, action key count, and source hashes.

### Task 4: Independent challenge verifier

**Files:**
- Create: `research/embodied-species-lab/Test-RecoveryMachine.ps1`

**Interfaces:**
- `./Test-RecoveryMachine.ps1` is the one-command challenger verifier.

- [x] Run all Python unit tests.
- [x] Build every recovery case twice into clean derived case directories.
- [x] Compare semantic manifests byte-for-byte across repeated runs.
- [x] Assert zero passive humanoid parts, the expected stepping foot and direction,
  bounded step travel, no body-floor penetration beyond tolerance, final center of mass
  inside the expanded support region, settled final motion, persisted baked action,
  non-empty FBX, and four non-empty diagnostics.
- [x] Preserve every failing case's second-run files so the renders remain inspectable.
- [x] Keep the anchored physical build as champion unless every recovery case passes.

### Task 5: Closeout

**Files:**
- Modify: `research/embodied-species-lab/README.md`
- Modify: `.agents/workstreams/art.md`

- [x] Document the command, guarantees, generated controls, and deliberate limits.
- [x] Record the challenger evidence without requesting Unreal or registry integration.
- [x] Run the champion and challenger verifiers fresh, then run `git diff --check`.
- [x] Leave all source changes uncommitted for human review.
