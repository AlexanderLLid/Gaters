import copy
import json
import unittest
from pathlib import Path

from rift_raid_harness import (
    adapt_built_site_export,
    canonical_json,
    evaluate_built_site_ensemble,
    evaluate_catalogs,
    format_summary,
    inspect_built_site_export,
)


ROOT = Path(__file__).resolve().parents[2]
SITE_PATH = ROOT / "research" / "raids-dungeons" / "synthetic-built-sites-v1.json"
SCENARIO_PATH = ROOT / "research" / "raids-dungeons" / "rift-raid-scenarios-v1.json"
COMBAT_PATH = ROOT / "research" / "combat" / "headless-combat-fixtures-v1.json"
EVIDENCE_PATH = ROOT / "research" / "raids-dungeons" / "generated-settlement-frontier-v1.json"
EXPORT_PATH = (
    ROOT
    / "research"
    / "settlements-bases-dungeons"
    / "generated-settlement-built-site-v1.json"
)


def load(path):
    return json.loads(path.read_text(encoding="utf-8"))


class RiftRaidHarnessTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.sites = load(SITE_PATH)
        cls.scenarios = load(SCENARIO_PATH)
        cls.combat = load(COMBAT_PATH)
        cls.export = load(EXPORT_PATH)
        cls.evidence = load(EVIDENCE_PATH)

    def test_production_export_is_adapted_and_ready_for_scenario(self):
        first = adapt_built_site_export(self.export)
        second = adapt_built_site_export(self.export)
        self.assertEqual(canonical_json(first), canonical_json(second))

        site = first["sites"][0]
        self.assertEqual("site:village:0", site["siteId"])
        self.assertEqual("settlement", site["siteKind"])
        self.assertEqual(765957514, site["checksum"])
        self.assertEqual(171, len(site["spaces"]))
        self.assertEqual(320, len(site["connections"]))
        self.assertEqual(320, len(site["visibility"]))
        self.assertEqual(150, len(site["blockers"]))
        self.assertEqual(129, len(site["placementSlots"]))
        self.assertEqual(
            {
                "placement": True,
                "traversalClearance": True,
                "visibility": True,
                "blockers": True,
                "sourceIds": ["site:village:0", "settlement-evidence-settings:1"],
            },
            site["evidenceCoverage"],
        )
        self.assertTrue(all(connection["movementModeIds"] for connection in site["connections"]))
        self.assertTrue(all(connection["widthCm"] > 0 for connection in site["connections"]))
        self.assertTrue(all(connection["headroomCm"] > 0 for connection in site["connections"]))
        self.assertEqual(
            self.export["siteRecipes"][0]["spaces"][0]["sourceIds"],
            site["spaces"][0]["sourceIds"],
        )

        results = inspect_built_site_export(self.export)
        self.assertEqual(canonical_json(results), canonical_json(inspect_built_site_export(self.export)))
        result = results[0]
        self.assertEqual("ready-for-scenario", result["evaluationState"])
        self.assertNotIn("policyMatrix", result)
        self.assertEqual([], result["findings"])
        self.assertIn(
            "site:village:0: evaluation=ready-for-scenario spaces=171 connections=320 findings=none",
            format_summary(results),
        )

    def test_production_representative_ensemble_is_deterministic_causal_and_analysis_only(self):
        source_before = canonical_json(self.export)
        first = evaluate_built_site_ensemble(self.export, self.scenarios, self.combat)
        second = evaluate_built_site_ensemble(self.export, self.scenarios, self.combat)

        self.assertEqual(canonical_json(first), canonical_json(second))
        self.assertEqual(source_before, canonical_json(self.export))
        self.assertEqual(1, len(first))

        result = first[0]
        self.assertEqual("analysis-only", result["evaluationState"])
        self.assertEqual("representative-ensemble", result["selection"]["policy"])
        self.assertEqual(
            ["abandoned-settlement-loot-v1"],
            result["scenario"]["compatibleScenarioIds"],
        )
        self.assertIn("evaluatedScenarioId", result["scenario"])
        self.assertEqual(
            "abandoned-settlement-loot-v1",
            result["scenario"]["evaluatedScenarioId"],
        )
        self.assertGreater(result["selection"]["physicalTupleCount"], 1)

        finding_codes = {item["code"] for item in result["findings"]}
        self.assertIn("world-approach-evidence-unknown", finding_codes)
        self.assertNotIn("no-compatible-scenario", finding_codes)
        self.assertNotIn("site-network-disconnected-objective", finding_codes)

        representatives = result["representatives"]
        buckets = {
            bucket
            for representative in representatives
            for bucket in representative["bucketIds"]
        }
        self.assertTrue(
            {
                "shallow",
                "medium",
                "deep",
                "visibility-exposed",
                "route-choice",
            }.issubset(buckets)
        )

        source_slot_ids = {
            slot["id"] for slot in self.export["siteRecipes"][0]["placementSlots"]
        }
        for representative in representatives:
            for key, slot_id in representative["roles"].items():
                if key.endswith("SlotId") and slot_id is not None:
                    self.assertIn(slot_id, source_slot_ids)

        by_bucket = {
            bucket: representative
            for representative in representatives
            for bucket in representative["bucketIds"]
        }
        shallow = by_bucket["shallow"]
        self.assertEqual(4, shallow["metrics"]["successfulRuns"])
        self.assertIn(
            "trivial-extraction",
            {item["code"] for item in shallow["findings"]},
        )

        medium = by_bucket["medium"]
        attacker_successes = {
            attacker_id: sum(
                run["outcome"] == "success"
                for run in medium["policyMatrix"]
                if run["attackerPolicyId"] == attacker_id
            )
            for attacker_id in {run["attackerPolicyId"] for run in medium["policyMatrix"]}
        }
        self.assertEqual({0, 2}, set(attacker_successes.values()))
        self.assertIn("policy-sensitive", {item["code"] for item in medium["findings"]})

        deep = by_bucket["deep"]
        self.assertEqual(0, deep["metrics"]["successfulRuns"])
        self.assertIn("raid-clock-impossible", {item["code"] for item in deep["findings"]})

        self.assertEqual(10, result["metrics"]["siteNetworkReachableObjectiveCount"])
        self.assertEqual(0, result["metrics"]["siteNetworkDisconnectedObjectiveCount"])

        self.assertIn(
            "site:village:0: evaluation=analysis-only representatives=5",
            format_summary(first),
        )

        exposed = by_bucket["visibility-exposed"]
        slots = {
            slot["id"]: slot for slot in self.export["siteRecipes"][0]["placementSlots"]
        }
        mutated = copy.deepcopy(self.export)
        recipe = mutated["siteRecipes"][0]
        recipe["visibility"] = [
            sight
            for sight in recipe["visibility"]
            if not (
                sight["fromSpaceId"] == slots[exposed["roles"]["guardSlotId"]]["spaceId"]
                and sight["toSpaceId"] == slots[exposed["roles"]["arrivalSlotId"]]["spaceId"]
            )
        ]
        changed = evaluate_built_site_ensemble(mutated, self.scenarios, self.combat)[0]
        changed_exposed = next(
            representative
            for representative in changed["representatives"]
            if "visibility-exposed" in representative["bucketIds"]
        )
        self.assertNotEqual(exposed["roles"], changed_exposed["roles"])

    def test_production_preflight_distinguishes_unknown_from_proved_empty_evidence(self):
        expected_unknown = {
            "placement": "placement-evidence-unknown",
            "traversalClearance": "traversal-clearance-unknown",
            "visibility": "visibility-evidence-unknown",
            "blockers": "blocker-evidence-unknown",
        }
        for coverage_key, expected_code in expected_unknown.items():
            with self.subTest(coverage=coverage_key):
                export = copy.deepcopy(self.export)
                export["siteRecipes"][0]["evidenceCoverage"][coverage_key] = False
                result = inspect_built_site_export(export)[0]
                self.assertEqual("insufficient-evidence", result["evaluationState"])
                self.assertIn(expected_code, {item["code"] for item in result["findings"]})

        export = copy.deepcopy(self.export)
        export["siteRecipes"][0]["placementSlots"] = []
        result = inspect_built_site_export(export)[0]
        self.assertIn("placement-evidence-empty", {item["code"] for item in result["findings"]})
        self.assertNotIn("placement-evidence-unknown", {item["code"] for item in result["findings"]})

        export = copy.deepcopy(self.export)
        export["siteRecipes"][0]["visibility"] = []
        result = inspect_built_site_export(export)[0]
        self.assertIn("visibility-evidence-empty", {item["code"] for item in result["findings"]})
        self.assertNotIn("visibility-evidence-unknown", {item["code"] for item in result["findings"]})

        export = copy.deepcopy(self.export)
        recipe = export["siteRecipes"][0]
        recipe["blockers"] = []
        for connection in recipe["connections"]:
            connection["blockerIds"] = []
        for sight in recipe["visibility"]:
            sight["blockerIds"] = []
        result = inspect_built_site_export(export)[0]
        self.assertEqual("ready-for-scenario", result["evaluationState"])
        self.assertNotIn("blocker-evidence-unknown", {item["code"] for item in result["findings"]})

    def test_ensemble_accepts_repaired_site_network_and_rejects_regression(self):
        result = evaluate_built_site_ensemble(self.export, self.scenarios, self.combat)[0]
        codes = {item["code"] for item in result["findings"]}

        self.assertNotIn("site-network-disconnected-objective", codes)
        self.assertEqual(10, result["metrics"]["siteNetworkReachableObjectiveCount"])
        self.assertEqual(0, result["metrics"]["siteNetworkDisconnectedObjectiveCount"])

        export = copy.deepcopy(self.export)
        target_space_id = "settlement:building:home:0:space"
        recipe = export["siteRecipes"][0]
        recipe["connections"] = [
            connection
            for connection in recipe["connections"]
            if target_space_id
            not in (connection["fromSpaceId"], connection["toSpaceId"])
        ]
        regressed = evaluate_built_site_ensemble(export, self.scenarios, self.combat)[0]
        disconnected = next(
            item
            for item in regressed["findings"]
            if item["code"] == "site-network-disconnected-objective"
        )
        self.assertEqual(
            {
                "slot:settlement:building:home:0:space",
            },
            {subject_id for subject_id in disconnected["subjectIds"] if subject_id.startswith("slot:")},
        )
        self.assertEqual(1, regressed["metrics"]["siteNetworkDisconnectedObjectiveCount"])

    def test_generated_settlement_evidence_matches_executable_ensemble(self):
        result = evaluate_built_site_ensemble(self.export, self.scenarios, self.combat)[0]
        snapshot = self.evidence["representativeEnsemble"]

        self.assertEqual(result["selection"]["physicalTupleCount"], snapshot["physicalTupleCount"])
        self.assertEqual(result["metrics"]["representativeCount"], snapshot["representativeCount"])
        self.assertEqual(
            [item["code"] for item in result["findings"]],
            snapshot["findings"],
        )
        actual = {
            bucket: representative
            for representative in result["representatives"]
            for bucket in representative["bucketIds"]
        }
        for expected in snapshot["representatives"]:
            representative = actual[expected["bucketId"]]
            for key in ("arrivalSlotId", "objectiveSlotId", "extractionSlotId"):
                if key in expected:
                    self.assertEqual(expected[key], representative["roles"][key])
            self.assertEqual(
                expected.get("successfulPolicyPairs", 0),
                representative["metrics"]["successfulRuns"],
            )
            self.assertEqual(
                expected["findingCodes"],
                [item["code"] for item in representative["findings"]],
            )

    def test_production_export_rejects_connection_without_movement_support(self):
        export = copy.deepcopy(self.export)
        export["siteRecipes"][0]["connections"][0]["movementModeIds"] = []
        with self.assertRaisesRegex(ValueError, "missing movement mode IDs"):
            adapt_built_site_export(export)

    def test_production_export_rejects_unsupported_units(self):
        export = copy.deepcopy(self.export)
        export["coordinateUnit"] = "metres"
        with self.assertRaisesRegex(ValueError, "unsupported Built Site export units"):
            adapt_built_site_export(export)

    def test_production_export_rejects_unsupported_recipe_contract(self):
        export = copy.deepcopy(self.export)
        export["siteRecipes"][0]["contractVersion"] = 2
        with self.assertRaisesRegex(ValueError, "unsupported Built Site Recipe contract"):
            adapt_built_site_export(export)

    def test_challenge_set_is_deterministic_and_causal(self):
        first = evaluate_catalogs(self.sites, self.scenarios, self.combat)
        second = evaluate_catalogs(self.sites, self.scenarios, self.combat)
        self.assertEqual(canonical_json(first), canonical_json(second))

        results = {result["caseId"]: result for result in first}
        self.assertEqual(
            {
                "open-settlement-raid-v1",
                "multi-route-fortress-raid-v1",
                "single-door-bunker-raid-v1",
                "sealed-dungeon-raid-v1",
            },
            set(results),
        )
        for result in results.values():
            self.assertEqual(1, result["evaluatorVersion"])
            self.assertEqual(1, result["combat"]["contractVersion"])
            self.assertEqual(4, len(result["policyMatrix"]))
            self.assertTrue(all("decisiveEventType" in run for run in result["policyMatrix"]))

        self.assertEqual(
            "pass",
            results["open-settlement-raid-v1"]["ratings"]["tacticalViability"],
        )
        self.assertGreaterEqual(
            results["multi-route-fortress-raid-v1"]["metrics"]["alternateApproachCount"],
            2,
        )

        bunker_findings = results["single-door-bunker-raid-v1"]["findings"]
        bunker = next(f for f in bunker_findings if f["code"] == "one-door-bunker")
        self.assertIn("bunker:arrival-to-choke", bunker["subjectIds"])
        self.assertIn("bunker:outer-mouth", bunker["sourceIds"])

        sealed = results["sealed-dungeon-raid-v1"]
        self.assertEqual("fail", sealed["ratings"]["tacticalViability"])
        unreachable = next(f for f in sealed["findings"] if f["code"] == "objective-unreachable")
        self.assertEqual(
            ["sealed:route", "sealed:slot:arrival", "sealed:slot:loot", "sealed:wall"],
            unreachable["subjectIds"],
        )

        summary = format_summary(first)
        self.assertIn("open-settlement-raid-v1: viability=pass approaches=1 success=4/4 findings=none", summary)
        self.assertIn("sealed-dungeon-raid-v1: viability=fail approaches=0 success=0/4", summary)

    def test_direct_prize_arrival_is_rejected_with_slot_ids(self):
        sites = copy.deepcopy(self.sites)
        open_site = next(site for site in sites["sites"] if site["siteId"] == "open-settlement-v1")
        arrival = next(slot for slot in open_site["placementSlots"] if "arrival" in slot["tags"])
        arrival["spaceId"] = "open:loot-room"

        result = next(
            item
            for item in evaluate_catalogs(sites, self.scenarios, self.combat)
            if item["caseId"] == "open-settlement-raid-v1"
        )
        finding = next(f for f in result["findings"] if f["code"] == "direct-prize-arrival")
        self.assertEqual(
            ["open:slot:arrival", "open:slot:loot"],
            finding["subjectIds"],
        )
        self.assertEqual("warn", result["ratings"]["fairness"])


if __name__ == "__main__":
    unittest.main()
