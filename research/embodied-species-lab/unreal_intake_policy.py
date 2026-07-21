import argparse
import hashlib
import json
from pathlib import Path


SCHEMA_VERSION = 1
EVALUATOR_VERSION = 4
EXPECTED_GOALS = {"foot_l_goal": "foot_l", "foot_r_goal": "foot_r"}
EXPECTED_FOOT_PLACEMENT_ROLES = {
    "ikRoot": "root",
    "pelvis": "pelvis",
    "left": {"fkFoot": "foot_l", "ball": "ball_l", "ikFoot": "ik_foot_l"},
    "right": {"fkFoot": "foot_r", "ball": "ball_r", "ikFoot": "ik_foot_r"},
}
EXPECTED_CHAINS = {
    "leg_l": {"start": "thigh_l", "end": "foot_l", "goal": "foot_l_goal"},
    "leg_r": {"start": "thigh_r", "end": "foot_r", "goal": "foot_r_goal"},
}
REQUIRED_CLIP_NAMES = (
    "A_Idle",
    "A_TurnLeft",
    "A_Walk",
    "A_Run",
    "A_Stop",
    "A_Jump",
    "A_Fall",
    "A_Land",
)


def _issue(issues, rule_id, message):
    issues.append({"ruleId": rule_id, "message": message})


def _hierarchy(bones):
    return [
        {
            "name": bone["name"],
            "parent": bone.get("parent"),
            "children": sorted(
                child["name"] for child in bones if child.get("parent") == bone["name"]
            ),
        }
        for bone in bones
    ]


def _expected_height_centimeters(bones):
    coordinates = [
        point[2]
        for bone in bones
        for point in (bone["headMeters"], bone["tailMeters"])
    ]
    return (max(coordinates) - min(coordinates)) * 100.0


def _joint_pairs(joints):
    return {(joint["parent"], joint["child"]) for joint in joints}


def _in_place(animations, maximum_translation):
    samples = [
        component
        for animation in animations
        for translation in animation.get("rootTranslationCentimeters", [])
        for component in translation
    ]
    return bool(samples) and all(abs(float(value)) <= maximum_translation for value in samples)


def _solver_matches(solvers):
    if len(solvers) != 1:
        return False
    solver = solvers[0]
    return (
        solver.get("enabled") is True
        and solver.get("controllerClass") == "IKRigFBIKController"
        and solver.get("startBone") == "pelvis"
        and set(solver.get("connectedGoals", [])) == set(EXPECTED_GOALS)
    )


def _evaluated_pose_passes(evidence, contract):
    if evidence.get("solverCount") != 1:
        return False
    if abs(
        float(evidence.get("goalOffsetCentimeters", -1.0))
        - float(contract["goalOffsetCentimeters"])
    ) > 0.000001:
        return False
    feet = evidence.get("feet", {})
    if set(feet) != {"left", "right"}:
        return False
    return all(
        float(measurement.get("drivenDisplacementCentimeters", -1.0))
        >= float(contract["minimumDrivenDisplacementCentimeters"])
        and float(measurement.get("goalErrorCentimeters", float("inf")))
        <= float(contract["maximumGoalErrorCentimeters"])
        and float(measurement.get("oppositeFootDisplacementCentimeters", float("inf")))
        <= float(contract["maximumOppositeFootDisplacementCentimeters"])
        for measurement in feet.values()
    )


