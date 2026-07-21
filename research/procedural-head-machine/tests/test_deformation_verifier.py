import copy
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from deformation_verifier import verify_deformation


MESH = {
    "vertices": [[-1.0, 0.0, -0.5], [1.0, 0.0, -0.5], [0.5, 0.0, 0.8]],
    "faces": [[0, 1, 2]],
}
REGIONS = {
    "weights": [
        {"jaw": 1.0, "skull": 0.0, "face": 0.0, "chin": 0.0},
        {"jaw": 1.0, "skull": 0.0, "face": 0.0, "chin": 0.0},
        {"jaw": 0.0, "skull": 1.0, "face": 0.0, "chin": 0.0},
    ]
}
COMMAND = {
    "schema": "head-deformation/0",
    "id": "widen-jaw",
    "operation": "widen_region",
    "region": "jaw",
    "preserve": "skull",
    "axis": "x",
    "amount": 0.25,
}


def valid_readback():
    return {
        "deformation_id": "widen-jaw",
        "graph": {"operation": "widen_region", "amount": 0.25},
        "vertices": [[-1.25, 0.0, -0.5], [1.25, 0.0, -0.5], [0.5, 0.0, 0.8]],
        "faces": copy.deepcopy(MESH["faces"]),
        "regions": copy.deepcopy(REGIONS["weights"]),
    }


class DeformationVerifierTests(unittest.TestCase):
    def test_accepts_expected_widening_and_protected_skull(self):
        report = verify_deformation(MESH, REGIONS, COMMAND, valid_readback())
        self.assertTrue(report["passed"])

    def test_rejects_missing_deformation(self):
        readback = valid_readback()
        readback["vertices"] = copy.deepcopy(MESH["vertices"])
        report = verify_deformation(MESH, REGIONS, COMMAND, readback)
        self.assertIn("DEFORM-POSITION-1", [failure["rule"] for failure in report["failures"]])

    def test_rejects_moved_protected_skull(self):
        readback = valid_readback()
        readback["vertices"][2][0] += 0.01
        report = verify_deformation(MESH, REGIONS, COMMAND, readback)
        self.assertIn("DEFORM-PRESERVE-1", [failure["rule"] for failure in report["failures"]])

    def test_rejects_topology_change(self):
        readback = valid_readback()
        readback["faces"][0] = [0, 2, 1]
        report = verify_deformation(MESH, REGIONS, COMMAND, readback)
        self.assertIn("DEFORM-TOPOLOGY-1", [failure["rule"] for failure in report["failures"]])

    def test_rejects_wrong_native_graph_parameter(self):
        readback = valid_readback()
        readback["graph"]["amount"] = 0.5
        report = verify_deformation(MESH, REGIONS, COMMAND, readback)
        self.assertIn("DEFORM-GRAPH-1", [failure["rule"] for failure in report["failures"]])


if __name__ == "__main__":
    unittest.main()
