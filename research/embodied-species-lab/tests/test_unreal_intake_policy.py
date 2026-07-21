import copy
import sys
import unittest
from pathlib import Path


LAB_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(LAB_ROOT))

from unreal_intake_policy import evaluate_intake
from humanoid_skeleton import humanoid_bones


BODY = {
    "heightMeters": 1.8,
    "shoulderWidthMeters": 0.44,
    "hipWidthMeters": 0.32,
    "depthMeters": 0.24,
}
BONE_SPECS = humanoid_bones(BODY)
BONENAMES = [bone["name"] for bone in BONE_SPECS]

MANIFEST = {
    "schemaVersion": 1,
    "validation": {"passed": True},
    "skeleton": {
        "bones": [
            {
                "name": bone["name"],
                "parent": bone["parent"],
                "headMeters": list(bone["head"]),
                "tailMeters": list(bone["tail"]),
            }
            for bone in BONE_SPECS
        ]
    },
}

PART_BONES = [
    "pelvis", "spine", "chest", "head",
    "upper_arm_l", "lower_arm_l", "upper_arm_r", "lower_arm_r",
    "thigh_l", "shin_l", "foot_l", "thigh_r", "shin_r", "foot_r",
]
JOINTS = [
    {"parent": bone["parent"], "child": bone["name"]}
    for bone in BONE_SPECS
    if bone["name"] in PART_BONES and bone["name"] != "pelvis"
]
PROFILE = {
    "schemaVersion": 1,
    "profileVersion": 1,
    "totalMassKg": 75.0,
    "parts": [{"bone": name, "massKg": 75.0 / len(PART_BONES)} for name in PART_BONES],
    "joints": JOINTS,
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

LOCOMOTION_MANIFEST = {
    "schemaVersion": 1,
    "locomotionVersion": 1,
    "validation": {"passed": True},
    "clips": [
        {
            "name": name,
            "fps": 30,
            "startFrame": 1,
            "endFrame": 31,
            "durationSeconds": 1.0,
            "looping": name in {"A_Idle", "A_Walk", "A_Run", "A_Fall"},
            "fbxFile": f"{name}.fbx",
        }
        for name in REQUIRED_CLIP_NAMES
    ],
}

IK_CONTRACT = {
    "schemaVersion": 1,
    "goalOffsetCentimeters": 10.0,
    "maximumRootTranslationCentimeters": 0.1,
    "minimumDrivenDisplacementCentimeters": 5.0,
    "maximumGoalErrorCentimeters": 2.0,
    "maximumOppositeFootDisplacementCentimeters": 2.0,
}

IK_EVIDENCE = {
    "schemaVersion": 1,
    "solverCount": 1,
    "goalOffsetCentimeters": 10.0,
    "feet": {
        "left": {
            "drivenDisplacementCentimeters": 9.8,
            "goalErrorCentimeters": 0.2,
            "oppositeFootDisplacementCentimeters": 0.1,
        },
        "right": {
            "drivenDisplacementCentimeters": 9.9,
            "goalErrorCentimeters": 0.1,
            "oppositeFootDisplacementCentimeters": 0.1,
        },
    },
}

REFERENCE_HIERARCHY = [
    {
        "name": bone["name"],
        "parent": bone["parent"],
        "children": sorted(child["name"] for child in BONE_SPECS if child["parent"] == bone["name"]),
    }
    for bone in BONE_SPECS
]

REPORT = {
    "schemaVersion": 3,
    "importerVersion": 3,
    "adapterVersion": 1,
    "manifestSha256": "manifest-hash",
    "profileSha256": "profile-hash",
    "locomotionManifestSha256": "locomotion-hash",
    "boneNames": BONENAMES,
    "referenceHierarchy": REFERENCE_HIERARCHY,
    "footPlacementRoles": {
        "ikRoot": "root",
        "pelvis": "pelvis",
        "left": {"fkFoot": "foot_l", "ball": "ball_l", "ikFoot": "ik_foot_l"},
        "right": {"fkFoot": "foot_r", "ball": "ball_r", "ikFoot": "ik_foot_r"},
    },
    "meshBoundsSizeCentimeters": [148.0, 48.0, 180.0],
    "physicsAsset": {
        "compatible": True,
        "derivation": "profile-topology-v1",
        "bodyBones": PART_BONES,
        "constraints": JOINTS,
    },
    "ikRig": {
        "compatible": True,
        "retargetRoot": "pelvis",
        "goals": {"foot_l_goal": "foot_l", "foot_r_goal": "foot_r"},
        "chains": {
            "leg_l": {"start": "thigh_l", "end": "foot_l", "goal": "foot_l_goal"},
            "leg_r": {"start": "thigh_r", "end": "foot_r", "goal": "foot_r_goal"},
        },
        "solvers": [
            {
                "enabled": True,
                "controllerClass": "IKRigFBIKController",
                "startBone": "pelvis",
                "connectedGoals": ["foot_l_goal", "foot_r_goal"],
            }
        ],
    },
    "animations": [
        {
            "name": name,
            "durationSeconds": 1.0,
            "sampledKeys": 31,
            "rootTranslationCentimeters": [
                [0.0, 0.0, 0.0],
                [0.0, 0.0, 0.0],
                [0.0, 0.0, 0.0],
            ],
        }
        for name in REQUIRED_CLIP_NAMES
    ],
}


class UnrealIntakePolicyTests(unittest.TestCase):
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

    def test_valid_native_intake_passes(self):
        result = self.evaluate(copy.deepcopy(REPORT))
        self.assertTrue(result["passed"], result["issues"])

    def test_missing_physics_body_fails_body_coverage(self):
        report = copy.deepcopy(REPORT)
        report["physicsAsset"]["bodyBones"].remove("foot_l")
        result = self.evaluate(report)
        self.assertIn("intake.physics.body_coverage", [issue["ruleId"] for issue in result["issues"]])

    def test_missing_profile_adapter_provenance_fails(self):
        report = copy.deepcopy(REPORT)
        del report["adapterVersion"]
        report["physicsAsset"]["derivation"] = "native-auto"
        result = self.evaluate(report)
        self.assertIn(
            "intake.physics.adapter",
            [issue["ruleId"] for issue in result["issues"]],
        )

    def test_missing_ik_goal_fails_goal_mapping(self):
        report = copy.deepcopy(REPORT)
        del report["ikRig"]["goals"]["foot_r_goal"]
        result = self.evaluate(report)
        self.assertIn("intake.ik.goals", [issue["ruleId"] for issue in result["issues"]])

    def test_missing_foot_placement_role_fails_causally(self):
        report = copy.deepcopy(REPORT)
        del report["footPlacementRoles"]["left"]["ball"]
        result = self.evaluate(report)
        self.assertIn(
            "intake.foot_placement.roles",
            [issue["ruleId"] for issue in result["issues"]],
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
        self.assertIn(
            "intake.ik.evaluated_pose",
            [issue["ruleId"] for issue in result["issues"]],
        )


if __name__ == "__main__":
    unittest.main()
