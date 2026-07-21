import json
import subprocess
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parent
REPO = ROOT.parents[2]
HYTHON = Path(r"C:\Program Files\Side Effects Software\Houdini 22.0.368\bin\hython.exe")
BODY_RUN = REPO / "research/procedural-head-machine/AnatomicalSurfaceRuns/anatomical-mannequin-20260721-235834-702/run-1"
GUIDE_RUN = REPO / "research/procedural-head-machine/AnatomyRuns/humanoid-guide-stick-humanoid-20260721-203941-305591/run-1"


@unittest.skipUnless(HYTHON.is_file(), "Houdini 22.0.368 is not installed")
class HumanoidFaceSurfaceTests(unittest.TestCase):
    def run_build(self, recipe: str, output: Path) -> subprocess.CompletedProcess:
        return subprocess.run(
            [
                str(HYTHON),
                str(ROOT / "build_face_surface.py"),
                str(BODY_RUN),
                str(GUIDE_RUN),
                str(ROOT / recipe),
                str(output),
            ],
            cwd=ROOT,
            capture_output=True,
            text=True,
        )

    def test_rejects_face_outside_head_contract(self):
        recipe = json.loads((ROOT / "face-recipe.json").read_text(encoding="utf-8"))
        recipe["eye_spacing_m"] = 0.60
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            bad_recipe = root / "bad.json"
            bad_recipe.write_text(json.dumps(recipe), encoding="utf-8")
            result = self.run_build(str(bad_recipe), root / "output")
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("eye_spacing_m", result.stdout + result.stderr)

    def test_builds_repeatable_face_on_accepted_body(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            hashes = []
            for index in (1, 2):
                output = root / f"run-{index}"
                result = self.run_build("face-recipe.json", output)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                summary = json.loads((output / "summary.json").read_text(encoding="utf-8"))
                self.assertTrue(summary["passed"])
                self.assertEqual(summary["skin_component_count"], 1)
                self.assertEqual(summary["source_body_id"], "anatomical-mannequin")
                self.assertLessEqual(summary["body_bounds_drift_m"], 0.04)
                self.assertLessEqual(summary["bilateral_feature_error_m"], 1e-6)
                self.assertTrue(
                    {"skin", "sclera", "iris", "pupil", "lip", "nostril"}.issubset(summary["features"])
                )
                self.assertGreater(summary["point_count"], 5000)
                self.assertTrue((output / "humanoid-face.hipnc").is_file())
                self.assertTrue((output / "humanoid-face.bgeo.sc").is_file())
                hashes.append(summary["geometry_sha256"])
            self.assertEqual(hashes[0], hashes[1])

    def test_held_out_recipe_changes_geometry_without_code_change(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            baseline = root / "baseline"
            held_out = root / "held-out"
            first = self.run_build("face-recipe.json", baseline)
            second = self.run_build("face-recipe-held-out.json", held_out)
            self.assertEqual(first.returncode, 0, first.stdout + first.stderr)
            self.assertEqual(second.returncode, 0, second.stdout + second.stderr)
            baseline_summary = json.loads((baseline / "summary.json").read_text(encoding="utf-8"))
            held_out_summary = json.loads((held_out / "summary.json").read_text(encoding="utf-8"))
            self.assertNotEqual(baseline_summary["geometry_sha256"], held_out_summary["geometry_sha256"])
            self.assertEqual(baseline_summary["generator_sha256"], held_out_summary["generator_sha256"])


if __name__ == "__main__":
    unittest.main()
