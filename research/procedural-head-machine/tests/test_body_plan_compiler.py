import json
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from body_plan_compiler import compile_body_plan


def load(name):
    return json.loads((ROOT / "recipes" / name).read_text(encoding="utf-8"))


class BodyPlanCompilerTests(unittest.TestCase):
    def setUp(self):
        self.plan = load("stick-humanoid-body-plan.json")
        self.body = compile_body_plan(self.plan)

    def test_compiles_all_declared_modules_and_connections(self):
        self.assertEqual({node["id"] for node in self.plan["nodes"]}, set(self.body["placements"]))
        self.assertEqual(10, len(self.body["connections"]))
        self.assertTrue(all(connection["contact_faces"] > 0 for connection in self.body["connections"]))

    def test_complete_humanoid_declares_terminal_anatomy(self):
        required = {
            "torso", "neck", "head",
            "left_arm", "right_arm", "left_hand", "right_hand",
            "left_leg", "right_leg", "left_foot", "right_foot",
        }
        self.assertEqual(required, set(self.body["placements"]))

    def test_output_is_one_connected_boundary_with_semantics(self):
        self.assertEqual("body-plan-mesh/0", self.body["schema"])
        self.assertEqual(len(self.body["vertices"]), len(self.body["vertex_modules"]))
        self.assertEqual(len(self.body["vertices"]), len(self.body["regions"]))
        self.assertEqual(len(self.body["faces"]), len(self.body["face_modules"]))

    def test_declared_pairs_are_geometric_mirrors(self):
        placements = self.body["placements"]
        for left, right in (
            ("left_arm", "right_arm"),
            ("left_hand", "right_hand"),
            ("left_leg", "right_leg"),
            ("left_foot", "right_foot"),
        ):
            left_bounds = placements[left]["bounds_cells"]
            right_bounds = placements[right]["bounds_cells"]
            self.assertEqual(left_bounds[0], [-right_bounds[1][0], right_bounds[0][1], right_bounds[0][2]])
            self.assertEqual(left_bounds[1], [-right_bounds[0][0], right_bounds[1][1], right_bounds[1][2]])

    def test_held_out_plan_changes_proportions_without_code_change(self):
        tall = compile_body_plan(load("stick-humanoid-body-plan-held-out.json"))
        self.assertLess(min(vertex[2] for vertex in tall["vertices"]), min(vertex[2] for vertex in self.body["vertices"]))
        self.assertGreater(max(vertex[0] for vertex in tall["vertices"]), max(vertex[0] for vertex in self.body["vertices"]))

    def test_rejects_overlap(self):
        broken = json.loads(json.dumps(self.plan))
        arm = next(node for node in broken["nodes"] if node["id"] == "left_arm")
        arm["attach"] = "+z"
        arm["size_cells"] = [4, 2, 3]
        arm["offset_cells"] = [0, 0]
        with self.assertRaisesRegex(ValueError, "BODY-PLAN-OVERLAP-1"):
            compile_body_plan(broken)


if __name__ == "__main__":
    unittest.main()
