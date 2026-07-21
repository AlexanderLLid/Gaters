import json
import subprocess
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parent
BLENDER = Path(r"C:\Program Files\Blender Foundation\Blender 5.2\blender.exe")


class BlenderContractTest(unittest.TestCase):
    def test_fixed_topology_mesh_improves_front_picture_and_preserves_depth(self):
        self.assertTrue(BLENDER.is_file(), BLENDER)
        with tempfile.TemporaryDirectory() as folder:
            output = Path(folder)
            subprocess.run(
                [str(BLENDER), "--background", "--python-exit-code", "1", "--python", str(ROOT / "build_proof.py"),
                 "--", "--output", str(output)],
                check=True,
            )
            subprocess.run(
                [str(BLENDER), "--background", "--python-exit-code", "1", str(output / "picture-to-mesh.blend"),
                 "--python", str(ROOT / "verify_proof.py"), "--", "--output", str(output)],
                check=True,
            )

            manifest = json.loads((output / "manifest.json").read_text(encoding="utf-8"))
            report = json.loads((output / "verification.json").read_text(encoding="utf-8"))
            self.assertIn("builderSha256", manifest["tools"])
            self.assertIn("fitterSha256", manifest["tools"])
            self.assertEqual(len(manifest["artifacts"]), 7)
            self.assertIn("verifierSha256", report["evidence"])
            self.assertTrue(report["promoted"], report)
            self.assertTrue(report["topologyUnchanged"])
            self.assertTrue(report["depthPreserved"])
            self.assertGreater(report["fitted"]["frontIoU"], report["baseline"]["frontIoU"] + 0.15)


if __name__ == "__main__":
    unittest.main()
