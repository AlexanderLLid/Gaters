import json
import math
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from pose_skeleton import apply_pose


SKELETON_RUN = ROOT / "SkeletonRuns" / "humanoid-guide-skeleton-stick-humanoid-20260721-175401-425940" / "run-1"


class PoseSkeletonTests(unittest.TestCase):
    def setUp(self):
        self.skeleton = json.loads((SKELETON_RUN / "skeleton.json").read_text(encoding="utf-8"))
        self.rest = {joint["name"]: joint for joint in self.skeleton["joints"]}

    def test_knee_pose_moves_only_declared_descendant(self):
        pose = {"joint": "left_knee", "descendants": ["left_ankle"], "axis": [1.0, 0.0, 0.0], "degrees": 65.0}
        posed = {joint["name"]: joint for joint in apply_pose(self.skeleton, pose)}
        self.assertGreater(math.dist(self.rest["left_ankle"]["position"], posed["left_ankle"]["position"]), 0.1)
        self.assertEqual(self.rest["left_knee"]["position"], posed["left_knee"]["position"])
        self.assertEqual(self.rest["right_ankle"], posed["right_ankle"])

    def test_twist_rotates_frames_around_joint_aim(self):
        pose = {"joint": "waist", "descendants": ["chest", "head_center"], "axis": "joint_aim", "degrees": 30.0}
        posed = {joint["name"]: joint for joint in apply_pose(self.skeleton, pose)}
        self.assertNotEqual(self.rest["waist"]["basis"], posed["waist"]["basis"])
        self.assertNotEqual(self.rest["chest"]["basis"], posed["chest"]["basis"])
        self.assertEqual(self.rest["pelvis"], posed["pelvis"])

    def test_accepts_legacy_named_world_axis(self):
        pose = {"joint": "left_elbow", "descendants": ["left_wrist"], "axis": "y", "degrees": 55.0}
        posed = {joint["name"]: joint for joint in apply_pose(self.skeleton, pose)}
        self.assertGreater(math.dist(self.rest["left_wrist"]["position"], posed["left_wrist"]["position"]), 0.1)

    def test_rejects_unknown_descendant(self):
        pose = {"joint": "left_elbow", "descendants": ["missing"], "axis": [0.0, 1.0, 0.0], "degrees": 55.0}
        with self.assertRaisesRegex(ValueError, "POSE-JOINT-1"):
            apply_pose(self.skeleton, pose)


if __name__ == "__main__":
    unittest.main()
