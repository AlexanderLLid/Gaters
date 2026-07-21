# FullBodyIK and Mechanical Motion Wave Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Generate the required in-place mechanical locomotion clips, import them onto the
generated humanoid, configure a real UE 5.8 FullBodyIK solver, and independently prove
that displaced foot goals change the evaluated native pose.

**Architecture:** Keep Blender as the offline source compiler and extend the existing
editor-only intake adapter through its Python orchestration. Generate one FBX per named
clip, import every clip onto the already-derived Skeleton, configure one FullBodyIK solver
in the IK Rig, then evaluate the saved cooked assets in a separate native automation
process using `FIKRigProcessor`. Extend the standalone Python policy to judge measured
clip and solver evidence. CharacterMovement is the next plan after this gate passes.

**Tech Stack:** Python 3 `unittest`, Blender 5.2, PowerShell 7, Unreal Engine 5.8 Python,
IKRig/FullBodyIK, `FIKRigProcessor`, Unreal Automation Tests, JSON evidence.

## Global Constraints

- Work only under `research/embodied-species-lab/`, its owned status/exchange files, and
  derived CharacterLab outputs.
- Do not modify shared Unreal source, `Prototype.uproject`, or `research/machines.json`.
- Do not branch, commit, or push.
- Before RunUAT, UnrealBuildTool, or `UnrealEditor-Cmd`, check for a running interactive
  Unreal Editor and ask the human to close it if found.
- Preserve the passing Physics Asset topology adapter, six native topology tests,
  malformed-profile rejection, and two-run intake policy.
- Use native UE 5.8 IKRig/FullBodyIK. Do not build a solver, movement stack, Animation
  Blueprint, Foot Placement node, Motion Warping path, or runtime plugin in this wave.
- Every clip is mechanical and in-place. The capsule will own motion in the later
  CharacterMovement wave.
- This wave advances `CHAR-1` and obeys `CHAR-2`; it does not claim full `CHAR-1`,
  `CHAR-3`, `ART-1`, or `ART-2` completion.

---

## File map

- `locomotion.py` — deterministic authoritative mechanical clip recipes.
- `tests/test_locomotion.py` — pure recipe and in-place contract tests.
- `generate_locomotion_humanoid.py` — Blender compiler for a source Blend and one FBX
  per clip.
- `Build-LocomotionHumanoid.ps1` and `Test-LocomotionMachine.ps1` — guarded two-build
  source champion.
- `species/ik-evaluation.json` — versioned goal offset and acceptance tolerances.
- `unreal/import_generated_humanoid.py` — import clips and configure FullBodyIK.
- `unreal_intake_policy.py` and `tests/test_unreal_intake_policy.py` — independent clip,
  solver, and evaluated-pose rules.
- `unreal/CharacterPhysicsProfileAdapter/.../CharacterEvaluatedIKTests.cpp` — native
  `FIKRigProcessor` evaluator; it is not part of the six pure topology tests.
- `Test-UnrealHumanoidIntake.ps1` — run evaluated IK after each import and compare evidence.

---

### Task 1: Define deterministic in-place mechanical clips

**Files:**

- Create: `research/embodied-species-lab/locomotion.py`
- Create: `research/embodied-species-lab/tests/test_locomotion.py`
- Create: `research/embodied-species-lab/species/ik-evaluation.json`

**Interfaces:**

- Produces: `LOCOMOTION_VERSION = 1`, `REQUIRED_CLIP_NAMES`, and
  `synthesize_locomotion_clips(fps: int = 30) -> list[dict]`.
- Every clip record contains `name`, `fps`, `startFrame`, `endFrame`, `looping`, and
  ordered `keyframes`. Every keyframe contains `frame`, zero `rootLocationMeters`, zero
  `rootEulerDegrees`, and `boneEulerDegrees`.

- [ ] **Step 1: Write the failing pure tests**

