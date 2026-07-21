import json
import subprocess
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
INSTALL_ROOT = Path(r"C:\Program Files\Side Effects Software")
HYTHON = next(
    (
        path / "bin" / "hython.exe"
        for path in sorted(INSTALL_ROOT.glob("Houdini *"), reverse=True)
        if (path / "bin" / "hython.exe").is_file()
    ),
    None,
)


@unittest.skipUnless(HYTHON, "Houdini is not installed")
class HoudiniAdapterTests(unittest.TestCase):
    def test_scene_round_trip_preserves_verified_anatomy_graph(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory)
            build = subprocess.run(
                [
                    str(HYTHON),
                    str(ROOT / "houdini" / "build_guides.py"),
                    str(ROOT / "recipes" / "winged-reptile.json"),
                    str(output),
                ],
                cwd=ROOT,
                capture_output=True,
                text=True,
            )
            self.assertEqual(build.returncode, 0, build.stdout + build.stderr)
            scenes = list(output.glob("creature-guide.hip*"))
            self.assertEqual(len(scenes), 1)

            inspect = subprocess.run(
                [
                    str(HYTHON),
                    str(ROOT / "houdini" / "inspect_guides.py"),
                    str(scenes[0]),
                    str(output / "anatomy-graph.json"),
                    str(output / "scene-verification.json"),
                ],
                cwd=ROOT,
                capture_output=True,
                text=True,
            )
            self.assertEqual(inspect.returncode, 0, inspect.stdout + inspect.stderr)
            report = json.loads((output / "scene-verification.json").read_text(encoding="utf-8"))
            self.assertTrue(report["passed"])
            self.assertEqual(report["point_count"], report["expected_joint_count"])
            self.assertEqual(report["curve_count"], report["expected_bone_count"])


if __name__ == "__main__":
    unittest.main()
