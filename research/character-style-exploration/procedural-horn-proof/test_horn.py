import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parent
HYTHON = Path(r"C:\Program Files\Side Effects Software\Houdini 22.0.368\bin\hython.exe")


@unittest.skipUnless(HYTHON.is_file(), "Houdini 22.0.368 is not installed")
class ProceduralHornTests(unittest.TestCase):
    def test_horn_is_closed_detailed_tapered_and_reopenable(self):
        recipe = json.loads((ROOT / "horn-recipe.json").read_text(encoding="utf-8"))
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory)
            build = subprocess.run(
                [str(HYTHON), str(ROOT / "build_horn.py"), str(ROOT / "horn-recipe.json"), str(output)],
                cwd=ROOT,
                capture_output=True,
                text=True,
            )
            self.assertEqual(build.returncode, 0, build.stdout + build.stderr)

            summary = json.loads((output / "horn-summary.json").read_text(encoding="utf-8"))
            self.assertTrue(summary["passed"])
            self.assertEqual(summary["point_count"], recipe["axial_samples"] * recipe["radial_segments"])
            self.assertEqual(
                summary["polygon_count"],
                (recipe["axial_samples"] - 1) * recipe["radial_segments"] + 2,
            )
            self.assertEqual(summary["nonfinite_point_count"], 0)
            self.assertEqual(summary["boundary_edge_count"], 0)
            self.assertEqual(summary["nonmanifold_edge_count"], 0)
            self.assertGreater(summary["minimum_polygon_area"], 1e-8)
            self.assertGreater(summary["base_mean_radius"], summary["tip_mean_radius"] * 8.0)
            self.assertEqual(summary["ridge_count"], recipe["ridge_count"])

            scene = output / "horn-detail.hipnc"
            self.assertTrue(scene.is_file())
            self.assertTrue((output / "horn-detail.obj").is_file())
            inspect = subprocess.run(
                [
                    str(HYTHON),
                    "-c",
                    (
                        "import hou; "
                        f"hou.hipFile.load(r'{scene}', suppress_save_prompt=True); "
                        "g=hou.node('/obj/BIOLOGICAL_HORN/OUT_HORN').geometry(); "
                        f"assert len(g.points()) == {summary['point_count']}; "
                        f"assert len(g.prims()) == {summary['polygon_count']}"
                    ),
                ],
                cwd=ROOT,
                capture_output=True,
                text=True,
            )
            self.assertEqual(inspect.returncode, 0, inspect.stdout + inspect.stderr)

            preview = output / "horn-preview.png"
            render = subprocess.run(
                [sys.executable, str(ROOT / "render_preview.py"), str(output / "horn-detail.obj"), str(preview)],
                cwd=ROOT,
                capture_output=True,
                text=True,
            )
            self.assertEqual(render.returncode, 0, render.stdout + render.stderr)
            self.assertTrue(preview.is_file())
            with Image.open(preview) as image:
                self.assertGreaterEqual(image.width, 1000)
                self.assertGreaterEqual(image.height, 600)


if __name__ == "__main__":
    unittest.main()
