import copy
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from socket_adapter_verifier import verify_socket_adapter


COMPOSITION = {
    "vertices": [[0.0, 0.0, 0.0], [1.0, 0.0, 0.0], [0.0, 1.0, 0.0]],
    "faces": [[0, 1, 2]],
    "regions": [
        {"head": 1.0, "neck": 0.0},
        {"head": 1.0, "neck": 0.0},
        {"head": 0.0, "neck": 1.0},
    ],
    "vertex_modules": ["head", "head", "neck"],
    "face_modules": ["head-neck"],
    "socket": {"interface_vertices": [0, 1], "first_neck_ring": [1, 2]},
}


class SocketAdapterVerifierTests(unittest.TestCase):
    def readback(self):
        return {
            "vertices": copy.deepcopy(COMPOSITION["vertices"]),
            "faces": copy.deepcopy(COMPOSITION["faces"]),
            "regions": copy.deepcopy(COMPOSITION["regions"]),
            "vertex_modules": copy.deepcopy(COMPOSITION["vertex_modules"]),
            "face_modules": copy.deepcopy(COMPOSITION["face_modules"]),
            "socket": copy.deepcopy(COMPOSITION["socket"]),
        }

    def test_accepts_exact_materialization(self):
        self.assertTrue(verify_socket_adapter(COMPOSITION, self.readback())["passed"])

    def test_accepts_position_rounding_within_tolerance(self):
        readback = self.readback()
        readback["vertices"][1][0] += 1e-8
        self.assertTrue(verify_socket_adapter(COMPOSITION, readback)["passed"])

    def test_rejects_topology_change(self):
        readback = self.readback()
        readback["faces"][0] = [0, 2, 1]
        report = verify_socket_adapter(COMPOSITION, readback)
        self.assertIn("SOCKET-ADAPTER-TOPOLOGY-1", [failure["rule"] for failure in report["failures"]])

    def test_rejects_semantic_change(self):
        readback = self.readback()
        readback["vertex_modules"][2] = "head"
        report = verify_socket_adapter(COMPOSITION, readback)
        self.assertIn("SOCKET-ADAPTER-SEMANTICS-1", [failure["rule"] for failure in report["failures"]])

    def test_rejects_socket_metadata_change(self):
        readback = self.readback()
        readback["socket"]["interface_vertices"] = [1, 2]
        report = verify_socket_adapter(COMPOSITION, readback)
        self.assertIn("SOCKET-ADAPTER-METADATA-1", [failure["rule"] for failure in report["failures"]])


if __name__ == "__main__":
    unittest.main()
