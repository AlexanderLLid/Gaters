import copy
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from compile_graph import compile_recipe, load_recipe
from verify_graph import verify


class VerifyGraphTests(unittest.TestCase):
    def setUp(self):
        self.recipe = load_recipe(ROOT / "recipes" / "winged-reptile.json")
        self.graph = compile_recipe(self.recipe)

    def rules(self, graph=None, recipe=None):
        report = verify(recipe or self.recipe, graph or self.graph)
        return report, {failure["rule"] for failure in report["failures"]}

    def test_valid_compiler_output_passes(self):
        report, rules = self.rules()

        self.assertTrue(report["passed"])
        self.assertEqual(rules, set())
        self.assertGreater(report["metrics"]["bone_count"], 0)

    def test_missing_joint_reference_fails_parent_rule(self):
        graph = copy.deepcopy(self.graph)
        graph["bones"][0]["parent_joint"] = "absent.joint"

        report, rules = self.rules(graph)

        self.assertFalse(report["passed"])
        self.assertIn("GRAPH-PARENT-1", rules)

    def test_zero_length_bone_fails_length_rule(self):
        graph = copy.deepcopy(self.graph)
        bone = graph["bones"][0]
        joints = {joint["id"]: joint for joint in graph["joints"]}
        joints[bone["child_joint"]]["position_m"] = list(joints[bone["parent_joint"]]["position_m"])

        report, rules = self.rules(graph)

        self.assertFalse(report["passed"])
        self.assertIn("GRAPH-LENGTH-1", rules)

    def test_removed_module_fails_count_rule(self):
        graph = copy.deepcopy(self.graph)
        graph["modules"] = [module for module in graph["modules"] if module["id"] != "legs.3"]

        report, rules = self.rules(graph)

        self.assertFalse(report["passed"])
        self.assertIn("GRAPH-COUNT-1", rules)

    def test_broken_head_taper_fails_taper_rule(self):
        graph = copy.deepcopy(self.graph)
        joints = {joint["id"]: joint for joint in graph["joints"]}
        joints["head.tip"]["radius_m"] = joints["head.base"]["radius_m"]

        report, rules = self.rules(graph)

        self.assertFalse(report["passed"])
        self.assertIn("GRAPH-TAPER-1", rules)

    def test_broken_pair_mirror_fails_mirror_rule(self):
        graph = copy.deepcopy(self.graph)
        joint = next(joint for joint in graph["joints"] if joint["id"] == "legs.0.knee")
        joint["position_m"][1] += 0.3

        report, rules = self.rules(graph)

        self.assertFalse(report["passed"])
        self.assertIn("GRAPH-MIRROR-1", rules)

    def test_disconnected_appendage_fails_connectivity_rule(self):
        graph = copy.deepcopy(self.graph)
        graph["bones"] = [bone for bone in graph["bones"] if bone["id"] != "legs.0.hip"]

        report, rules = self.rules(graph)

        self.assertFalse(report["passed"])
        self.assertIn("GRAPH-CONNECT-1", rules)


if __name__ == "__main__":
    unittest.main()
