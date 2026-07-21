import copy
import sys
import unittest
from pathlib import Path


LAB_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(LAB_ROOT))

from character_movement_policy import evaluate_character_movement


PHASES = ["idle", "walk", "run", "turn", "stop", "jump", "fall", "land"]
CLIPS = [
    "A_Idle",
    "A_Walk",
    "A_Run",
    "A_TurnLeft",
    "A_Stop",
    "A_Jump",
    "A_Fall",
    "A_Land",
]
CONTRACT = {
    "schemaVersion": 1,
    "routeVersion": 1,
    "requiredPhases": PHASES,
    "requiredClips": CLIPS,
    "minimumForwardDisplacementCentimeters": 150.0,
    "minimumTurnDisplacementCentimeters": 25.0,
    "minimumFallingSamples": 1,
    "minimumGroundedSpeedFraction": 0.9,
    "walkSpeedCentimetersPerSecond": 150.0,
    "runSpeedCentimetersPerSecond": 450.0,
    "maximumFinalSpeedCentimetersPerSecond": 5.0,
}
REPORT = {
    "schemaVersion": 1,
    "challengerVersion": 1,
    "routeVersion": 1,
    "map": "/Game/Gaters/Generated/CharacterLab/L_CharacterLab",
    "assets": {
        "skeletalMesh": "/Game/Gaters/Generated/CharacterLab/SK_GeneratedHumanoid.SK_GeneratedHumanoid",
        "skeleton": "/Game/Gaters/Generated/CharacterLab/SK_GeneratedHumanoid_Skeleton.SK_GeneratedHumanoid_Skeleton",
        "physicsAsset": "/Game/Gaters/Generated/CharacterLab/SK_GeneratedHumanoid_PhysicsAsset.SK_GeneratedHumanoid_PhysicsAsset",
        "ikRig": "/Game/Gaters/Generated/CharacterLab/IK_GeneratedHumanoid.IK_GeneratedHumanoid",
        "clips": {
            phase: f"/Game/Gaters/Generated/CharacterLab/{clip}.{clip}"
            for phase, clip in zip(PHASES, CLIPS)
        },
    },
    "runtime": {
        "actorClass": "Character",
        "movementComponentClass": "CharacterMovementComponent",
        "updatedComponentClass": "CapsuleComponent",
        "editorAdapterLoaded": False,
    },
    "observedPhases": PHASES,
    "samples": [
        {
            "phase": phase,
            "clip": clip,
            "locationCentimeters": [x, y, z],
            "velocityCentimetersPerSecond": [vx, vy, vz],
            "accelerationCentimetersPerSecondSquared": [ax, ay, az],
            "facingDegrees": facing,
            "movementMode": mode,
            "hasWalkableFloor": floor,
        }
        for phase, clip, x, y, z, vx, vy, vz, ax, ay, az, facing, mode, floor in [
            ("idle", "A_Idle", 0, 0, 90, 0, 0, 0, 0, 0, 0, 0, "Walking", True),
            ("walk", "A_Walk", 75, 0, 90, 150, 0, 0, 300, 0, 0, 0, "Walking", True),
            ("run", "A_Run", 225, 0, 90, 450, 0, 0, 600, 0, 0, 0, "Walking", True),
            ("turn", "A_TurnLeft", 250, -75, 90, 0, -150, 0, 0, -300, 0, -90, "Walking", True),
            ("stop", "A_Stop", 250, -90, 90, 0, 0, 0, 0, 0, 0, -90, "Walking", True),
            ("jump", "A_Jump", 250, -90, 130, 0, 0, 300, 0, 0, -980, -90, "Falling", False),
            ("fall", "A_Fall", 250, -90, 145, 0, 0, -100, 0, 0, -980, -90, "Falling", False),
            ("land", "A_Land", 250, -90, 90, 0, 0, 0, 0, 0, 0, -90, "Walking", True),
        ]
    ],
    "final": {
        "locationCentimeters": [250.0, -90.0, 90.0],
        "speedCentimetersPerSecond": 0.0,
        "movementMode": "Walking",
        "hasWalkableFloor": True,
    },
}


class CharacterMovementPolicyTests(unittest.TestCase):
    def evaluate(self, report=None, contract=None):
        return evaluate_character_movement(
            copy.deepcopy(REPORT if report is None else report),
            copy.deepcopy(CONTRACT if contract is None else contract),
        )

    def rule_ids(self, result):
        return [issue["ruleId"] for issue in result["issues"]]

    def test_valid_native_route_passes(self):
        result = self.evaluate()
        self.assertTrue(result["passed"], result["issues"])

    def test_wrong_phase_order_fails(self):
        report = copy.deepcopy(REPORT)
        report["observedPhases"][2:4] = reversed(report["observedPhases"][2:4])
        self.assertIn("movement.route.order", self.rule_ids(self.evaluate(report)))

    def test_missing_clip_fails(self):
        report = copy.deepcopy(REPORT)
        del report["assets"]["clips"]["fall"]
        self.assertIn("movement.assets.clips", self.rule_ids(self.evaluate(report)))

    def test_wrong_native_owner_fails(self):
        report = copy.deepcopy(REPORT)
        report["runtime"]["movementComponentClass"] = "CustomMovementComponent"
        self.assertIn("movement.runtime.native_authority", self.rule_ids(self.evaluate(report)))

    def test_editor_adapter_loaded_fails(self):
        report = copy.deepcopy(REPORT)
        report["runtime"]["editorAdapterLoaded"] = True
        self.assertIn("movement.runtime.editor_boundary", self.rule_ids(self.evaluate(report)))

    def test_grounded_phase_without_floor_fails(self):
        report = copy.deepcopy(REPORT)
        report["samples"][1]["hasWalkableFloor"] = False
        self.assertIn("movement.floor.grounded", self.rule_ids(self.evaluate(report)))

    def test_missing_falling_state_fails(self):
        report = copy.deepcopy(REPORT)
        for sample in report["samples"]:
            if sample["phase"] in {"jump", "fall"}:
                sample["movementMode"] = "Walking"
                sample["hasWalkableFloor"] = True
        self.assertIn("movement.floor.fall_land", self.rule_ids(self.evaluate(report)))

    def test_insufficient_forward_displacement_fails(self):
        report = copy.deepcopy(REPORT)
        report["samples"][2]["locationCentimeters"][0] = 100.0
        self.assertIn("movement.route.forward", self.rule_ids(self.evaluate(report)))

    def test_insufficient_turn_displacement_fails(self):
        report = copy.deepcopy(REPORT)
        report["samples"][3]["locationCentimeters"][1] = -10.0
        self.assertIn("movement.route.turn", self.rule_ids(self.evaluate(report)))

    def test_collapsed_run_speed_fails(self):
        report = copy.deepcopy(REPORT)
        report["samples"][2]["velocityCentimetersPerSecond"] = [0.0, 1.0, 0.0]
        self.assertIn("movement.route.speeds", self.rule_ids(self.evaluate(report)))

    def test_final_airborne_state_fails(self):
        report = copy.deepcopy(REPORT)
        report["final"]["movementMode"] = "Falling"
        report["final"]["hasWalkableFloor"] = False
        self.assertIn("movement.route.final", self.rule_ids(self.evaluate(report)))


if __name__ == "__main__":
    unittest.main()
