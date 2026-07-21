import json
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from run_socket import run


SOURCE = ROOT / "Runs" / "baseline-20260721-150140-151922" / "run-1"


class RunSocketTests(unittest.TestCase):
    def test_two_runs_produce_identical_verified_compositions(self):
        with tempfile.TemporaryDirectory() as temporary:
            summary = run(
                SOURCE,
                ROOT / "recipes" / "head-neck-socket.json",
                Path(temporary),
                repeat=2,
            )
            self.assertTrue(summary["passed"])
            receipts = [
                json.loads((Path(path) / "receipt.json").read_text(encoding="utf-8"))
                for path in summary["runs"]
            ]
            self.assertEqual(receipts[0]["composition_sha256"], receipts[1]["composition_sha256"])
            self.assertEqual(receipts[0]["topology_sha256"], receipts[1]["topology_sha256"])
            self.assertTrue((Path(summary["runs"][0]) / "head-neck.obj").is_file())


if __name__ == "__main__":
    unittest.main()
