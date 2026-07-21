import json
import subprocess
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parent
BLENDER = Path(r"C:\Program Files\Blender Foundation\Blender 5.2\blender.exe")
SOURCE = ROOT / "Derived" / "search-runs" / "anatomical-front-fit-v0-20260720-1" / "round-9" / "9032fd4dc9db2de64564b3f19e32ca5e6199881df111c2fa2d37feb1c7cc12f8" / "face-sculpt.blend"


class StyleMacroBlenderContractTest(unittest.TestCase):
    def test_builds_editable_macro_shape_key_without_topology_change(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory)
            subprocess.run(
                [str(BLENDER), "--background", "--python-exit-code", "1", str(SOURCE),
                 "--python", str(ROOT / "build_style_macro.py"), "--",
                 "--recipe", str(ROOT / "style-macro-v1.json"), "--output", str(output)],
                check=True,
            )
            subprocess.run(
                [str(BLENDER), "--background", "--python-exit-code", "1",
                 str(output / "face-style-macro.blend"),
                 "--python", str(ROOT / "verify_style_macro.py"), "--", "--output", str(output)],
                check=True,
            )
            manifest = json.loads((output / "manifest.json").read_text(encoding="utf-8"))
            verification = json.loads((output / "verification.json").read_text(encoding="utf-8"))
            self.assertEqual(manifest["status"], "built-unverified")
            self.assertEqual(manifest["topology"]["vertexCount"], 19158)
            self.assertEqual(manifest["shapeKey"], "StyleMacroDirect")
            self.assertEqual(manifest["maximumYDisplacement"], 0.0)
            self.assertEqual(verification["decision"], "promote-macro-control")
            self.assertTrue(verification["topologyUnchanged"])
            self.assertTrue(verification["profileDepthPreserved"])
            self.assertLess(verification["candidateScore"], verification["baselineScore"])
            self.assertEqual(set(verification["eyeOpeningWidths"]), {"left", "right"})
            self.assertEqual(set(verification["targetEyeOpeningWidths"]), {"left", "right"})
            self.assertIsInstance(verification["eyeAperture"], float)
            self.assertIsInstance(verification["targetEyeAperture"], float)
            for name in ("face-style-macro.blend", "front.png", "three-quarter.png", "profile.png"):
                self.assertTrue((output / name).is_file())

    def test_builds_large_presentation_pass_without_image_textures(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory)
            subprocess.run(
                [str(BLENDER), "--background", "--python-exit-code", "1", str(SOURCE),
                 "--python", str(ROOT / "build_style_macro.py"), "--",
                 "--recipe", str(ROOT / "style-presentation-v1.json"), "--output", str(output)],
                check=True,
            )
            subprocess.run(
                [str(BLENDER), "--background", "--python-exit-code", "1",
                 str(output / "face-style-macro.blend"),
                 "--python", str(ROOT / "verify_style_macro.py"), "--", "--output", str(output)],
                check=True,
            )
            verification = json.loads((output / "verification.json").read_text(encoding="utf-8"))
            self.assertTrue(verification["presentation"]["browsVisible"])
            self.assertTrue(verification["presentation"]["proceduralPupils"])
            self.assertTrue(verification["presentation"]["lipMaterialAssigned"])
            self.assertTrue(verification["presentation"]["visibleMaterialsUseNoImages"])


if __name__ == "__main__":
    unittest.main()
