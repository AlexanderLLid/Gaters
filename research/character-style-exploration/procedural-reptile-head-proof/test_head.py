import json
import subprocess
import tempfile
import unittest
from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parent
HYTHON = Path(r"C:\Program Files\Side Effects Software\Houdini 22.0.368\bin\hython.exe")


@unittest.skipUnless(HYTHON.is_file(), "Houdini 22.0.368 is not installed")
class ProceduralReptileHeadTests(unittest.TestCase):
    def test_recipe_rejects_invalid_resolution(self):
        recipe = json.loads((ROOT / "head-recipe.json").read_text(encoding="utf-8"))
        recipe["surface_u"] = 3
        with tempfile.TemporaryDirectory() as directory:
            bad = Path(directory) / "bad.json"
            bad.write_text(json.dumps(recipe), encoding="utf-8")
            result = subprocess.run(
                [str(HYTHON), str(ROOT / "build_head.py"), str(bad), directory],
                cwd=ROOT,
                capture_output=True,
                text=True,
            )
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("surface_u", result.stdout + result.stderr)

    def test_head_has_required_geometry_contract(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory)
            result = subprocess.run(
                [str(HYTHON), str(ROOT / "build_head.py"), str(ROOT / "head-recipe.json"), str(output)],
                cwd=ROOT,
                capture_output=True,
                text=True,
            )
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            summary = json.loads((output / "head-summary.json").read_text(encoding="utf-8"))
            self.assertTrue(summary["passed"])
            self.assertEqual(summary["nonfinite_point_count"], 0)
            self.assertGreater(summary["minimum_polygon_area"], 1e-8)
            self.assertGreater(summary["point_count"], 2500)
            self.assertGreater(summary["polygon_count"], 2400)
            self.assertTrue({"skin", "eye", "pupil", "mouth", "nostril"}.issubset(summary["features"]))
            self.assertGreater(summary["snout_width_ratio"], 0.40)
            self.assertEqual(len(summary["generator_sha256"]), 64)
            self.assertIn("Apprentice", summary["license_category"])
            scene = output / "reptile-head.hipnc"
            self.assertTrue(scene.is_file())
            self.assertTrue((output / "reptile-head.bgeo.sc").is_file())
            self.assertEqual(
                summary["cameras"],
                ["CAM_GAMEPLAY", "CAM_CLOSE", "CAM_INSPECT"],
            )

            inspect = subprocess.run(
                [
                    str(HYTHON),
                    "-c",
                    (
                        "import hou; "
                        f"hou.hipFile.load(r'{scene}', suppress_save_prompt=True); "
                        "g=hou.node('/obj/REPTILE_HEAD/HEAD_SURFACE').geometry(); "
                        f"assert len(g.points()) == {summary['point_count']}; "
                        f"assert len(g.prims()) == {summary['polygon_count']}; "
                        "assert all(hou.node('/obj/'+n) is not None for n in "
                        "('CAM_GAMEPLAY','CAM_CLOSE','CAM_INSPECT')); "
                        "assert hou.node('/obj/REPTILE_HEAD/SKIN_VDB') is not None; "
                        "assert hou.node('/obj/REPTILE_HEAD/SKIN_SMOOTH') is not None; "
                        "assert hou.node('/obj/REPTILE_HEAD/SKIN_SURFACE') is not None; "
                        "assert len(hou.node('/obj/REPTILE_HEAD/SKIN_SURFACE').geometry().prims()) > 1000; "
                        "assert hou.node('/obj/REPTILE_HEAD/SKIN_PAINT') is not None; "
                        "assert len(hou.node('/obj/REPTILE_HEAD/OUT_HEAD').geometry().prims()) > 0"
                    ),
                ],
                cwd=ROOT,
                capture_output=True,
                text=True,
            )
            self.assertEqual(inspect.returncode, 0, inspect.stdout + inspect.stderr)

            render = subprocess.run(
                [
                    str(HYTHON),
                    str(ROOT / "render_views.py"),
                    str(scene),
                    str(output),
                ],
                cwd=ROOT,
                capture_output=True,
                text=True,
            )
            self.assertEqual(render.returncode, 0, render.stdout + render.stderr)
            for name in ("A-gameplay.png", "B-gameplay-close.png", "C-inspection.png"):
                image_path = output / name
                self.assertTrue(image_path.is_file())
                with Image.open(image_path) as image:
                    self.assertGreaterEqual(image.width, 960)
                    self.assertGreaterEqual(image.height, 540)
                    pixels = list(image.convert("RGB").crop((0, 60, image.width - 180, image.height)).getdata())
                    foreground = [sum(pixel) / 3 for pixel in pixels if max(pixel) > 8]
                    self.assertTrue(foreground)
                    self.assertGreater(sum(foreground) / len(foreground), 45.0)

            evaluation = json.loads((output / "evaluation.json").read_text(encoding="utf-8"))
            for label in evaluation["labels"]:
                label.encode("ascii")


if __name__ == "__main__":
    unittest.main()
