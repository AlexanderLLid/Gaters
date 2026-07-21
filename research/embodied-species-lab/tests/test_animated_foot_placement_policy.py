import copy
import sys
import unittest
from pathlib import Path


LAB_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(LAB_ROOT))

from animated_foot_placement_policy import evaluate


CONTRACT = {
    "schemaVersion": 1,
    "routeVersion": 1,
    "requiredCases": ["split_left_high", "split_right_high", "step", "slope", "stop"],
    "maximumAbsoluteContactErrorCentimeters": 5.0,
    "maximumAbsolutePelvisOffsetCentimeters": 50.0,
    "minimumTerrainTargetDeltaCentimeters": 2.0,
    "minimumRouteForwardDisplacementCentimeters": 100.0,
    "minimumCharacterFloorHeightDeltaCentimeters": 2.0,
    "maximumFinalSpeedCentimetersPerSecond": 2.0,
    "generatedWalkClip": "/Game/Gaters/Generated/CharacterLab/A_Walk.A_Walk",
    "minimumAnimationTimeAdvanceSeconds": 0.4,
    "minimumAnimatedThighDirectionDelta": 0.05,
}


def case(name, x, floor, left_delta=4.0, right_delta=4.0):
    return {
        "name": name,
        "movementMode": "Walking",
        "hasWalkableFloor": True,
        "hasNaN": False,
        "pelvisOffsetCentimeters": 4.0,
        "speedCentimetersPerSecond": 0.0,
        "actorLocationCentimeters": [x, 0.0, floor],
        "characterFloorHeightCentimeters": floor,
        "feet": {
            "left": {
                "walkableHit": True,
                "contactErrorCentimeters": 1.0,
                "terrainTargetDeltaCentimeters": left_delta,
            },
            "right": {
                "walkableHit": True,
                "contactErrorCentimeters": -1.0,
                "terrainTargetDeltaCentimeters": right_delta,
            },
        },
    }


REPORT = {
    "schemaVersion": 1,
    "challengerVersion": 1,
    "animationEvidenceVersion": 1,
    "routeVersion": 1,
    "nativeNodes": {
        "source": "FAnimNode_SequencePlayer_Standalone",
        "footPlacement": "FAnimNode_FootPlacement",
        "legSolver": "FAnimNode_TwoBoneIK",
        "movement": "CharacterMovementComponent",
    },
    "footPlacementExperimental": True,
    "editorAdapterLoaded": False,
    "roles": {
        "ikRoot": "root",
        "pelvis": "pelvis",
        "left": {"fkFoot": "foot_l", "ball": "ball_l", "ikFoot": "ik_foot_l"},
        "right": {"fkFoot": "foot_r", "ball": "ball_r", "ikFoot": "ik_foot_r"},
    },
    "sourceAnimation": {
        "asset": "/Game/Gaters/Generated/CharacterLab/A_Walk.A_Walk",
        "name": "A_Walk",
        "looping": True,
    },
    "animationSamples": [
        {
            "name": "phase_a",
            "assetTimeSeconds": 0.25,
            "leftThighDirection": [0.0, 0.0, -1.0],
            "rightThighDirection": [0.0, 0.0, -1.0],
        },
        {
            "name": "phase_b",
            "assetTimeSeconds": 0.75,
            "leftThighDirection": [0.5, 0.0, -0.866025],
            "rightThighDirection": [-0.5, 0.0, -0.866025],
        },
    ],
    "cases": [
        case("split_left_high", 0.0, 0.0, 8.0, 0.0),
        case("split_right_high", 0.0, 0.0, 0.0, 8.0),
        case("step", 75.0, 8.0),
        case("slope", 150.0, 5.0),
        case("stop", 155.0, 5.0),
    ],
}


class AnimatedFootPlacementPolicyTests(unittest.TestCase):
    def evaluate(self, report):
        return evaluate(copy.deepcopy(report), copy.deepcopy(CONTRACT))

    def rule_ids(self, report):
        return [issue["ruleId"] for issue in self.evaluate(report)["issues"]]

    def test_valid_animated_grounding_report_passes(self):
        result = self.evaluate(REPORT)
        self.assertTrue(result["passed"], result["issues"])

    def test_reference_pose_source_fails_native_source(self):
        report = copy.deepcopy(REPORT)
        report["nativeNodes"]["source"] = "FAnimNode_RefPose"
        self.assertIn("animated_foot_placement.native_source", self.rule_ids(report))

    def test_wrong_clip_fails_source_animation(self):
        report = copy.deepcopy(REPORT)
        report["sourceAnimation"]["asset"] = "/Game/Other.A_Walk"
        self.assertIn("animated_foot_placement.source_animation", self.rule_ids(report))

    def test_frozen_asset_time_fails_advancement(self):
        report = copy.deepcopy(REPORT)
        report["animationSamples"][1]["assetTimeSeconds"] = 0.25
        self.assertIn("animated_foot_placement.time", self.rule_ids(report))

    def test_static_thighs_fail_evaluated_pose(self):
        report = copy.deepcopy(REPORT)
        report["animationSamples"][1]["leftThighDirection"] = [0.0, 0.0, -1.0]
        report["animationSamples"][1]["rightThighDirection"] = [0.0, 0.0, -1.0]
        self.assertIn("animated_foot_placement.pose", self.rule_ids(report))

    def test_grounding_regression_is_preserved(self):
        report = copy.deepcopy(REPORT)
        report["cases"][2]["feet"]["left"]["contactErrorCentimeters"] = 6.0
        self.assertIn("foot_placement.contact", self.rule_ids(report))


if __name__ == "__main__":
    unittest.main()
