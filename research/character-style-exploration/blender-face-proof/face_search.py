import hashlib
import json
import argparse
import re
from secrets import randbits
from copy import deepcopy
from pathlib import Path


ARTIFACT_ROOT = Path(__file__).resolve().parent
RUNS_ROOT = ARTIFACT_ROOT / "Runs"
EVALUATOR_CONTRACT_PATH = ARTIFACT_ROOT / "evaluator-contract.json"


TARGET_MAP = {
    "head_width": ("head", "head-scale-horiz-decr", "head-scale-horiz-incr"),
    "head_height": ("head", "head-scale-vert-decr", "head-scale-vert-incr"),
    "eye_spacing": (
        ("eyes", "l-eye-trans-in", "l-eye-trans-out"),
        ("eyes", "r-eye-trans-in", "r-eye-trans-out"),
    ),
}


def _canonical(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"))


def _candidate_id(schema_version, parent_id, round_index, axis, direction, parameters):
    payload = {
        "schemaVersion": schema_version,
        "parentId": parent_id,
        "round": round_index,
        "axis": axis,
        "direction": direction,
        "parameters": parameters,
    }
    return hashlib.sha256(_canonical(payload).encode("utf-8")).hexdigest()


def candidate_to_controls(parameters: dict) -> dict:
    """Map one validated semantic candidate onto Blender control weights."""
    controls = {
        "jaw.taper": parameters["jaw_taper"],
        "nose.width": parameters["nose_width"],
        "sculpt.nose.tip": round(parameters["nose_width"] * 0.70, 12),
        "sculpt.nose.nostrils": round(parameters["nose_width"] * 0.60, 12),
        "nose.length": parameters["nose_length"],
        "nose.bridgeDepth": parameters["nose_projection"],
        "sculpt.cheek.left.bone": parameters["cheek_definition"],
        "sculpt.cheek.right.bone": parameters["cheek_definition"] * (5.0 / 6.0),
        "sculpt.cheek.left.volume.decr": parameters["cheek_definition"] * (7.0 / 6.0),
        "sculpt.cheek.right.volume.decr": parameters["cheek_definition"] * (4.0 / 3.0),
        "sculpt.eye_bag.left": parameters["eye_bag"],
        "sculpt.eye_bag.right": parameters["eye_bag"] * (19.0 / 22.0),
    }
    for axis, narrower, wider in (
        ("head_width", "sculpt.head.narrower", "sculpt.head.wider"),
        ("head_height", "sculpt.head.shorter", "sculpt.head.taller"),
        ("chin_projection", "sculpt.chin.recess", "sculpt.chin.prominent"),
    ):
        value = parameters[axis]
        if value:
            controls[narrower if value < 0 else wider] = abs(value)
    lip_volume = parameters["lip_volume"]
    if lip_volume:
        direction = "decr" if lip_volume < 0 else "incr"
        controls[f"sculpt.upperlip.volume.{direction}"] = abs(lip_volume)
        controls[f"sculpt.lowerlip.volume.{direction}"] = abs(lip_volume) * 0.75
    eye_spacing = parameters["eye_spacing"]
    if eye_spacing:
        direction = "in" if eye_spacing < 0 else "out"
        controls[f"sculpt.eye.left.{direction}"] = abs(eye_spacing)
        controls[f"sculpt.eye.right.{direction}"] = abs(eye_spacing)
    return controls


def load_space(path: Path) -> dict:
    with path.open(encoding="utf-8") as stream:
        space = json.load(stream)
    if space.get("schemaVersion") != 1 or not space.get("axes"):
        raise ValueError("unsupported face search space")
    return space


def _evaluator_contract() -> dict:
    with EVALUATOR_CONTRACT_PATH.open(encoding="utf-8") as stream:
        return json.load(stream)


def _slot_candidate(slot) -> str:
    if isinstance(slot, str) and slot:
        return slot
    if (
        isinstance(slot, dict)
        and isinstance(slot.get("candidateId"), str)
        and slot["candidateId"]
        and isinstance(slot.get("images"), list)
        and len(slot["images"]) == 3
        and all(isinstance(image, str) and image for image in slot["images"])
    ):
        return slot["candidateId"]
    raise ValueError("package slot must map to a candidate ID and three images")


def _validate_blind_package(package: dict) -> dict:
    if not isinstance(package, dict) or not isinstance(package.get("slots"), dict):
        raise ValueError("package requires slots")
    forbidden = {
        "parameters", "direction", "directions", "diagnosis", "producerDiagnosis", "producer_diagnosis",
    }
    if forbidden & set(package):
        raise ValueError("package is not blind")
    slots = package["slots"]
    if set(slots) != {"left", "center", "right"}:
        raise ValueError("package requires left, center, and right slots")
    for slot in slots.values():
        if isinstance(slot, dict) and forbidden & set(slot):
            raise ValueError("package is not blind")
        _slot_candidate(slot)
    return package


def build_ballot_packages(
    target_image: str, champion_id: str, champion_images: list[str], challengers: list[dict]
) -> tuple[dict, dict]:
    """Build the primary/audit packages without exposing search deltas."""
    if not isinstance(target_image, str) or not target_image or not isinstance(champion_id, str) or not champion_id:
        raise ValueError("package target and champion must be non-empty strings")
    center = {"candidateId": champion_id, "images": champion_images}
    if len(challengers) != 2:
        raise ValueError("package requires two challengers")
    first, second = challengers
    for candidate in (center, first, second):
        _slot_candidate(candidate)
    if len({champion_id, first["candidateId"], second["candidateId"]}) != 3:
        raise ValueError("package candidates must be distinct")
    if randbits(1):
        first, second = second, first

    def package(left, right):
        value = {
            "schemaVersion": 1,
            "targetImage": target_image,
            "slots": {"left": deepcopy(left), "center": deepcopy(center), "right": deepcopy(right)},
        }
        _validate_blind_package(value)
        return value

    return package(first, second), package(second, first)


def validate_ballot(space: dict, package: dict, ballot: dict) -> dict:
    """Validate an evaluator's blind slot choice and resolve it only at the root."""
    contract = _evaluator_contract()
    _validate_blind_package(package)
    required = {
        "schemaVersion", "evaluatorId", "winnerSlot", "confidence", "dimensions",
        "hardFailures", "severeRegressions", "diagnosis", "nextAxis",
    }
    optional = {"modelIdentity", "instructions", "response"}
    if not isinstance(ballot, dict) or not required <= set(ballot):
        raise ValueError("ballot is incomplete")
    extras = set(ballot) - required - optional
    if "freshVerifierPassed" in extras:
        raise ValueError("fresh verifier evidence is root-owned")
    if "winnerId" in extras or any("candidate" in key.lower() for key in extras):
        raise ValueError("ballot must not contain a candidate ID")
    if extras:
        raise ValueError("ballot has unknown fields")
    if ballot["schemaVersion"] != contract["schemaVersion"]:
        raise ValueError("unsupported ballot schema")
    if not isinstance(ballot["evaluatorId"], str) or not ballot["evaluatorId"]:
        raise ValueError("ballot evaluator ID is required")
    if ballot["winnerSlot"] not in contract["winnerSlots"]:
        raise ValueError("ballot winner slot is unknown")
    confidence = ballot["confidence"]
    if isinstance(confidence, bool) or not isinstance(confidence, (int, float)) or not 0 <= confidence <= 1:
        raise ValueError("ballot confidence must be within [0, 1]")
    dimensions = ballot["dimensions"]
    if not isinstance(dimensions, dict) or set(dimensions) != set(contract["dimensions"]):
        raise ValueError("ballot dimensions are unknown or incomplete")
    if any(value not in contract["dimensionValues"] for value in dimensions.values()):
        raise ValueError("ballot dimension value is unknown")
    failures = ballot["hardFailures"]
    if not isinstance(failures, list) or any(failure not in contract["hardFailures"] for failure in failures):
        raise ValueError("ballot hard failure is unknown")
    regressions = ballot["severeRegressions"]
    if not isinstance(regressions, list) or any(not isinstance(item, str) or not item for item in regressions):
        raise ValueError("ballot severe regressions must be non-empty strings")
    if not isinstance(ballot["diagnosis"], list) or any(not isinstance(item, str) or not item for item in ballot["diagnosis"]):
        raise ValueError("ballot diagnosis must be a list of non-empty strings")
    if ballot["nextAxis"] not in space.get("axes", {}):
        raise ValueError("ballot next axis is unknown")
    evidence = {"modelIdentity", "instructions", "response"}
    if set(ballot) & evidence and (
        not evidence <= set(ballot)
        or any(not isinstance(ballot[key], str) or not ballot[key] for key in evidence)
    ):
        raise ValueError("ballot evaluator evidence must be complete")
    resolved = deepcopy(ballot)
    resolved["winnerId"] = None if ballot["winnerSlot"] == "tie" else _slot_candidate(package["slots"][ballot["winnerSlot"]])
    return resolved


def _candidate(space, parent_id, round_index, axis, direction, parameters):
    return {
        "candidateId": _candidate_id(
            space["schemaVersion"], parent_id, round_index, axis, direction, parameters
        ),
        "parentId": parent_id,
        "round": round_index,
        "axis": axis,
        "direction": direction,
        "parameters": parameters,
    }


def _file_hash(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def new_state(space: dict, run_id: str) -> dict:
    parameters = {name: values["initial"] for name, values in space["axes"].items()}
    champion = _candidate(space, None, 0, None, "initial", parameters)
    recipe_path = Path(__file__).with_name("face-brief.json")
    return {
        "schemaVersion": space["schemaVersion"],
        "runId": run_id,
        "champion": champion,
        "searchSpace": deepcopy(space),
        "axes": deepcopy(space["axes"]),
        "steps": {name: values["step"] for name, values in space["axes"].items()},
        "minimumSteps": {name: values["minimumStep"] for name, values in space["axes"].items()},
        "maxRounds": space["maxRounds"],
        "stopAfterNoPromotion": space["stopAfterNoPromotion"],
        "stopAfterDisagreement": space["stopAfterDisagreement"],
        "noPromotionCount": 0,
        "disagreementCount": 0,
        "history": [],
        "learning": [],
        "artifactRoot": str(ARTIFACT_ROOT),
        "runDirectory": f"Runs/{run_id}",
        "provenance": {
            "acceptedChampion": {
                "identity": "sculpt-v13",
                "recipe": {"path": recipe_path.name, "hash": _file_hash(recipe_path)},
            }
        },
    }


def validate_candidate(space: dict, candidate: dict) -> None:
    required = {"candidateId", "parentId", "round", "axis", "direction", "parameters"}
    missing = required - set(candidate)
    if missing:
        raise ValueError(f"candidate missing: {', '.join(sorted(missing))}")
    parameters = candidate.get("parameters")
    if not isinstance(parameters, dict):
        raise ValueError("candidate parameters missing")
    for axis, values in space["axes"].items():
        value = parameters.get(axis)
        if isinstance(value, bool) or not isinstance(value, (int, float)):
            raise ValueError(f"{axis} must be numeric")
        if value < values["minimum"] or value > values["maximum"]:
            raise ValueError(f"{axis} outside [{values['minimum']}, {values['maximum']}]")
    if set(parameters) != set(space["axes"]):
        raise ValueError("candidate has unknown parameters")
    expected_id = _candidate_id(
        space["schemaVersion"],
        candidate["parentId"],
        candidate["round"],
        candidate["axis"],
        candidate["direction"],
        parameters,
    )
    if candidate["candidateId"] != expected_id:
        raise ValueError("candidateId does not match canonical candidate content")


def _artifact_root(root: Path) -> Path:
    resolved_root = root.resolve()
    if resolved_root != ARTIFACT_ROOT:
        raise ValueError("artifact root must be the face-proof directory")
    return resolved_root


def resolve_run_path(run_id: str) -> Path:
    if (
        not isinstance(run_id, str)
        or run_id in {".", ".."}
        or not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9._-]*", run_id)
    ):
        raise ValueError("run ID must be a safe single path component")
    path = (RUNS_ROOT / run_id / "run.json").resolve()
    try:
        path.relative_to(RUNS_ROOT.resolve())
    except ValueError as error:
        raise ValueError("run ID must resolve beneath the face-proof Runs directory") from error
    return path


def _artifact_path(root: Path, relative_path: str, require_exists: bool = True) -> Path:
    if not isinstance(relative_path, str) or Path(relative_path).is_absolute():
        raise ValueError("artifact path must be relative")
    resolved_root = _artifact_root(root)
    resolved_path = (resolved_root / relative_path).resolve()
    try:
        resolved_path.relative_to(RUNS_ROOT.resolve())
    except ValueError as error:
        raise ValueError("artifact path escapes the face-proof Runs directory") from error
    if require_exists and not resolved_path.is_file():
        raise ValueError("artifact path does not exist")
    return resolved_path


def _write_candidate_recipe(root: Path, relative_path: str, candidate: dict) -> str:
    path = _artifact_path(root, relative_path, require_exists=False)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(candidate, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return _file_hash(path)


def propose_pair(
    state: dict, axis: str, round_index: int, artifact_root: Path | None = None
) -> tuple[dict, dict]:
    if round_index > state["maxRounds"]:
        raise ValueError("maxRounds reached")
    if state["noPromotionCount"] >= state["stopAfterNoPromotion"]:
        raise ValueError("stopAfterNoPromotion reached")
    if state["disagreementCount"] >= state["stopAfterDisagreement"]:
        raise ValueError("stopAfterDisagreement reached")
    if round_index != len(state["history"]) + 1:
        raise ValueError("round must be the next unrecorded round")
    if state.get("activeProposal") and not state["activeProposal"].get("finalized"):
        raise ValueError("active proposal must be finalized before another proposal")
    champion = state["champion"]
    if axis not in state["steps"]:
        raise ValueError(f"unknown axis: {axis}")
    values = state["axes"][axis]
    current = champion["parameters"][axis]
    step = state["steps"][axis]
    lower_value = max(values["minimum"], current - step)
    upper_value = min(values["maximum"], current + step)
    if lower_value == current or upper_value == current:
        raise ValueError(f"{axis} cannot produce a distinct pair")
    lower_parameters = deepcopy(champion["parameters"])
    upper_parameters = deepcopy(champion["parameters"])
    lower_parameters[axis] = lower_value
    upper_parameters[axis] = upper_value
    space = {"schemaVersion": state["schemaVersion"]}
    lower = _candidate(space, champion["candidateId"], round_index, axis, "lower", lower_parameters)
    upper = _candidate(space, champion["candidateId"], round_index, axis, "upper", upper_parameters)
    root = _artifact_root(Path(artifact_root or state["artifactRoot"]))
    state["artifactRoot"] = str(root)
    run_directory = state["runDirectory"]
    lower_path = f"{run_directory}/candidates/{lower['candidateId']}.json"
    upper_path = f"{run_directory}/candidates/{upper['candidateId']}.json"
    state["activeProposal"] = {
        "axis": axis,
        "round": round_index,
        "championBefore": deepcopy(champion),
        "axisReason": ["selected bounded face-geometry axis"],
        "stepBefore": step,
        "candidates": [lower, upper],
        "recipes": [
            {"candidateId": lower["candidateId"], "delta": lower_value - current},
            {"candidateId": upper["candidateId"], "delta": upper_value - current},
        ],
        "artifacts": [
            {"candidateId": lower["candidateId"], "kind": "candidateRecipe", "hash": _write_candidate_recipe(root, lower_path, lower), "path": lower_path},
            {"candidateId": upper["candidateId"], "kind": "candidateRecipe", "hash": _write_candidate_recipe(root, upper_path, upper), "path": upper_path},
        ],
        "agents": {
            "source": {
                "identity": state["provenance"]["acceptedChampion"]["identity"],
                "instructions": "Use the accepted sculpt-v13 champion as the parent.",
                "response": _canonical(state["provenance"]["acceptedChampion"]),
            },
            "tool": {
                "identity": "face_search.py",
                "instructions": "Generate a bounded symmetric pair on one geometry axis.",
                "response": f"proposed {axis} round {round_index}",
            },
        },
    }
    return lower, upper


def _validate_replay_references(value, artifact_root: Path) -> None:
    if isinstance(value, list):
        for item in value:
            _validate_replay_references(item, artifact_root)
        return
    if not isinstance(value, dict):
        return
    for payload, hash_key, path_key in (
        ("imageBytes", "imageHash", "imagePath"),
        ("blenderLogs", "blenderLogHash", "blenderLogPath"),
    ):
        if payload in value:
            path = value.get(path_key)
            if (
                not isinstance(value.get(hash_key), str)
                or not re.fullmatch(r"[0-9a-f]{64}", value[hash_key])
            ):
                raise ValueError(f"{payload} requires a replay reference hash and relative path")
            reference_path = _artifact_path(artifact_root, path)
            if _file_hash(reference_path) != value[hash_key]:
                raise ValueError(f"{payload} replay reference hash does not match file bytes")
    for item in value.values():
        _validate_replay_references(item, artifact_root)


def _validate_artifacts(artifacts, candidates, artifact_root: Path) -> None:
    if not isinstance(artifacts, list) or len(artifacts) != 2:
        raise ValueError("round record requires artifact records")
    candidate_by_id = {candidate["candidateId"]: candidate for candidate in candidates}
    if len(candidate_by_id) != 2:
        raise ValueError("round record requires two distinct candidates")
    artifact_ids = set()
    for artifact in artifacts:
        if (
            not isinstance(artifact, dict)
            or not {"candidateId", "kind", "hash", "path"} <= set(artifact)
            or artifact["kind"] != "candidateRecipe"
            or not isinstance(artifact["hash"], str)
            or not re.fullmatch(r"[0-9a-f]{64}", artifact["hash"])
        ):
            raise ValueError("artifact records require hash and relative path")
        if artifact["candidateId"] not in candidate_by_id or artifact["candidateId"] in artifact_ids:
            raise ValueError("artifact records must map exactly once to each candidate")
        artifact_ids.add(artifact["candidateId"])
        path = _artifact_path(artifact_root, artifact["path"])
        if _file_hash(path) != artifact["hash"]:
            raise ValueError("artifact hash does not match file bytes")
        try:
            recipe = _read_json(path)
        except (OSError, json.JSONDecodeError) as error:
            raise ValueError("candidate recipe is unreadable") from error
        candidate = candidate_by_id[artifact["candidateId"]]
        if not isinstance(recipe, dict) or _canonical(recipe) != _canonical(candidate):
            raise ValueError("candidate recipe does not match the recorded candidate")
    if artifact_ids != set(candidate_by_id):
        raise ValueError("artifact records must map exactly once to each candidate")


def _validate_ballot_packages(packages, artifact_root: Path) -> None:
    if not packages:
        return
    if not isinstance(packages, dict) or set(packages) != {"primary", "audit"}:
        raise ValueError("ballot packages require primary and audit entries")
    loaded = []
    for entry in packages.values():
        if not isinstance(entry, dict) or not {"path", "hash", "package"} <= set(entry):
            raise ValueError("ballot package requires path, hash, and package")
        path = _artifact_path(artifact_root, entry["path"])
        if not isinstance(entry["hash"], str) or _file_hash(path) != entry["hash"]:
            raise ValueError("ballot package hash does not match")
        package = _read_json(path)
        if _canonical(package) != _canonical(entry["package"]):
            raise ValueError("ballot package does not match recorded package")
        _validate_blind_package(package)
        loaded.append(package)
    if (
        loaded[0]["slots"]["left"] != loaded[1]["slots"]["right"]
        or loaded[0]["slots"]["right"] != loaded[1]["slots"]["left"]
        or loaded[0]["slots"]["center"] != loaded[1]["slots"]["center"]
    ):
        raise ValueError("ballot packages must reverse challengers")


def project_learning_row(record: dict, artifact_root: Path | None = None) -> dict:
    root = _artifact_root(Path(artifact_root or ARTIFACT_ROOT))
    if "artifacts" in record:
        _validate_artifacts(record["artifacts"], record.get("candidates", []), root)
    _validate_replay_references(record, root)

    def prune(value):
        if isinstance(value, list):
            return [prune(item) for item in value]
        if isinstance(value, dict):
            return {
                key: prune(item)
                for key, item in value.items()
                if key not in {"imageBytes", "blenderLogs"}
            }
        return deepcopy(value)

    return prune(record)


def _decision_axis(state: dict) -> str:
    proposal = state.get("activeProposal")
    return proposal["axis"] if proposal else "nose_width"


def _derived_output_path(relative_path: str) -> Path:
    if not isinstance(relative_path, str) or Path(relative_path).is_absolute():
        raise ValueError("verifier output path must be relative")
    path = (ARTIFACT_ROOT / relative_path).resolve()
    derived_root = ARTIFACT_ROOT / "Derived" / "search-runs"
    try:
        path.relative_to(derived_root.resolve())
    except ValueError as error:
        raise ValueError("verifier output path escapes Derived/search-runs") from error
    return path


def _valid_verifier_receipt(candidates: list[dict], receipt) -> bool:
    if not isinstance(receipt, dict):
        return False
    required = {
        "candidateId", "candidate", "outputPath", "manifestHash", "verifierLogPath",
        "verifierLogHash", "verifierScriptHash",
    }
    if not required <= set(receipt):
        return False
    candidate = next((item for item in candidates if item["candidateId"] == receipt["candidateId"]), None)
    if candidate is None or _canonical(candidate) != _canonical(receipt["candidate"]):
        return False
    try:
        output = _derived_output_path(receipt["outputPath"])
        manifest = output / "manifest.json"
        log = _artifact_path(ARTIFACT_ROOT, receipt["verifierLogPath"])
        if (
            _file_hash(manifest) != receipt["manifestHash"]
            or _file_hash(log) != receipt["verifierLogHash"]
            or _file_hash(Path(__file__).with_name("verify_sculpt.py")) != receipt["verifierScriptHash"]
            or "SCULPT_VERIFY_OK" not in log.read_text(encoding="utf-8")
        ):
            return False
        from sculpt_integrity import validate_manifest_integrity
        return _canonical(validate_manifest_integrity(output)) == _canonical(candidate)
    except (OSError, ValueError, json.JSONDecodeError):
        return False


def record_fresh_verifier_pass(state: dict, candidate_id: str, output: Path, verifier_log: Path) -> dict:
    """Record root-owned fresh-reopen evidence for one active challenger."""
    proposal = state.get("activeProposal")
    candidates = proposal.get("candidates", []) if isinstance(proposal, dict) else []
    candidate = next((item for item in candidates if item["candidateId"] == candidate_id), None)
    if candidate is None:
        raise ValueError("verifier receipt candidate is not active")
    output = Path(output).resolve()
    try:
        output_path = output.relative_to(ARTIFACT_ROOT).as_posix()
        verifier_log_path = Path(verifier_log).resolve().relative_to(ARTIFACT_ROOT).as_posix()
    except ValueError as error:
        raise ValueError("verifier receipt artifacts must be beneath the face-proof directory") from error
    receipt = {
        "candidateId": candidate_id,
        "candidate": deepcopy(candidate),
        "outputPath": output_path,
        "manifestHash": _file_hash(output / "manifest.json"),
        "verifierLogPath": verifier_log_path,
        "verifierLogHash": _file_hash(Path(verifier_log)),
        "verifierScriptHash": _file_hash(Path(__file__).with_name("verify_sculpt.py")),
    }
    if not _valid_verifier_receipt(candidates, receipt):
        raise ValueError("fresh verifier receipt is invalid")
    proposal.setdefault("verifierReceipts", {})[candidate_id] = receipt
    return deepcopy(receipt)


def _promotion_outcome(champion_id: str, candidate_ids: set[str], verified_candidate_ids: set[str], ballots: list[dict], step: float, minimum_step: float) -> dict:
    allowed_ids = candidate_ids | {champion_id}
    for ballot in ballots:
        winner_id = ballot.get("winnerId")
        if winner_id is not None and winner_id not in allowed_ids:
            raise ValueError("ballot winner must be the champion or a candidate in the active proposal")
    evaluator_ids = [ballot.get("evaluatorId") for ballot in ballots]
    winner_ids = [ballot.get("winnerId") for ballot in ballots]
    failures = any(ballot.get("hardFailures") or ballot.get("severeRegressions") for ballot in ballots)
    regressions = any(
        ballot["dimensions"][dimension] == "worse"
        for ballot in ballots
        for dimension in ("groundedMatureGeometry", "threeDimensionalPlausibility")
    )
    promote = (
        len(ballots) == 2
        and set(evaluator_ids) == {"primary", "audit"}
        and len(set(winner_ids)) == 1
        and winner_ids[0] in candidate_ids
        and not failures
        and not regressions
        and all(ballot["confidence"] >= 0.60 for ballot in ballots)
        and winner_ids[0] in verified_candidate_ids
    )
    disagreement = len(set(winner_ids)) > 1
    return {
        "promote": promote,
        "winnerId": winner_ids[0] if promote else None,
        "nextStep": step if promote else max(minimum_step, step / 2.0),
        "disagreement": disagreement,
    }


def decide_promotion(state: dict, ballots: list[dict]) -> dict:
    axis = _decision_axis(state)
    contract = _evaluator_contract()
    ballot_fields = {
        "schemaVersion", "evaluatorId", "modelIdentity", "instructions", "response", "winnerSlot",
        "winnerId", "confidence", "dimensions", "hardFailures", "severeRegressions", "diagnosis",
        "nextAxis",
    }
    for ballot in ballots:
        if not isinstance(ballot, dict) or not ballot_fields <= set(ballot):
            raise ValueError("ballots require verbatim evaluator evidence")
        if any(not isinstance(ballot[field], str) or not ballot[field] for field in ("evaluatorId", "modelIdentity", "instructions", "response")):
            raise ValueError("ballots require verbatim evaluator evidence")
        if (
            ballot["schemaVersion"] != contract["schemaVersion"]
            or ballot["winnerSlot"] not in contract["winnerSlots"]
            or isinstance(ballot["confidence"], bool)
            or not isinstance(ballot["confidence"], (int, float))
            or not 0 <= ballot["confidence"] <= 1
            or not isinstance(ballot["dimensions"], dict)
            or set(ballot["dimensions"]) != set(contract["dimensions"])
            or any(value not in contract["dimensionValues"] for value in ballot["dimensions"].values())
            or not isinstance(ballot["hardFailures"], list)
            or any(item not in contract["hardFailures"] for item in ballot["hardFailures"])
            or not isinstance(ballot["severeRegressions"], list)
            or any(not isinstance(item, str) or not item for item in ballot["severeRegressions"])
            or not isinstance(ballot["diagnosis"], list)
            or any(not isinstance(item, str) or not item for item in ballot["diagnosis"])
            or ballot["nextAxis"] not in state["axes"]
        ):
            raise ValueError("ballots require validated evaluator contract data")
    active_ids = {candidate["candidateId"] for candidate in state.get("activeProposal", {}).get("candidates", [])}
    proposal = state.get("activeProposal", {})
    verifier_receipts = proposal.get("verifierReceipts", {}) if isinstance(proposal, dict) else {}
    verified_ids = {
        candidate_id for candidate_id in active_ids
        if _valid_verifier_receipt(proposal.get("candidates", []), verifier_receipts.get(candidate_id))
    }
    step = state["steps"][axis]
    outcome = _promotion_outcome(
        state["champion"]["candidateId"], active_ids, verified_ids, ballots, step, state["minimumSteps"][axis]
    )
    if outcome["promote"]:
        return {
            "promote": True,
            "winnerId": outcome["winnerId"],
            "nextStep": outcome["nextStep"],
            "noPromotionCount": 0,
            "disagreementCount": 0,
        }
    return {
        "promote": False,
        "winnerId": None,
        "nextStep": outcome["nextStep"],
        "noPromotionCount": state["noPromotionCount"] + 1,
        "disagreementCount": state["disagreementCount"] + int(outcome["disagreement"]),
    }


def validate_round_record(record: dict, artifact_root: Path | None = None) -> None:
    required = {
        "championBefore",
        "searchSpace",
        "selectedAxis",
        "axisReason",
        "stepBefore",
        "candidates",
        "recipes",
        "agents",
        "ballots",
        "failures",
        "decision",
        "championAfter",
        "stepAfter",
        "artifacts",
        "nextExperiment",
    }
    missing = required - set(record)
    if missing:
        raise ValueError(f"round record missing: {', '.join(sorted(missing))}")
    space = record["searchSpace"]
    root = _artifact_root(Path(artifact_root or ARTIFACT_ROOT))
    if not isinstance(space, dict) or space.get("schemaVersion") != 1 or not isinstance(space.get("axes"), dict):
        raise ValueError("round record requires a searchSpace")
    if not isinstance(record["axisReason"], list) or not record["axisReason"]:
        raise ValueError("axisReason must be a non-empty list")
    if any(isinstance(record[key], bool) or not isinstance(record[key], (int, float)) for key in ("stepBefore", "stepAfter")):
        raise ValueError("round steps must be numeric")
    candidates = record["candidates"]
    candidate_fields = {"candidateId", "parentId", "round", "axis", "direction", "parameters"}
    if not isinstance(candidates, list) or len(candidates) != 2:
        raise ValueError("round record requires two candidates")
    if any(not candidate_fields <= set(candidate) for candidate in candidates):
        raise ValueError("round record candidates must be complete")
    validate_candidate(space, record["championBefore"])
    for candidate in candidates:
        validate_candidate(space, candidate)
    validate_candidate(space, record["championAfter"])
    recipes = record["recipes"]
    if not isinstance(recipes, list) or len(recipes) != 2 or any(
        not {"candidateId", "delta"} <= set(recipe)
        or isinstance(recipe["delta"], bool)
        or not isinstance(recipe["delta"], (int, float))
        for recipe in recipes
    ):
        raise ValueError("round record requires candidate recipes and numeric deltas")
    recipe_by_id = {recipe["candidateId"]: recipe for recipe in recipes}
    if set(recipe_by_id) != {candidate["candidateId"] for candidate in candidates}:
        raise ValueError("round record recipes must map exactly once to each candidate")
    selected_axis = record["selectedAxis"]
    if selected_axis not in space["axes"]:
        raise ValueError("selectedAxis is not in the search space")
    champion_parameters = record["championBefore"]["parameters"]
    for candidate in candidates:
        changed_axes = {
            axis for axis in space["axes"]
            if candidate["parameters"][axis] != champion_parameters[axis]
        }
        if changed_axes != {selected_axis}:
            raise ValueError("only selectedAxis may differ from championBefore")
        expected_delta = candidate["parameters"][selected_axis] - champion_parameters[selected_axis]
        if recipe_by_id[candidate["candidateId"]]["delta"] != expected_delta:
            raise ValueError("recipe delta must exactly match the selectedAxis change")
    agents = record["agents"]
    if not isinstance(agents, dict) or not agents:
        raise ValueError("round record requires agents")
    for agent in agents.values():
        if not isinstance(agent, dict) or not {"identity", "instructions", "response"} <= set(agent):
            raise ValueError("agents require identity, verbatim instructions, and response")
    ballots = record["ballots"]
    ballot_fields = {
        "schemaVersion", "evaluatorId", "modelIdentity", "instructions", "response", "winnerSlot", "winnerId",
        "confidence", "dimensions", "hardFailures", "severeRegressions", "diagnosis", "nextAxis",
        "validated",
    }
    if not isinstance(ballots, list) or len(ballots) != 2:
        raise ValueError("round record requires two ballots")
    if any(not ballot_fields <= set(ballot) or ballot["validated"] is not True for ballot in ballots):
        raise ValueError("round record requires validated ballots")
    required_agents = {"source", "tool"} | {ballot["evaluatorId"] for ballot in ballots}
    missing_agents = required_agents - set(agents)
    if missing_agents:
        raise ValueError(f"agents missing identities: {', '.join(sorted(missing_agents))}")
    for ballot in ballots:
        agent = agents[ballot["evaluatorId"]]
        if (
            not isinstance(ballot["modelIdentity"], str)
            or not ballot["modelIdentity"]
            or not isinstance(ballot["instructions"], str)
            or not ballot["instructions"]
            or not isinstance(ballot["response"], str)
            or not ballot["response"]
            or agent.get("modelIdentity") != ballot["modelIdentity"]
            or agent["instructions"] != ballot["instructions"]
            or agent["response"] != ballot["response"]
        ):
            raise ValueError("round record requires unchanged verbatim evaluator evidence")
    if not isinstance(record["failures"], list) or not isinstance(record["decision"], dict):
        raise ValueError("round record failures and decision must be recorded")
    candidate_ids = {candidate["candidateId"] for candidate in candidates}
    receipts = record.get("verifierReceipts", {})
    if not isinstance(receipts, dict):
        raise ValueError("round record verifier receipts must be a map")
    verified_ids = {
        candidate_id for candidate_id in candidate_ids
        if _valid_verifier_receipt(candidates, receipts.get(candidate_id))
    }
    outcome = _promotion_outcome(
        record["championBefore"]["candidateId"],
        candidate_ids,
        verified_ids,
        ballots,
        record["stepBefore"],
        space["axes"][selected_axis]["minimumStep"],
    )
    decision = record["decision"]
    if (
        decision.get("promote") != outcome["promote"]
        or decision.get("winnerId") != outcome["winnerId"]
        or decision.get("nextStep") != outcome["nextStep"]
    ):
        raise ValueError("round record decision does not match ballots")
    expected_champion = (
        next(candidate for candidate in candidates if candidate["candidateId"] == outcome["winnerId"])
        if outcome["promote"]
        else record["championBefore"]
    )
    if _canonical(record["championAfter"]) != _canonical(expected_champion):
        raise ValueError("round record championAfter does not match the decision")
    _validate_artifacts(record["artifacts"], candidates, root)
    _validate_ballot_packages(record.get("ballotPackages"), root)
    _validate_replay_references(record, root)


def _read_json(path: Path) -> dict | list:
    with path.open(encoding="utf-8") as stream:
        return json.load(stream)


def _write_json(path: Path, value: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def _package_image_paths(path: Path) -> list[str]:
    images = [path / f"{view}.png" for view in ("front", "three-quarter", "profile")]
    if any(not image.is_file() for image in images):
        raise ValueError("package images require front, three-quarter, and profile PNGs")
    try:
        return [image.resolve().relative_to(ARTIFACT_ROOT).as_posix() for image in images]
    except ValueError as error:
        raise ValueError("package images must be beneath the face-proof directory") from error


def _package_target_path(path: Path) -> str:
    if not path.is_file():
        raise ValueError("package target image does not exist")
    try:
        return path.resolve().relative_to(ARTIFACT_ROOT).as_posix()
    except ValueError as error:
        raise ValueError("package target image must be beneath the face-proof directory") from error


def _write_ballot_packages(state: dict, target_image: Path, champion_images: Path, challenger_images: list[Path]) -> tuple[Path, Path]:
    proposal = state.get("activeProposal")
    if not proposal or proposal.get("finalized"):
        raise ValueError("active proposal is required for packages")
    candidates = proposal["candidates"]
    primary, audit = build_ballot_packages(
        _package_target_path(target_image),
        state["champion"]["candidateId"],
        _package_image_paths(champion_images),
        [
            {"candidateId": candidate["candidateId"], "images": _package_image_paths(path)}
            for candidate, path in zip(candidates, challenger_images, strict=True)
        ],
    )
    run_path = resolve_run_path(state["runId"])
    package_root = run_path.parent / f"round-{proposal['round']}"
    paths = (package_root / "primary-package.json", package_root / "audit-package.json")
    if any(path.exists() for path in paths):
        raise ValueError("round ballot packages already exist and will not be overwritten")
    for path, package in zip(paths, (primary, audit), strict=True):
        _write_json(path, package)
    proposal["ballotPackages"] = {
        evaluator: {
            "path": path.relative_to(ARTIFACT_ROOT).as_posix(),
            "hash": _file_hash(path),
            "package": package,
        }
        for evaluator, path, package in zip(("primary", "audit"), paths, (primary, audit), strict=True)
    }
    return paths


def _apply_decision(state: dict, decision: dict, ballots: list[dict] | None = None) -> None:
    proposal = state.get("activeProposal")
    if not proposal or proposal.get("finalized"):
        raise ValueError("active proposal must be finalized exactly once")
    axis = _decision_axis(state)
    state["steps"][axis] = decision["nextStep"]
    state["noPromotionCount"] = decision["noPromotionCount"]
    state["disagreementCount"] = decision["disagreementCount"]
    if decision["promote"]:
        for candidate in state.get("activeProposal", {}).get("candidates", []):
            if candidate["candidateId"] == decision["winnerId"]:
                state["champion"] = candidate
                break
    recorded_ballots = deepcopy(ballots or [])
    for ballot in recorded_ballots:
        ballot["validated"] = True
    agents = deepcopy(proposal["agents"])
    for ballot in recorded_ballots:
        evaluator_id = ballot["evaluatorId"]
        agents[evaluator_id] = {
            "identity": evaluator_id,
            "modelIdentity": ballot["modelIdentity"],
            "instructions": ballot["instructions"],
            "response": ballot["response"],
        }
    failures = []
    for ballot in recorded_ballots:
        failures.extend(f"{ballot['evaluatorId']}: {item}" for item in ballot.get("hardFailures", []))
        failures.extend(f"{ballot['evaluatorId']}: {item}" for item in ballot.get("severeRegressions", []))
    if len({ballot.get("winnerId") for ballot in recorded_ballots}) > 1:
        failures.append("evaluator-disagreement")
    if not decision["promote"] and not failures:
        failures.append("no-promotion")
    stopped = (
        state["noPromotionCount"] >= state["stopAfterNoPromotion"]
        or state["disagreementCount"] >= state["stopAfterDisagreement"]
    )
    record = {
        "searchSpace": deepcopy(state["searchSpace"]),
        "championBefore": deepcopy(proposal["championBefore"]),
        "selectedAxis": axis,
        "axisReason": deepcopy(proposal["axisReason"]),
        "stepBefore": proposal["stepBefore"],
        "candidates": deepcopy(proposal["candidates"]),
        "recipes": deepcopy(proposal["recipes"]),
        "agents": agents,
        "ballots": recorded_ballots,
        "failures": failures,
        "decision": deepcopy(decision),
        "championAfter": deepcopy(state["champion"]),
        "stepAfter": state["steps"][axis],
        "artifacts": deepcopy(proposal["artifacts"]),
        "ballotPackages": deepcopy(proposal.get("ballotPackages", {})),
        "verifierReceipts": deepcopy(proposal.get("verifierReceipts", {})),
        "nextExperiment": (
            {"action": "stop", "reason": "configured stop threshold reached"}
            if stopped
            else {"axis": axis, "reason": "continue bounded geometry search"}
        ),
    }
    artifact_root = Path(state["artifactRoot"])
    validate_round_record(record, artifact_root)
    immutable_record = json.loads(_canonical(record))
    state["history"].append(immutable_record)
    state["learning"].append(project_learning_row(immutable_record, artifact_root))
    proposal["finalized"] = True


def main(argv=None) -> int:
    root = Path(__file__).resolve().parent
    parser = argparse.ArgumentParser(description="Bounded JSON face search")
    commands = parser.add_subparsers(dest="command", required=True)
    init = commands.add_parser("init")
    init.add_argument("--run-id", required=True)
    init.add_argument("--space", type=Path, default=root / "face-search-space.json")
    propose = commands.add_parser("propose")
    propose.add_argument("--run-id", required=True)
    propose.add_argument("--axis", required=True)
    propose.add_argument("--round", type=int, required=True)
    package = commands.add_parser("package")
    package.add_argument("--run-id", required=True)
    package.add_argument("--target-image", type=Path, required=True)
    package.add_argument("--champion-images", type=Path, required=True)
    package.add_argument("--challenger-images", type=Path, nargs=2, required=True)
    validate = commands.add_parser("validate")
    validate.add_argument("--space", type=Path, default=root / "face-search-space.json")
    validate.add_argument("--candidate", type=Path, required=True)
    promote = commands.add_parser("promote")
    promote.add_argument("--run-id", required=True)
    promote.add_argument("--ballots", type=Path, required=True)
    args = parser.parse_args(argv)
    if args.command == "init":
        run_path = resolve_run_path(args.run_id)
        if run_path.exists() or run_path.parent.exists():
            raise ValueError("run already exists and init will not overwrite it")
        state = new_state(load_space(args.space), args.run_id)
        state["artifactRoot"] = str(ARTIFACT_ROOT)
        state["runDirectory"] = run_path.parent.relative_to(ARTIFACT_ROOT).as_posix()
        _write_json(run_path, state)
        print(run_path)
        return 0
    if args.command == "propose":
        run_path = resolve_run_path(args.run_id)
        state = _read_json(run_path)
        pair = propose_pair(state, args.axis, args.round)
        _write_json(run_path, state)
        print(json.dumps(pair, sort_keys=True))
        return 0
    if args.command == "package":
        run_path = resolve_run_path(args.run_id)
        state = _read_json(run_path)
        paths = _write_ballot_packages(
            state, args.target_image, args.champion_images, args.challenger_images
        )
        _write_json(run_path, state)
        print(json.dumps([path.as_posix() for path in paths]))
        return 0
    if args.command == "validate":
        validate_candidate(load_space(args.space), _read_json(args.candidate))
        return 0
    run_path = resolve_run_path(args.run_id)
    state = _read_json(run_path)
    raw_ballots = _read_json(args.ballots)
    packages = state.get("activeProposal", {}).get("ballotPackages", {})
    if (
        not isinstance(raw_ballots, list)
        or set(packages) != {"primary", "audit"}
        or any(not isinstance(ballot, dict) or ballot.get("evaluatorId") not in packages for ballot in raw_ballots)
    ):
        raise ValueError("promote requires primary and audit ballot packages")
    ballots = [
        validate_ballot(state["searchSpace"], packages[ballot["evaluatorId"]]["package"], ballot)
        for ballot in raw_ballots
    ]
    decision = decide_promotion(state, ballots)
    _apply_decision(state, decision, ballots)
    _write_json(run_path, state)
    print(json.dumps(decision, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