def evaluate_intake(
    manifest,
    profile,
    locomotion_manifest,
    report,
    ik_evidence,
    ik_contract,
    manifest_sha256,
    profile_sha256,
    locomotion_sha256,
):
    issues = []
    bones = manifest.get("skeleton", {}).get("bones", [])
    expected_bones = [bone.get("name") for bone in bones]
    expected_body_bones = [part.get("bone") for part in profile.get("parts", [])]
    expected_joints = profile.get("joints", [])
    physics = report.get("physicsAsset", {})
    ik_rig = report.get("ikRig", {})
    animations = report.get("animations", [])
    solvers = ik_rig.get("solvers", [])

    if (
        manifest.get("schemaVersion") != 1
        or profile.get("schemaVersion") != 1
        or profile.get("profileVersion") != 1
        or locomotion_manifest.get("schemaVersion") != 1
        or locomotion_manifest.get("locomotionVersion") != 1
        or ik_evidence.get("schemaVersion") != 1
        or ik_contract.get("schemaVersion") != 1
        or report.get("schemaVersion") != 3
        or report.get("importerVersion") != 3
        or not manifest.get("validation", {}).get("passed", False)
        or not locomotion_manifest.get("validation", {}).get("passed", False)
    ):
        _issue(issues, "intake.input.version", "Unsupported or unvalidated intake input.")

    if (
        report.get("manifestSha256") != manifest_sha256
        or report.get("profileSha256") != profile_sha256
        or report.get("locomotionManifestSha256") != locomotion_sha256
    ):
        _issue(issues, "intake.input.provenance", "Report input hashes do not match.")

    if (
        report.get("boneNames") != expected_bones
        or report.get("referenceHierarchy") != _hierarchy(bones)
    ):
        _issue(issues, "intake.skeleton.hierarchy", "Imported hierarchy differs from the manifest.")

    if report.get("footPlacementRoles") != EXPECTED_FOOT_PLACEMENT_ROLES:
        _issue(
            issues,
            "intake.foot_placement.roles",
            "Imported skeleton does not expose the exact Foot Placement role contract.",
        )

    bounds = report.get("meshBoundsSizeCentimeters", [])
    expected_height = _expected_height_centimeters(bones) if bones else 0.0
    if len(bounds) != 3 or abs(float(bounds[2]) - expected_height) > 1.0:
        _issue(issues, "intake.mesh.scale", "Imported mesh height differs by more than 1 cm.")

    if not physics.get("compatible", False):
        _issue(issues, "intake.physics.compatibility", "Unreal did not create a compatible Physics Asset.")

    if (
        report.get("adapterVersion") != 1
        or physics.get("derivation") != "profile-topology-v1"
    ):
        _issue(
            issues,
            "intake.physics.adapter",
            "Physics Asset topology was not produced by profile adapter v1.",
        )

    if set(physics.get("bodyBones", [])) != set(expected_body_bones):
        _issue(issues, "intake.physics.body_coverage", "Physics bodies do not exactly cover profile parts.")

    if _joint_pairs(physics.get("constraints", [])) != _joint_pairs(expected_joints):
        _issue(issues, "intake.physics.constraints", "Physics constraints do not match profile joints.")

    if not ik_rig.get("compatible", False):
        _issue(issues, "intake.ik.compatibility", "Unreal did not create a compatible IK Rig.")

    if ik_rig.get("retargetRoot") != "pelvis":
        _issue(issues, "intake.ik.root", "IK retarget root must be pelvis.")

    if ik_rig.get("goals") != EXPECTED_GOALS:
        _issue(issues, "intake.ik.goals", "IK goals do not match the humanoid adapter contract.")

    if ik_rig.get("chains") != EXPECTED_CHAINS:
        _issue(issues, "intake.ik.chains", "IK chains do not match the humanoid adapter contract.")

    manifest_clip_names = tuple(
        clip.get("name") for clip in locomotion_manifest.get("clips", [])
    )
    imported_clip_names = tuple(animation.get("name") for animation in animations)
    if manifest_clip_names != REQUIRED_CLIP_NAMES or imported_clip_names != REQUIRED_CLIP_NAMES:
        _issue(
            issues,
            "intake.motion.clips",
            "Imported animations do not exactly match the ordered clip contract.",
        )

    maximum_translation = float(
        ik_contract.get("maximumRootTranslationCentimeters", -1.0)
    )
    if maximum_translation < 0.0 or not _in_place(animations, maximum_translation):
        _issue(
            issues,
            "intake.motion.in_place",
            "A sampled root translation exceeds the in-place tolerance.",
        )

    if not _solver_matches(solvers):
        _issue(
            issues,
            "intake.ik.solver",
            "IK Rig must contain one enabled pelvis-rooted FullBodyIK solver connected to both feet.",
        )

    if not _evaluated_pose_passes(ik_evidence, ik_contract):
        _issue(
            issues,
            "intake.ik.evaluated_pose",
            "Native evaluated-pose evidence does not meet the foot-goal response contract.",
        )

    return {
        "schemaVersion": SCHEMA_VERSION,
        "evaluatorVersion": EVALUATOR_VERSION,
        "passed": not issues,
        "checks": {
            "boneCount": len(expected_bones),
            "expectedHeightCentimeters": round(expected_height, 6),
            "physicsBodyCount": len(physics.get("bodyBones", [])),
            "physicsConstraintCount": len(physics.get("constraints", [])),
            "animationCount": len(animations),
            "solverCount": len(solvers),
        },
        "issues": issues,
        "expectedPhysics": {
            "bodyBones": expected_body_bones,
            "constraints": expected_joints,
        },
        "measuredPhysics": {
            "bodyBones": physics.get("bodyBones", []),
            "constraints": physics.get("constraints", []),
        },
    }


def _load(path):
    return json.loads(path.read_text(encoding="utf-8"))


def _sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description="Evaluate a generated humanoid Unreal intake report.")
    parser.add_argument("--manifest", required=True, type=Path)
    parser.add_argument("--profile", required=True, type=Path)
    parser.add_argument("--locomotion-manifest", required=True, type=Path)
    parser.add_argument("--report", required=True, type=Path)
    parser.add_argument("--ik-evidence", required=True, type=Path)
    parser.add_argument("--ik-contract", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    result = evaluate_intake(
        _load(args.manifest),
        _load(args.profile),
        _load(args.locomotion_manifest),
        _load(args.report),
        _load(args.ik_evidence),
        _load(args.ik_contract),
        _sha256(args.manifest),
        _sha256(args.profile),
        _sha256(args.locomotion_manifest),
    )
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"Unreal intake policy: {'PASS' if result['passed'] else 'FAIL'}")
    for issue in result["issues"]:
        print(f"- {issue['ruleId']}: {issue['message']}")
    return 0 if result["passed"] else 2


if __name__ == "__main__":
    raise SystemExit(main())
