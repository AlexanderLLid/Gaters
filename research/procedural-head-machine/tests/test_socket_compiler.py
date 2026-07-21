import json
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from socket_compiler import compose_head_neck


SOURCE = ROOT / "Runs" / "baseline-20260721-150140-151922" / "run-1"


def load_json(path):
    return json.loads(path.read_text(encoding="utf-8"))


class SocketCompilerTests(unittest.TestCase):
    def setUp(self):
        self.mesh = load_json(SOURCE / "mesh.json")
        self.regions = load_json(SOURCE / "regions.json")
        self.socket = load_json(ROOT / "recipes" / "head-neck-socket.json")

    def test_composition_preserves_head_prefix_and_reuses_interface(self):
        composed = compose_head_neck(self.mesh, self.regions, self.socket)
        interface_end = max(self.socket["interface_vertices"])
        preserved_faces = [
            face for face in self.mesh["faces"]
            if all(vertex <= interface_end for vertex in face)
        ]

        self.assertEqual(self.mesh["vertices"][: interface_end + 1], composed["vertices"][: interface_end + 1])
        self.assertEqual(preserved_faces, composed["faces"][: len(preserved_faces)])
        self.assertEqual(self.socket["interface_vertices"], composed["socket"]["interface_vertices"])
        self.assertEqual(list(range(145, 161)), composed["socket"]["first_neck_ring"])

    def test_composition_has_expected_stable_topology(self):
        composed = compose_head_neck(self.mesh, self.regions, self.socket)
        self.assertEqual("composed-character-mesh/0", composed["schema"])
        self.assertEqual(210, len(composed["vertices"]))
        self.assertEqual(224, len(composed["faces"]))
        self.assertEqual(len(composed["vertices"]), len(composed["vertex_modules"]))
        self.assertEqual(len(composed["faces"]), len(composed["face_modules"]))

    def test_held_out_long_neck_changes_shape_without_topology_change(self):
        baseline = compose_head_neck(self.mesh, self.regions, self.socket)
        long_socket = load_json(ROOT / "recipes" / "head-neck-socket-long.json")
        challenger = compose_head_neck(self.mesh, self.regions, long_socket)

        self.assertEqual(baseline["faces"], challenger["faces"])
        self.assertEqual(len(baseline["vertices"]), len(challenger["vertices"]))
        self.assertLess(min(vertex[2] for vertex in challenger["vertices"]), min(vertex[2] for vertex in baseline["vertices"]))


if __name__ == "__main__":
    unittest.main()
