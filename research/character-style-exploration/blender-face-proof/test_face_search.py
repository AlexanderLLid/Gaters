from copy import deepcopy
from contextlib import redirect_stderr, redirect_stdout
import hashlib
from io import StringIO
import json
from pathlib import Path
import re
import shutil
from tempfile import TemporaryDirectory
import unittest

from face_search import (
    build_ballot_packages,
    candidate_to_controls,
    load_space,
    main,
    decide_promotion,
    _apply_decision,
    _candidate_id,
    new_state,
    project_learning_row,
    propose_pair,
    record_fresh_verifier_pass,
    resolve_run_path,
    validate_ballot,
    validate_candidate,
    validate_round_record,
)

from macro_grid import (
    build_grid,
    contour_extrema,
    decide_nomination,
    initialize_run,
    mouth_corner_indices,
    score_metrics,
    validate_metric_record,
)

from feature_grid import (
    build_feature_grid,
    body_signed_extrema,
    decide_feature_nomination,
    feature_effective_controls,
    feature_score,
    initialize_feature_run,
    macro_regression_gate,
    validate_feature_recipe,
    validate_feature_metric,
)
from feature_calibration import (
    build_calibration_grid,
    calibration_effective_controls,
    derive_calibration,
    load_task8_sources,
    validate_calibration_recipe,
)
from detail_grid import (
    build_detail_grid,
    direct_detail_shape_key_values,
    detail_macro_regression_gate,
    detail_ratios,
    detail_score,
    target_binding,
    target_detail_ratios,
    select_frontmost_landmark_vertex,
    validate_detail_recipe,
    validate_detail_target,
)

try:
    from sculpt_integrity import detail_override_duplicates, required_tool_scripts, temporary_run_root, validate_manifest_integrity
except ImportError:
    detail_override_duplicates = None
    required_tool_scripts = None
    temporary_run_root = None
    validate_manifest_integrity = None

try:
    from sculpt_integrity import invalidated_run_ids
except ImportError:
    invalidated_run_ids = None


ROOT = Path(__file__).resolve().parent


