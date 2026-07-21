import json
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from run_proof import run_recipe


class RunProofTests(unittest.TestCase):
    def test_run_preserves_recipe_graph_report_and_obj_guide(self):
        with tempfile.TemporaryDirectory() as directory:
            result = run_recipe(ROOT / "recipes" / "winged-reptile.json", Path(directory))
            output = Path(directory)

            self.assertTrue(result["passed"])
            self.assertRegex(result["graph_sha256"], r"^[0-9a-f]{64}$")
            self.assertEqual(
                {path.name for path in output.iterdir()},
                {"recipe.json", "anatomy-graph.json", "verification.json", "guide.obj", "run.json"},
            )
            report = json.loads((output / "verification.json").read_text(encoding="utf-8"))
            self.assertTrue(report["passed"])

            obj_lines = (output / "guide.obj").read_text(encoding="utf-8").splitlines()
            graph = json.loads((output / "anatomy-graph.json").read_text(encoding="utf-8"))
            self.assertEqual(sum(line.startswith("v ") for line in obj_lines), len(graph["joints"]))
            self.assertEqual(sum(line.startswith("l ") for line in obj_lines), len(graph["bones"]))

    def test_same_recipe_produces_same_graph_hash_in_separate_outputs(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            first = run_recipe(ROOT / "recipes" / "winged-reptile.json", root / "first")
            second = run_recipe(ROOT / "recipes" / "winged-reptile.json", root / "second")

            self.assertEqual(first["graph_sha256"], second["graph_sha256"])
            self.assertEqual(
                (root / "first" / "anatomy-graph.json").read_bytes(),
                (root / "second" / "anatomy-graph.json").read_bytes(),
            )


if __name__ == "__main__":
    unittest.main()
