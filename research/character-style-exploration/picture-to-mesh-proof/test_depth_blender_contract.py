import json
import subprocess
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parent
BLENDER = Path(r"C:\Program Files\Blender Foundation\Blender 5.2\blender.exe")


class DepthBlenderContractTest(unittest.TestCase):
    def test_front_depth_picture_creates_valid_fixed_topology_relief(self):
        with tempfile.TemporaryDirectory() as folder:
            output = Path(folder)
            subprocess.run(
                [str(BLENDER), "--background", "--python-exit-code", "1", "--python",
                 str(ROOT / "build_depth_proof.py"), "--", "--output", str(output)],
                check=True,
            )
            subprocess.run(
                [str(BLENDER), "--background", "--python-exit-code", "1",
                 str(output / "picture-to-mesh-depth.blend"), "--python", str(ROOT / "verify_depth_proof.py"),
                 "--", "--output", str(output)],
                check=True,
            )

            report = json.loads((output / "verification.json").read_text(encoding="utf-8"))
            self.assertTrue(report["promoted"], report)
            self.assertTrue(report["topologyUnchanged"])
            self.assertTrue(report["imagePlaneCoordinatesUnchanged"])
            self.assertLessEqual(report["depthRmse"], 0.002)
            self.assertLessEqual(report["maximumEdgeStretch"], 1.5)


if __name__ == "__main__":
    unittest.main()
