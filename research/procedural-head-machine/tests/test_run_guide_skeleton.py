import json
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from run_guide_skeleton import run


GUIDE_RUN = ROOT / "AnatomyRuns" / "humanoid-guide-stick-humanoid-20260721-174142-274786" / "run-1"
RECIPE = ROOT / "recipes" / "humanoid-guide-skeleton.json"


class RunGuideSkeletonTests(unittest.TestCase):
    def test_two_runs_are_identical_verified_and_visible(self):
        with tempfile.TemporaryDirectory() as temporary:
            summary = run(GUIDE_RUN, RECIPE, Path(temporary))
            self.assertTrue(summary["passed"])
            self.assertEqual(2, len(summary["runs"]))
            self.assertTrue(Path(summary["preview"]).is_file())
            hashes = [json.loads((Path(path) / "receipt.json").read_text())["skeleton_sha256"] for path in summary["runs"]]
            self.assertEqual(1, len(set(hashes)))


if __name__ == "__main__":
    unittest.main()
