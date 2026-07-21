import json
import math
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from guide_skeleton_compiler import compile_guide_skeleton


def load(path):
    return json.loads(path.read_text(encoding="utf-8"))


GUIDE_RUN = ROOT / "AnatomyRuns" / "humanoid-guide-stick-humanoid-20260721-174142-274786" / "run-1"
RECIPE = load(ROOT / "recipes" / "humanoid-guide-skeleton.json")


class GuideSkeletonCompilerTests(unittest.TestCase):
    def setUp(self):
        self.guide = load(GUIDE_RUN / "guide.json")
        self.skeleton = compile_guide_skeleton(self.guide, RECIPE, "guide-hash")
        self.joints = {joint["name"]: joint for joint in self.skeleton["joints"]}

    def test_compiles_one_rooted_joint_tree(self):
        self.assertEqual(16, len(self.joints))
        self.assertIsNone(self.joints["pelvis"]["parent"])
        self.assertEqual("pelvis", self.joints["waist"]["parent"])
        self.assertEqual("chest", self.joints["left_shoulder"]["parent"])
        self.assertEqual("left_knee", self.joints["left_ankle"]["parent"])

    def test_joint_positions_are_exact_guide_landmarks(self):
        for name, joint in self.joints.items():
            self.assertEqual(self.guide["landmarks"][name], joint["position"])

    def test_joint_frames_are_orthonormal(self):
        for joint in self.joints.values():
            axes = joint["basis"]
            for axis in axes.values():
                self.assertAlmostEqual(1.0, math.sqrt(sum(value * value for value in axis)))
            self.assertAlmostEqual(0.0, sum(a * b for a, b in zip(axes["aim"], axes["up"])))
            cross = [
                axes["aim"][1] * axes["up"][2] - axes["aim"][2] * axes["up"][1],
                axes["aim"][2] * axes["up"][0] - axes["aim"][0] * axes["up"][2],
                axes["aim"][0] * axes["up"][1] - axes["aim"][1] * axes["up"][0],
            ]
            for actual, expected in zip(cross, axes["side"]):
                self.assertAlmostEqual(expected, actual)

    def test_preserves_source_guide_identity(self):
        self.assertEqual("guide-hash", self.skeleton["source_guide_sha256"])
        self.assertEqual(self.guide["id"], self.skeleton["guide_id"])

    def test_rejects_disconnected_segment_graph(self):
        broken = json.loads(json.dumps(self.guide))
        broken["skeleton_segments"].pop()
        with self.assertRaisesRegex(ValueError, "SKELETON-GRAPH-1"):
            compile_guide_skeleton(broken, RECIPE, "guide-hash")


if __name__ == "__main__":
    unittest.main()
