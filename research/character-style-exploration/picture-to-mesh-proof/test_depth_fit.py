import tempfile
import unittest
from pathlib import Path

from depth_fit import fit_relief, read_pgm_depth, sample_depth


class DepthFitTests(unittest.TestCase):
    def test_reads_and_samples_depth_in_mesh_coordinates(self):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "depth.pgm"
            path.write_text("P2\n2 2\n100\n0 100\n50 25\n", encoding="ascii")
            image = read_pgm_depth(path)

            self.assertEqual(sample_depth(image, 0.0, 0.0), 0.5)
            self.assertEqual(sample_depth(image, 1.0, 1.0), 1.0)
            self.assertEqual(sample_depth(image, 0.5, 0.5), 0.4375)

    def test_relief_changes_only_depth_without_reordering_vertices(self):
        vertices = [(-1.0, 0.0, -1.0), (1.0, 0.0, 1.0)]
        original = list(vertices)
        image = (2, 2, [[0.0, 1.0], [0.5, 0.25]])

        fitted = fit_relief(vertices, image, depth_scale=0.4)

        self.assertEqual(vertices, original)
        self.assertEqual(fitted, [(-1.0, -0.2, -1.0), (1.0, -0.4, 1.0)])


if __name__ == "__main__":
    unittest.main()
