import json
import unittest
from pathlib import Path

from headless_raid_resolver import canonical_json, run_acceptance_case


ROOT = Path(__file__).resolve().parents[2]
FIXTURE_PATH = ROOT / "research" / "combat" / "headless-combat-fixtures-v1.json"


class HeadlessRaidResolverTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.catalog = json.loads(FIXTURE_PATH.read_text(encoding="utf-8"))

    def test_acceptance_cases_match_expected_matrices_and_are_deterministic(self):
        results = []
        for case in self.catalog["acceptanceCases"]:
            first = run_acceptance_case(self.catalog, case["id"])
            second = run_acceptance_case(self.catalog, case["id"])

            self.assertEqual(canonical_json(first), canonical_json(second), case["id"])
            self.assertEqual(case["expected"]["outcome"], first["outcome"], case["id"])
            self.assertEqual(
                case["expected"]["terminationReason"],
                first["terminationReason"],
                case["id"],
            )
            self.assertEqual(
                case["expected"]["decisiveEventType"],
                first["decisiveEvent"]["type"],
                case["id"],
            )

            for key in ("implicatedBlockerIds", "implicatedLinkIds"):
                if key in case["expected"]:
                    self.assertEqual(case["expected"][key], first[key], case["id"])

            self.assertIn("contractVersion", first)
            self.assertIn("fixtureVersion", first)
            self.assertIn("attackerPolicy", first)
            self.assertIn("defenderPolicy", first)
            self.assertIn("capabilityProfiles", first)
            self.assertTrue(first["events"], case["id"])
            results.append(first)

        by_arena = {}
        for result in results:
            by_arena.setdefault(result["arena"]["id"], []).append(result)

        self.assertEqual(4, sum(r["outcome"] == "success" for r in by_arena["open"]))
        self.assertEqual(1, sum(r["outcome"] == "success" for r in by_arena["fortified"]))
        self.assertEqual(0, sum(r["outcome"] == "success" for r in by_arena["sealed"]))


if __name__ == "__main__":
    unittest.main()
