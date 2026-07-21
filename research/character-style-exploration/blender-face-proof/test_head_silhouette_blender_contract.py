import json
import subprocess
import tempfile
import unittest
from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parent
BLENDER = Path(r"C:\Program Files\Blender Foundation\Blender 5.2\blender.exe")
SOURCE = ROOT / "Derived" / "style-presentation-v5" / "face-style-macro.blend"


class HeadSilhouetteBlenderContractTest(unittest.TestCase):
    def test_head_fit_emits_aligned_review_evidence_without_self_promoting(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory)
            subprocess.run(
                [str(BLENDER), "--background", "--python-exit-code", "1", str(SOURCE),
                 "--python", str(ROOT / "build_head_silhouette.py"), "--",
                 "--recipe", str(ROOT / "head-silhouette-v9.json"), "--output", str(output)],
                check=True,
            )
            subprocess.run(
                [str(BLENDER), "--background", "--python-exit-code", "1",
                 str(output / "face-head-silhouette.blend"),
                 "--python", str(ROOT / "verify_head_silhouette.py"), "--", "--output", str(output)],
                check=True,
            )
            subprocess.run(
                ["python", str(ROOT / "make_head_overlay.py"),
                 "--candidate", str(output / "front.png"),
                 "--candidate-alignment", str(output / "front-alignment.json"),
                 "--target-profile", str(ROOT / "target-head-silhouette-v1.json"),
                 "--output", str(output / "head-overlay.png")],
                check=True,
            )
            report = json.loads((output / "verification.json").read_text(encoding="utf-8"))
            manifest = json.loads((output / "manifest.json").read_text(encoding="utf-8"))
            self.assertGreater(manifest["excludedEarVertexCount"], 0)
            self.assertEqual(manifest["maximumEarDisplacement"], 0.0)
            self.assertEqual(set(manifest["appliedMpfbTargets"]), {"chinHeightIncrease", "chinWidthDecrease"})
            self.assertEqual(report["decision"], "mechanical-pass-awaiting-visual-review")
            self.assertFalse(report["promotionEligible"])
            self.assertEqual(report["automaticScoreCoverage"]["excluded"], "ears-jaw-chin-neck")
            self.assertEqual(set(report["regionalErrors"]["fitted"]), {"skull", "temples"})
            self.assertEqual(report["maximumDepthChange"], 0.0)
            self.assertTrue(report["topologyUnchanged"])
            for name in ("front.png", "three-quarter.png", "profile.png", "front-alignment.json",
                         "a-target-head-aligned.png", "b-generated-head-aligned.png",
                         "c-center-wipe.png", "d-aligned-blink.gif", "head-overlay.png"):
                self.assertTrue((output / name).is_file())
            target = Image.open(output / "a-target-head-aligned.png").convert("RGB")
            generated = Image.open(output / "b-generated-head-aligned.png").convert("RGB")
            wipe = Image.open(output / "c-center-wipe.png").convert("RGB")
            y = wipe.height // 2
            self.assertEqual(wipe.getpixel((wipe.width // 4, y)), target.getpixel((target.width // 4, y)))
            self.assertEqual(wipe.getpixel((wipe.width * 3 // 4, y)), generated.getpixel((generated.width * 3 // 4, y)))
            with Image.open(output / "d-aligned-blink.gif") as blink:
                self.assertEqual(blink.n_frames, 2)


if __name__ == "__main__":
    unittest.main()
