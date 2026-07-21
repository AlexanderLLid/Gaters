import json
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from anatomical_guide_compiler import compile_anatomical_guide
from body_plan_compiler import compile_body_plan
from render_anatomical_guide_preview import render


class AnatomicalGuidePreviewTests(unittest.TestCase):
    def test_renders_new_body_modules_without_a_color_table_change(self):
        plan = json.loads((ROOT / "recipes" / "stick-humanoid-body-plan.json").read_text(encoding="utf-8"))
        recipe = json.loads((ROOT / "recipes" / "humanoid-anatomical-guide.json").read_text(encoding="utf-8"))
        guide = compile_anatomical_guide(compile_body_plan(plan), recipe)
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "preview.png"
            render(guide, output)
            self.assertGreater(output.stat().st_size, 0)


if __name__ == "__main__":
    unittest.main()
