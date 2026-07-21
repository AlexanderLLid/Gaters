import copy
import math
import sys
import unittest
from pathlib import Path


LAB_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(LAB_ROOT))

from recovery import physical_structure_passed, plan_recovery


STANCE = {
    "foot_l": [0.16, 0.0, 0.05],
    "foot_r": [-0.16, 0.0, 0.05],
}


def impact(direction, strength=0.8):
    return {
        "eventId": "impact.test",
        "type": "impact",
        "point": "chest",
        "direction": direction,
        "strength": strength,
    }


class RecoveryPlannerTests(unittest.TestCase):
    def test_weak_impact_holds_stance(self):
        plan = plan_recovery(impact([1.0, 0.0, 0.0], strength=0.2), copy.deepcopy(STANCE))
        self.assertEqual(plan["mode"], "hold")
        self.assertIsNone(plan["stepFoot"])
        self.assertEqual(plan["stepLengthMeters"], 0.0)

    def test_left_and_right_impacts_select_foot_on_force_side(self):
        left = plan_recovery(impact([1.0, 0.0, 0.0]), copy.deepcopy(STANCE))
        right = plan_recovery(impact([-1.0, 0.0, 0.0]), copy.deepcopy(STANCE))
        self.assertEqual(left["stepFoot"], "foot_l")
        self.assertEqual(right["stepFoot"], "foot_r")

    def test_step_moves_in_horizontal_force_direction(self):
        plan = plan_recovery(impact([0.2, -1.0, 0.4]), copy.deepcopy(STANCE))
        start = STANCE[plan["stepFoot"]]
        target = plan["targetMeters"]
        displacement = [target[0] - start[0], target[1] - start[1]]
        horizontal_force = [0.2, -1.0]
        dot = sum(a * b for a, b in zip(displacement, horizontal_force))
        self.assertGreater(dot, 0.0)
        self.assertAlmostEqual(math.hypot(*displacement), plan["stepLengthMeters"], places=6)

    def test_step_length_and_lift_are_bounded(self):
        plan = plan_recovery(impact([1.0, 0.0, 0.0], strength=1.0), copy.deepcopy(STANCE))
        self.assertGreaterEqual(plan["stepLengthMeters"], 0.18)
        self.assertLessEqual(plan["stepLengthMeters"], 0.40)
        self.assertGreaterEqual(plan["liftMeters"], 0.06)
        self.assertLessEqual(plan["liftMeters"], 0.14)

    def test_schedule_is_ordered_and_settles_in_three_seconds(self):
        plan = plan_recovery(impact([1.0, 0.0, 0.0]), copy.deepcopy(STANCE), fps=30)
        self.assertEqual(
            plan["frames"], {"release": 8, "apex": 16, "plant": 24, "settled": 91}
        )

    def test_same_input_produces_same_plan(self):
        first = plan_recovery(impact([0.5, 0.5, 0.0]), copy.deepcopy(STANCE))
        second = plan_recovery(impact([0.5, 0.5, 0.0]), copy.deepcopy(STANCE))
        self.assertEqual(first, second)

    def test_balance_target_is_centered_between_final_feet(self):
        plan = plan_recovery(impact([1.0, 0.0, 0.0]), copy.deepcopy(STANCE))
        expected = [
            (plan["targetMeters"][0] + STANCE["foot_r"][0]) * 0.5,
            (plan["targetMeters"][1] + STANCE["foot_r"][1]) * 0.5,
        ]
        self.assertEqual(plan["supportCenterMeters"], [round(value, 6) for value in expected])

    def test_new_stance_ignores_old_return_to_origin_check_only(self):
        checks = {"allPartsCreated": True, "reactionSettled": False}
        self.assertTrue(physical_structure_passed(checks))
        checks["allPartsCreated"] = False
        self.assertFalse(physical_structure_passed(checks))


if __name__ == "__main__":
    unittest.main()