```python
import unittest

from locomotion import REQUIRED_CLIP_NAMES, synthesize_locomotion_clips


class LocomotionRecipeTests(unittest.TestCase):
    def test_required_clips_are_unique_and_complete(self):
        clips = synthesize_locomotion_clips()
        self.assertEqual(tuple(clip["name"] for clip in clips), REQUIRED_CLIP_NAMES)
        self.assertEqual(len({clip["name"] for clip in clips}), len(REQUIRED_CLIP_NAMES))

    def test_every_clip_is_ordered_and_in_place(self):
        for clip in synthesize_locomotion_clips():
            frames = [key["frame"] for key in clip["keyframes"]]
            self.assertEqual(frames, sorted(frames))
            self.assertEqual(frames[0], clip["startFrame"])
            self.assertEqual(frames[-1], clip["endFrame"])
            self.assertTrue(all(key["rootLocationMeters"] == [0.0, 0.0, 0.0]
                                for key in clip["keyframes"]))
            self.assertTrue(all(key["rootEulerDegrees"] == [0.0, 0.0, 0.0]
                                for key in clip["keyframes"]))

    def test_non_idle_clips_change_at_least_one_pose_bone(self):
        for clip in synthesize_locomotion_clips()[1:]:
            poses = [key["boneEulerDegrees"] for key in clip["keyframes"]]
            self.assertTrue(any(pose != poses[0] for pose in poses[1:]), clip["name"])
```

- [ ] **Step 2: Run the focused test and verify RED**

```powershell
& 'C:\Users\alexa\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe' `
  -m unittest discover -s research/embodied-species-lab/tests -p test_locomotion.py -v
```

Expected: import failure because `locomotion.py` does not exist.

- [ ] **Step 3: Implement the smallest recipe generator**

Use these exact stable names and poses; all world motion remains zero:

```python
LOCOMOTION_VERSION = 1
REQUIRED_CLIP_NAMES = (
    "A_Idle", "A_TurnLeft", "A_Walk", "A_Run",
    "A_Stop", "A_Jump", "A_Fall", "A_Land",
)
ANIMATED_BONES = (
    "pelvis", "chest", "upper_arm_l", "upper_arm_r",
    "thigh_l", "thigh_r", "shin_l", "shin_r",
)


def _key(frame, **bones):
    pose = {name: [0.0, 0.0, 0.0] for name in ANIMATED_BONES}
    pose.update({name: list(value) for name, value in bones.items()})
    return {
        "frame": frame,
        "rootLocationMeters": [0.0, 0.0, 0.0],
        "rootEulerDegrees": [0.0, 0.0, 0.0],
        "boneEulerDegrees": {name: pose[name] for name in sorted(pose)},
    }


def _clip(name, end_frame, keys, looping=False, fps=30):
    return {
        "name": name,
        "fps": fps,
        "startFrame": 1,
        "endFrame": end_frame,
        "looping": looping,
        "keyframes": keys,
    }


