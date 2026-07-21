import unittest

from mask_fit import fit_closed_mask, relax_relief


class MaskFitTests(unittest.TestCase):
    def test_relief_relaxation_spreads_a_sharp_local_change(self):
        self.assertEqual(
            relax_relief([0.0, 1.0, 0.0], [(0, 1), (1, 2)], {0, 2}, iterations=1, factor=0.5),
            [0.0, 0.5, 0.0],
        )

    def test_relief_relaxation_can_retain_source_evidence(self):
        self.assertEqual(
            relax_relief(
                [0.0, 1.0, 0.0],
                [(0, 1), (1, 2)],
                {0, 2},
                iterations=1,
                factor=0.5,
                source_weight=0.2,
            ),
            [0.0, 0.6, 0.0],
        )

    def test_front_relief_preserves_rear_vertices_and_topology_order(self):
        vertices = [
            (0.0, -1.0, -1.0),
            (0.0, 1.0, -1.0),
            (0.0, -1.0, 1.0),
            (0.0, 1.0, 1.0),
        ]
        depth_image = (2, 2, [[1.0, 1.0], [1.0, 1.0]])

        fitted = fit_closed_mask(
            vertices,
            front_spans=[(-1.0, 1.0), (-1.0, 1.0)],
            depth_image=depth_image,
            depth_scale=0.4,
            neutral_depth=0.5,
        )

        self.assertEqual(len(fitted), len(vertices))
        self.assertEqual(fitted[0], (0.0, -1.2, -1.0))
        self.assertEqual(fitted[1], (0.0, 1.0, -1.0))
        self.assertEqual(fitted[2], (0.0, -1.2, 1.0))
        self.assertEqual(fitted[3], (0.0, 1.0, 1.0))


if __name__ == "__main__":
    unittest.main()