class FaceSearchTests(unittest.TestCase):
    def setUp(self):
        self.space = load_space(ROOT / "face-search-space.json")
        self.state = new_state(self.space, "test-run")
        self.temp = TemporaryDirectory(dir=ROOT / "Runs")
        self.artifact_root = ROOT
        self.state["runDirectory"] = f"Runs/{Path(self.temp.name).name}"
        self.derived_roots = []

    def tearDown(self):
        for root in self.derived_roots:
            if root.parent != (ROOT / "Derived" / "search-runs").resolve():
                raise RuntimeError("verifier test cleanup escaped Derived/search-runs")
            shutil.rmtree(root, ignore_errors=True)
        self.temp.cleanup()

    def propose(self, axis="nose_width", round_index=1):
        return propose_pair(self.state, axis, round_index, self.artifact_root)

    def reset_state(self, run_id):
        self.state = new_state(self.space, run_id)
        self.state["runDirectory"] = f"Runs/{Path(self.temp.name).name}"

    def replay_reference(self, name, contents):
        relative_path = f"{self.state['runDirectory']}/{name}"
        path = ROOT / relative_path
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(contents)
        return hashlib.sha256(contents).hexdigest(), relative_path

    def ballot(
        self,
        evaluator_id,
        winner_id,
        confidence=0.78,
        dimensions=None,
        hard_failures=None,
        severe_regressions=None,
        fresh_verifier_passed=None,
    ):
        raw = {
            "schemaVersion": 1,
            "evaluatorId": evaluator_id,
            "winnerSlot": "right",
            "confidence": confidence,
            "dimensions": dimensions or {
                "frontLikeness": "better",
                "threeDimensionalPlausibility": "same",
                "groundedMatureGeometry": "same",
                "individualCharacter": "better",
            },
            "hardFailures": hard_failures or [],
            "severeRegressions": severe_regressions or [],
            "diagnosis": ["nose remains too narrow"],
            "nextAxis": "nose_projection",
            "modelIdentity": f"{evaluator_id}-model-v1",
            "instructions": f"verbatim {evaluator_id} instruction",
            "response": f"verbatim {evaluator_id} response",
        }
        if fresh_verifier_passed is not None:
            raw["freshVerifierPassed"] = fresh_verifier_passed
        return validate_ballot(
            self.space,
            {"slots": {"left": "other", "center": self.state["champion"]["candidateId"], "right": winner_id}},
            raw,
        )

    def record_verifier_pass(self, candidate):
        output_root = (ROOT / "Derived" / "search-runs" / Path(self.temp.name).name).resolve()
        output = output_root / f"round-{candidate['round']}" / candidate["candidateId"]
        output.mkdir(parents=True, exist_ok=True)
        self.derived_roots.append(output_root)
        artifact = next(
            item for item in self.state["activeProposal"]["artifacts"]
            if item["candidateId"] == candidate["candidateId"]
        )
        source = ROOT / "Derived" / "face-v2" / "face-proof.blend"
        manifest = {
            "candidate": deepcopy(candidate),
            "candidateRecipe": {"path": artifact["path"], "sha256": artifact["hash"]},
            "controls": candidate_to_controls(candidate["parameters"]),
            "sourceBlend": {"path": str(source.resolve()), "sha256": hashlib.sha256(source.read_bytes()).hexdigest()},
            "toolVersions": {
                "sculptScript": hashlib.sha256((ROOT / "sculpt_clay.py").read_bytes()).hexdigest(),
                "faceSearch": hashlib.sha256((ROOT / "face_search.py").read_bytes()).hexdigest(),
            },
        }
        (output / "manifest.json").write_text(json.dumps(manifest), encoding="utf-8")
        log = ROOT / self.state["runDirectory"] / f"verify-{candidate['candidateId']}.log"
        log.write_text("SCULPT_VERIFY_OK views=3\n", encoding="utf-8")
        return record_fresh_verifier_pass(self.state, candidate["candidateId"], output, log)

    def round_record(self):
        lower, upper = self.propose()
        receipt = self.record_verifier_pass(upper)
        step = self.state["steps"]["nose_width"]
        return {
            "searchSpace": self.space,
            "championBefore": deepcopy(self.state["champion"]),
            "selectedAxis": "nose_width",
            "axisReason": ["nose remains too narrow"],
            "stepBefore": step,
            "candidates": [lower, upper],
            "recipes": [
                {"candidateId": lower["candidateId"], "delta": -step},
                {"candidateId": upper["candidateId"], "delta": step},
            ],
            "agents": {
                "source": {"identity": "sculpt-v13", "instructions": "source instructions", "response": "source response"},
                "tool": {"identity": "face_search.py", "instructions": "tool instructions", "response": "tool response"},
                "primary": {"identity": "primary", "modelIdentity": "primary-model-v1", "instructions": "verbatim primary instruction", "response": "verbatim primary response"},
                "audit": {"identity": "audit", "modelIdentity": "audit-model-v1", "instructions": "verbatim audit instruction", "response": "verbatim audit response"},
            },
            "ballots": [
                {**self.ballot("primary", upper["candidateId"]), "validated": True},
                {**self.ballot("audit", upper["candidateId"]), "validated": True},
            ],
            "failures": [],
            "decision": {"promote": True, "winnerId": upper["candidateId"], "nextStep": step},
            "championAfter": deepcopy(upper),
            "stepAfter": step,
            "artifacts": deepcopy(self.state["activeProposal"]["artifacts"]),
            "verifierReceipts": {upper["candidateId"]: receipt},
            "nextExperiment": {"axis": "nose_length"},
        }

    def test_geometry_only_search_uses_extended_limits(self):
        self.assertEqual(self.space["maxRounds"], 16)
        self.assertEqual(self.space["stopAfterNoPromotion"], 4)
        self.assertEqual(self.space["stopAfterDisagreement"], 3)

    def test_nose_width_maps_to_all_width_controls(self):
        controls = candidate_to_controls(
            {**self.state["champion"]["parameters"], "nose_width": 0.40}
        )
        self.assertEqual(controls["nose.width"], 0.40)
        self.assertEqual(controls["sculpt.nose.tip"], 0.28)
        self.assertEqual(controls["sculpt.nose.nostrils"], 0.24)

    def test_eye_spacing_is_bilateral(self):
        controls = candidate_to_controls(
            {**self.state["champion"]["parameters"], "eye_spacing": 0.10}
        )
        self.assertEqual(controls["sculpt.eye.left.out"], 0.10)
        self.assertEqual(controls["sculpt.eye.right.out"], 0.10)

    def test_pair_is_symmetric_around_champion(self):
        lower, upper = self.propose()
        current = self.state["champion"]["parameters"]["nose_width"]
        self.assertAlmostEqual(current - lower["parameters"]["nose_width"], 0.07)
        self.assertAlmostEqual(upper["parameters"]["nose_width"] - current, 0.07)
        self.assertEqual(lower["parentId"], self.state["champion"]["candidateId"])
        self.assertEqual(upper["parentId"], self.state["champion"]["candidateId"])

    def test_initial_champion_has_accepted_sculpt_v13_provenance(self):
        source = self.state["provenance"]["acceptedChampion"]
        self.assertEqual(source["identity"], "sculpt-v13")
        self.assertEqual(source["recipe"]["path"], "face-brief.json")
        self.assertEqual(len(source["recipe"]["hash"]), 64)

    def test_candidate_outside_axis_bounds_is_rejected(self):
        candidate = deepcopy(self.state["champion"])
        candidate["parameters"]["nose_width"] = 9.0
        with self.assertRaisesRegex(ValueError, "nose_width outside"):
            validate_candidate(self.space, candidate)

    def test_candidate_with_a_tampered_id_is_rejected(self):
        candidate = deepcopy(self.state["champion"])
        candidate["candidateId"] = "not-the-canonical-hash"
        with self.assertRaisesRegex(ValueError, "candidateId"):
            validate_candidate(self.space, candidate)

    def test_bool_parameter_is_rejected_as_non_numeric(self):
        candidate = deepcopy(self.state["champion"])
        candidate["parameters"]["nose_width"] = True
        with self.assertRaisesRegex(ValueError, "nose_width must be numeric"):
            validate_candidate(self.space, candidate)

    def test_ballot_resolves_blind_slot_without_parameter_access(self):
        package = {"slots": {"left": "id-a", "center": "champion", "right": "id-b"}}
        ballot = {
            "schemaVersion": 1,
            "evaluatorId": "primary",
            "winnerSlot": "right",
            "confidence": 0.78,
            "dimensions": {
                "frontLikeness": "better",
                "threeDimensionalPlausibility": "same",
                "groundedMatureGeometry": "same",
                "individualCharacter": "better"
            },
            "hardFailures": [],
            "severeRegressions": [],
            "diagnosis": ["nose remains too narrow"],
            "nextAxis": "nose_projection"
        }
        resolved = validate_ballot(self.space, package, ballot)
        self.assertEqual(resolved["winnerId"], "id-b")

    def test_ballot_rejects_unknown_dimension_axis_slot_failure_id_and_confidence(self):
        package = {"slots": {"left": "id-a", "center": "champion", "right": "id-b"}}
        ballot = {
            "schemaVersion": 1,
            "evaluatorId": "primary",
            "winnerSlot": "right",
            "confidence": 0.78,
            "dimensions": {
                "frontLikeness": "better",
                "threeDimensionalPlausibility": "same",
                "groundedMatureGeometry": "same",
                "individualCharacter": "better",
            },
            "hardFailures": [],
            "severeRegressions": [],
            "diagnosis": ["nose remains too narrow"],
            "nextAxis": "nose_projection",
        }
        for field, value, error in (
            ("winnerSlot", "outside", "slot"),
            ("confidence", 1.01, "confidence"),
            ("nextAxis", "outside", "axis"),
            ("hardFailures", ["outside"], "hard failure"),
        ):
            invalid = deepcopy(ballot)
            invalid[field] = value
            with self.assertRaisesRegex(ValueError, error):
                validate_ballot(self.space, package, invalid)
        invalid = deepcopy(ballot)
        invalid["dimensions"]["groundedMatureStyle"] = "same"
        with self.assertRaisesRegex(ValueError, "dimension"):
            validate_ballot(self.space, package, invalid)
        invalid = deepcopy(ballot)
        invalid["winnerId"] = "id-b"
        with self.assertRaisesRegex(ValueError, "candidate ID"):
            validate_ballot(self.space, package, invalid)

    def test_packages_reverse_challengers_without_blind_leaks(self):
        primary, audit = build_ballot_packages(
            "target.png",
            "champion",
            ["champion-front.png", "champion-three-quarter.png", "champion-profile.png"],
            [
                {"candidateId": "id-a", "images": ["a-front.png", "a-three-quarter.png", "a-profile.png"]},
                {"candidateId": "id-b", "images": ["b-front.png", "b-three-quarter.png", "b-profile.png"]},
            ],
        )
        self.assertEqual(primary["slots"]["left"]["candidateId"], audit["slots"]["right"]["candidateId"])
        self.assertEqual(primary["slots"]["right"]["candidateId"], audit["slots"]["left"]["candidateId"])
        self.assertEqual(primary["slots"]["center"], audit["slots"]["center"])
        self.assertEqual(primary["slots"]["left"]["images"], audit["slots"]["right"]["images"])
        self.assertNotRegex(json.dumps([primary, audit]), r'"(?:parameters|direction|diagnosis)"')

    def test_package_cli_writes_opposite_order_blind_packages(self):
        run_id = f"package-{Path(self.temp.name).name}"
        run_path = ROOT / "Runs" / run_id / "run.json"
        paths = []
        try:
            state = new_state(self.space, run_id)
            state["runDirectory"] = f"Runs/{run_id}"
            lower, upper = propose_pair(state, "nose_width", 1, ROOT)
            run_path.parent.mkdir(parents=True, exist_ok=True)
            run_path.write_text(json.dumps(state), encoding="utf-8")
            target = ROOT / state["runDirectory"] / "target.png"
            target.write_bytes(b"target")
            for name in ("champion", lower["candidateId"], upper["candidateId"]):
                image_root = ROOT / state["runDirectory"] / name
                image_root.mkdir()
                for view in ("front", "three-quarter", "profile"):
                    path = image_root / f"{view}.png"
                    path.write_bytes(name.encode("utf-8"))
                    paths.append(path)
            with redirect_stdout(StringIO()):
                self.assertEqual(main([
                    "package", "--run-id", run_id, "--target-image", str(target),
                    "--champion-images", str(target.parent / "champion"),
                    "--challenger-images", str(target.parent / lower["candidateId"]), str(target.parent / upper["candidateId"]),
                ]), 0)
            primary = json.loads((run_path.parent / "round-1" / "primary-package.json").read_text(encoding="utf-8"))
            audit = json.loads((run_path.parent / "round-1" / "audit-package.json").read_text(encoding="utf-8"))
            self.assertEqual(primary["slots"]["left"]["images"], audit["slots"]["right"]["images"])
            self.assertNotIn("parameters", json.dumps([primary, audit]))
        finally:
            shutil.rmtree(run_path.parent, ignore_errors=True)

    def test_package_cli_preserves_each_rounds_hash_bound_packages(self):
        run_id = f"package-rounds-{Path(self.temp.name).name}"
        run_path = ROOT / "Runs" / run_id / "run.json"
        try:
            state = new_state(self.space, run_id)
            state["runDirectory"] = f"Runs/{run_id}"
            lower, upper = propose_pair(state, "nose_width", 1, ROOT)
            run_path.parent.mkdir(parents=True, exist_ok=True)
            run_path.write_text(json.dumps(state), encoding="utf-8")
            target = ROOT / state["runDirectory"] / "target.png"
            target.write_bytes(b"target")

            def write_images(candidate_id):
                image_root = ROOT / state["runDirectory"] / candidate_id
                image_root.mkdir(exist_ok=True)
                for view in ("front", "three-quarter", "profile"):
                    (image_root / f"{view}.png").write_bytes(candidate_id.encode("utf-8"))
                return image_root

            champion = write_images("champion")
            lower_images = write_images(lower["candidateId"])
            upper_images = write_images(upper["candidateId"])
            with redirect_stdout(StringIO()):
                main(["package", "--run-id", run_id, "--target-image", str(target), "--champion-images", str(champion), "--challenger-images", str(lower_images), str(upper_images)])
            first_primary = run_path.parent / "round-1" / "primary-package.json"
            first_audit = run_path.parent / "round-1" / "audit-package.json"
            first_bytes = (first_primary.read_bytes(), first_audit.read_bytes())

            state = json.loads(run_path.read_text(encoding="utf-8"))
            state["activeProposal"]["finalized"] = True
            state["history"] = [{}]
            run_path.write_text(json.dumps(state), encoding="utf-8")
            with redirect_stdout(StringIO()):
                main(["propose", "--run-id", run_id, "--axis", "nose_width", "--round", "2"])
            state = json.loads(run_path.read_text(encoding="utf-8"))
            next_lower, next_upper = state["activeProposal"]["candidates"]
            with redirect_stdout(StringIO()):
                main(["package", "--run-id", run_id, "--target-image", str(target), "--champion-images", str(champion), "--challenger-images", str(write_images(next_lower["candidateId"])), str(write_images(next_upper["candidateId"]))])
            self.assertEqual(first_bytes, (first_primary.read_bytes(), first_audit.read_bytes()))
            self.assertTrue((run_path.parent / "round-2" / "primary-package.json").is_file())
        finally:
            shutil.rmtree(run_path.parent, ignore_errors=True)

    def test_promotion_requires_two_agreeing_blind_ballots(self):
        lower, upper = self.propose()
        self.record_verifier_pass(upper)
        ballots = [self.ballot("primary", upper["candidateId"]), self.ballot("audit", upper["candidateId"])]
        decision = decide_promotion(self.state, ballots)
        self.assertEqual(decision["winnerId"], upper["candidateId"])
        self.assertTrue(decision["promote"])

    def test_evaluator_verifier_flags_without_a_root_receipt_are_rejected(self):
        _, upper = self.propose()
        for evaluator_id in ("primary", "audit"):
            with self.assertRaisesRegex(ValueError, "fresh verifier"):
                self.ballot(evaluator_id, upper["candidateId"], fresh_verifier_passed=True)

    def test_promotion_requires_confidence_no_regressions_and_root_verifier_receipt(self):
        _, upper = self.propose()
        self.record_verifier_pass(upper)
        for primary in (
            self.ballot("primary", upper["candidateId"], confidence=0.59),
            self.ballot("primary", upper["candidateId"], dimensions={
                "frontLikeness": "better",
                "threeDimensionalPlausibility": "same",
                "groundedMatureGeometry": "worse",
                "individualCharacter": "better",
            }),
        ):
            decision = decide_promotion(self.state, [primary, self.ballot("audit", upper["candidateId"])])
            self.assertFalse(decision["promote"])
            self.assertEqual(decision["nextStep"], self.state["steps"]["nose_width"] / 2.0)

    def test_missing_root_verifier_receipt_keeps_the_champion(self):
        _, upper = self.propose()
        ballots = [self.ballot("primary", upper["candidateId"]), self.ballot("audit", upper["candidateId"])]
        decision = decide_promotion(self.state, ballots)
        self.assertFalse(decision["promote"])
        self.assertEqual(decision["nextStep"], self.state["steps"]["nose_width"] / 2.0)

    def test_promotion_requires_verbatim_evaluator_model_evidence(self):
        _, upper = self.propose()
        ballots = [self.ballot("primary", upper["candidateId"]), self.ballot("audit", upper["candidateId"])]
        del ballots[0]["instructions"]
        with self.assertRaisesRegex(ValueError, "verbatim"):
            decide_promotion(self.state, ballots)

    def test_proposal_writes_real_candidate_recipe_artifacts(self):
        lower, upper = self.propose()
        artifacts = self.state["activeProposal"]["artifacts"]
        self.assertEqual({artifact["candidateId"] for artifact in artifacts}, {lower["candidateId"], upper["candidateId"]})
        for artifact in artifacts:
            path = self.artifact_root / artifact["path"]
            self.assertTrue(path.is_file())
            self.assertEqual(artifact["hash"], hashlib.sha256(path.read_bytes()).hexdigest())

    def test_disagreement_keeps_champion_and_halves_step(self):
        lower, upper = self.propose()
        before = self.state["steps"]["nose_width"]
        ballots = [self.ballot("primary", lower["candidateId"]), self.ballot("audit", upper["candidateId"])]
        decision = decide_promotion(self.state, ballots)
        self.assertFalse(decision["promote"])
        self.assertEqual(decision["nextStep"], before / 2.0)

    def test_promotion_rejects_winner_outside_active_proposal(self):
        self.propose()
        ballots = [self.ballot("primary", "outside"), self.ballot("audit", "outside")]
        with self.assertRaisesRegex(ValueError, "active proposal"):
            decide_promotion(self.state, ballots)

    def test_proposal_stops_at_round_and_failure_limits(self):
        with self.assertRaisesRegex(ValueError, "maxRounds"):
            self.propose("nose_width", 17)
        self.state["noPromotionCount"] = 4
        with self.assertRaisesRegex(ValueError, "stopAfterNoPromotion"):
            self.propose()
        self.state["noPromotionCount"] = 0
        self.state["disagreementCount"] = 3
        with self.assertRaisesRegex(ValueError, "stopAfterDisagreement"):
            self.propose()

    def test_proposal_requires_the_next_unrecorded_round(self):
        _, upper = self.propose()
        self.record_verifier_pass(upper)
        ballots = [self.ballot("primary", upper["candidateId"]), self.ballot("audit", upper["candidateId"])]
        _apply_decision(self.state, decide_promotion(self.state, ballots), ballots)
        with self.assertRaisesRegex(ValueError, "next unrecorded round"):
            self.propose()

    def test_promotion_appends_immutable_round_and_learning_history(self):
        lower, upper = self.propose()
        self.record_verifier_pass(upper)
        ballots = [self.ballot("primary", upper["candidateId"]), self.ballot("audit", upper["candidateId"])]
        _apply_decision(self.state, decide_promotion(self.state, ballots), ballots)
        self.assertEqual(len(self.state["history"]), 1)
        self.assertEqual(len(self.state["learning"]), 1)
        record = self.state["history"][0]
        self.assertEqual(record["recipes"][0]["delta"], -0.07)
        self.assertEqual(record["decision"]["winnerId"], upper["candidateId"])
        self.assertEqual(record["artifacts"][0]["path"].endswith("/candidates/" + lower["candidateId"] + ".json"), True)
        expected_lower_width = lower["parameters"]["nose_width"]
        self.state["activeProposal"]["candidates"][0]["parameters"]["nose_width"] = 0.0
        self.assertEqual(record["candidates"][0]["parameters"]["nose_width"], expected_lower_width)

    def test_learning_row_keeps_rejected_candidates_and_failures(self):
        record = {
            "championBefore": "champion",
            "selectedAxis": "nose_width",
            "axisReason": ["nose remains too narrow"],
            "stepBefore": 0.07,
            "candidates": [
                {"candidateId": "lower", "delta": -0.07},
                {"candidateId": "upper", "delta": 0.07}
            ],
            "agents": {"sculptor": {}, "primary": {}, "audit": {}},
            "ballots": [],
            "failures": ["primary-audit-disagreement"],
            "decision": {"promote": False, "humanAccepted": False},
            "championAfter": "champion",
            "stepAfter": 0.035,
            "nextExperiment": {"axis": "nose_projection"}
        }
        row = project_learning_row(record)
        self.assertEqual([item["candidateId"] for item in row["candidates"]], ["lower", "upper"])
        self.assertEqual(row["failures"], ["primary-audit-disagreement"])
        self.assertFalse(row["decision"]["humanAccepted"])

    def test_learning_row_removes_only_redundant_payloads(self):
        image_hash, image_path = self.replay_reference("Images/rejected.png", b"raw-bytes")
        log_hash, log_path = self.replay_reference("Logs/rejected.log", b"raw-log")
        record = {
            "candidates": [
                {
                    "candidateId": "rejected",
                    "diagnosis": "nose too wide",
                    "imageBytes": "raw-bytes",
                    "imageHash": image_hash,
                    "imagePath": image_path,
                    "blenderLogs": "raw-log",
                    "blenderLogHash": log_hash,
                    "blenderLogPath": log_path
                }
            ],
            "failures": ["audit-rejected"],
        }
        row = project_learning_row(record)
        candidate = row["candidates"][0]
        self.assertNotIn("imageBytes", candidate)
        self.assertNotIn("blenderLogs", candidate)
        self.assertEqual(candidate["imageHash"], image_hash)
        self.assertEqual(candidate["blenderLogPath"], log_path)
        self.assertEqual(row["failures"], ["audit-rejected"])

    def test_learning_row_rejects_payload_without_replay_reference(self):
        with self.assertRaisesRegex(ValueError, "replay reference"):
            project_learning_row({"candidate": {"imageBytes": "raw"}})

    def test_learning_row_rejects_traversal_missing_and_bad_hash_references(self):
        with self.assertRaisesRegex(ValueError, "path"):
            project_learning_row({"imageBytes": "raw", "imageHash": "0" * 64, "imagePath": "../escape.png"})
        with self.assertRaisesRegex(ValueError, "does not exist"):
            project_learning_row({"blenderLogs": "raw", "blenderLogHash": "0" * 64, "blenderLogPath": f"{self.state['runDirectory']}/missing.log"})
        _, path = self.replay_reference("Logs/hash.log", b"actual")
        with self.assertRaisesRegex(ValueError, "hash"):
            project_learning_row({"blenderLogs": "raw", "blenderLogHash": "0" * 64, "blenderLogPath": path})

    def test_learning_row_rejects_artifact_without_hash_and_relative_path(self):
        with self.assertRaisesRegex(ValueError, "artifact records"):
            project_learning_row({"artifacts": [{"candidateId": "candidate", "kind": "still"}]})

    def test_round_record_requires_replayable_agent_and_ballot_evidence(self):
        record = self.round_record()
        validate_round_record(record, self.artifact_root)
        record["agents"] = {}
        with self.assertRaisesRegex(ValueError, "agents"):
            validate_round_record(record, self.artifact_root)
        self.reset_state("missing-source-record")
        record = self.round_record()
        del record["agents"]["source"]
        with self.assertRaisesRegex(ValueError, "source"):
            validate_round_record(record, self.artifact_root)

    def test_round_record_rejects_invalid_candidate_identity_and_bounds(self):
        record = self.round_record()
        record["candidates"][0]["candidateId"] = "wrong"
        with self.assertRaisesRegex(ValueError, "candidateId"):
            validate_round_record(record, self.artifact_root)
        self.reset_state("second-round-record")
        record = self.round_record()
        record["candidates"][0]["parameters"]["nose_width"] = True
        with self.assertRaisesRegex(ValueError, "nose_width must be numeric"):
            validate_round_record(record, self.artifact_root)

    def test_round_record_rejects_payload_without_replay_reference(self):
        record = self.round_record()
        record["artifacts"][0]["imageBytes"] = "raw"
        with self.assertRaisesRegex(ValueError, "replay reference"):
            validate_round_record(record, self.artifact_root)

    def test_round_record_rejects_traversal_and_recipe_mismatch(self):
        record = self.round_record()
        record["artifacts"][0]["path"] = "../escape.json"
        with self.assertRaisesRegex(ValueError, "artifact"):
            validate_round_record(record, self.artifact_root)
        self.reset_state("recipe-mismatch")
        record = self.round_record()
        artifact = record["artifacts"][0]
        path = self.artifact_root / artifact["path"]
        recipe = json.loads(path.read_text(encoding="utf-8"))
        recipe["parameters"]["nose_width"] = 0.0
        path.write_text(json.dumps(recipe), encoding="utf-8")
        artifact["hash"] = hashlib.sha256(path.read_bytes()).hexdigest()
        with self.assertRaisesRegex(ValueError, "recipe"):
            validate_round_record(record, self.artifact_root)

    def test_round_record_rejects_inexact_or_cross_axis_delta(self):
        record = self.round_record()
        record["recipes"][0]["delta"] = -0.01
        with self.assertRaisesRegex(ValueError, "delta"):
            validate_round_record(record, self.artifact_root)
        self.reset_state("cross-axis-delta")
        record = self.round_record()
        artifact = record["artifacts"][0]
        path = self.artifact_root / artifact["path"]
        recipe = json.loads(path.read_text(encoding="utf-8"))
        recipe["parameters"]["nose_length"] += 0.01
        recipe["candidateId"] = _candidate_id(
            self.space["schemaVersion"],
            recipe["parentId"],
            recipe["round"],
            recipe["axis"],
            recipe["direction"],
            recipe["parameters"],
        )
        candidate = record["candidates"][0]
        candidate.update(recipe)
        record["recipes"][0]["candidateId"] = recipe["candidateId"]
        artifact["candidateId"] = recipe["candidateId"]
        path.write_text(json.dumps(recipe), encoding="utf-8")
        artifact["hash"] = hashlib.sha256(path.read_bytes()).hexdigest()
        with self.assertRaisesRegex(ValueError, "selectedAxis"):
            validate_round_record(record, self.artifact_root)

    def test_round_record_rejects_contradictory_ballots_decision_and_champion(self):
        record = self.round_record()
        record["ballots"][0]["winnerId"] = "outside"
        with self.assertRaisesRegex(ValueError, "winner"):
            validate_round_record(record, self.artifact_root)
        self.reset_state("contradictory-decision")
        record = self.round_record()
        record["decision"]["promote"] = False
        with self.assertRaisesRegex(ValueError, "decision"):
            validate_round_record(record, self.artifact_root)
        self.reset_state("contradictory-champion")
        record = self.round_record()
        record["championAfter"] = deepcopy(record["championBefore"])
        with self.assertRaisesRegex(ValueError, "championAfter"):
            validate_round_record(record, self.artifact_root)

    def test_init_cli_writes_a_json_run_record(self):
        with TemporaryDirectory(dir=ROOT / "Runs") as directory:
            run_id = f"test-cli-{Path(directory).name}"
            run_path = ROOT / "Runs" / run_id / "run.json"
            try:
                with redirect_stdout(StringIO()):
                    result = main(["init", "--run-id", run_id])
                self.assertEqual(result, 0)
                self.assertEqual(load_space(ROOT / "face-search-space.json")["schemaVersion"], 1)
                self.assertTrue(run_path.is_file())
            finally:
                shutil.rmtree(run_path.parent, ignore_errors=True)

    def test_init_refuses_existing_run_without_overwriting_evidence(self):
        run_id = f"init-collision-{Path(self.temp.name).name}"
        run_path = ROOT / "Runs" / run_id / "run.json"
        try:
            with redirect_stdout(StringIO()):
                main(["init", "--run-id", run_id])
            state = json.loads(run_path.read_text(encoding="utf-8"))
            _, upper = propose_pair(state, "nose_width", 1, ROOT)
            ballots = [self.ballot("primary", upper["candidateId"]), self.ballot("audit", upper["candidateId"])]
            _apply_decision(state, decide_promotion(state, ballots), ballots)
            run_path.write_text(json.dumps(state, sort_keys=True), encoding="utf-8")
            before = run_path.read_bytes()
            with self.assertRaisesRegex(ValueError, "already exists"):
                main(["init", "--run-id", run_id])
            self.assertTrue(state["history"])
            self.assertTrue(state["learning"])
            self.assertEqual(run_path.read_bytes(), before)
        finally:
            shutil.rmtree(run_path.parent, ignore_errors=True)

    def test_resolve_run_path_rejects_escape_run_ids_without_creating_external_files(self):
        escape = ROOT.parent / f"escape-{Path(self.temp.name).name}"
        for run_id in (".", "..", "../../escape", "a/b", "a\\b", str(escape), "C:\\escape"):
            with self.assertRaisesRegex(ValueError, "run ID"):
                resolve_run_path(run_id)
            with self.assertRaisesRegex(ValueError, "run ID"):
                main(["init", "--run-id", run_id])
        self.assertFalse(escape.exists())

    def test_cli_accepts_only_run_id_for_propose_and_promote(self):
        with redirect_stderr(StringIO()):
            with self.assertRaises(SystemExit):
                main(["propose", "--run-json", "outside/run.json", "--axis", "nose_width", "--round", "1"])
            with self.assertRaises(SystemExit):
                main(["promote", "--run-json", "outside/run.json", "--ballots", "outside/ballots.json"])


