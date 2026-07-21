import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from anatomical_surface_verifier import measure_chain_radii


GUIDE = {
    "landmarks": {
        "left_elbow": [-0.6, 0.0, 0.5],
        "left_wrist": [-0.9, 0.0, 0.5],
    }
}
SURFACE = {
    "vertices": [
        [-0.6, 0.10, 0.5], [-0.6, -0.10, 0.5], [-0.6, 0.0, 0.60], [-0.6, 0.0, 0.40],
        [-0.9, 0.05, 0.5], [-0.9, -0.05, 0.5], [-0.9, 0.0, 0.55], [-0.9, 0.0, 0.45],
    ],
    "vertex_modules": ["left_arm"] * 8,
}


class AnatomicalSurfaceVerifierTests(unittest.TestCase):
    def test_measures_real_distal_taper_from_surface_vertices(self):
        radii = measure_chain_radii(SURFACE, GUIDE, "left_arm", ("left_elbow", "left_wrist"), axis=0, band=0.01)
        self.assertAlmostEqual(0.10, radii["left_elbow"])
        self.assertAlmostEqual(0.05, radii["left_wrist"])

    def test_reports_missing_band_as_nonfinite(self):
        radii = measure_chain_radii(SURFACE, GUIDE, "right_arm", ("left_elbow",), axis=0, band=0.01)
        self.assertEqual(float("inf"), radii["left_elbow"])


if __name__ == "__main__":
    unittest.main()
