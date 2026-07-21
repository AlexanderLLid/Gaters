import json
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from run_anatomical_guide import run


BODY_RUN = ROOT / "BodyPlanRuns" / "stick-humanoid-20260721-162041-004309" / "run-1"


class RunAnatomicalGuideTests(unittest.TestCase):
    def test_two_runs_are_identical_verified_and_visible(self):
        with tempfile.TemporaryDirectory() as temporary:
            summary = run(BODY_RUN, ROOT / "recipes" / "humanoid-anatomical-guide.json", Path(temporary))
            self.assertTrue(summary["passed"])
            self.assertTrue(Path(summary["preview"]).is_file())
            first, second = [Path(path) for path in summary["runs"]]
            self.assertEqual(
                json.loads((first / "receipt.json").read_text())["guide_sha256"],
                json.loads((second / "receipt.json").read_text())["guide_sha256"],
            )


if __name__ == "__main__":
    unittest.main()
