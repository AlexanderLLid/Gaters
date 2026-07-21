import json
import math
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from head_compiler import compile_recipe


def load_recipe(name):
    return json.loads((ROOT / "recipes" / name).read_text(encoding="utf-8"))


class HeadCompilerTests(unittest.TestCase):
    def test_baseline_has_stable_closed_topology_and_regions(self):
        recipe = load_recipe("baseline.json")
        mesh = compile_recipe(recipe)

        self.assertEqual("head-mesh/0", mesh["schema"])
        self.assertEqual(2 + (recipe["rings"] - 1) * recipe["segments"], len(mesh["vertices"]))
        self.assertEqual(2 * recipe["segments"] + (recipe["rings"] - 2) * recipe["segments"], len(mesh["faces"]))

        for face in mesh["faces"]:
            self.assertIn(len(face), (3, 4))
            self.assertEqual(len(face), len(set(face)))
            self.assertTrue(all(0 <= index < len(mesh["vertices"]) for index in face))

        self.assertEqual(len(mesh["vertices"]), len(mesh["regions"]))
        for weights in mesh["regions"]:
            self.assertEqual({"skull", "face", "jaw", "chin"}, set(weights))
            self.assertTrue(all(math.isfinite(value) and 0.0 <= value <= 1.0 for value in weights.values()))
            self.assertAlmostEqual(1.0, sum(weights.values()), places=9)

    def test_declared_dimensions_are_realized(self):
        recipe = load_recipe("baseline.json")
        mesh = compile_recipe(recipe)
        axes = list(zip(*mesh["vertices"]))
        realized = [max(axis) - min(axis) for axis in axes]

        self.assertAlmostEqual(recipe["width_m"], realized[0], places=9)
        self.assertAlmostEqual(recipe["depth_m"], realized[1], places=9)
        self.assertAlmostEqual(recipe["height_m"], realized[2], places=9)

    def test_broad_jaw_changes_only_lower_shape_not_topology(self):
        baseline = compile_recipe(load_recipe("baseline.json"))
        broad = compile_recipe(load_recipe("broad-jaw.json"))

        self.assertEqual(baseline["faces"], broad["faces"])
        self.assertEqual(len(baseline["vertices"]), len(broad["vertices"]))
        jaw_ids = [
            index for index, weights in enumerate(baseline["regions"])
            if weights["jaw"] == max(weights.values())
        ]
        skull_ids = [
            index for index, weights in enumerate(baseline["regions"])
            if weights["skull"] == max(weights.values())
        ]
        baseline_jaw_width = max(abs(baseline["vertices"][index][0]) for index in jaw_ids)
        broad_jaw_width = max(abs(broad["vertices"][index][0]) for index in jaw_ids)

        self.assertGreater(broad_jaw_width, baseline_jaw_width)
        self.assertEqual(
            [baseline["vertices"][index] for index in skull_ids],
            [broad["vertices"][index] for index in skull_ids],
        )

    def test_rejects_jaw_wider_than_declared_envelope(self):
        recipe = load_recipe("baseline.json")
        recipe["jaw_width"] = 1.01
        with self.assertRaisesRegex(ValueError, "HEAD-INPUT-1"):
            compile_recipe(recipe)


if __name__ == "__main__":
    unittest.main()
