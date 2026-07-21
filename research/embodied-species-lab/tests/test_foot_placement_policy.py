import copy
import sys
import unittest
from pathlib import Path


LAB_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(LAB_ROOT))

from foot_placement_policy import evaluate


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
}


def case(name, left_delta=8.0, right_delta=0.0):
    route_x = {
        "split_left_high": 0.0,
        "split_right_high": 0.0,
        "step": 75.0,
        "slope": 150.0,
        "stop": 155.0,
    }[name]
    floor_height = {"step": 8.0, "slope": 5.0, "stop": 5.0}.get(name, 0.0)
    return {
        "name": name,
        "movementMode": "Walking",
        "hasWalkableFloor": True,
        "hasNaN": False,
        "pelvisOffsetCentimeters": -4.0,
        "speedCentimetersPerSecond": 0.0,
        "actorLocationCentimeters": [route_x, 0.0, floor_height],
        "characterFloorHeightCentimeters": floor_height,
        "feet": {
            "left": {
                "walkableHit": True,
                "contactErrorCentimeters": 0.5,
                "terrainTargetDeltaCentimeters": left_delta,
            },
            "right": {
                "walkableHit": True,
                "contactErrorCentimeters": -0.25,
                "terrainTargetDeltaCentimeters": right_delta,
            },
        },
    }


REPORT = {
    "schemaVersion": 1,
    "challengerVersion": 1,
    "routeVersion": 1,
    "nativeNodes": {
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
    "cases": [
        case("split_left_high", 8.0, 0.0),
        case("split_right_high", 0.0, 8.0),
        case("step", 10.0, 3.0),
        case("slope", 4.0, 4.0),
        case("stop", 3.0, 3.0),
    ],
}


class FootPlacementPolicyTests(unittest.TestCase):
    def evaluate(self, report):
        return evaluate(copy.deepcopy(report), copy.deepcopy(CONTRACT))

    def rule_ids(self, report):
        return [issue["ruleId"] for issue in self.evaluate(report)["issues"]]

    def test_valid_native_report_passes(self):
        result = self.evaluate(REPORT)
        self.assertTrue(result["passed"], result["issues"])

    def test_custom_solver_fails_native_identity(self):
        report = copy.deepcopy(REPORT)
        report["nativeNodes"]["legSolver"] = "GatersCustomLegSolver"
        self.assertIn("foot_placement.native_nodes", self.rule_ids(report))

    def test_loaded_editor_adapter_fails_runtime_boundary(self):
        report = copy.deepcopy(REPORT)
        report["editorAdapterLoaded"] = True
        self.assertIn("foot_placement.runtime_boundary", self.rule_ids(report))

    def test_missing_role_fails_mapping(self):
        report = copy.deepcopy(REPORT)
        del report["roles"]["left"]["ball"]
        self.assertIn("foot_placement.roles", self.rule_ids(report))

    def test_missing_case_fails_route(self):
        report = copy.deepcopy(REPORT)
        report["cases"] = report["cases"][:-1]
        self.assertIn("foot_placement.route", self.rule_ids(report))

    def test_unwalkable_or_airborne_case_fails_floor(self):
        report = copy.deepcopy(REPORT)
        report["cases"][0]["feet"]["left"]["walkableHit"] = False
        self.assertIn("foot_placement.floor", self.rule_ids(report))

    def test_excess_contact_error_fails_contact(self):
        report = copy.deepcopy(REPORT)
        report["cases"][1]["feet"]["right"]["contactErrorCentimeters"] = 6.0
        self.assertIn("foot_placement.contact", self.rule_ids(report))

    def test_no_terrain_response_fails_causality(self):
        report = copy.deepcopy(REPORT)
        for item in report["cases"]:
            for foot in item["feet"].values():
                foot["terrainTargetDeltaCentimeters"] = 0.0
        self.assertIn("foot_placement.causality", self.rule_ids(report))

    def test_nan_or_excess_pelvis_fails_pose(self):
        report = copy.deepcopy(REPORT)
        report["cases"][2]["hasNaN"] = True
        report["cases"][2]["pelvisOffsetCentimeters"] = 51.0
        self.assertIn("foot_placement.pose", self.rule_ids(report))

    def test_moving_final_stop_fails_finish(self):
        report = copy.deepcopy(REPORT)
        report["cases"][-1]["speedCentimetersPerSecond"] = 3.0
        self.assertIn("foot_placement.finish", self.rule_ids(report))

    def test_static_capsule_and_flat_floor_fail_character_movement(self):
        report = copy.deepcopy(REPORT)
        for item in report["cases"]:
            item["actorLocationCentimeters"] = [0.0, 0.0, 0.0]
            item["characterFloorHeightCentimeters"] = 0.0
        self.assertIn("foot_placement.character_movement", self.rule_ids(report))


if __name__ == "__main__":
    unittest.main()