def synthesize_locomotion_clips(fps=30):
    neutral = _key(1)
    return [
        _clip("A_Idle", 31, [neutral, _key(16, chest=(2, 0, 0)), _key(31)], True, fps),
        _clip("A_TurnLeft", 21, [neutral, _key(11, pelvis=(0, 0, 12), chest=(0, 0, -6)), _key(21)], False, fps),
        _clip("A_Walk", 33, [
            _key(1, thigh_l=(25, 0, 0), thigh_r=(-25, 0, 0)),
            _key(9, thigh_l=(0, 0, 0), thigh_r=(0, 0, 0)),
            _key(17, thigh_l=(-25, 0, 0), thigh_r=(25, 0, 0)),
            _key(25, thigh_l=(0, 0, 0), thigh_r=(0, 0, 0)),
            _key(33, thigh_l=(25, 0, 0), thigh_r=(-25, 0, 0)),
        ], True, fps),
        _clip("A_Run", 25, [
            _key(1, thigh_l=(40, 0, 0), thigh_r=(-40, 0, 0), chest=(8, 0, 0)),
            _key(7),
            _key(13, thigh_l=(-40, 0, 0), thigh_r=(40, 0, 0), chest=(8, 0, 0)),
            _key(19),
            _key(25, thigh_l=(40, 0, 0), thigh_r=(-40, 0, 0), chest=(8, 0, 0)),
        ], True, fps),
        _clip("A_Stop", 16, [_key(1, thigh_l=(20, 0, 0), thigh_r=(-20, 0, 0)), _key(8, thigh_l=(6, 0, 0), thigh_r=(-6, 0, 0)), _key(16)], False, fps),
        _clip("A_Jump", 19, [_key(1), _key(7, thigh_l=(24, 0, 0), thigh_r=(24, 0, 0), shin_l=(-35, 0, 0), shin_r=(-35, 0, 0)), _key(13, upper_arm_l=(-25, 0, 0), upper_arm_r=(-25, 0, 0)), _key(19)], False, fps),
        _clip("A_Fall", 19, [_key(1), _key(10, upper_arm_l=(0, 0, 35), upper_arm_r=(0, 0, -35), chest=(-8, 0, 0)), _key(19, upper_arm_l=(0, 0, 45), upper_arm_r=(0, 0, -45), chest=(-12, 0, 0))], True, fps),
        _clip("A_Land", 19, [_key(1, thigh_l=(28, 0, 0), thigh_r=(28, 0, 0), shin_l=(-40, 0, 0), shin_r=(-40, 0, 0)), _key(10, thigh_l=(10, 0, 0), thigh_r=(10, 0, 0), shin_l=(-15, 0, 0), shin_r=(-15, 0, 0)), _key(19)], False, fps),
    ]
```

Create the evaluator contract as data:

```json
{
  "schemaVersion": 1,
  "goalOffsetCentimeters": 10.0,
  "maximumRootTranslationCentimeters": 0.1,
  "minimumDrivenDisplacementCentimeters": 5.0,
  "maximumGoalErrorCentimeters": 2.0,
  "maximumOppositeFootDisplacementCentimeters": 2.0
}
```

- [ ] **Step 4: Run the focused and complete unit suites to verify GREEN**

Expected: the new tests and all existing lab tests pass.

---

### Task 2: Compile and verify Blender motion artifacts

**Files:**

- Create: `research/embodied-species-lab/generate_locomotion_humanoid.py`
- Create: `research/embodied-species-lab/Build-LocomotionHumanoid.ps1`
- Create: `research/embodied-species-lab/Test-LocomotionMachine.ps1`

**Interfaces:**

- Consumes: `species/humanoid.json` body data and
  `synthesize_locomotion_clips()`.
- Produces: ignored `Derived/humanoid-locomotion-v1/`, one source Blend,
  `locomotion-manifest.json`, and one non-authoritative FBX per clip.

- [ ] **Step 1: Write the failing public harness**

The harness must run the unit suite, call `Build-LocomotionHumanoid.ps1` twice, require
the source Blend and all eight FBXs, and compare both manifest texts byte-for-byte.

```powershell
$RequiredNames = @('A_Idle','A_TurnLeft','A_Walk','A_Run','A_Stop','A_Jump','A_Fall','A_Land')
& $Builder -Blender $Blender
if ($LASTEXITCODE -ne 0) { throw "Locomotion build failed with exit code $LASTEXITCODE." }
$First = Get-Content -Raw -LiteralPath $Manifest
& $Builder -Blender $Blender
if ($LASTEXITCODE -ne 0) { throw "Repeated locomotion build failed with exit code $LASTEXITCODE." }
$Second = Get-Content -Raw -LiteralPath $Manifest
if ($First -cne $Second) { throw 'Repeated locomotion manifests differ.' }
$Data = $Second | ConvertFrom-Json
if (($Data.clips.name -join ',') -cne ($RequiredNames -join ',')) {
    throw 'Locomotion manifest clip order differs from the contract.'
}
foreach ($Clip in $Data.clips) {
    if (-not (Test-Path -LiteralPath (Join-Path $Output $Clip.fbxFile) -PathType Leaf)) {
        throw "Missing locomotion FBX: $($Clip.fbxFile)"
    }
}
```

- [ ] **Step 2: Run the harness and verify RED**

Expected: failure because `Build-LocomotionHumanoid.ps1` does not exist.

- [ ] **Step 3: Implement the Blender compiler**

Reuse `humanoid_bones`, `make_armature`, `make_weighted_mesh`, `rounded_vector`, and
`export_fbx` from `generate_humanoid.py`. Do not duplicate body construction.

```python
def make_action(armature, clip):
    action = bpy.data.actions.new(clip["name"])
    action.use_fake_user = True
    armature.animation_data_create()
    armature.animation_data.action = action
    for key in clip["keyframes"]:
        root = armature.pose.bones["root"]
        root.location = Vector(key["rootLocationMeters"])
        root.rotation_mode = "XYZ"
        root.rotation_euler = Euler(tuple(math.radians(v) for v in key["rootEulerDegrees"]), "XYZ")
        root.keyframe_insert("location", frame=key["frame"], group="root")
        root.keyframe_insert("rotation_euler", frame=key["frame"], group="root")
        for name, degrees in key["boneEulerDegrees"].items():
            bone = armature.pose.bones[name]
            bone.rotation_mode = "XYZ"
            bone.rotation_euler = Euler(tuple(math.radians(v) for v in degrees), "XYZ")
            bone.keyframe_insert("rotation_euler", frame=key["frame"], group=name)
    return action


