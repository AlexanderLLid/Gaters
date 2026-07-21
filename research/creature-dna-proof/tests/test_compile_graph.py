import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from compile_graph import canonical_bytes, compile_recipe, graph_sha256, load_recipe


class CompileGraphTests(unittest.TestCase):
    def compile(self, name):
        return compile_recipe(load_recipe(ROOT / "recipes" / name))

    def test_winged_reptile_realizes_requested_appendages(self):
        graph = self.compile("winged-reptile.json")
        types = [module["type"] for module in graph["modules"]]

        self.assertEqual(types.count("leg"), 4)
        self.assertEqual(types.count("wing"), 2)
        self.assertEqual(types.count("torso"), 1)
        self.assertEqual(types.count("tapered_head"), 1)
        self.assertEqual(types.count("tail"), 1)

    def test_explicit_layout_supports_odd_leg_count_without_wings(self):
        graph = self.compile("five-legged-challenge.json")
        legs = [module for module in graph["modules"] if module["type"] == "leg"]

        self.assertEqual(len(legs), 5)
        self.assertFalse(any(module["type"] == "wing" for module in graph["modules"]))
        self.assertEqual(
            {module["attachment_id"] for module in legs},
            {"rear_left", "front_left", "rear_right", "front_right", "center"},
        )

    def test_graph_has_unique_names_and_nonzero_bones(self):
        graph = self.compile("winged-reptile.json")
        joint_ids = [joint["id"] for joint in graph["joints"]]
        bone_ids = [bone["id"] for bone in graph["bones"]]

        self.assertEqual(len(joint_ids), len(set(joint_ids)))
        self.assertEqual(len(bone_ids), len(set(bone_ids)))
        self.assertTrue(all(bone["parent_joint"] != bone["child_joint"] for bone in graph["bones"]))

    def test_canonical_hash_ignores_toolchain_metadata(self):
        first = self.compile("winged-reptile.json")
        second = self.compile("winged-reptile.json")
        first["toolchain"]["houdini"] = "21.0.1"
        second["toolchain"]["houdini"] = "21.0.2"

        self.assertEqual(canonical_bytes(first), canonical_bytes(second))
        self.assertEqual(graph_sha256(first), graph_sha256(second))

    def test_odd_bilateral_count_is_rejected(self):
        recipe = load_recipe(ROOT / "recipes" / "winged-reptile.json")
        next(module for module in recipe["modules"] if module["type"] == "leg")["count"] = 5

        with self.assertRaisesRegex(ValueError, "bilateral count must be even"):
            compile_recipe(recipe)


if __name__ == "__main__":
    unittest.main()
