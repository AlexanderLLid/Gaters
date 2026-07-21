import copy
import sys
import unittest
from pathlib import Path


LAB_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(LAB_ROOT))

from reaction import synthesize_reaction


EVENT = {
    "eventId": "impact.demo",
    "type": "impact",
    "point": "chest",
    "direction": [1.0, -0.25, 0.1],
    "strength": 0.7,
}


class ReactionTests(unittest.TestCase):
    def test_same_event_and_seed_produce_same_recipe(self):
        first = synthesize_reaction(copy.deepcopy(EVENT), seed=17, fps=30)
        second = synthesize_reaction(copy.deepcopy(EVENT), seed=17, fps=30)
        self.assertEqual(first, second)

    def test_different_seed_changes_valid_reaction(self):
        first = synthesize_reaction(copy.deepcopy(EVENT), seed=17, fps=30)
        second = synthesize_reaction(copy.deepcopy(EVENT), seed=18, fps=30)
        self.assertNotEqual(first["variation"], second["variation"])
        self.assertNotEqual(first["keyframes"], second["keyframes"])

    def test_reaction_is_bounded_and_recovers_to_neutral(self):
        recipe = synthesize_reaction(copy.deepcopy(EVENT), seed=17, fps=30)
        self.assertLessEqual(recipe["limits"]["maximumRootDisplacementMeters"], 0.35)
        for keyframe in recipe["keyframes"]:
            self.assertLessEqual(
                sum(component * component for component in keyframe["rootLocationMeters"]) ** 0.5,
                recipe["limits"]["maximumRootDisplacementMeters"],
            )
        final = recipe["keyframes"][-1]
        self.assertEqual(final["rootLocationMeters"], [0.0, 0.0, 0.0])
        self.assertTrue(all(rotation == [0.0, 0.0, 0.0] for rotation in final["boneEulerDegrees"].values()))

    def test_keyframes_are_ordered_and_cover_one_second(self):
        recipe = synthesize_reaction(copy.deepcopy(EVENT), seed=17, fps=30)
        frames = [keyframe["frame"] for keyframe in recipe["keyframes"]]
        self.assertEqual(frames, sorted(set(frames)))
        self.assertEqual(frames[0], 1)
        self.assertEqual(frames[-1], 31)
        self.assertEqual(recipe["durationSeconds"], 1.0)


if __name__ == "__main__":
    unittest.main()
