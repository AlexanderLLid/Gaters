import copy
import json
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from head_compiler import compile_recipe
from head_verifier import verify


def load_baseline():
    recipe = json.loads((ROOT / "recipes" / "baseline.json").read_text(encoding="utf-8"))
    return recipe, compile_recipe(recipe)


class HeadVerifierTests(unittest.TestCase):
    def test_accepts_valid_head(self):
        recipe, mesh = load_baseline()
        report = verify(recipe, mesh)
        self.assertTrue(report["passed"])
        self.assertEqual([], report["failures"])

    def test_rejects_nonfinite_coordinate(self):
        recipe, mesh = load_baseline()
        mesh["vertices"][4][0] = float("nan")
        report = verify(recipe, mesh)
        self.assertIn("HEAD-FINITE-1", [failure["rule"] for failure in report["failures"]])

    def test_rejects_open_surface(self):
        recipe, mesh = load_baseline()
        mesh["faces"].pop()
        report = verify(recipe, mesh)
        self.assertIn("HEAD-MANIFOLD-1", [failure["rule"] for failure in report["failures"]])

    def test_rejects_invalid_face_index(self):
        recipe, mesh = load_baseline()
        mesh["faces"][0][1] = len(mesh["vertices"])
        report = verify(recipe, mesh)
        self.assertIn("HEAD-FACE-1", [failure["rule"] for failure in report["failures"]])

    def test_rejects_unnormalized_regions(self):
        recipe, mesh = load_baseline()
        mesh["regions"][8] = {"skull": 1.0, "face": 1.0, "jaw": 0.0, "chin": 0.0}
        report = verify(recipe, mesh)
        self.assertIn("HEAD-REGION-1", [failure["rule"] for failure in report["failures"]])

    def test_rejects_wrong_dimensions(self):
        recipe, mesh = load_baseline()
        changed = copy.deepcopy(mesh)
        changed["vertices"][0][2] += 0.01
        report = verify(recipe, changed)
        self.assertIn("HEAD-BOUNDS-1", [failure["rule"] for failure in report["failures"]])


if __name__ == "__main__":
    unittest.main()
