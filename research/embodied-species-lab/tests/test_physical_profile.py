import copy
import sys
import unittest
from pathlib import Path


LAB_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(LAB_ROOT))

from physical_profile import generate_physical_profile


BODY = {
    "heightMeters": 1.8,
    "shoulderWidthMeters": 0.44,
    "hipWidthMeters": 0.32,
    "depthMeters": 0.24,
}


class PhysicalProfileTests(unittest.TestCase):
    def test_default_humanoid_has_75kg_total_mass(self):
        profile = generate_physical_profile(copy.deepcopy(BODY))
        self.assertAlmostEqual(sum(part["massKg"] for part in profile["parts"]), 75.0, places=5)
        self.assertTrue(all(part["massKg"] > 0.0 for part in profile["parts"]))

    def test_every_non_root_physical_part_has_one_joint(self):
        profile = generate_physical_profile(copy.deepcopy(BODY))
        parts = {part["bone"] for part in profile["parts"]}
        children = [joint["child"] for joint in profile["joints"]]
        self.assertEqual(set(children), parts - {"pelvis"})
        self.assertEqual(len(children), len(set(children)))
        self.assertTrue(all(joint["parent"] in parts for joint in profile["joints"]))

    def test_elbows_and_knees_cannot_reverse(self):
        profile = generate_physical_profile(copy.deepcopy(BODY))
        joints = {joint["child"]: joint for joint in profile["joints"]}
        for name in ("lower_arm_l", "lower_arm_r", "shin_l", "shin_r"):
            hinge = joints[name]["angularLimitsDegrees"]["z"]
            self.assertGreaterEqual(hinge[0], 0.0)
            self.assertLessEqual(hinge[1], 160.0)
            self.assertLessEqual(
                joints[name]["angularLimitsDegrees"]["x"][1]
                - joints[name]["angularLimitsDegrees"]["x"][0],
                12.0,
            )
            self.assertLessEqual(
                joints[name]["angularLimitsDegrees"]["y"][1]
                - joints[name]["angularLimitsDegrees"]["y"][0],
                12.0,
            )

    def test_same_body_produces_same_profile(self):
        first = generate_physical_profile(copy.deepcopy(BODY))
        second = generate_physical_profile(copy.deepcopy(BODY))
        self.assertEqual(first, second)

    def test_height_scales_body_mass_allometrically(self):
        short = generate_physical_profile({**BODY, "heightMeters": 1.6})
        tall = generate_physical_profile({**BODY, "heightMeters": 2.0})
        self.assertLess(short["totalMassKg"], tall["totalMassKg"])

    def test_active_profile_preserves_body_but_anchors_no_parts(self):
        anchored = generate_physical_profile(copy.deepcopy(BODY))
        active = generate_physical_profile(copy.deepcopy(BODY), anchored_feet=False)
        self.assertTrue(any(part["anchored"] for part in anchored["parts"]))
        self.assertFalse(any(part["anchored"] for part in active["parts"]))
        self.assertEqual(anchored["totalMassKg"], active["totalMassKg"])
        self.assertEqual(anchored["joints"], active["joints"])


if __name__ == "__main__":
    unittest.main()
