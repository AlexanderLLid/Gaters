import copy
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from smooth_body_verifier import verify_smooth_body


BODY = {
    "body_plan_id": "one-module",
    "vertices": [[-1.0, -1.0, -1.0], [1.0, 1.0, 1.0]],
    "placements": {"torso": {"role": "core", "bounds_cells": [[-1, -1, -1], [1, 1, 1]]}},
    "connections": [],
    "cells": [{"position": [0, 0, 0], "module": "torso"}],
    "mirrors": [],
}
RECIPE = {
    "schema": "body-surface/0",
    "id": "test-surface",
    "voxel_size_m": 0.1,
    "smooth_iterations": 2,
    "adaptivity": 0.0,
    "module_overlap_m": 0.0,
    "minimum_non_axial_face_fraction": 0.25,
    "bounds_tolerance_m": 0.4,
    "maximum_faces": 100,
    "target_faces": 80,
}
SURFACE = {
    "schema": "smooth-body-readback/0",
    "vertices": [[1.0, 0.0, -0.7], [-1.0, 0.0, -0.7], [0.0, 1.0, 0.7], [0.0, -1.0, 0.7]],
    "faces": [[0, 2, 1], [0, 1, 3], [0, 3, 2], [1, 2, 3]],
    "regions": [{"core": 1.0, "head": 0.0, "limb": 0.0}] * 4,
    "vertex_modules": ["torso"] * 4,
    "face_modules": ["torso"] * 4,
    "body_metadata": {key: copy.deepcopy(BODY[key]) for key in ("body_plan_id", "placements", "connections", "cells", "mirrors")},
    "native_graph": ["MODULE_PRIMITIVES", "SURFACE_VDB", "SURFACE_SMOOTH", "SURFACE_POLYGONS", "SURFACE_REDUCE", "SURFACE_LABELS", "OUT_SMOOTH_BODY"],
    "parameters": {key: RECIPE[key] for key in ("voxel_size_m", "smooth_iterations", "adaptivity", "module_overlap_m", "target_faces")},
}


class SmoothBodyVerifierTests(unittest.TestCase):
    def rules(self, surface=None):
        report = verify_smooth_body(BODY, RECIPE, surface or SURFACE)
        return report, [failure["rule"] for failure in report["failures"]]

    def test_accepts_closed_non_axial_surface(self):
        report, _ = self.rules()
        self.assertTrue(report["passed"])
        self.assertEqual(1, report["metrics"]["connected_components"])

    def test_rejects_open_surface(self):
        broken = copy.deepcopy(SURFACE)
        broken["faces"].pop()
        broken["face_modules"].pop()
        _, rules = self.rules(broken)
        self.assertIn("SMOOTH-BODY-MANIFOLD-1", rules)

    def test_rejects_metadata_drift(self):
        broken = copy.deepcopy(SURFACE)
        broken["body_metadata"]["body_plan_id"] = "wrong"
        _, rules = self.rules(broken)
        self.assertIn("SMOOTH-BODY-PROVENANCE-1", rules)

    def test_rejects_wrong_native_parameter(self):
        broken = copy.deepcopy(SURFACE)
        broken["parameters"]["smooth_iterations"] = 9
        _, rules = self.rules(broken)
        self.assertIn("SMOOTH-BODY-GRAPH-1", rules)

    def test_rejects_missing_polygon_target_graph(self):
        broken = copy.deepcopy(SURFACE)
        broken["native_graph"].remove("SURFACE_REDUCE")
        _, rules = self.rules(broken)
        self.assertIn("SMOOTH-BODY-GRAPH-1", rules)

    def test_rejects_polygon_target_drift(self):
        broken = copy.deepcopy(SURFACE)
        broken["parameters"]["target_faces"] = 999
        _, rules = self.rules(broken)
        self.assertIn("SMOOTH-BODY-GRAPH-1", rules)

    def test_rejects_missing_module_label(self):
        broken = copy.deepcopy(SURFACE)
        broken["vertex_modules"] = ["unknown"] * 4
        _, rules = self.rules(broken)
        self.assertIn("SMOOTH-BODY-SEMANTIC-1", rules)


if __name__ == "__main__":
    unittest.main()
