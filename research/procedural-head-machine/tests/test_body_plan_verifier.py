import copy
import json
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from body_plan_compiler import compile_body_plan
from body_plan_verifier import verify_body_plan


PLAN = json.loads((ROOT / "recipes" / "stick-humanoid-body-plan.json").read_text(encoding="utf-8"))


class BodyPlanVerifierTests(unittest.TestCase):
    def setUp(self):
        self.body = compile_body_plan(PLAN)

    def rules(self, body=None):
        report = verify_body_plan(PLAN, body or self.body)
        return report, [failure["rule"] for failure in report["failures"]]

    def test_accepts_connected_watertight_body_plan(self):
        report, _ = self.rules()
        self.assertTrue(report["passed"])
        self.assertEqual(1, report["metrics"]["connected_components"])
        self.assertEqual(0.0, report["metrics"]["max_mirror_error_cells"])

    def test_rejects_open_boundary(self):
        broken = copy.deepcopy(self.body)
        broken["faces"].pop()
        broken["face_modules"].pop()
        _, rules = self.rules(broken)
        self.assertIn("BODY-PLAN-MANIFOLD-1", rules)

    def test_rejects_disconnected_cell_graph(self):
        broken = copy.deepcopy(self.body)
        head_cells = [cell for cell in broken["cells"] if cell["module"] == "head"]
        for cell in head_cells:
            cell["position"][2] += 10
        _, rules = self.rules(broken)
        self.assertIn("BODY-PLAN-CONNECTIVITY-1", rules)

    def test_rejects_asymmetric_mirror_pair(self):
        broken = copy.deepcopy(self.body)
        right_arm = next(cell for cell in broken["cells"] if cell["module"] == "right_arm")
        right_arm["position"][1] += 1
        _, rules = self.rules(broken)
        self.assertIn("BODY-PLAN-MIRROR-1", rules)

    def test_rejects_false_contact_receipt(self):
        broken = copy.deepcopy(self.body)
        broken["connections"][0]["contact_faces"] += 1
        _, rules = self.rules(broken)
        self.assertIn("BODY-PLAN-CONTACT-1", rules)

    def test_rejects_missing_semantic_module(self):
        broken = copy.deepcopy(self.body)
        broken["face_modules"][0] = "unknown"
        _, rules = self.rules(broken)
        self.assertIn("BODY-PLAN-SEMANTIC-1", rules)


if __name__ == "__main__":
    unittest.main()
