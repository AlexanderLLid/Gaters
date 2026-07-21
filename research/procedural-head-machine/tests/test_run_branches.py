import json
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from run_branches import run


class RunBranchesTests(unittest.TestCase):
    def test_two_runs_produce_identical_verified_compositions(self):
        with tempfile.TemporaryDirectory() as temporary:
            summary = run(
                ROOT / "recipes" / "torso-branch-parent.json",
                ROOT / "recipes" / "torso-mirrored-branches.json",
                Path(temporary),
            )
            self.assertTrue(summary["passed"])
            first, second = [Path(path) for path in summary["runs"]]
            self.assertEqual(
                json.loads((first / "receipt.json").read_text())["composition_sha256"],
                json.loads((second / "receipt.json").read_text())["composition_sha256"],
            )
            self.assertTrue(json.loads((first / "verification.json").read_text())["passed"])


if __name__ == "__main__":
    unittest.main()
