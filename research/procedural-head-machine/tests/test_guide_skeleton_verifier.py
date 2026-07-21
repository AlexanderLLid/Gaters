import json
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from guide_skeleton_compiler import compile_guide_skeleton
from guide_skeleton_verifier import verify_guide_skeleton


def load(path):
    return json.loads(path.read_text(encoding="utf-8"))


GUIDE_RUN = ROOT / "AnatomyRuns" / "humanoid-guide-stick-humanoid-20260721-174142-274786" / "run-1"
RECIPE = load(ROOT / "recipes" / "humanoid-guide-skeleton.json")


class GuideSkeletonVerifierTests(unittest.TestCase):
    def setUp(self):
        self.guide = load(GUIDE_RUN / "guide.json")
        self.skeleton = compile_guide_skeleton(self.guide, RECIPE, "guide-hash")

    def test_accepts_valid_skeleton(self):
        self.assertTrue(verify_guide_skeleton(self.guide, RECIPE, self.skeleton, "guide-hash")["passed"])

    def test_rejects_landmark_drift(self):
        broken = json.loads(json.dumps(self.skeleton))
        broken["joints"][4]["position"][0] += 0.1
        report = verify_guide_skeleton(self.guide, RECIPE, broken, "guide-hash")
        self.assertIn("GUIDE-SKELETON-POSITION-1", {failure["rule"] for failure in report["failures"]})

    def test_rejects_parent_change(self):
        broken = json.loads(json.dumps(self.skeleton))
        broken["joints"][-1]["parent"] = "pelvis"
        report = verify_guide_skeleton(self.guide, RECIPE, broken, "guide-hash")
        self.assertIn("GUIDE-SKELETON-HIERARCHY-1", {failure["rule"] for failure in report["failures"]})

    def test_rejects_nonorthogonal_frame(self):
        broken = json.loads(json.dumps(self.skeleton))
        broken["joints"][0]["basis"]["up"] = broken["joints"][0]["basis"]["aim"]
        report = verify_guide_skeleton(self.guide, RECIPE, broken, "guide-hash")
        self.assertIn("GUIDE-SKELETON-FRAME-1", {failure["rule"] for failure in report["failures"]})

    def test_rejects_guide_provenance_drift(self):
        report = verify_guide_skeleton(self.guide, RECIPE, self.skeleton, "other-hash")
        self.assertIn("GUIDE-SKELETON-PROVENANCE-1", {failure["rule"] for failure in report["failures"]})


if __name__ == "__main__":
    unittest.main()
