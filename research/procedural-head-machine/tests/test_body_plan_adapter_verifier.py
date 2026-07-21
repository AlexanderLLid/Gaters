import copy
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from body_plan_adapter_verifier import verify_body_plan_adapter


BODY = {
    "vertices": [[0.0, 0.0, 0.0], [1.0, 0.0, 0.0], [0.0, 1.0, 0.0]],
    "faces": [[0, 1, 2]],
    "regions": [{"core": 1.0, "head": 0.0, "limb": 0.0}] * 3,
    "vertex_modules": ["torso"] * 3,
    "face_modules": ["torso"],
    "placements": {"torso": {"bounds_cells": [[0, 0, 0], [1, 1, 1]], "role": "core"}},
    "connections": [],
    "cells": [{"position": [0, 0, 0], "module": "torso"}],
    "mirrors": [],
}


class BodyPlanAdapterVerifierTests(unittest.TestCase):
    def test_accepts_roundtrip_with_float_tolerance(self):
        readback = copy.deepcopy(BODY)
        readback["vertices"][1][0] += 1e-8
        self.assertTrue(verify_body_plan_adapter(BODY, readback)["passed"])

    def test_rejects_metadata_drift(self):
        readback = copy.deepcopy(BODY)
        readback["cells"][0]["module"] = "head"
        report = verify_body_plan_adapter(BODY, readback)
        self.assertIn("BODY-ADAPTER-METADATA-1", [failure["rule"] for failure in report["failures"]])

    def test_rejects_geometry_drift(self):
        readback = copy.deepcopy(BODY)
        readback["faces"][0] = [0, 2, 1]
        report = verify_body_plan_adapter(BODY, readback)
        self.assertIn("BODY-ADAPTER-TOPOLOGY-1", [failure["rule"] for failure in report["failures"]])


if __name__ == "__main__":
    unittest.main()
