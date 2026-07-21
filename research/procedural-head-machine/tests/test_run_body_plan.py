import json
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from run_body_plan import run


class RunBodyPlanTests(unittest.TestCase):
    def test_two_runs_are_identical_and_verified(self):
        with tempfile.TemporaryDirectory() as temporary:
            summary = run(ROOT / "recipes" / "stick-humanoid-body-plan.json", Path(temporary))
            self.assertTrue(summary["passed"])
            first, second = [Path(path) for path in summary["runs"]]
            first_receipt = json.loads((first / "receipt.json").read_text())
            second_receipt = json.loads((second / "receipt.json").read_text())
            self.assertEqual(first_receipt["composition_sha256"], second_receipt["composition_sha256"])
            self.assertTrue(json.loads((first / "verification.json").read_text())["passed"])
            self.assertTrue(Path(summary["preview"]).is_file())


if __name__ == "__main__":
    unittest.main()
