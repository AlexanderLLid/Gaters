import unittest

from head_silhouette_fit import (
    blend_valid_width,
    deform_head_point,
    interpolate_profile,
    piecewise_vertical,
    regional_profile_errors,
    residual_corrected_width,
    smooth_profile,
    visual_review_decision,
)


class HeadSilhouetteFitTests(unittest.TestCase):
    def test_profile_interpolation_clamps_and_blends(self):
        samples = [(-1.0, 0.8), (0.0, 1.0), (1.0, 0.6)]
        self.assertEqual(interpolate_profile(samples, -2.0), 0.8)
        self.assertEqual(interpolate_profile(samples, 2.0), 0.6)
        self.assertAlmostEqual(interpolate_profile(samples, 0.5), 0.8)

    def test_profile_smoothing_reduces_single_row_spike(self):
        smoothed = smooth_profile([(-1.0, 1.0), (0.0, 2.0), (1.0, 1.0)], radius=1.1)
        self.assertLess(smoothed[1][1], 2.0)
        self.assertGreater(smoothed[1][1], 1.0)

    def test_vertical_mapping_locks_eye_line_and_hits_top_and_chin(self):
        self.assertEqual(piecewise_vertical(0.0, 1.8, -1.7, 1.6, -1.8), 0.0)
        self.assertAlmostEqual(piecewise_vertical(1.8, 1.8, -1.7, 1.6, -1.8), 1.6)
        self.assertAlmostEqual(piecewise_vertical(-1.7, 1.8, -1.7, 1.6, -1.8), -1.8)

    def test_boundary_moves_to_target_without_changing_depth_or_center(self):
        common = dict(
            center_x=0.0, eye_z=1.5, ipd=0.06,
            current_top=1.8, current_chin=-1.7,
            target_top=1.6, target_chin=-1.8,
            current_half_width=lambda _height: 1.2,
            target_half_width=lambda _height: 1.0,
            side_power=4.0, neck_fade=0.5,
        )
        boundary = deform_head_point((0.072, -0.17, 1.5), **common)
        center = deform_head_point((0.0, -0.18, 1.5), **common)

        self.assertAlmostEqual(boundary[0], 0.06)
        self.assertEqual(boundary[1], -0.17)
        self.assertEqual(center, (0.0, -0.18, 1.5))

    def test_upper_only_fit_leaves_lower_face_vertical_position_unchanged(self):
        common = dict(
            center_x=0.0, eye_z=1.5, ipd=0.06,
            current_top=1.8, current_chin=-1.7,
            target_top=1.6, target_chin=-2.0,
            current_half_width=lambda _height: 1.0,
            target_half_width=lambda _height: 0.9,
            side_power=1.5, neck_fade=0.5,
            minimum_vertical_height=0.2, vertical_blend=0.4,
            minimum_valid_outer_height=0.2, valid_outer_blend=0.4,
        )
        lower = (0.05, -0.12, 1.5)
        upper = (0.05, -0.12, 1.5 + 1.8 * 0.06)
        self.assertEqual(deform_head_point(lower, **common), lower)
        self.assertAlmostEqual(deform_head_point(upper, **common)[2], 1.5 + 1.6 * 0.06)

    def test_neck_falloff_returns_to_unchanged_mesh(self):
        point = (0.05, -0.12, 1.5 - 2.3 * 0.06)
        fitted = deform_head_point(
            point, center_x=0.0, eye_z=1.5, ipd=0.06,
            current_top=1.8, current_chin=-1.7,
            target_top=1.6, target_chin=-1.8,
            current_half_width=lambda _height: 1.0,
            target_half_width=lambda _height: 0.9,
            side_power=4.0, neck_fade=0.5,
        )
        self.assertEqual(fitted, point)

    def test_invalid_lower_outer_boundary_cannot_drive_width(self):
        self.assertEqual(blend_valid_width(1.2, 0.7, -1.3, -1.0, 0.2), 1.2)
        self.assertEqual(blend_valid_width(1.2, 0.7, -0.8, -1.0, 0.2), 0.7)
        self.assertAlmostEqual(blend_valid_width(1.2, 0.7, -0.9, -1.0, 0.2), 0.95)

    def test_profile_errors_are_reported_by_region_and_exclude_neck_rows(self):
        target = [(-1.4, 9.0), (-0.8, 1.0), (0.0, 1.0), (0.8, 1.0)]
        candidate = [(-1.4, 0.0), (-0.8, 1.2), (0.0, 1.1), (0.8, 1.3)]
        errors = regional_profile_errors(
            candidate, target,
            {"skull": (0.4, 2.0), "temples": (-0.4, 0.4), "faceSides": (-1.0, -0.4)},
        )
        self.assertAlmostEqual(errors["skull"], 0.3)
        self.assertAlmostEqual(errors["temples"], 0.1)
        self.assertAlmostEqual(errors["faceSides"], 0.2)

    def test_every_visual_head_region_must_pass_for_promotion(self):
        passing = {name: "pass" for name in ("skull", "temples", "ears", "jawChin", "overall")}
        self.assertEqual(visual_review_decision(passing), "pass")
        passing["jawChin"] = "reject"
        self.assertEqual(visual_review_decision(passing), "reject")
        del passing["ears"]
        with self.assertRaisesRegex(ValueError, "missing visual head regions"):
            visual_review_decision(passing)

    def test_raster_residual_correction_is_bounded(self):
        self.assertAlmostEqual(residual_corrected_width(1.0, 1.1, 0.15), 1.0 / 1.1)
        self.assertEqual(residual_corrected_width(1.0, 2.0, 0.15), 0.85)
        self.assertEqual(residual_corrected_width(1.0, 0.5, 0.15), 1.15)


if __name__ == "__main__":
    unittest.main()
