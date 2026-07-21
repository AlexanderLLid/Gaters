import copy
import json
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from joint_limit_compiler import compile_joint_limits
from joint_limit_verifier import verify_joint_limits
from run_joint_limits import run


SKELETON_RUN = ROOT / "SkeletonRuns" / "humanoid-guide-skeleton-stick-humanoid-20260721-175401-425940" / "run-1"


class JointLimitTests(unittest.TestCase):
    def setUp(self):
        self.skeleton = json.loads((SKELETON_RUN / "skeleton.json").read_text(encoding="utf-8"))
        self.skeleton_sha256 = json.loads((SKELETON_RUN / "receipt.json").read_text(encoding="utf-8"))["skeleton_sha256"]
        self.recipe = json.loads((ROOT / "recipes" / "humanoid-joint-limits.json").read_text(encoding="utf-8"))
        self.pose_suite = json.loads((ROOT / "recipes" / "biharmonic-pose-suite.json").read_text(encoding="utf-8"))

    def test_compiles_limits_and_accepts_existing_deformation_poses(self):
        limits = compile_joint_limits(self.skeleton, self.recipe, self.skeleton_sha256)
        verification = verify_joint_limits(self.skeleton, self.recipe, limits, self.pose_suite, self.skeleton_sha256)
        self.assertTrue(verification["passed"], verification["failures"])
        self.assertEqual("joint-limits/0", limits["schema"])
        self.assertEqual(9, len(limits["controls"]))

    def test_rejects_missing_skeleton_joint(self):
        broken = copy.deepcopy(self.recipe)
        broken["controls"][0]["joint"] = "missing"
        with self.assertRaisesRegex(ValueError, "JOINT-LIMIT-JOINT-1"):
            compile_joint_limits(self.skeleton, broken, self.skeleton_sha256)

    def test_verifier_rejects_provenance_and_asymmetric_mirror(self):
        limits = compile_joint_limits(self.skeleton, self.recipe, self.skeleton_sha256)
        limits["source_skeleton_sha256"] = "wrong"
        limits["controls"]["right_elbow_flex"]["maximum_degrees"] += 1.0
        verification = verify_joint_limits(self.skeleton, self.recipe, limits, self.pose_suite, self.skeleton_sha256)
        rules = {failure["rule"] for failure in verification["failures"]}
        self.assertIn("JOINT-LIMIT-PROVENANCE-1", rules)
        self.assertIn("JOINT-LIMIT-MIRROR-1", rules)

    def test_verifier_rejects_recipe_or_mirror_table_drift(self):
        limits = compile_joint_limits(self.skeleton, self.recipe, self.skeleton_sha256)
        limits["mirror_pairs"] = []
        limits["controls"]["left_elbow_flex"]["maximum_degrees"] = 160.0
        verification = verify_joint_limits(self.skeleton, self.recipe, limits, self.pose_suite, self.skeleton_sha256)
        rules = {failure["rule"] for failure in verification["failures"]}
        self.assertIn("JOINT-LIMIT-MIRROR-1", rules)
        self.assertIn("JOINT-LIMIT-RECIPE-1", rules)

    def test_run_replays_identically(self):
        import tempfile

        with tempfile.TemporaryDirectory() as directory:
            summary = run(SKELETON_RUN, ROOT / "recipes" / "humanoid-joint-limits.json", ROOT / "recipes" / "biharmonic-pose-suite.json", Path(directory))
        self.assertTrue(summary["passed"])
        self.assertEqual(2, len(summary["runs"]))


if __name__ == "__main__":
    unittest.main()