class SculptManifestIntegrityTests(unittest.TestCase):
    def setUp(self):
        self.temp = TemporaryDirectory(dir=ROOT / "Runs")
        self.run_id = Path(self.temp.name).name
        self.candidate = json.loads(
            (ROOT / "Runs" / "task2-smoke-v3" / "candidates" /
             "b67952f4ca72a15f6a32cfd135a00c92697fe98a36aff6ab6cf6c8758004ea09.json").read_text(encoding="utf-8")
        )
        self.recipe_path = Path(self.temp.name) / "candidates" / f"{self.candidate['candidateId']}.json"
        self.recipe_path.parent.mkdir()
        self.recipe_path.write_text(json.dumps(self.candidate), encoding="utf-8")
        self.cleanup_target = None if temporary_run_root is None else temporary_run_root(self.run_id)
        self.output = (self.cleanup_target or ROOT / "Derived" / "search-runs" / self.run_id) / "round-1" / self.candidate["candidateId"]
        self.output.mkdir(parents=True)
        source = ROOT / "Derived" / "face-v2" / "face-proof.blend"
        self.manifest = {
            "candidate": deepcopy(self.candidate),
            "candidateRecipe": {
                "path": self.recipe_path.relative_to(ROOT).as_posix(),
                "sha256": hashlib.sha256(self.recipe_path.read_bytes()).hexdigest(),
            },
            "controls": candidate_to_controls(self.candidate["parameters"]),
            "sourceBlend": {"path": str(source.resolve()), "sha256": hashlib.sha256(source.read_bytes()).hexdigest()},
            "toolVersions": {
                "sculptScript": hashlib.sha256((ROOT / "sculpt_clay.py").read_bytes()).hexdigest(),
                "faceSearch": hashlib.sha256((ROOT / "face_search.py").read_bytes()).hexdigest(),
            },
        }

    def tearDown(self):
        if self.cleanup_target is not None:
            expected_parent = (ROOT / "Derived" / "search-runs").resolve()
            if self.cleanup_target.parent != expected_parent or self.cleanup_target in {expected_parent, expected_parent.parent}:
                raise RuntimeError("temporary cleanup escaped its unique run directory")
            shutil.rmtree(self.cleanup_target, ignore_errors=True)
        self.temp.cleanup()

    def validate(self, output=None, manifest=None):
        self.assertIsNotNone(validate_manifest_integrity, "manifest integrity helper is missing")
        target = output or self.output
        (target / "manifest.json").write_text(json.dumps(manifest or self.manifest), encoding="utf-8")
        return validate_manifest_integrity(target)

    def test_manifest_integrity_accepts_bound_candidate(self):
        self.assertEqual(self.validate()["candidateId"], self.candidate["candidateId"])

    def test_temporary_cleanup_is_scoped_to_unique_run_directory(self):
        self.assertIsNotNone(temporary_run_root, "temporary run root helper is missing")
        root = temporary_run_root(self.run_id)
        derived_root = (ROOT / "Derived" / "search-runs").resolve()
        self.assertEqual(root, (derived_root / self.run_id).resolve())
        self.assertEqual(root.parent, derived_root)
        self.assertNotIn(root, {derived_root, derived_root.parent})

    def test_manifest_integrity_rejects_recipe_manifest_mismatch(self):
        manifest = deepcopy(self.manifest)
        manifest["candidate"]["parameters"]["nose_width"] = 0.20
        with self.assertRaisesRegex(ValueError, "recipe does not match"):
            self.validate(manifest=manifest)

    def test_manifest_integrity_rejects_candidate_path_escape(self):
        manifest = deepcopy(self.manifest)
        manifest["candidateRecipe"]["path"] = "Runs/../face-search-space.json"
        with self.assertRaisesRegex(ValueError, "candidate recipe path"):
            self.validate(manifest=manifest)

    def test_manifest_integrity_rejects_changed_source_blend_hash(self):
        manifest = deepcopy(self.manifest)
        manifest["sourceBlend"]["sha256"] = "0" * 64
        with self.assertRaisesRegex(ValueError, "source blend hash"):
            self.validate(manifest=manifest)

    def test_manifest_integrity_rejects_wrong_controls(self):
        manifest = deepcopy(self.manifest)
        manifest["controls"]["nose.width"] = 0.99
        with self.assertRaisesRegex(ValueError, "controls"):
            self.validate(manifest=manifest)

    def test_manifest_integrity_rejects_unbound_feature_controls(self):
        parent = json.loads(
            (ROOT / "Runs" / "macro-grid-20260720-071000" / "candidates" /
             "eaec583dac7c9adf295510e82a06dd721dceb2aea371ced31bfdcaa88b884d5b.json").read_text(encoding="utf-8")
        )
        recipe = build_feature_grid(load_space(ROOT / "face-search-space.json"), parent)[0]
        recipe_path = Path(self.temp.name) / "candidates" / f"{recipe['candidateId']}.json"
        recipe_path.write_text(json.dumps(recipe), encoding="utf-8")
        output = temporary_run_root(self.run_id) / "round-8" / recipe["candidateId"]
        output.mkdir(parents=True)
        manifest = deepcopy(self.manifest)
        manifest.update(
            candidate=recipe,
            candidateRecipe={"path": recipe_path.relative_to(ROOT).as_posix(), "sha256": hashlib.sha256(recipe_path.read_bytes()).hexdigest()},
            controls=candidate_to_controls(recipe["parameters"]),
            featureControls={"values": {"eye_aperture": 0.11, "nose_length": 0.30, "nose_width": 0.26}, "sha256": "0" * 64},
        )
        (output / "manifest.json").write_text(json.dumps(manifest), encoding="utf-8")
        with self.assertRaisesRegex(ValueError, "featureControls"):
            validate_manifest_integrity(output)

    def test_feature_manifest_requires_a_hashed_static_scene_lock(self):
        parent = json.loads(
            (ROOT / "Runs" / "macro-grid-20260720-071000" / "candidates" /
             "eaec583dac7c9adf295510e82a06dd721dceb2aea371ced31bfdcaa88b884d5b.json").read_text(encoding="utf-8")
        )
        recipe = build_feature_grid(load_space(ROOT / "face-search-space.json"), parent)[0]
        recipe_path = Path(self.temp.name) / "candidates" / f"{recipe['candidateId']}.json"
        recipe_path.write_text(json.dumps(recipe), encoding="utf-8")
        output = temporary_run_root(self.run_id) / "round-8" / recipe["candidateId"]
        output.mkdir(parents=True)
        manifest = deepcopy(self.manifest)
        manifest.update(
            candidate=recipe,
            candidateRecipe={"path": recipe_path.relative_to(ROOT).as_posix(), "sha256": hashlib.sha256(recipe_path.read_bytes()).hexdigest()},
            controls=feature_effective_controls(recipe["parameters"], recipe["featureControls"]),
            featureControls=recipe["featureControls"],
        )
        (output / "manifest.json").write_text(json.dumps(manifest), encoding="utf-8")
        with self.assertRaisesRegex(ValueError, "scene lock"):
            validate_manifest_integrity(output)

    def test_manifest_integrity_rejects_wrong_output_path(self):
        wrong_output = self.output.parent / "wrong-candidate"
        wrong_output.mkdir()
        with self.assertRaisesRegex(ValueError, "output path"):
            self.validate(output=wrong_output)

    def test_manifest_integrity_rejects_invalidated_v3_evidence(self):
        output = ROOT / "Derived" / "search-runs" / "task2-smoke-v3" / "round-1" / self.candidate["candidateId"]
        with self.assertRaisesRegex(ValueError, "invalidated"):
            validate_manifest_integrity(output)

    def test_manifest_integrity_rejects_v4_evidence_after_face_search_changes(self):
        output = ROOT / "Derived" / "search-runs" / "task2-smoke-v4" / "round-1" / self.candidate["candidateId"]
        with self.assertRaisesRegex(ValueError, "faceSearch hash"):
            validate_manifest_integrity(output)

    def test_manifest_integrity_rejects_derived_output_without_canonical_recipe(self):
        run_id = f"missing-recipe-{self.run_id}"
        run_root = ROOT / "Derived" / "search-runs" / run_id
        try:
            output = run_root / "round-1" / self.candidate["candidateId"]
            output.mkdir(parents=True)
            manifest = deepcopy(self.manifest)
            manifest["candidateRecipe"]["path"] = f"Runs/{run_id}/candidates/{self.candidate['candidateId']}.json"
            (output / "manifest.json").write_text(json.dumps(manifest), encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "candidate recipe path"):
                validate_manifest_integrity(output)
        finally:
            if run_root.parent.resolve() != (ROOT / "Derived" / "search-runs").resolve():
                raise RuntimeError("derived test cleanup escaped its unique run directory")
            shutil.rmtree(run_root, ignore_errors=True)

    def test_malformed_artifact_loss_record_is_rejected(self):
        self.assertIsNotNone(invalidated_run_ids, "incident reader is missing")
        incident = ROOT / "Runs" / f"{self.run_id}-artifact-loss.json"
        try:
            incident.write_text("[]", encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "incident"):
                invalidated_run_ids()
        finally:
            incident.unlink(missing_ok=True)

    def test_invalidated_runs_are_collected_from_nested_incidents_only(self):
        self.assertIsNotNone(invalidated_run_ids, "incident reader is missing")
        incident_dir = Path(self.temp.name) / "incidents" / "nested"
        incident_dir.mkdir(parents=True)
        (incident_dir / "metric-incident.json").write_text(json.dumps({
            "incident": "metric-semantic",
            "invalidatedRunIds": ["nested-invalid", "also-invalid"],
        }), encoding="utf-8")
        (incident_dir / "chronology-leak-incident.json").write_text(json.dumps({
            "incident": "chronology-leak-artifact-only",
            "runId": self.run_id,
        }), encoding="utf-8")
        (incident_dir / "run-status-incident.json").write_text(json.dumps({
            "incident": "failed-calibration",
            "status": "invalid-for-promotion",
            "runId": "status-invalid",
        }), encoding="utf-8")

        invalidated = invalidated_run_ids()

        self.assertTrue({"nested-invalid", "also-invalid", "status-invalid"} <= invalidated)
        self.assertNotIn(self.run_id, invalidated)

    def test_calibration_manifest_binds_new_script_and_scene_lock(self):
        parent = json.loads((ROOT / "Runs" / "macro-grid-20260720-071000" / "candidates" /
                             "eaec583dac7c9adf295510e82a06dd721dceb2aea371ced31bfdcaa88b884d5b.json").read_text(encoding="utf-8"))
        sources = load_task8_sources(ROOT)
        target_record = json.loads((ROOT / "target-feature-geometry.json").read_text(encoding="utf-8"))
        calibration = derive_calibration(sources, target_record)
        recipe = build_calibration_grid(load_space(ROOT / "face-search-space.json"), parent, calibration)[0]
        recipe_path = Path(self.temp.name) / "candidates" / f"{recipe['candidateId']}.json"
        recipe_path.write_text(json.dumps(recipe), encoding="utf-8")
        output = temporary_run_root(self.run_id) / "round-9" / recipe["candidateId"]
        output.mkdir(parents=True)
        target = json.loads((ROOT / "target-feature-geometry.json").read_text(encoding="utf-8"))
        payload = {key: {} for key in ("camera", "objectTransforms", "visibleMaterials", "lights", "suppressedObjects", "world")}
        payload["targetImage"] = target["image"]
        lock = {"payload": payload, "sha256": hashlib.sha256(json.dumps(payload, sort_keys=True, separators=(",", ":")).encode()).hexdigest()}
        manifest = deepcopy(self.manifest)
        manifest.update(
            candidate=recipe,
            candidateRecipe={"path": recipe_path.relative_to(ROOT).as_posix(), "sha256": hashlib.sha256(recipe_path.read_bytes()).hexdigest()},
            controls=calibration_effective_controls(recipe["parameters"], recipe["calibrationControls"]),
            calibrationControls=recipe["calibrationControls"],
            calibration=calibration,
            sceneLock=lock,
        )
        manifest["toolVersions"]["calibrationScript"] = hashlib.sha256((ROOT / "sculpt_calibrated.py").read_bytes()).hexdigest()
        manifest["toolVersions"]["calibrationContract"] = hashlib.sha256((ROOT / "feature_calibration.py").read_bytes()).hexdigest()
        manifest["toolVersions"]["featureScript"] = hashlib.sha256((ROOT / "sculpt_features.py").read_bytes()).hexdigest()
        (output / "manifest.json").write_text(json.dumps(manifest), encoding="utf-8")

        self.assertEqual(validate_manifest_integrity(output)["candidateId"], recipe["candidateId"])
        tampered = deepcopy(manifest)
        tampered["toolVersions"]["calibrationContract"] = "0" * 64
        (output / "manifest.json").write_text(json.dumps(tampered), encoding="utf-8")
        with self.assertRaisesRegex(ValueError, "calibrationContract hash"):
            validate_manifest_integrity(output)


