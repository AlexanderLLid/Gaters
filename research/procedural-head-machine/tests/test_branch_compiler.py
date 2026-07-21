import json
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from branch_compiler import compile_parent, compose_mirrored_branches


def load(name):
    return json.loads((ROOT / "recipes" / name).read_text(encoding="utf-8"))


class BranchCompilerTests(unittest.TestCase):
    def setUp(self):
        self.parent_recipe = load("torso-branch-parent.json")
        self.branch_recipe = load("torso-mirrored-branches.json")
        self.parent = compile_parent(self.parent_recipe)
        self.composed = compose_mirrored_branches(self.parent, self.branch_recipe)

    def test_parent_emits_mirrored_local_frames(self):
        left, right = self.parent["sockets"]
        self.assertEqual([-value for value in right["origin"]], left["origin"])
        self.assertEqual([-1.0, 0.0, 0.0], left["tangent"])
        self.assertEqual([1.0, 0.0, 0.0], right["tangent"])

    def test_composition_reuses_parent_interfaces(self):
        self.assertEqual(self.parent["vertices"], self.composed["vertices"][:8])
        self.assertEqual(self.parent["faces"], self.composed["faces"][:4])
        self.assertEqual([0, 1, 2, 3], self.composed["socket"]["interfaces"]["left"])
        self.assertEqual([4, 5, 6, 7], self.composed["socket"]["interfaces"]["right"])

    def test_branches_are_exact_mirrors(self):
        for left_id, right_id in self.composed["socket"]["mirror_pairs"]:
            left = self.composed["vertices"][left_id]
            right = self.composed["vertices"][right_id]
            self.assertEqual(left, [-right[0], right[1], right[2]])

    def test_baseline_topology_is_stable(self):
        self.assertEqual(34, len(self.composed["vertices"]))
        self.assertEqual(36, len(self.composed["faces"]))

    def test_held_out_recipe_changes_topology_and_shape_deliberately(self):
        parent = compile_parent(load("torso-branch-parent-held-out.json"))
        composed = compose_mirrored_branches(parent, load("torso-mirrored-branches-held-out.json"))
        self.assertEqual(42, len(composed["vertices"]))
        self.assertEqual(44, len(composed["faces"]))
        self.assertGreater(max(vertex[0] for vertex in composed["vertices"]), max(vertex[0] for vertex in self.composed["vertices"]))


if __name__ == "__main__":
    unittest.main()