def export_clip(armature, mesh, clip, action, output):
    armature.animation_data.action = action
    bpy.context.scene.frame_start = clip["startFrame"]
    bpy.context.scene.frame_end = clip["endFrame"]
    export_fbx(armature, mesh, output / f"{clip['name']}.fbx")
```

Before writing the manifest, reopen the Blend and require:

- exactly the eight required action names;
- each action frame range matches its clip;
- every root location and root rotation curve evaluates to zero at every authored key;
- skeleton hierarchy and full rigid weights still match the humanoid recipe.

Manifest clip records contain `name`, `fps`, `startFrame`, `endFrame`, `durationSeconds`,
`looping`, and `fbxFile`. Record source hashes and Blender version; record FBX byte
nondeterminism honestly and do not hash FBXs.

- [ ] **Step 4: Implement the guarded PowerShell builder**

Require the Blender executable, generator, brief, and output to exist or resolve under
the lab. The builder invokes Blender background mode with the new generator and returns
nonzero on any validation failure.

- [ ] **Step 5: Run the public locomotion harness twice to verify GREEN**

Expected: `PASS locomotion-machine clips=8`, with byte-identical manifests.

---

### Task 3: Add independent motion and solver policy rules

**Files:**

- Modify: `research/embodied-species-lab/tests/test_unreal_intake_policy.py`
- Modify: `research/embodied-species-lab/unreal_intake_policy.py`

**Interfaces:**

- Consumes: intake report schema `3`, locomotion manifest, evaluated-IK evidence, and
  the IK evaluation contract through
  `evaluate_intake(manifest, profile, locomotion_manifest, report, ik_evidence,
  ik_contract, manifest_sha256, profile_sha256, locomotion_sha256)`.
- Produces: rules `intake.motion.clips`, `intake.motion.in_place`,
  `intake.ik.solver`, and `intake.ik.evaluated_pose`.

- [ ] **Step 1: Extend fixtures and write failing policy tests**

The valid fixture must contain eight animation records, one solver record, and left/right
evaluation records. Add one focused counterexample for each new rule:

```python
def evaluate(self, report, evidence=None):
    return evaluate_intake(
        copy.deepcopy(MANIFEST),
        copy.deepcopy(PROFILE),
        copy.deepcopy(LOCOMOTION_MANIFEST),
        report,
        copy.deepcopy(IK_EVIDENCE if evidence is None else evidence),
        copy.deepcopy(IK_CONTRACT),
        "manifest-hash",
        "profile-hash",
        "locomotion-hash",
    )


def test_missing_required_clip_fails(self):
    report = copy.deepcopy(REPORT)
    report["animations"] = report["animations"][:-1]
    result = self.evaluate(report)
    self.assertIn("intake.motion.clips", [issue["ruleId"] for issue in result["issues"]])


