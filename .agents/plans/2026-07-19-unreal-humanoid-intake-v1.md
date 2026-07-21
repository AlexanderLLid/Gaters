# Unreal Humanoid Intake v1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Import the generated recovery humanoid into UE 5.8 twice, derive native Physics Asset and IK Rig assets, and independently report whether native derivation satisfies the source body contract.

**Architecture:** A pure standard-library policy evaluates source manifests against an Unreal-native report. A small Unreal Python adapter borrows the existing FBX import pattern plus native SkeletalMeshEditorSubsystem and IK Rig scripting APIs. A PowerShell harness rebuilds one source case, clears only ignored generated outputs, runs two imports, compares stable reports, and preserves any falsifying evidence.

**Tech Stack:** Python standard library, Unreal Engine 5.8 Python API, PowerShell, Blender 5.2 source fixture.

## Global Constraints

- Work only in `research/embodied-species-lab/`, ignored `Unreal/Prototype/Content/Gaters/Generated/CharacterLab/`, ignored `Unreal/Prototype/Saved/CharacterLab/`, this plan, and the owned workstream status.
- Do not modify shared Unreal source, `Prototype.uproject`, `research/machines.json`, branches, commits, or remotes.
- Preserve `Test-HumanoidMachine.ps1`, `Test-PhysicalHumanoid.ps1`, and `Test-RecoveryMachine.ps1` as champions.
- Use native UE Physics Asset and IK Rig APIs before custom implementations.
- The policy is independent of the Unreal adapter and must reject isolated counterexamples.
- A failed native guarantee is a valid result only when the report and next isolated experiment are preserved.
- Requirements checked: Global none recorded; generated content boundary. Exceptions: none.

---

### Task 1: Independent intake policy

**Files:**
- Create: `research/embodied-species-lab/tests/test_unreal_intake_policy.py`
- Create: `research/embodied-species-lab/unreal_intake_policy.py`

**Interfaces:**
- Consumes: `evaluate_intake(manifest: dict, profile: dict, report: dict, manifest_sha256: str, profile_sha256: str) -> dict`
- Produces: schema-1 evaluation with `passed`, `checks`, `issues`, `expectedPhysics`, and `measuredPhysics`.

- [ ] **Step 1: Write failing positive and counterexample tests**

Create fixtures inline in `test_unreal_intake_policy.py`. The positive report must contain:

```python
REPORT = {
    "schemaVersion": 1,
    "importerVersion": 1,
    "manifestSha256": "manifest-hash",
    "profileSha256": "profile-hash",
    "boneNames": ["root", "pelvis", "spine", "chest", "neck", "head",
                  "upper_arm_l", "lower_arm_l", "upper_arm_r", "lower_arm_r",
                  "thigh_l", "shin_l", "foot_l", "thigh_r", "shin_r", "foot_r"],
    "meshBoundsSizeCentimeters": [148.0, 48.0, 180.0],
    "physicsAsset": {
        "compatible": True,
        "bodyBones": [part["bone"] for part in PROFILE["parts"]],
        "constraints": [{"parent": joint["parent"], "child": joint["child"]}
                        for joint in PROFILE["joints"]],
    },
    "ikRig": {
        "compatible": True,
        "retargetRoot": "pelvis",
        "goals": {"foot_l_goal": "foot_l", "foot_r_goal": "foot_r"},
        "chains": {
            "leg_l": {"start": "thigh_l", "end": "foot_l", "goal": "foot_l_goal"},
            "leg_r": {"start": "thigh_r", "end": "foot_r", "goal": "foot_r_goal"},
        },
    },
}
```

Tests must assert the positive passes, deleting `foot_l` from `bodyBones` yields
`intake.physics.body_coverage`, and deleting `foot_r_goal` yields `intake.ik.goals`.
Call the policy with `"manifest-hash"` and `"profile-hash"`; the positive report above
must match them.

- [ ] **Step 2: Run the focused test and verify RED**

Run:

```powershell
python -m unittest research/embodied-species-lab/tests/test_unreal_intake_policy.py -v
```

Expected: import error because `unreal_intake_policy.py` does not exist.

- [ ] **Step 3: Implement the minimum policy**

Implement `evaluate_intake` with set equality for body bones and constraint pairs, exact
hierarchy order, bounded height difference, and exact humanoid IK mappings. Add issues
through one local helper:

```python
def issue(issues, rule_id, message):
    issues.append({"ruleId": rule_id, "message": message})
```

Do not add classes, dependencies, configuration objects, or repair logic.
Add a CLI that accepts `--manifest`, `--profile`, `--report`, and `--output`, computes the
two source hashes, writes sorted JSON, and exits `0` on pass or `2` on policy failure.

- [ ] **Step 4: Run focused and full pure tests and verify GREEN**

Run:

```powershell
python -m unittest research/embodied-species-lab/tests/test_unreal_intake_policy.py -v
python -m unittest discover research/embodied-species-lab/tests -v
```

Expected: all tests pass with no warnings.

---

### Task 2: Native Unreal intake adapter

