import unittest

from PIL import Image

from extract_head_silhouette import extract_profile


class ExtractHeadSilhouetteTests(unittest.TestCase):
    def test_extracts_normalized_width_profile_above_declared_chin(self):
        image = Image.new("RGB", (7, 7), (10, 10, 10))
        for y, span in ((1, (3, 3)), (2, (2, 4)), (3, (1, 5)), (4, (2, 4)), (5, (3, 3))):
            for x in range(span[0], span[1] + 1):
                image.putpixel((x, y), (100, 100, 100))

        profile = extract_profile(
            image, threshold=50, eye_center=(3.0, 3.0), ipd=2.0, chin_y=5, step=1
        )

        self.assertEqual(profile["targetTop"], 1.0)
        self.assertEqual(profile["targetChin"], -1.0)
        self.assertEqual(profile["samples"][2], {"height": 0.0, "halfWidth": 1.0})


if __name__ == "__main__":
    unittest.main()
