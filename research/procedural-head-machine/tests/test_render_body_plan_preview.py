import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from render_body_plan_preview import _lit_color


class BodyPlanPreviewTests(unittest.TestCase):
    def test_face_lighting_preserves_color_and_exposes_form(self):
        base = (160, 140, 120)
        lit = _lit_color(base, (0.0, -1.0, 1.0))
        dark = _lit_color(base, (0.0, 1.0, -1.0))
        self.assertGreater(sum(lit), sum(dark))
        self.assertTrue(all(0 <= value <= original for value, original in zip(lit, base)))


if __name__ == "__main__":
    unittest.main()