**Files:**
- Create: `research/embodied-species-lab/unreal/import_generated_humanoid.py`
- Create: `research/embodied-species-lab/Test-UnrealHumanoidIntake.ps1`

**Interfaces:**
- Consumes environment variables `GATERS_CHARACTER_MANIFEST`, `GATERS_CHARACTER_PROFILE`, `GATERS_CHARACTER_DESTINATION`, `GATERS_CHARACTER_REPORT`.
- Produces one Skeletal Mesh, Skeleton, Physics Asset, IK Rig, and schema-1 JSON report.

- [ ] **Step 1: Add a black-box invocation before the adapter exists**

Create the harness with path validation, the `impact-forward` source build, bounded
generated-directory cleanup, environment variables, and one UnrealEditor-Cmd invocation.
Run it before creating the adapter. The correct RED is path validation naming
`import_generated_humanoid.py` as missing; no production adapter code exists yet.

- [ ] **Step 2: Implement source validation and skeletal import**

Reuse the established `FbxImportUI` settings from
`Unreal/Prototype/Scripts/ImportMotionFixture.py`: automated skeletal import, scene-unit
conversion, no materials/textures/animations, no automatic Physics Asset. Require the
destination to start with `/Game/Gaters/Generated/CharacterLab`.

- [ ] **Step 3: Borrow native Physics Asset derivation**

Call:

```python
subsystem = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
physics_asset = subsystem.create_physics_asset(skeletal_mesh, True, 0)
```

Report compatibility, body setup object names, and each constraint's parent/child bone
from `default_instance`. Do not repair missing bodies or constraints in v1.

- [ ] **Step 4: Build the minimum IK Rig**

Create `IK_GeneratedHumanoid` with `IKRigDefinitionFactory`, bind the Skeletal Mesh with
`IKRigController`, set pelvis as retarget root, add `foot_l_goal` and `foot_r_goal`, then
add `leg_l` and `leg_r` chains from thigh to foot. Report the values read back through
the controller; do not add Control Rig, FullBodyIK solver tuning, or runtime animation.

- [ ] **Step 5: Write and save the native report**

Record source hashes, engine/importer versions, asset paths/classes, reference hierarchy,
bounds, Physics Asset facts, IK facts, and import settings. Save all derived assets and
write sorted JSON plus a terminal success marker.

- [ ] **Step 6: Run the adapter once and classify the result**

Expected outcomes:

- PASS: native derivation covers all fourteen physical parts, thirteen source joint
  pairs, and both leg IK mappings.
- FALSIFIED: import/IK succeeds but native Physics Asset coverage differs. Preserve the
  report; the next experiment becomes a physical-profile-to-PhysicsAsset adapter.

Any API error is a bug in the adapter, not a falsified machine guarantee; diagnose and
fix it with a reproducing test or black-box assertion.

---

### Task 3: Repeatable one-command challenger

**Files:**
- Modify: `research/embodied-species-lab/Test-UnrealHumanoidIntake.ps1`
- Modify: `research/embodied-species-lab/README.md`
- Modify: `.agents/workstreams/Character Generation & Animation.md`

**Interfaces:**
- `./Test-UnrealHumanoidIntake.ps1` rebuilds `impact-forward`, imports twice, evaluates, and exits 0 only for a passing guarantee.

- [ ] **Step 1: Complete the safe harness**

The harness must:

- validate the UE 5.8 editor, project, source builder, importer, and policy paths;
- run `Build-RecoveryHumanoid.ps1 -CaseId impact-forward`;
- resolve and verify generated-content deletion remains inside `Unreal/Prototype`;
- clear only `Content/Gaters/Generated/CharacterLab` and `Saved/CharacterLab`;
- invoke UnrealEditor-Cmd twice with `-unattended -nop4 -nosplash -nullrhi -nosound`;
- compare stable report fields and run the pure policy after each import;
- preserve the second report and logs on failure.

- [ ] **Step 2: Run the full challenger**

Run:

```powershell
research/embodied-species-lab/Test-UnrealHumanoidIntake.ps1
```

Expected: PASS, or a preserved policy failure naming the exact missing Physics Asset or
IK contract. Adapter exceptions, missing reports, nondeterministic stable fields, and
unclassified log errors are harness failures.

- [ ] **Step 3: Run champion regression checks**

Run:

```powershell
research/embodied-species-lab/Test-HumanoidMachine.ps1
research/embodied-species-lab/Test-PhysicalHumanoid.ps1
research/embodied-species-lab/Test-RecoveryMachine.ps1
```

Expected: all champions remain green.

- [ ] **Step 4: Document only the evidence produced**

Add the command, current guarantee or falsified guarantee, report path, and next isolated
experiment to the lab README and owned status. Do not claim registry promotion.

- [ ] **Step 5: Close coordination**

If the challenger passes, create a new `INTEGRATE` exchange for Primary Builder. If it
falsifies native Physics Asset coverage, keep `CHAR-1` open and record the explicit
profile-adapter experiment instead.

No commit step: project instructions require human review before commits.