class FaceSearchFalsificationEvidenceTests(unittest.TestCase):
    """Read-only checks for the final, post-search evaluator falsification."""

    RUN_ID = "face-search-20260720-032234"

    @property
    def run_root(self):
        return ROOT / "Runs" / self.RUN_ID

    def read_json(self, relative_path):
        with (self.run_root / relative_path).open(encoding="utf-8") as stream:
            return json.load(stream)

    def test_completed_rounds_project_exactly_to_deterministic_learning_jsonl(self):
        state = self.read_json("run.json")
        self.assertEqual(len(state["history"]), 3)
        self.assertEqual(len(state["learning"]), 3)
        rebuilt = []
        for record in state["history"]:
            validate_round_record(record)
            self.assertEqual(len(record["candidates"]), 2)
            self.assertEqual(
                {candidate["candidateId"] for candidate in record["candidates"]},
                {recipe["candidateId"] for recipe in record["recipes"]},
            )
            self.assertEqual({ballot["evaluatorId"] for ballot in record["ballots"]}, {"primary", "audit"})
            self.assertTrue(record["failures"])
            self.assertTrue(all(ballot["diagnosis"] for ballot in record["ballots"]))
            self.assertNotIn("humanAccepted", record["decision"])
            row = project_learning_row(record)
            self.assertNotIn("humanAccepted", json.dumps(row, sort_keys=True))
            self.assertNotIn("humanAcceptance", json.dumps(row, sort_keys=True))
            rebuilt.append(row)
        self.assertEqual(state["learning"], rebuilt)
        expected = b"".join(
            json.dumps(row, sort_keys=True, separators=(",", ":")).encode("utf-8") + b"\n"
            for row in rebuilt
        )
        projection = (self.run_root / "generator-learning.jsonl").read_bytes()
        self.assertEqual(hashlib.sha256(projection).hexdigest(), hashlib.sha256(expected).hexdigest())
        self.assertEqual(projection, expected)

    def test_zero_promotions_keep_the_initial_champion_without_human_acceptance(self):
        state = self.read_json("run.json")
        initial = state["history"][0]["championBefore"]
        self.assertEqual(state["champion"], initial)
        self.assertFalse(any(record["decision"]["promote"] for record in state["history"]))
        stop = self.read_json("run-stop.json")
        self.assertEqual(stop["promotions"], 0)
        self.assertEqual(stop["championCandidateId"], initial["candidateId"])
        self.assertFalse(stop["humanAcceptance"])

    def test_heldout_candidates_never_enter_search_history_or_learning(self):
        state = self.read_json("run.json")
        selected = self.read_json("heldout-final.json")
        self.assertEqual(selected["status"], "selected-post-search-counterexample")
        self.assertFalse(selected["humanAcceptance"])
        for reference in ("sourceRecipe", "sourceManifest"):
            path = ROOT / selected[reference]["path"]
            self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(), selected[reference]["sha256"])
        source_manifest = json.loads((ROOT / selected["sourceManifest"]["path"]).read_text(encoding="utf-8"))
        self.assertEqual(source_manifest["candidate"]["candidateId"], selected["candidateId"])
        heldout_ids = set()
        for path in self.run_root.glob("heldout*.json"):
            value = json.loads(path.read_text(encoding="utf-8"))
            candidate_id = value.get("candidateId") or value.get("candidate", {}).get("candidateId")
            if candidate_id:
                heldout_ids.add(candidate_id)
        self.assertTrue(heldout_ids)
        seen = json.dumps({"history": state["history"], "learning": state["learning"]}, sort_keys=True)
        self.assertTrue(all(candidate_id not in seen for candidate_id in heldout_ids))
        for ballot_path in self.run_root.glob("round-*/ballots.json"):
            ballots = ballot_path.read_text(encoding="utf-8")
            self.assertTrue(all(candidate_id not in ballots for candidate_id in heldout_ids))

    def test_final_replay_matches_semantics_and_render_dimensions_not_bytes(self):
        replay_record = self.read_json("final-replay.json")
        self.assertFalse(replay_record["humanAcceptance"])
        for reference in ("sourceRecipe", "manifest"):
            path = ROOT / replay_record[reference]["path"]
            self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(), replay_record[reference]["sha256"])
        recipe_path = ROOT / replay_record["sourceRecipe"]["path"]
        replay_path = ROOT / replay_record["manifest"]["path"]
        recipe = json.loads(recipe_path.read_text(encoding="utf-8"))
        replay = json.loads(replay_path.read_text(encoding="utf-8"))
        self.assertEqual(replay_record["candidateId"], recipe["candidateId"])
        self.assertEqual(replay["candidate"], recipe)
        self.assertEqual(replay["candidateRecipe"]["path"], replay_record["sourceRecipe"]["path"])
        self.assertEqual(replay["controls"], candidate_to_controls(recipe["parameters"]))
        self.assertEqual(replay["views"], ["front", "three-quarter", "profile"])
        self.assertTrue(replay["hardChecks"]["completed"])
        self.assertTrue(replay["hardChecks"]["referenceProjection"])
        self.assertTrue(replay["hardChecks"]["editableShapeKey"])
        self.assertFalse(replay["hardChecks"]["hasArmature"])
        self.assertFalse(replay["hardChecks"]["hasAnimation"])
        self.assertEqual(replay["hardChecks"]["renderSize"], [768, 768])
        self.assertTrue(replay["hardChecks"]["visibleMaterialsUseNoImages"])
        self.assertTrue(replay["toolVersions"])
        self.assertIn("candidate identity", replay_record["semanticResult"])
        self.assertIn("byte identity was not required", replay_record["semanticResult"])
        for view, output_name in (("front", "front"), ("three-quarter", "threeQuarter"), ("profile", "profile")):
            replay_image = replay_path.parent / f"{view}.png"
            self.assertEqual(replay_image.read_bytes()[16:24], b"\x00\x00\x03\x00\x00\x00\x03\x00")

    def test_final_audits_are_opposite_order_and_reject_the_selected_counterexample(self):
        heldout = self.read_json("heldout-final.json")
        primary = self.read_json("final-audit-primary.json")
        audit = self.read_json("final-audit-audit.json")
        slot_map = self.read_json("final-audit/slot-map.json")
        self.assertTrue(slot_map["notProvidedToEvaluators"])
        for record, evaluator_id in ((primary, "primary"), (audit, "audit")):
            self.assertEqual(record["evaluatorId"], evaluator_id)
            self.assertTrue(record["rejectCounterexample"])
            self.assertFalse(record["humanAcceptance"])
            self.assertIn(record["initialVsFinal"], {"initial", "final", "tie"})
            package_path = ROOT / record["package"]["path"]
            self.assertEqual(hashlib.sha256(package_path.read_bytes()).hexdigest(), record["package"]["sha256"])
            package = json.loads(package_path.read_text(encoding="utf-8"))
            self.assertEqual(set(package["slots"]), {"left", "center", "right"})
            package_text = json.dumps(package, sort_keys=True).lower()
            for leak in ("sculpt-v13", "round-", "initial", "final-replay", "counterexample"):
                self.assertNotIn(leak, package_text)
            self.assertIsNone(re.search(r"(?<![0-9a-f])[0-9a-f]{64}(?![0-9a-f])", package_text))
            self.assertNotIn("parameters", package_text)
            self.assertTrue(all(len(slot) == 3 for slot in package["slots"].values()))
            for slot, image_paths in package["slots"].items():
                actual_hashes = [hashlib.sha256((ROOT / path).read_bytes()).hexdigest() for path in image_paths]
                self.assertEqual(actual_hashes, slot_map["imageHashes"][evaluator_id][slot])
            self.assertEqual(slot_map[evaluator_id][record["counterexampleSlot"]], "counterexample")
            self.assertEqual(
                slot_map["imageHashes"][evaluator_id][record["counterexampleSlot"]],
                [heldout["outputs"][view]["sha256"] for view in ("front", "threeQuarter", "profile")],
            )
        self.assertEqual(slot_map["primary"]["left"], slot_map["audit"]["right"])
        self.assertEqual(slot_map["primary"]["right"], slot_map["audit"]["left"])
        self.assertEqual(slot_map["imageHashes"]["primary"]["left"], slot_map["imageHashes"]["audit"]["right"])
        self.assertEqual(slot_map["imageHashes"]["primary"]["right"], slot_map["imageHashes"]["audit"]["left"])