def test_root_translation_fails_in_place_rule(self):
    report = copy.deepcopy(REPORT)
    report["animations"][0]["rootTranslationCentimeters"][1] = [1.0, 0.0, 0.0]
    result = self.evaluate(report)
    self.assertIn("intake.motion.in_place", [issue["ruleId"] for issue in result["issues"]])


def test_missing_full_body_solver_fails(self):
    report = copy.deepcopy(REPORT)
    report["ikRig"]["solvers"] = []
    result = self.evaluate(report)
    self.assertIn("intake.ik.solver", [issue["ruleId"] for issue in result["issues"]])


def test_unmoved_driven_foot_fails_evaluated_pose(self):
    evidence = copy.deepcopy(IK_EVIDENCE)
    evidence["feet"]["left"]["drivenDisplacementCentimeters"] = 0.0
    result = self.evaluate(copy.deepcopy(REPORT), evidence)
    self.assertIn("intake.ik.evaluated_pose", [issue["ruleId"] for issue in result["issues"]])
```

- [ ] **Step 2: Run the focused policy suite and verify RED**

Expected: all four new tests fail because the rules do not exist.

- [ ] **Step 3: Implement minimal independent rules**

- Require exact ordered clip names from `REQUIRED_CLIP_NAMES` copied into the policy as
  the downstream contract.
- Require every sampled root translation component to remain within
  `ik_contract["maximumRootTranslationCentimeters"]`.
- Require exactly one enabled solver, `controllerClass == "IKRigFBIKController"`, start
  bone `pelvis`, and both goal connections.
- Load `species/ik-evaluation.json` in the CLI and compare native evidence against its
  minimum driven displacement, maximum goal error, and maximum opposite-foot movement.
- Increment `EVALUATOR_VERSION`; require report schema/importer version `3`.
- Extend the CLI with required `--locomotion-manifest`, `--ik-evidence`, and
  `--ik-contract` paths. Include the locomotion-manifest SHA-256 in the intake report and
  require it to match the evaluator input.

- [ ] **Step 4: Run the focused policy suite and verify GREEN**

Expected: every policy test passes.

---

### Task 4: Import and measure all mechanical clips

**Files:**

- Modify: `research/embodied-species-lab/unreal/import_generated_humanoid.py`
- Modify: `research/embodied-species-lab/Test-UnrealHumanoidIntake.ps1`

**Interfaces:**

- Consumes: `GATERS_LOCOMOTION_MANIFEST` and the existing generated Skeleton.
- Produces: eight saved `AnimSequence` assets and report `animations` under
  schema/importer version `3`. Solver rules deliberately remain RED for Task 5.

- [ ] **Step 1: Wire the new inputs and run integration RED**

Call `Test-LocomotionMachine.ps1` from the public intake, require its manifest, set
`GATERS_LOCOMOTION_MANIFEST`, and pass it to Unreal. Do not configure a solver yet.

Run the public intake. Expected: policy failure for `intake.motion.clips` and
`intake.ik.solver` because the importer still reports neither.

- [ ] **Step 2: Import all animations onto the existing Skeleton**

For each manifest clip, use `FbxImportUI` with `import_mesh=False`,
`import_animations=True`, `FBXIT_ANIMATION`, the derived Skeleton, scene-unit conversion,
and `import_uniform_scale=100.0`. Delete only the matching derived animation before
replacement.

Read back each `AnimSequence` and record:

```python
{
    "name": clip["name"],
    "asset": animation.get_path_name(),
    "durationSeconds": round(float(animation.get_editor_property("sequence_length")), 6),
    "sampledKeys": int(animation.get_editor_property("number_of_sampled_keys")),
    "rootTranslationCentimeters": [
        vector_record(unreal.AnimationLibrary.get_bone_pose_for_frame(
            animation, "root", frame, False).translation)
        for frame in (0,
                      int(animation.get_editor_property("number_of_sampled_keys")) // 2,
                      int(animation.get_editor_property("number_of_sampled_keys")) - 1)
    ],
}
```

Require exactly one imported `AnimSequence` at each expected path and save it explicitly.

- [ ] **Step 3: Run import and policy tests**

Expected interim result: clip and in-place rules pass. Solver and evaluated-pose rules
remain RED because no solver exists and no native evidence has run.

---

### Task 5: Prove actual FullBodyIK pose evaluation natively

**Files:**

- Modify: `research/embodied-species-lab/unreal/CharacterPhysicsProfileAdapter/Source/CharacterPhysicsProfileAdapter/CharacterPhysicsProfileAdapter.Build.cs`
- Create: `research/embodied-species-lab/unreal/CharacterPhysicsProfileAdapter/Source/CharacterPhysicsProfileAdapter/Private/Tests/CharacterEvaluatedIKTests.cpp`
- Modify: `research/embodied-species-lab/unreal/import_generated_humanoid.py`
- Modify: `research/embodied-species-lab/Test-UnrealHumanoidIntake.ps1`

**Interfaces:**

- Consumes: saved `/Game/Gaters/Generated/CharacterLab/SK_GeneratedHumanoid`,
  `IK_GeneratedHumanoid`, `GATERS_IK_EVIDENCE`, and `GATERS_IK_CONTRACT`.
- Produces: `Gaters.CharacterLab.EvaluatedIK.GeneratedHumanoid` and stable JSON evidence
  for left/right foot challenges.

- [ ] **Step 1: Add the failing native evaluator**

Add private module dependencies `IKRig` and `Json`. The automation test loads both assets,
initializes `FIKRigProcessor`, calls `SetInputPoseToRefPose`, and captures
`Processor.GetSkeleton().RefPoseGlobal`.

For each side, set both component-space goals so the challenged foot is offset upward by
the contract amount and the other remains at its reference transform:

```cpp
FIKRigGoal MakeGoal(FName GoalName, FName BoneName, const FTransform& Transform)
{
    return FIKRigGoal(
        GoalName,
        BoneName,
        Transform.GetTranslation(),
        Transform.GetRotation(),
        1.0f,
        1.0f,
        EIKRigGoalSpace::Component,
        EIKRigGoalSpace::Component,
        true);
}
```

Call `Solve`, get output global transforms, and calculate:

- challenged foot displacement from reference;
- challenged foot distance to the requested goal;
- opposite foot displacement from reference.

Assert all three against values loaded from `GATERS_IK_CONTRACT`. Write deterministic
JSON with schema version, asset paths, goal offset, and left/right measurements to the
validated `GATERS_IK_EVIDENCE` path.

```json
{
  "schemaVersion": 1,
  "skeletalMesh": "/Game/Gaters/Generated/CharacterLab/SK_GeneratedHumanoid.SK_GeneratedHumanoid",
  "ikRig": "/Game/Gaters/Generated/CharacterLab/IK_GeneratedHumanoid.IK_GeneratedHumanoid",
  "solverCount": 1,
  "goalOffsetCentimeters": 10.0,
  "feet": {
    "left": {
      "drivenDisplacementCentimeters": 10.0,
      "goalErrorCentimeters": 0.0,
      "oppositeFootDisplacementCentimeters": 0.0
    },
    "right": {
      "drivenDisplacementCentimeters": 10.0,
      "goalErrorCentimeters": 0.0,
      "oppositeFootDisplacementCentimeters": 0.0
    }
  }
}
```

Values above define the schema shape; the native evaluator writes measured rounded values,
not copied expectations.

- [ ] **Step 2: Package, import without solver configuration, and verify RED**

Run the evaluator against the pre-solver imported IK Rig from Task 4. Expected: the test
fails because the challenged foot does not reach the goal. Confirm the failure is the
pose-response assertion, not asset loading or JSON parsing.

- [ ] **Step 3: Configure one FullBodyIK solver through the native controller**

After creating both goals in `create_ik_rig`:

```python
solver_index = int(controller.add_solver("/Script/IKRig.FullBodyIKSolver"))
if solver_index != 0:
    raise RuntimeError(f"Expected first FullBodyIK solver at index 0, got {solver_index}")
