import sys
import unittest
from pathlib import Path


LAB_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(LAB_ROOT))

from locomotion import REQUIRED_CLIP_NAMES, synthesize_locomotion_clips


class LocomotionRecipeTests(unittest.TestCase):
    def test_required_clips_are_unique_and_complete(self):
        clips = synthesize_locomotion_clips()
        self.assertEqual(tuple(clip["name"] for clip in clips), REQUIRED_CLIP_NAMES)
        self.assertEqual(len({clip["name"] for clip in clips}), len(REQUIRED_CLIP_NAMES))

    def test_every_clip_is_ordered_and_in_place(self):
        for clip in synthesize_locomotion_clips():
            frames = [key["frame"] for key in clip["keyframes"]]
            self.assertEqual(frames, sorted(frames))
            self.assertEqual(frames[0], clip["startFrame"])
            self.assertEqual(frames[-1], clip["endFrame"])
            self.assertTrue(
                all(
                    key["rootLocationMeters"] == [0.0, 0.0, 0.0]
                    for key in clip["keyframes"]
                )
            )
            self.assertTrue(
                all(
                    key["rootEulerDegrees"] == [0.0, 0.0, 0.0]
                    for key in clip["keyframes"]
                )
            )

    def test_non_idle_clips_change_at_least_one_pose_bone(self):
        for clip in synthesize_locomotion_clips()[1:]:
            poses = [key["boneEulerDegrees"] for key in clip["keyframes"]]
            self.assertTrue(any(pose != poses[0] for pose in poses[1:]), clip["name"])


if __name__ == "__main__":
    unittest.main()
