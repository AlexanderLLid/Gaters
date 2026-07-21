import copy
import json
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from socket_compiler import compose_head_neck
from socket_verifier import verify_socket


SOURCE = ROOT / "Runs" / "baseline-20260721-150140-151922" / "run-1"


def load_json(path):
    return json.loads(path.read_text(encoding="utf-8"))


class SocketVerifierTests(unittest.TestCase):
    def setUp(self):
        self.mesh = load_json(SOURCE / "mesh.json")
        self.regions = load_json(SOURCE / "regions.json")
        self.socket = load_json(ROOT / "recipes" / "head-neck-socket.json")
        self.composed = compose_head_neck(self.mesh, self.regions, self.socket)

    def rules(self, candidate):
        return [failure["rule"] for failure in verify_socket(self.mesh, self.socket, candidate)["failures"]]

    def test_accepts_shared_manifold_tangent_continuous_socket(self):
        report = verify_socket(self.mesh, self.socket, self.composed)
        self.assertTrue(report["passed"])

    def test_rejects_changed_upstream_head(self):
        candidate = copy.deepcopy(self.composed)
        candidate["vertices"][10][0] += 0.01
        self.assertIn("SOCKET-PRESERVE-1", self.rules(candidate))

    def test_rejects_open_surface(self):
        candidate = copy.deepcopy(self.composed)
        candidate["faces"].pop()
        candidate["face_modules"].pop()
        self.assertIn("SOCKET-MANIFOLD-1", self.rules(candidate))

    def test_rejects_duplicate_seam_ring(self):
        candidate = copy.deepcopy(self.composed)
        interface = candidate["socket"]["interface_vertices"]
        first = candidate["socket"]["first_neck_ring"]
        for source_id, target_id in zip(interface, first):
            candidate["vertices"][target_id] = list(candidate["vertices"][source_id])
        self.assertIn("SOCKET-SEAM-1", self.rules(candidate))

    def test_rejects_tangent_break(self):
        candidate = copy.deepcopy(self.composed)
        first_id = candidate["socket"]["first_neck_ring"][0]
        candidate["vertices"][first_id][1] += 0.01
        self.assertIn("SOCKET-TANGENT-1", self.rules(candidate))


if __name__ == "__main__":
    unittest.main()
