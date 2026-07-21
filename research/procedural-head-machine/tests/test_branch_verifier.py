import copy
import json
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from branch_compiler import compile_parent, compose_mirrored_branches
from branch_verifier import verify_mirrored_branches


def load(name):
    return json.loads((ROOT / "recipes" / name).read_text(encoding="utf-8"))


class BranchVerifierTests(unittest.TestCase):
    def setUp(self):
        self.recipe = load("torso-mirrored-branches.json")
        self.parent = compile_parent(load("torso-branch-parent.json"))
        self.composed = compose_mirrored_branches(self.parent, self.recipe)

    def rules(self, composed=None, recipe=None):
        report = verify_mirrored_branches(self.parent, recipe or self.recipe, composed or self.composed)
        return report, [failure["rule"] for failure in report["failures"]]

    def test_accepts_valid_mirrored_branches(self):
        report, _ = self.rules()
        self.assertTrue(report["passed"])
        self.assertEqual(0.0, report["metrics"]["max_mirror_error_m"])

    def test_rejects_open_surface(self):
        broken = copy.deepcopy(self.composed)
        broken["faces"].pop()
        _, rules = self.rules(broken)
        self.assertIn("BRANCH-MANIFOLD-1", rules)

    def test_rejects_asymmetric_branch(self):
        broken = copy.deepcopy(self.composed)
        right_id = broken["socket"]["mirror_pairs"][-2][1]
        broken["vertices"][right_id][1] += 0.01
        _, rules = self.rules(broken)
        self.assertIn("BRANCH-MIRROR-1", rules)

    def test_rejects_nonorthogonal_frame(self):
        broken = copy.deepcopy(self.composed)
        broken["socket"]["frames"]["right"]["u"] = [1.0, 1.0, 0.0]
        _, rules = self.rules(broken)
        self.assertIn("BRANCH-FRAME-1", rules)

    def test_rejects_interface_substitution(self):
        broken = copy.deepcopy(self.composed)
        broken["socket"]["interfaces"]["left"][0] = broken["socket"]["branch_rings"]["left"][0][0]
        _, rules = self.rules(broken)
        self.assertIn("BRANCH-SEAM-1", rules)

    def test_rejects_wrong_realized_length(self):
        wrong_recipe = {**self.recipe, "branch_length_m": 0.9}
        _, rules = self.rules(recipe=wrong_recipe)
        self.assertIn("BRANCH-LENGTH-1", rules)


if __name__ == "__main__":
    unittest.main()
