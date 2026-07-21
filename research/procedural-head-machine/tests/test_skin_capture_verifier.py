import copy
import json
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from skin_capture_verifier import verify_skin_capture


def load(path):
    return json.loads(path.read_text(encoding="utf-8"))


SURFACE_RUN = ROOT / "AnatomicalSurfaceRuns" / "anatomical-mannequin-20260721-194548-295" / "run-1"
SKELETON_RUN = ROOT / "SkeletonRuns" / "humanoid-guide-skeleton-stick-humanoid-20260721-175401-425940" / "run-1"
CAPTURE_RUN = ROOT / "SkinCaptureRuns" / "20260721-201340-476"
RECIPE = load(ROOT / "recipes" / "proximity-skin-capture.json")


class SkinCaptureVerifierTests(unittest.TestCase):
    def setUp(self):
        self.surface = load(SURFACE_RUN / "readback.json")
        self.skeleton = load(SKELETON_RUN / "skeleton.json")
        self.skeleton_hash = load(SKELETON_RUN / "receipt.json")["skeleton_sha256"]
        self.readback = load(CAPTURE_RUN / "readback.json")

    def test_accepts_normalized_proximity_capture(self):
        report = verify_skin_capture(self.surface, self.skeleton, self.skeleton_hash, RECIPE, self.readback)
        self.assertTrue(report["passed"])
        self.assertGreater(report["metrics"]["elbow_thickness_ratio_p10"], 0.0)

    def test_rejects_unnormalized_weights(self):
        broken = copy.deepcopy(self.readback)
        broken["capture_weights"][0][0] = 0.0
        rules = {failure["rule"] for failure in verify_skin_capture(self.surface, self.skeleton, self.skeleton_hash, RECIPE, broken)["failures"]}
        self.assertIn("SKIN-CAPTURE-NORMALIZATION-1", rules)

    def test_rejects_cross_side_influence(self):
        broken = copy.deepcopy(self.readback)
        index = broken["modules"].index("left_arm")
        broken["capture_indices"][index][0] = broken["capture_paths"].index("right_wrist")
        rules = {failure["rule"] for failure in verify_skin_capture(self.surface, self.skeleton, self.skeleton_hash, RECIPE, broken)["failures"]}
        self.assertIn("SKIN-CAPTURE-SIDE-1", rules)

    def test_rejects_surface_position_drift(self):
        broken = copy.deepcopy(self.readback)
        broken["positions"][0][0] += 0.1
        rules = {failure["rule"] for failure in verify_skin_capture(self.surface, self.skeleton, self.skeleton_hash, RECIPE, broken)["failures"]}
        self.assertIn("SKIN-CAPTURE-SURFACE-1", rules)

    def test_rejects_skeleton_provenance_drift(self):
        broken = copy.deepcopy(self.readback)
        broken["skeleton_sha256"] = "wrong"
        rules = {failure["rule"] for failure in verify_skin_capture(self.surface, self.skeleton, self.skeleton_hash, RECIPE, broken)["failures"]}
        self.assertIn("SKIN-CAPTURE-PROVENANCE-1", rules)

    def test_rejects_pose_that_does_not_move_active_limb(self):
        broken = copy.deepcopy(self.readback)
        broken["posed_positions"] = copy.deepcopy(broken["positions"])
        rules = {failure["rule"] for failure in verify_skin_capture(self.surface, self.skeleton, self.skeleton_hash, RECIPE, broken)["failures"]}
        self.assertIn("SKIN-CAPTURE-POSE-ACTIVE-1", rules)


if __name__ == "__main__":
    unittest.main()
