import copy
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from adapter_verifier import verify_adapter


SOURCE_MESH = {
    "vertices": [[0.0, 0.0, 0.0], [1.0, 0.0, 0.0], [0.0, 1.0, 0.0]],
    "faces": [[0, 1, 2]],
}
SOURCE_REGIONS = {
    "weights": [
        {"skull": 1.0, "face": 0.0, "jaw": 0.0, "chin": 0.0},
        {"skull": 0.0, "face": 1.0, "jaw": 0.0, "chin": 0.0},
        {"skull": 0.0, "face": 0.0, "jaw": 1.0, "chin": 0.0},
    ]
}


class AdapterVerifierTests(unittest.TestCase):
    def test_accepts_float_rounding_within_tolerance(self):
        readback = {**copy.deepcopy(SOURCE_MESH), "regions": copy.deepcopy(SOURCE_REGIONS["weights"])}
        readback["vertices"][1][0] += 1e-8
        report = verify_adapter(SOURCE_MESH, SOURCE_REGIONS, readback)
        self.assertTrue(report["passed"])

    def test_rejects_topology_change(self):
        readback = {**copy.deepcopy(SOURCE_MESH), "regions": copy.deepcopy(SOURCE_REGIONS["weights"])}
        readback["faces"][0] = [0, 2, 1]
        report = verify_adapter(SOURCE_MESH, SOURCE_REGIONS, readback)
        self.assertIn("ADAPTER-TOPOLOGY-1", [failure["rule"] for failure in report["failures"]])

    def test_rejects_position_drift(self):
        readback = {**copy.deepcopy(SOURCE_MESH), "regions": copy.deepcopy(SOURCE_REGIONS["weights"])}
        readback["vertices"][1][0] += 1e-4
        report = verify_adapter(SOURCE_MESH, SOURCE_REGIONS, readback)
        self.assertIn("ADAPTER-POSITION-1", [failure["rule"] for failure in report["failures"]])

    def test_rejects_region_drift(self):
        readback = {**copy.deepcopy(SOURCE_MESH), "regions": copy.deepcopy(SOURCE_REGIONS["weights"])}
        readback["regions"][0]["skull"] = 0.5
        report = verify_adapter(SOURCE_MESH, SOURCE_REGIONS, readback)
        self.assertIn("ADAPTER-REGION-1", [failure["rule"] for failure in report["failures"]])


if __name__ == "__main__":
    unittest.main()
