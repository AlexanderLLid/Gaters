import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from skin_capture_comparator import compare


class SkinCaptureComparatorTests(unittest.TestCase):
    def test_promotes_better_bounded_challenger(self):
        recipe = {"minimum_thickness_ratio_p10": 0.88, "minimum_improvement": 0.05, "maximum_influences": 4}
        champion = {"passed": True, "metrics": {"elbow_thickness_ratio_p10": 0.79, "max_influences": 2}}
        challenger = {"passed": True, "metrics": {"elbow_thickness_ratio_p10": 0.91, "max_influences": 4}}
        self.assertTrue(compare(champion, challenger, recipe)["promote_challenger"])

    def test_rejects_challenger_with_excess_influences(self):
        recipe = {"minimum_thickness_ratio_p10": 0.88, "minimum_improvement": 0.05, "maximum_influences": 4}
        champion = {"passed": True, "metrics": {"elbow_thickness_ratio_p10": 0.79, "max_influences": 2}}
        challenger = {"passed": True, "metrics": {"elbow_thickness_ratio_p10": 0.95, "max_influences": 5}}
        self.assertFalse(compare(champion, challenger, recipe)["promote_challenger"])


if __name__ == "__main__":
    unittest.main()