class MacroGeometryTests(unittest.TestCase):
    """Pure contract for Task 7's geometry-only macro grid."""

    def setUp(self):
        self.space = load_space(ROOT / "face-search-space.json")
        self.retained = json.loads(
            (ROOT / "Runs" / "face-search-20260720-032234" / "candidates" /
             "9e7d3f47b84eb6eb3f015cd078dfbdab6747321a49f832b47746a7990f618c44.json").read_text(encoding="utf-8")
        )
        self.target = {
            "image": {"sha256": "a" * 64, "size": [1254, 1254]},
            "ratios": {"cheekWidth": 1.8, "jawWidth": 1.7, "mouthWidth": 1.0, "eyeToChinHeight": 1.8, "lowerFaceTaper": 0.94},
        }
        self.metrics = {
            "schemaVersion": 1,
            "targetImage": {"sha256": "a" * 64},
            "camera": {"sha256": "b" * 64},
            "topology": {"vertexCount": 100, "sha256": "c" * 64},
            "anchors": {"leftEye": [1], "rightEye": [2], "leftCheek": [3], "rightCheek": [4], "leftJaw": [5], "rightJaw": [6], "leftMouth": [7], "rightMouth": [8], "chin": [9]},
            "ratios": {"cheekWidth": 1.8, "jawWidth": 1.7, "mouthWidth": 1.0, "eyeToChinHeight": 1.8, "lowerFaceTaper": 0.94},
        }

    def test_macro_grid_has_exactly_eight_unique_bounded_recipes(self):
        grid = build_grid(self.space, self.retained, "macro-test")
        self.assertEqual(len(grid), 8)
        self.assertEqual(len({candidate["candidateId"] for candidate in grid}), 8)
        self.assertEqual(
            {(c["parameters"]["head_width"], c["parameters"]["head_height"], c["parameters"]["jaw_taper"]) for c in grid},
            {(-0.08, 0.0, 0.085), (-0.08, 0.0, 0.0), (-0.08, -0.2, 0.085), (-0.08, -0.2, 0.0),
             (0.1, 0.0, 0.085), (0.1, 0.0, 0.0), (0.1, -0.2, 0.085), (0.1, -0.2, 0.0)},
        )

    def test_contour_bands_match_narrower_jaw_semantics_and_penalize_extra_taper(self):
        points = [
            {"x": -1.00, "z": 0.65, "isEar": False}, {"x": 1.00, "z": 0.65, "isEar": False},
            {"x": -0.75, "z": 0.35, "isEar": False}, {"x": 0.75, "z": 0.35, "isEar": False},
            {"x": -1.50, "z": 0.65, "isEar": True}, {"x": 1.50, "z": 0.65, "isEar": True},
            {"x": -2.00, "z": 0.65, "isEar": False, "isBody": False},
        ]
        cheek = contour_extrema(points, 0.35, 0.02, 1.0, 0.0)
        jaw = contour_extrema(points, 0.65, 0.02, 1.0, 0.0)
        self.assertEqual((cheek["left"]["x"], cheek["right"]["x"]), (-1.0, 1.0))
        self.assertEqual((jaw["left"]["x"], jaw["right"]["x"]), (-0.75, 0.75))
        closer = deepcopy(self.metrics)
        closer["ratios"]["lowerFaceTaper"] = 0.95
        extra_taper = deepcopy(self.metrics)
        extra_taper["ratios"]["lowerFaceTaper"] = 0.80
        self.assertLess(score_metrics(self.target, closer), score_metrics(self.target, extra_taper))

    def test_mouth_corners_use_maximum_signed_horizontal_target_displacement(self):
        corners = mouth_corner_indices([(20, -0.001), (21, -0.014), (22, 0.013), (23, 0.002)])
        self.assertEqual(corners, (21, 22))

    def test_metric_record_rejects_non_anatomical_mouth_width(self):
        record = deepcopy(self.metrics)
        record["ratios"]["mouthWidth"] = 1.6
        with self.assertRaisesRegex(ValueError, "mouth width"):
            validate_metric_record(self.target, record, "b" * 64, 100, "c" * 64)

    def test_geometry_score_is_order_independent(self):
        first = score_metrics(self.target, self.metrics)
        shuffled = {key: self.metrics[key] for key in reversed(tuple(self.metrics))}
        shuffled["ratios"] = dict(reversed(tuple(self.metrics["ratios"].items())))
        self.assertEqual(first, score_metrics(self.target, shuffled))

    def test_metric_record_rejects_changed_provenance_and_invalid_geometry(self):
        validate_metric_record(self.target, self.metrics, "b" * 64, 100, "c" * 64)
        cases = (
            ("target", lambda value: value["targetImage"].update(sha256="0" * 64)),
            ("camera", lambda value: value["camera"].update(sha256="0" * 64)),
            ("topology", lambda value: value["topology"].update(vertexCount=99)),
            ("anchors", lambda value: value["anchors"].pop("chin")),
            ("finite", lambda value: value["ratios"].update(jawWidth=float("nan"))),
        )
        for name, change in cases:
            with self.subTest(name=name):
                record = deepcopy(self.metrics)
                change(record)
                with self.assertRaises(ValueError):
                    validate_metric_record(self.target, record, "b" * 64, 100, "c" * 64)

    def test_nomination_requires_material_improvement_margin(self):
        retained = {"candidateId": "retained", "score": 0.100}
        close = {"candidateId": "close", "score": 0.091}
        winner = {"candidateId": "winner", "score": 0.079}
        self.assertEqual(decide_nomination(retained, [close], 0.02)["status"], "retained")
        result = decide_nomination(retained, [winner, close], 0.02)
        self.assertEqual(result["status"], "nominated")
        self.assertEqual(result["candidateId"], "winner")

    def test_macro_run_initialization_writes_all_and_only_grid_recipes(self):
        with TemporaryDirectory() as directory:
            root = Path(directory)
            run = initialize_run(self.space, self.retained, self.target, "macro-test", root)
            recipes = sorted((root / "candidates").glob("*.json"))
            self.assertEqual(len(recipes), 8)
            self.assertEqual(run["candidateCount"], 8)
            self.assertEqual(run["retainedCandidate"]["candidateId"], self.retained["candidateId"])
            self.assertEqual(run["minimumImprovementMargin"], 0.02)
            with self.assertRaises(FileExistsError):
                initialize_run(self.space, self.retained, self.target, "macro-test", root)


