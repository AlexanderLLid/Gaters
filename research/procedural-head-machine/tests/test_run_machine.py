import json
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from run_machine import run


class RunMachineTests(unittest.TestCase):
    def test_two_builds_preserve_identical_artifacts_and_receipts(self):
        with tempfile.TemporaryDirectory() as temporary:
            summary = run(ROOT / "recipes" / "baseline.json", Path(temporary), repeat=2)
            self.assertTrue(summary["passed"])
            self.assertEqual(2, len(summary["runs"]))

            receipts = [
                json.loads((Path(path) / "receipt.json").read_text(encoding="utf-8"))
                for path in summary["runs"]
            ]
            self.assertEqual(receipts[0]["mesh_sha256"], receipts[1]["mesh_sha256"])
            self.assertEqual(receipts[0]["topology_sha256"], receipts[1]["topology_sha256"])
            for path in map(Path, summary["runs"]):
                self.assertTrue((path / "recipe.json").is_file())
                self.assertTrue((path / "mesh.json").is_file())
                self.assertTrue((path / "regions.json").is_file())
                self.assertTrue((path / "verification.json").is_file())
                self.assertTrue((path / "head.obj").is_file())


if __name__ == "__main__":
    unittest.main()
