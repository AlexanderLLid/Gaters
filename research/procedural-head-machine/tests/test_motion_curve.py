import copy
import hashlib
import json
import math
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from joint_limit_compiler import compile_joint_limits
from motion_curve_compiler import compile_motion
from motion_curve_verifier import verify_motion
from run_motion_curve import run


SKELETON_RUN = ROOT / "SkeletonRuns" / "humanoid-guide-skeleton-stick-humanoid-20260721-175401-425940" / "run-1"


def sha256(value):
    canonical = json.dumps(value, sort_keys=True, separators=(",", ":"), allow_nan=False).encode("utf-8")
    return hashlib.sha256(canonical).hexdigest()


class MotionCurveTests(unittest.TestCase):
    def setUp(self):
        self.skeleton = json.loads((SKELETON_RUN / "skeleton.json").read_text(encoding="utf-8"))
        self.skeleton_sha256 = json.loads((SKELETON_RUN / "receipt.json").read_text(encoding="utf-8"))["skeleton_sha256"]
        limits_recipe = json.loads((ROOT / "recipes" / "humanoid-joint-limits.json").read_text(encoding="utf-8"))
        self.limits = compile_joint_limits(self.skeleton, limits_recipe, self.skeleton_sha256)
        self.limits_sha256 = sha256(self.limits)
        self.recipe = json.loads((ROOT / "recipes" / "humanoid-motion-proof.json").read_text(encoding="utf-8"))

    def test_compiles_rest_peak_rest_skeleton_frames(self):
        motion = compile_motion(self.skeleton, self.limits, self.recipe, self.skeleton_sha256, self.limits_sha256)
        verification = verify_motion(self.skeleton, self.limits, self.recipe, motion, self.skeleton_sha256, self.limits_sha256)
        self.assertTrue(verification["passed"], verification["failures"])
        self.assertEqual(13, len(motion["frames"]))
        self.assertEqual(self.skeleton["joints"], motion["frames"][0]["joints"])
        self.assertEqual(self.skeleton["joints"], motion["frames"][-1]["joints"])
        rest = {joint["name"]: joint for joint in self.skeleton["joints"]}
        peak = {joint["name"]: joint for joint in motion["frames"][6]["joints"]}
        self.assertGreater(math.dist(rest["left_wrist"]["position"], peak["left_wrist"]["position"]), 0.1)

    def test_rejects_key_outside_joint_limit(self):
        broken = copy.deepcopy(self.recipe)
        broken["tracks"][0]["keys"][1]["degrees"] = 999.0
        with self.assertRaisesRegex(ValueError, "MOTION-LIMIT-1"):
            compile_motion(self.skeleton, self.limits, broken, self.skeleton_sha256, self.limits_sha256)

    def test_verifier_rejects_modified_frame_and_provenance(self):
        motion = compile_motion(self.skeleton, self.limits, self.recipe, self.skeleton_sha256, self.limits_sha256)
        motion["source_joint_limits_sha256"] = "wrong"
        motion["frames"][6]["joints"][0]["position"][0] += 1.0
        verification = verify_motion(self.skeleton, self.limits, self.recipe, motion, self.skeleton_sha256, self.limits_sha256)
        rules = {failure["rule"] for failure in verification["failures"]}
        self.assertIn("MOTION-PROVENANCE-1", rules)
        self.assertIn("MOTION-REPLAY-1", rules)

    def test_verifier_rejects_nonfinite_skeleton_position(self):
        motion = compile_motion(self.skeleton, self.limits, self.recipe, self.skeleton_sha256, self.limits_sha256)
        motion["frames"][6]["joints"][0]["position"][0] = math.nan
        verification = verify_motion(self.skeleton, self.limits, self.recipe, motion, self.skeleton_sha256, self.limits_sha256)
        self.assertIn("MOTION-REPLAY-1", {failure["rule"] for failure in verification["failures"]})

    def test_run_replays_identically(self):
        import tempfile

        with tempfile.TemporaryDirectory() as directory:
            summary = run(SKELETON_RUN, self.limits, self.limits_sha256, ROOT / "recipes" / "humanoid-motion-proof.json", Path(directory))
        self.assertTrue(summary["passed"])
        self.assertEqual(2, len(summary["runs"]))


if __name__ == "__main__":
    unittest.main()