class FeatureGeometryTests(unittest.TestCase):
    """Pure Task 8 contract: named front-supported feature controls only."""

    def setUp(self):
        self.space = load_space(ROOT / "face-search-space.json")
        self.parent = json.loads(
            (ROOT / "Runs" / "macro-grid-20260720-071000" / "candidates" /
             "eaec583dac7c9adf295510e82a06dd721dceb2aea371ced31bfdcaa88b884d5b.json").read_text(encoding="utf-8")
        )
        self.target = {
            "image": {"sha256": "a" * 64, "size": [1254, 1254]},
            "ratios": {"eyeAperture": 0.17, "noseLength": 0.60, "alarWidth": 0.74},
        }
        self.metric = {
            "targetImage": {"sha256": "a" * 64},
            "camera": {"sha256": "b" * 64},
            "sceneLock": {"sha256": "c" * 64},
            "topology": {"vertexCount": 19158, "sha256": "d" * 64},
            "anchors": {
                "leftUpperLid": {"index": 1}, "leftLowerLid": {"index": 2},
                "rightUpperLid": {"index": 3}, "rightLowerLid": {"index": 4},
                "nasion": {"index": 5}, "subnasale": {"index": 6},
                "leftAlare": {"index": 7}, "rightAlare": {"index": 8},
                "leftEyeCenter": {"index": 9}, "rightEyeCenter": {"index": 10},
            },
            "ratios": {"eyeAperture": 0.17, "noseLength": 0.60, "alarWidth": 0.74},
        }

    def test_feature_grid_has_exactly_eight_full_vector_recipes_and_semantic_replay(self):
        recipes = build_feature_grid(self.space, self.parent)
        self.assertEqual(len(recipes), 8)
        self.assertEqual(len({recipe["candidateId"] for recipe in recipes}), 8)
        vectors = {tuple(recipe["featureControls"]["values"].values()) for recipe in recipes}
        self.assertEqual(vectors, {
            (0.0, 0.30, 0.26), (0.0, 0.30, 0.33), (0.0, 0.37, 0.26), (0.0, 0.37, 0.33),
            (0.12, 0.30, 0.26), (0.12, 0.30, 0.33), (0.12, 0.37, 0.26), (0.12, 0.37, 0.33),
        })
        replay = next(recipe for recipe in recipes if recipe["featureControls"]["values"] == {
            "eye_aperture": 0.0, "nose_length": 0.30, "nose_width": 0.26,
        })
        self.assertEqual(replay["parameters"], self.parent["parameters"])
        for recipe in recipes:
            self.assertEqual(recipe["parameters"]["head_width"], self.parent["parameters"]["head_width"])
            self.assertEqual(recipe["parameters"]["jaw_taper"], self.parent["parameters"]["jaw_taper"])

    def test_feature_sculpt_adds_its_directory_to_blender_import_path_before_local_imports(self):
        source = (ROOT / "sculpt_features.py").read_text(encoding="utf-8")
        self.assertLess(source.index("sys.path.insert"), source.index("from face_search import"))

    def test_feature_metric_adds_its_directory_to_blender_import_path_before_local_imports(self):
        source = (ROOT / "measure_feature_geometry.py").read_text(encoding="utf-8")
        self.assertLess(source.index("sys.path.insert"), source.index("from feature_grid import"))

    def test_feature_metric_does_not_shadow_scene_lock_function(self):
        source = (ROOT / "measure_feature_geometry.py").read_text(encoding="utf-8")
        self.assertNotIn("scene_lock = manifest", source)

    def test_feature_identity_rejects_undeclared_or_unhashed_control_change(self):
        recipe = build_feature_grid(self.space, self.parent)[0]
        changed = deepcopy(recipe)
        changed["featureControls"]["values"]["eye_aperture"] = 0.11
        with self.assertRaisesRegex(ValueError, "featureControls"):
            validate_feature_recipe(self.space, self.parent, changed)

    def test_feature_run_initialization_preserves_exact_recipe_set(self):
        with TemporaryDirectory() as directory:
            root = Path(directory)
            run = initialize_feature_run(self.space, self.parent, self.target, "feature-test", root)
            self.assertEqual(run["candidateCount"], 8)
            self.assertEqual(len(list((root / "candidates").glob("*.json"))), 8)
            self.assertEqual(run["allLowReplay"]["featureControls"]["values"], {"eye_aperture": 0.0, "nose_length": 0.30, "nose_width": 0.26})
            with self.assertRaises(FileExistsError):
                initialize_feature_run(self.space, self.parent, self.target, "feature-test", root)

    def test_feature_metric_requires_role_equivalent_finite_geometry(self):
        validate_feature_metric(self.target, self.metric, "b" * 64, "c" * 64, 19158, "d" * 64)
        for key, change in (
            ("target", lambda value: value["targetImage"].update(sha256="0" * 64)),
            ("scene", lambda value: value["sceneLock"].update(sha256="0" * 64)),
            ("anchor", lambda value: value["anchors"].pop("nasion")),
            ("finite", lambda value: value["ratios"].update(alarWidth=float("nan"))),
        ):
            with self.subTest(key=key):
                record = deepcopy(self.metric)
                change(record)
                with self.assertRaises(ValueError):
                    validate_feature_metric(self.target, record, "b" * 64, "c" * 64, 19158, "d" * 64)

    def test_feature_target_extrema_exclude_non_body_helper_vertices(self):
        records = [(1, 0.0, 0.02, 0.0), (2, 0.0, -0.02, 0.0), (99, 0.0, -0.90, 0.0)]
        self.assertEqual(body_signed_extrema(records, {1, 2}, 2), (1, 2))
        with self.assertRaisesRegex(ValueError, "body"):
            body_signed_extrema(records, {1}, 2)

    def test_feature_target_ratios_use_the_metric_horizontal_eye_center_distance(self):
        target = json.loads((ROOT / "target-feature-geometry.json").read_text(encoding="utf-8"))
        points = target["points"]
        distance = abs(points["rightEyeCenter"][0] - points["leftEyeCenter"][0])
        expected = {
            "eyeAperture": ((abs(points["leftUpperLid"][1] - points["leftLowerLid"][1]) + abs(points["rightUpperLid"][1] - points["rightLowerLid"][1])) / 2) / distance,
            "noseLength": abs(points["nasion"][1] - points["subnasale"][1]) / distance,
            "alarWidth": abs(points["rightAlare"][0] - points["leftAlare"][0]) / distance,
        }
        self.assertEqual(target["ratios"], expected)
        self.assertEqual(target["uncertaintyRatios"], {"eyeAperture": 8 / distance, "noseLength": 13 / distance, "alarWidth": 10 / distance})
        self.assertEqual(target["totalUncertaintyRatio"], sum(target["uncertaintyRatios"].values()))

    def test_feature_nomination_requires_margin_and_macro_hard_gates(self):
        baseline = {"candidateId": "baseline", "score": 0.10}
        winner = {"candidateId": "winner", "score": 0.07}
        self.assertEqual(feature_score(self.target, self.metric), 0.0)
        self.assertEqual(decide_feature_nomination(baseline, [winner], 0.02, False, True)["status"], "retained")
        self.assertEqual(decide_feature_nomination(baseline, [winner], 0.02, True, False)["status"], "retained")
        self.assertEqual(decide_feature_nomination(baseline, [winner], 0.02, True, True)["status"], "nominated")
        self.assertEqual(decide_feature_nomination(baseline, [winner], 0.02, True, True, 0.04)["status"], "retained")


