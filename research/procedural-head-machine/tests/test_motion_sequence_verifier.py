import copy
import json
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from motion_sequence_verifier import verify_motion_sequence


SOURCE_RUN = ROOT / "SkinCaptureRuns" / "20260721-203829-025"
MOTION_RUN = ROOT / "MotionRuns" / "humanoid-generated-motion-proof-20260721-190427-582753" / "run-1"


class MotionSequenceVerifierTests(unittest.TestCase):
    def setUp(self):
        self.source = json.loads((SOURCE_RUN / "readback.json").read_text(encoding="utf-8"))
        self.motion = json.loads((MOTION_RUN / "motion.json").read_text(encoding="utf-8"))
        self.motion_sha256 = json.loads((MOTION_RUN / "receipt.json").read_text(encoding="utf-8"))["motion_sha256"]
        frames = []
        peak = max(frame["controls"]["left_elbow_flex"] for frame in self.motion["frames"])
        for frame in self.motion["frames"]:
            factor = frame["controls"]["left_elbow_flex"] / peak
            positions = copy.deepcopy(self.source["positions"])
            positions[0][0] += 0.2 * factor
            frames.append({
                "index": frame["index"],
                "time_seconds": frame["time_seconds"],
                "positions": positions,
                "skeleton_positions": {joint["name"]: joint["position"] for joint in frame["joints"]},
            })
        self.readback = {
            "schema": "houdini-motion-readback/0",
            "positions": self.source["positions"],
            "faces": self.source["faces"],
            "modules": self.source["modules"],
            "frames": frames,
            "source_skin_scene_sha256": "scene-hash",
            "motion_sha256": self.motion_sha256,
            "frame_range": [1.0, 13.0],
        }

    def test_accepts_reopened_generated_sequence(self):
        verification = verify_motion_sequence(self.source, self.motion, self.readback, self.motion_sha256, "scene-hash")
        self.assertTrue(verification["passed"], verification["failures"])
        self.assertEqual(13, verification["frame_count"])
        self.assertGreater(verification["maximum_surface_motion_m"], 0.1)

    def test_rejects_topology_skeleton_and_provenance_drift(self):
        broken = copy.deepcopy(self.readback)
        broken["faces"][0] = list(reversed(broken["faces"][0]))
        broken["frames"][6]["skeleton_positions"]["left_wrist"][0] += 1.0
        broken["motion_sha256"] = "wrong"
        verification = verify_motion_sequence(self.source, self.motion, broken, self.motion_sha256, "scene-hash")
        rules = {failure["rule"] for failure in verification["failures"]}
        self.assertIn("MOTION-SEQUENCE-TOPOLOGY-1", rules)
        self.assertIn("MOTION-SEQUENCE-SKELETON-1", rules)
        self.assertIn("MOTION-SEQUENCE-PROVENANCE-1", rules)

    def test_rejects_wrong_native_playback_range(self):
        self.readback["frame_range"] = [1.0, 12.0]
        verification = verify_motion_sequence(self.source, self.motion, self.readback, self.motion_sha256, "scene-hash")
        self.assertIn("MOTION-SEQUENCE-FRAMES-1", {failure["rule"] for failure in verification["failures"]})

    def test_rejects_stale_motion_receipt_identity(self):
        mutated_motion = copy.deepcopy(self.motion)
        mutated_motion["frames"][6]["controls"]["left_elbow_flex"] += 1.0
        verification = verify_motion_sequence(self.source, mutated_motion, self.readback, self.motion_sha256, "scene-hash")
        self.assertIn("MOTION-SEQUENCE-PROVENANCE-1", {failure["rule"] for failure in verification["failures"]})

    def test_rejects_nonfinite_surface_position(self):
        self.readback["frames"][6]["positions"][0][0] = float("nan")
        verification = verify_motion_sequence(self.source, self.motion, self.readback, self.motion_sha256, "scene-hash")
        self.assertFalse(verification["passed"])


if __name__ == "__main__":
    unittest.main()
