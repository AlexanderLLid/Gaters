import unittest

from PIL import Image

from calibrate_head_target import visible_row_half_width


class CalibrateHeadTargetTests(unittest.TestCase):
    def test_top_antialias_row_uses_nearest_visible_row(self):
        image = Image.new("L", (7, 7))
        image.putpixel((2, 4), 100)
        image.putpixel((4, 4), 100)
        self.assertEqual(visible_row_half_width(image, 3, 3.0, 2.0, 45, 2), 0.5)


if __name__ == "__main__":
    unittest.main()
