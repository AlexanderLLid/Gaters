import json
import subprocess
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parent
BLENDER = Path(r"C:\Program Files\Blender Foundation\Blender 5.2\blender.exe")


class MaskBlenderContractTest(unittest.TestCase):
    def test_one_front_picture_builds_readable_closed_mask_geometry(self):
        with tempfile.TemporaryDirectory() as folder:
            output = Path(folder)
            subprocess.run(
                [str(BLENDER), "--background", "--python-exit-code", "1", "--python",
                 str(ROOT / "build_mask_proof.py"), "--", "--output", str(output), "--variant", "balanced"],
                check=True,
            )
            subprocess.run(
                [str(BLENDER), "--background", "--python-exit-code", "1",
                 str(output / "picture-to-mesh-mask.blend"), "--python", str(ROOT / "verify_mask_proof.py"),
                 "--", "--output", str(output)],
                check=True,
            )

            report = json.loads((output / "verification.json").read_text(encoding="utf-8"))
            self.assertTrue(report["promoted"], report)
            self.assertGreaterEqual(report["frontSilhouetteIoU"], 0.95)
            self.assertTrue(report["topologyUnchanged"])
            self.assertTrue(report["rearReliefPreserved"])
            self.assertTrue(all(report["depthRelations"].values()))


if __name__ == "__main__":
    unittest.main()
