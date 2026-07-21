import copy
import json
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from anatomical_guide_compiler import compile_anatomical_guide
from anatomical_guide_verifier import verify_anatomical_guide


BODY_RUN = ROOT / "BodyPlanRuns" / "stick-humanoid-20260721-162041-004309" / "run-1"
BODY = json.loads((BODY_RUN / "composed-mesh.json").read_text(encoding="utf-8"))
RECIPE = json.loads((ROOT / "recipes" / "humanoid-anatomical-guide.json").read_text(encoding="utf-8"))


class AnatomicalGuideVerifierTests(unittest.TestCase):
    def setUp(self):
        self.guide = compile_anatomical_guide(BODY, RECIPE)

    def rules(self, guide=None):
        report = verify_anatomical_guide(BODY, RECIPE, guide or self.guide)
        return report, [failure["rule"] for failure in report["failures"]]

    def test_accepts_valid_guide(self):
        report, _ = self.rules()
        self.assertTrue(report["passed"])
        self.assertEqual(0.0, report["metrics"]["max_symmetry_error_m"])

    def test_rejects_asymmetric_landmark(self):
        broken = copy.deepcopy(self.guide)
        broken["landmarks"]["right_elbow"][2] += 0.02
        _, rules = self.rules(broken)
        self.assertIn("ANATOMY-SYMMETRY-1", rules)

    def test_rejects_joint_order_reversal(self):
        broken = copy.deepcopy(self.guide)
        broken["landmarks"]["left_knee"][2] = broken["landmarks"]["left_ankle"][2] - 0.1
        _, rules = self.rules(broken)
        self.assertIn("ANATOMY-ORDER-1", rules)

    def test_rejects_invalid_radius(self):
        broken = copy.deepcopy(self.guide)
        broken["surface_segments"][0]["end_radius_m"] = -1.0
        _, rules = self.rules(broken)
        self.assertIn("ANATOMY-SURFACE-1", rules)

    def test_rejects_body_provenance_drift(self):
        broken = copy.deepcopy(self.guide)
        broken["body_metadata"]["body_plan_id"] = "wrong"
        _, rules = self.rules(broken)
        self.assertIn("ANATOMY-PROVENANCE-1", rules)


if __name__ == "__main__":
    unittest.main()
