import copy
import json
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from pose_suite_verifier import verify_pose_suite


def load(path):
    return json.loads(path.read_text(encoding="utf-8"))


SURFACE = load(ROOT / "AnatomicalSurfaceRuns" / "anatomical-mannequin-20260721-194548-295" / "run-1" / "readback.json")
SKELETON = load(ROOT / "SkeletonRuns" / "humanoid-guide-skeleton-stick-humanoid-20260721-175401-425940" / "run-1" / "skeleton.json")
CAPTURE = load(ROOT / "SkinCaptureRuns" / "20260721-201347-346" / "readback.json")
SUITE_CAPTURE = load(ROOT / "SkinCaptureRuns" / "20260721-203609-297" / "readback.json")
POSE = load(ROOT / "recipes" / "biharmonic-pose-suite.json")["diagnostic_poses"][0]


class PoseSuiteVerifierTests(unittest.TestCase):
    def setUp(self):
        self.recipe = {"diagnostic_poses": [POSE]}
        self.readback = {
            "positions": CAPTURE["positions"],
            "modules": CAPTURE["modules"],
            "poses": {
                POSE["id"]: {
                    "positions": CAPTURE["posed_positions"],
                    "skeleton_positions": CAPTURE["posed_skeleton_positions"],
                }
            },
        }

    def test_accepts_real_biharmonic_elbow_pose(self):
        report = verify_pose_suite(SURFACE, SKELETON, self.recipe, self.readback)
        self.assertTrue(report["passed"])
        self.assertGreater(report["poses"][POSE["id"]]["thickness_ratio_p10"], 0.88)

    def test_rejects_inactive_pose(self):
        broken = copy.deepcopy(self.readback)
        broken["poses"][POSE["id"]]["positions"] = copy.deepcopy(broken["positions"])
        report = verify_pose_suite(SURFACE, SKELETON, self.recipe, broken)
        self.assertIn("POSE-SUITE-ACTIVE-1", {failure["rule"] for failure in report["failures"]})

    def test_rejects_missing_pose(self):
        broken = copy.deepcopy(self.readback)
        broken["poses"] = {}
        report = verify_pose_suite(SURFACE, SKELETON, self.recipe, broken)
        self.assertIn("POSE-SUITE-COVERAGE-1", {failure["rule"] for failure in report["failures"]})

    def test_rejects_excessive_transition_module_motion(self):
        shoulder = copy.deepcopy(load(ROOT / "recipes" / "biharmonic-pose-suite.json")["diagnostic_poses"][2])
        readback = {
            "positions": SUITE_CAPTURE["positions"],
            "modules": SUITE_CAPTURE["modules"],
            "poses": {shoulder["id"]: copy.deepcopy(SUITE_CAPTURE["poses"][shoulder["id"]])},
        }
        index = readback["modules"].index("torso")
        readback["poses"][shoulder["id"]]["positions"][index][0] += 0.1
        report = verify_pose_suite(SURFACE, SKELETON, {"diagnostic_poses": [shoulder]}, readback)
        self.assertIn("POSE-SUITE-TRANSITION-1", {failure["rule"] for failure in report["failures"]})


if __name__ == "__main__":
    unittest.main()