if not controller.set_start_bone("pelvis", solver_index):
    raise RuntimeError("Could not set FullBodyIK pelvis root")
for goal_name in ("foot_l_goal", "foot_r_goal"):
    if not controller.connect_goal_to_solver(goal_name, solver_index):
        raise RuntimeError(f"Could not connect {goal_name} to FullBodyIK")
solver_controller = controller.get_solver_controller(solver_index)
```

Read back report `ikRig.solvers` using `get_num_solvers`, `get_solver_enabled`,
`get_start_bone`, `is_goal_connected_to_solver`, and
`solver_controller.get_class().get_name()`.

- [ ] **Step 4: Run the native evaluator and verify GREEN**

Run one import followed by:

```powershell
$env:GATERS_IK_EVIDENCE = $Evidence
$env:GATERS_IK_CONTRACT = $IkContract
& $UnrealEditor $Project '-unattended' '-nop4' '-nosplash' '-nullrhi' '-nosound' `
  "-PLUGIN=$PackagedPlugin" `
  '-ExecCmds=Automation RunTests Gaters.CharacterLab.EvaluatedIK;Quit' `
  '-TestExit=Automation Test Queue Empty' "-abslog=$IkLog"
```

Expected: one native evaluated-IK test passes; solver and evaluated-pose policy rules pass.

