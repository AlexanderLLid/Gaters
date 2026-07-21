import tempfile
import unittest
from pathlib import Path

from silhouette_fit import fit_fixed_topology, read_pgm_spans


class SilhouetteFitTests(unittest.TestCase):
    def test_reads_bottom_to_top_normalized_spans(self):
        with tempfile.TemporaryDirectory() as folder:
            image = Path(folder) / "mask.pgm"
            image.write_text(
                "P2\n5 4\n255\n"
                "0 255 255 255 0\n"
                "0 0 255 0 0\n"
                "0 255 255 255 0\n"
                "0 0 0 0 0\n",
                encoding="ascii",
            )

            self.assertEqual(
                read_pgm_spans(image),
                [(-0.5, 0.5), (0.0, 0.0), (-0.5, 0.5)],
            )

    def test_molds_existing_ring_vertices_without_changing_order(self):
        vertices = [
            (1.0, 0.0, -1.0),
            (0.0, 1.0, -1.0),
            (-1.0, 0.0, -1.0),
            (0.0, -1.0, -1.0),
            (1.0, 0.0, 1.0),
            (0.0, 1.0, 1.0),
            (-1.0, 0.0, 1.0),
            (0.0, -1.0, 1.0),
        ]
        original = list(vertices)

        fitted = fit_fixed_topology(
            vertices,
            front_spans=[(-0.5, 0.5), (-1.0, 1.0)],
        )

        self.assertEqual(vertices, original)
        self.assertEqual(len(fitted), len(vertices))
        self.assertEqual(fitted[0], (0.5, 0.0, -1.0))
        self.assertEqual(fitted[1], (0.0, 1.0, -1.0))
        self.assertEqual(fitted[4], (1.0, 0.0, 1.0))
        self.assertEqual(fitted[5], (0.0, 1.0, 1.0))


if __name__ == "__main__":
    unittest.main()