class FeatureCalibrationTests(unittest.TestCase):
    def setUp(self):
        self.parent = json.loads((ROOT / "Runs" / "macro-grid-20260720-071000" / "candidates" /
                                  "eaec583dac7c9adf295510e82a06dd721dceb2aea371ced31bfdcaa88b884d5b.json").read_text())
        self.space = load_space(ROOT / "face-search-space.json")
        self.sources = [
            {"candidateId": "low", "controls": {"eye_aperture": 0.0, "nose_length": 0.30, "nose_width": 0.26}, "ratios": {"eyeAperture": 0.10, "noseLength": 0.76, "alarWidth": 0.70}, "sha256": "a" * 64},
            {"candidateId": "high-eye", "controls": {"eye_aperture": 0.12, "nose_length": 0.30, "nose_width": 0.26}, "ratios": {"eyeAperture": 0.12, "noseLength": 0.76, "alarWidth": 0.70}, "sha256": "b" * 64},
            {"candidateId": "low-nose", "controls": {"eye_aperture": 0.0, "nose_length": 0.37, "nose_width": 0.26}, "ratios": {"eyeAperture": 0.10, "noseLength": 0.78, "alarWidth": 0.70}, "sha256": "c" * 64},
        ]
        self.target = {"ratios": {"eyeAperture": 0.17, "noseLength": 0.70, "alarWidth": 0.70}}

    def test_calibration_derives_slopes_and_binds_source_hashes(self):
        calibration = derive_calibration(self.sources, self.target)
        self.assertEqual(set(calibration["sourceMetricHashes"].values()), {"a" * 64, "b" * 64, "c" * 64})
        self.assertAlmostEqual(calibration["responses"]["eye_aperture"]["slope"], 1 / 6)
        self.assertAlmostEqual(calibration["responses"]["nose_length"]["predictedCrossing"], 0.09)

    def test_calibration_grid_has_exactly_four_parent_preserving_recipes(self):
        calibration = derive_calibration(self.sources, self.target)
        recipes = build_calibration_grid(self.space, self.parent, calibration)
        self.assertEqual(len(recipes), 4)
        self.assertEqual({(r["calibrationControls"]["values"]["eye_aperture"], r["calibrationControls"]["values"]["nose_length"], r["calibrationControls"]["values"]["nose_width"]) for r in recipes}, {(0.42, 0.08, 0.27), (0.42, 0.14, 0.27), (0.52, 0.08, 0.27), (0.52, 0.14, 0.27)})
        for recipe in recipes:
            validate_calibration_recipe(self.space, self.parent, calibration, recipe)
            self.assertEqual(recipe["parameters"]["jaw_taper"], self.parent["parameters"]["jaw_taper"])

    def test_feature_macro_regression_gate_is_reproducible_from_receipts(self):
        baseline = {"ratios": {"jawWidth": 1.7, "mouthWidth": 1.1}}
        exact = {"ratios": {"jawWidth": 1.7, "mouthWidth": 1.1}}
        drifted = {"ratios": {"jawWidth": 1.700000002, "mouthWidth": 1.1}}
        self.assertTrue(macro_regression_gate(baseline, [exact], 1e-9)["passed"])
        result = macro_regression_gate(baseline, [drifted], 1e-9)
        self.assertFalse(result["passed"])
        self.assertGreater(result["maximumObservedDelta"], result["tolerance"])


