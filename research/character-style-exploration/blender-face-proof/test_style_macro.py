import unittest

from style_macro import (
    bounded_parameter,
    deform_point,
    resolve_eye_height,
    resolve_eye_narrows,
    resolve_eye_scale_decrease,
    resolve_eye_scale_decreases,
    resolve_mouth_narrow,
    resolve_mouth_width,
)


class StyleMacroTests(unittest.TestCase):
    def test_named_presentation_controls_are_bounded(self):
        self.assertEqual(bounded_parameter({}, "eyeBagLeft", 0.4), 0.4)
        self.assertEqual(bounded_parameter({"eyeBagLeft": 0.12}, "eyeBagLeft", 0.4), 0.12)
        with self.assertRaises(ValueError):
            bounded_parameter({"browHeightScale": 1.2}, "browHeightScale", 1.0)

    def test_mouth_width_override_is_explicit_and_bounded(self):
        self.assertEqual(resolve_mouth_width({}, 0.14), 0.14)
        self.assertEqual(resolve_mouth_width({"mouthWidthValue": 0.05}, 0.14), 0.05)
        with self.assertRaises(ValueError):
            resolve_mouth_width({"mouthWidthValue": -0.01}, 0.14)

    def test_mouth_narrow_control_defaults_off_and_is_bounded(self):
        self.assertEqual(resolve_mouth_narrow({}), 0.0)
        self.assertEqual(resolve_mouth_narrow({"mouthNarrowValue": 0.17}), 0.17)
        with self.assertRaises(ValueError):
            resolve_mouth_narrow({"mouthNarrowValue": 1.01})

    def test_eye_controls_are_explicit_and_bounded(self):
        self.assertEqual(resolve_eye_scale_decrease({}), 0.0)
        self.assertEqual(resolve_eye_scale_decrease({"eyeScaleDecrease": 0.08}), 0.08)
        self.assertEqual(resolve_eye_height({}, 0.52), 0.52)
        self.assertEqual(resolve_eye_height({"eyeHeightValue": 0.61}, 0.52), 0.61)
        with self.assertRaises(ValueError):
            resolve_eye_scale_decrease({"eyeScaleDecrease": 1.01})
        with self.assertRaises(ValueError):
            resolve_eye_height({"eyeHeightValue": -0.01}, 0.52)

    def test_eye_scale_can_be_calibrated_per_side(self):
        self.assertEqual(resolve_eye_scale_decreases({"eyeScaleDecrease": 0.08}), (0.08, 0.08))
        self.assertEqual(
            resolve_eye_scale_decreases({"eyeScaleDecreaseLeft": 0.4, "eyeScaleDecreaseRight": 0.2}),
            (0.4, 0.2),
        )
        with self.assertRaises(ValueError):
            resolve_eye_scale_decreases({"eyeScaleDecreaseLeft": 1.2})

    def test_inherited_eye_narrow_controls_can_be_disabled(self):
        self.assertEqual(resolve_eye_narrows({}, 0.14, 0.17), (0.14, 0.17))
        self.assertEqual(
            resolve_eye_narrows({"eyeNarrowLeft": 0.0, "eyeNarrowRight": 0.0}, 0.14, 0.17),
            (0.0, 0.0),
        )
        with self.assertRaises(ValueError):
            resolve_eye_narrows({"eyeNarrowRight": -0.1}, 0.14, 0.17)

    def test_shortens_upper_and_lower_head_around_fixed_eye_line(self):
        common = dict(
            center_x=0.0, eye_z=1.5, chin_z=1.3, top_z=1.8,
            side_radius=0.05, upper_height_scale=0.96, lower_height_scale=0.96,
            cheek_scale=0.98, jaw_scale=1.04, cheek_fraction=0.35,
            jaw_fraction=0.73, lateral_power=4.0,
        )

        self.assertEqual(deform_point((0.0, -0.1, 1.5), **common), (0.0, -0.1, 1.5))
        self.assertAlmostEqual(deform_point((0.0, -0.1, 1.8), **common)[2], 1.788)
        self.assertAlmostEqual(deform_point((0.0, -0.1, 1.3), **common)[2], 1.308)

    def test_moves_jaw_sides_without_widening_center(self):
        common = dict(
            center_x=0.0, eye_z=1.5, chin_z=1.3, top_z=1.8,
            side_radius=0.05, upper_height_scale=1.0, lower_height_scale=1.0,
            cheek_scale=0.98, jaw_scale=1.04, cheek_fraction=0.35,
            jaw_fraction=0.73, lateral_power=4.0,
        )
        jaw_z = 1.5 - (1.5 - 1.3) * 0.73

        self.assertAlmostEqual(deform_point((0.05, -0.1, jaw_z), **common)[0], 0.052)
        self.assertAlmostEqual(deform_point((0.0, -0.1, jaw_z), **common)[0], 0.0)
        self.assertAlmostEqual(deform_point((0.025, -0.1, jaw_z), **common)[0], 0.0250625)


if __name__ == "__main__":
    unittest.main()
