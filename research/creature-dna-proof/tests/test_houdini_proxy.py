import json
import subprocess
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
HYTHON = Path(r"C:\Program Files\Side Effects Software\Houdini 22.0.368\bin\hython.exe")


@unittest.skipUnless(HYTHON.is_file(), "Houdini 22.0.368 is not installed")
class HoudiniProxyTests(unittest.TestCase):
    def test_proxy_builds_reopenable_polygon_creature(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory)
            build = subprocess.run(
                [
                    str(HYTHON),
                    str(ROOT / "houdini" / "build_proxy.py"),
                    str(ROOT / "recipes" / "winged-reptile.json"),
                    str(output),
                ],
                cwd=ROOT,
                capture_output=True,
                text=True,
            )
            self.assertEqual(build.returncode, 0, build.stdout + build.stderr)
            self.assertTrue((output / "creature-proxy.hipnc").is_file())
            self.assertTrue((output / "creature-proxy.obj").is_file())

            summary = json.loads((output / "proxy-summary.json").read_text(encoding="utf-8"))
            self.assertTrue(summary["passed"])
            self.assertGreater(summary["point_count"], 1000)
            self.assertGreater(summary["polygon_count"], 1000)
            self.assertEqual(summary["source_joint_count"], 35)
            self.assertEqual(summary["source_bone_count"], 34)

            inspect = subprocess.run(
                [
                    str(HYTHON),
                    "-c",
                    (
                        "import hou; "
                        f"hou.hipFile.load(r'{output / 'creature-proxy.hipnc'}', suppress_save_prompt=True); "
                        "g=hou.node('/obj/CREATURE_DNA/OUT_PROXY').geometry(); "
                        "assert len(g.points()) > 1000 and len(g.prims()) > 1000"
                    ),
                ],
                cwd=ROOT,
                capture_output=True,
                text=True,
            )
            self.assertEqual(inspect.returncode, 0, inspect.stdout + inspect.stderr)


if __name__ == "__main__":
    unittest.main()