class DetailGridTests(unittest.TestCase):
    def setUp(self):
        self.space = load_space(ROOT / "face-search-space.json")
        self.parent = json.loads((ROOT / "Runs" / "feature-calibration-20260720-100000" / "candidates" /
                                  "9032fd4dc9db2de64564b3f19e32ca5e6199881df111c2fa2d37feb1c7cc12f8.json").read_text())
        self.target = {"path": "target-detail-geometry.json", "sha256": "a" * 64}
        self.receipts = {"featureGeometry": "b" * 64, "macroGeometry": "c" * 64, "parentManifest": "d" * 64}

    def test_detail_grid_has_exactly_eight_parent_preserving_recipes(self):
        recipes = build_detail_grid(self.space, self.parent, self.target, self.receipts)
        self.assertEqual(len(recipes), 8)
        self.assertEqual({(recipe["detailControls"]["values"]["eye_outer_corner_down"], recipe["detailControls"]["values"]["mouth_width"], recipe["detailControls"]["values"]["upper_lip_height"], recipe["detailControls"]["values"]["nose_point_down"]) for recipe in recipes}, {
            (0.08, 0.24, 0.08, 0.15), (0.08, 0.24, 0.08, 0.25), (0.08, 0.24, 0.18, 0.15), (0.08, 0.24, 0.18, 0.25),
            (0.08, 0.36, 0.08, 0.15), (0.08, 0.36, 0.08, 0.25), (0.08, 0.36, 0.18, 0.15), (0.08, 0.36, 0.18, 0.25),
        })
        for recipe in recipes:
            validate_detail_recipe(self.space, self.parent, self.target, self.receipts, recipe)
            self.assertEqual(recipe["parameters"], self.parent["parameters"])
            self.assertEqual(recipe["calibrationControls"], self.parent["calibrationControls"])

    def test_detail_metric_uses_frozen_equivalent_landmarks_and_excludes_macro_mouth_width(self):
        anchors = {
            "leftEyeOuterCorner": {"camera": [0.1, 0.4, 1]}, "leftEyeInnerCorner": {"camera": [0.3, 0.5, 1]},
            "rightEyeInnerCorner": {"camera": [0.7, 0.5, 1]}, "rightEyeOuterCorner": {"camera": [0.9, 0.4, 1]},
            "mouthLeftCorner": {"camera": [0.3, 0.7, 1]}, "mouthRightCorner": {"camera": [0.7, 0.7, 1]},
            "upperLipTopCenter": {"camera": [0.5, 0.6, 1]}, "mouthSeamCenter": {"camera": [0.5, 0.7, 1]},
            "lowerLipBottomCenter": {"camera": [0.5, 0.8, 1]}, "noseTipLowestCenter": {"camera": [0.5, 0.65, 1]},
        }
        ratios = detail_ratios(anchors)
        self.assertAlmostEqual(ratios["mouthCommissureWidth"], 2 / 3)
        self.assertAlmostEqual(ratios["eyeOuterCornerTilt"], -1 / 6)
        target = {"metric": {"ratios": ratios}}
        self.assertEqual(detail_score(target, {"ratios": ratios}), 0.0)
        gate = detail_macro_regression_gate({"ratios": {"cheekWidth": 1, "jawWidth": 1, "eyeToChinHeight": 1, "lowerFaceTaper": 1, "mouthWidth": 1}},
                                            [{"ratios": {"cheekWidth": 1, "jawWidth": 1, "eyeToChinHeight": 1, "lowerFaceTaper": 1, "mouthWidth": 99}}], 0.0)
        self.assertEqual(gate["keys"], ["cheekWidth", "jawWidth", "eyeToChinHeight", "lowerFaceTaper"])
        self.assertTrue(gate["passed"])

    def test_detail_target_preserves_both_blind_annotations_and_binds_the_frozen_map(self):
        target = json.loads((ROOT / "target-detail-geometry.json").read_text())
        feature_target = json.loads((ROOT / "target-feature-geometry.json").read_text())
        self.assertEqual([item["id"] for item in target["blindAnnotations"]], ["task8_design", "feature_annotation_a"])
        self.assertEqual(target["consensus"]["uncertaintyPixels"]["mouthLeftCorner"], 16)
        self.assertEqual(target["consensus"]["uncertaintyPixels"]["mouthRightCorner"], 16)
        self.assertEqual(target_detail_ratios(target["consensus"]["targetPoints"], feature_target["points"]), target["metric"]["ratios"])
        self.assertIsNone(validate_detail_target(target))
        self.assertEqual(target["stableVertexIndices"]["indices"]["lowerLipBottomCenter"], 492)

    def test_landmark_mapping_prefers_frontmost_vertex_over_closer_hidden_projection(self):
        selected = select_frontmost_landmark_vertex([
            {"index": 10, "pixelDistance": 0.1, "cameraDepth": 1.159},
            {"index": 11, "pixelDistance": 1.5, "cameraDepth": 1.000},
            {"index": 12, "pixelDistance": 7.0, "cameraDepth": 0.900},
        ])
        self.assertEqual(selected["index"], 11)

    def test_detail_manifest_requires_only_scripts_used_by_detail_build(self):
        scripts = dict(required_tool_scripts(False, True, True))
        self.assertNotIn("calibrationScript", scripts)
        self.assertIn("calibrationContract", scripts)
        self.assertIn("detailScript", scripts)

    def test_detail_absolute_controls_replace_existing_shape_keys_without_duplicates(self):
        controls = {"mouth.width": 0.36, "nose.pointDown": 0.25}
        self.assertEqual(direct_detail_shape_key_values(controls), controls)
        self.assertEqual(detail_override_duplicates([
            {"name": "mouth.width", "value": 0.36},
            {"name": "nose.pointDown", "value": 0.25},
        ]), [])
        self.assertEqual(detail_override_duplicates([
            {"name": "mouth.width", "value": 0.14},
            {"name": "mouth.width.001", "value": 0.36},
        ]), ["mouth.width.001"])

    def test_completed_detail_selection_binds_all_receipts_and_audits(self):
        run_id = "detail-grid-20260720-063059"
        run_root = ROOT / "Runs" / run_id
        derived = ROOT / "Derived" / "search-runs" / run_id / "round-10"
        selection = json.loads((run_root / "selection.json").read_text(encoding="utf-8"))
        run = json.loads((run_root / "run.json").read_text(encoding="utf-8"))
        self.assertEqual(selection["status"], run["status"])
        self.assertFalse(selection["humanAcceptance"])
        self.assertFalse(selection["machinePromotion"])
        self.assertEqual(len(selection["ranking"]), 8)
        self.assertEqual(selection["decision"]["machineBestCandidateId"], selection["ranking"][0]["candidateId"])
        for candidate_id, receipts in selection["receiptHashes"].items():
            for key, name in (("manifest", "manifest.json"), ("detail", "detail-geometry.json"),
                              ("feature", "feature-geometry.json"), ("macro", "macro-geometry.json")):
                self.assertEqual(hashlib.sha256((derived / candidate_id / name).read_bytes()).hexdigest(), receipts[key])
        for audit in (selection["visualAudits"]["auditA"], selection["visualAudits"]["auditB"]):
            self.assertFalse(audit["clearImprovement"])
            self.assertEqual(hashlib.sha256((run_root / audit["path"]).read_bytes()).hexdigest(), audit["sha256"])
        review = selection["independentEvidenceReview"]
        self.assertEqual(review["decision"], "APPROVED")
        self.assertEqual(hashlib.sha256((run_root / review["path"]).read_bytes()).hexdigest(), review["sha256"])


if __name__ == "__main__":
    unittest.main()