- [ ] **Step 5: Run evaluated IK after both imports**

Produce `evaluated-ik-1.json` and `evaluated-ik-2.json`; compare them byte-for-byte. Also
compare their policy outputs. In each intake iteration, run native IK evaluation before
calling `unreal_intake_policy.py`, then pass the iteration's evidence plus locomotion
manifest and IK contract to the policy. The six pure topology tests remain counted
separately.

---

### Task 6: Full regression and evidence-backed closeout

**Files:**

- Modify after passing evidence: `research/embodied-species-lab/README.md`
- Modify after passing evidence: `.agents/workstreams/Character Generation & Animation.md`
- Create after passing evidence: `.agents/exchanges/CHAR-3-fullbodyik-motion-handoff.md`

**Interfaces:**

- Produces: the passing wave-three evidence set and the next wave-four
  CharacterMovement objective.

- [ ] **Step 1: Run fresh complete verification**

```powershell
& 'C:\Users\alexa\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe' `
  -m unittest discover -s research/embodied-species-lab/tests -v
./research/embodied-species-lab/Test-HumanoidMachine.ps1
./research/embodied-species-lab/Test-PhysicalHumanoid.ps1
./research/embodied-species-lab/Test-RecoveryMachine.ps1
./research/embodied-species-lab/Test-LocomotionMachine.ps1
./research/embodied-species-lab/Test-UnrealHumanoidIntake.ps1
```

Expected: every command exits `0`; valid Unreal logs contain no Python, Interchange,
ensure, or fatal markers; native topology count remains six; evaluated IK count is one;
two intake, policy, and evaluated-IK evidence files are byte-identical.

- [ ] **Step 2: Audit requirements and ownership**

- `CHAR-1`: claim clips and evaluated IK only; CharacterMovement and later gates remain
  open.
- `CHAR-2`: runtime evaluation consumes cooked assets; editor adapter stays editor-only.
- `CHAR-3`: remains pending.
- `ART-1`, `ART-2`: no art claim or art file change.
- Confirm no shared Unreal source, `Prototype.uproject`, `research/machines.json`,
  quadruped, species-aesthetic, lineage, evolution, or runtime-generation change.

- [ ] **Step 3: Record measured status and prepare the handoff**

Update the lab README and owned workstream with exact measured clip/solver/evaluation
results. Create one `INTEGRATE` packet for Primary Builder that preserves the accepted
editor/runtime boundary and requests no machine promotion unless the evidence justifies
one independently.

- [ ] **Step 4: Verify coordination documents**

```powershell
./research/Test-SharedAgentDocs.ps1
git diff --check
```

Expected: both commands pass. Do not commit.

Requirements checked: Global none recorded; `ART-1`, `ART-2`, `CHAR-1`, `CHAR-2`,
`CHAR-3`, generated-content boundary. Exceptions: none.
