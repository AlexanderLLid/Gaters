import sys
import unittest
from pathlib import Path


LAB_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(LAB_ROOT))

from humanoid_skeleton import FOOT_PLACEMENT_ROLES, humanoid_bones


BODY = {
    "heightMeters": 1.8,
    "shoulderWidthMeters": 0.44,
    "hipWidthMeters": 0.32,
    "depthMeters": 0.24,
}


class HumanoidSkeletonTests(unittest.TestCase):
    def test_foot_placement_roles_are_unique_and_parented(self):
        bones = humanoid_bones(BODY)
        by_name = {bone["name"]: bone for bone in bones}

        self.assertEqual(len(by_name), len(bones))
        self.assertEqual(
            FOOT_PLACEMENT_ROLES,
            ("foot_l", "ball_l", "ik_foot_l", "foot_r", "ball_r", "ik_foot_r"),
        )
        self.assertEqual(by_name["ball_l"]["parent"], "foot_l")
        self.assertEqual(by_name["ball_r"]["parent"], "foot_r")
        self.assertEqual(by_name["ik_foot_l"]["parent"], "root")
        self.assertEqual(by_name["ik_foot_r"]["parent"], "root")

    def test_only_ik_targets_are_non_deforming(self):
        by_name = {bone["name"]: bone for bone in humanoid_bones(BODY)}

        self.assertTrue(by_name["foot_l"]["deform"])
        self.assertTrue(by_name["ball_l"]["deform"])
        self.assertTrue(by_name["foot_r"]["deform"])
        self.assertTrue(by_name["ball_r"]["deform"])
        self.assertFalse(by_name["ik_foot_l"]["deform"])
        self.assertFalse(by_name["ik_foot_r"]["deform"])


if __name__ == "__main__":
    unittest.main()
