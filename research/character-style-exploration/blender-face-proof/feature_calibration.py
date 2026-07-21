"""Measured Task 8 response derivation and bounded Task 9 recipes."""

from copy import deepcopy
import hashlib
import json
import math
from pathlib import Path
import re

from face_search import _candidate_id, candidate_to_controls, validate_candidate


GRID = {"eye_aperture": (0.42, 0.52), "nose_length": (0.08, 0.14), "nose_width": (0.27,)}


def canonical(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"))


def digest(value):
    return hashlib.sha256(canonical(value).encode()).hexdigest()


def calibration_controls(values):
    return {"values": values, "sha256": digest(values)}


def derive_calibration(sources, target):
    """Derive one isolated finite-difference response per Task 8 control."""
    if not isinstance(target, dict) or not isinstance(target.get("ratios"), dict):
        raise ValueError("target feature ratios are missing")
    for item in sources:
        if (
            not isinstance(item, dict)
            or not isinstance(item.get("candidateId"), str)
            or not re.fullmatch(r"[0-9a-f]{64}", item.get("sha256", ""))
            or set(item.get("controls", {})) != set(GRID)
            or not isinstance(item.get("ratios"), dict)
        ):
            raise ValueError("source receipt is incomplete")
    by_id = {item["candidateId"]: item for item in sources}
    if len(by_id) != len(sources) or not sources:
        raise ValueError("source receipts must have unique IDs")
    baseline = next((item for item in sources if item["controls"] == {"eye_aperture": 0.0, "nose_length": 0.30, "nose_width": 0.26}), None)
    if baseline is None:
        raise ValueError("Task 8 all-low receipt is missing")
    responses = {}
    for axis, high in (("eye_aperture", 0.12), ("nose_length", 0.37)):
        peer = next((item for item in sources if all(item["controls"][key] == (high if key == axis else baseline["controls"][key]) for key in baseline["controls"])), None)
        if peer is None:
            raise ValueError(f"Task 8 isolated {axis} receipt is missing")
        metric = "eyeAperture" if axis == "eye_aperture" else "noseLength"
        if not all(isinstance(value, (int, float)) and math.isfinite(value) for value in (peer["ratios"].get(metric), baseline["ratios"].get(metric), target["ratios"].get(metric))):
            raise ValueError(f"Task 8 {axis} metric is not finite")
        slope = (peer["ratios"][metric] - baseline["ratios"][metric]) / (peer["controls"][axis] - baseline["controls"][axis])
        if slope == 0:
            raise ValueError(f"Task 8 {axis} response is not monotonic")
        crossing = baseline["controls"][axis] + (target["ratios"][metric] - baseline["ratios"][metric]) / slope
        responses[axis] = {"metric": metric, "lowCandidateId": baseline["candidateId"], "highCandidateId": peer["candidateId"], "slope": slope, "predictedCrossing": crossing, "bracket": [GRID[axis][0], GRID[axis][-1]]}
    return {"schemaVersion": 1, "sourceMetricHashes": {item["candidateId"]: item["sha256"] for item in sources}, "responses": responses}


def load_task8_sources(root, run_id="feature-grid-20260720-094000", invalidated=None):
    root = Path(root)
    if invalidated is not None and run_id in invalidated:
        raise ValueError(f"Task 8 source run is invalidated: {run_id}")
    result = []
    for recipe_path in sorted((root / "Runs" / run_id / "candidates").glob("*.json")):
        recipe = json.loads(recipe_path.read_text())
        metric_path = root / "Derived" / "search-runs" / run_id / "round-8" / recipe["candidateId"] / "feature-geometry.json"
        metric = json.loads(metric_path.read_text())
        result.append({"candidateId": recipe["candidateId"], "controls": recipe["featureControls"]["values"], "ratios": metric["ratios"], "sha256": hashlib.sha256(metric_path.read_bytes()).hexdigest()})
    return result


def calibration_effective_controls(parameters, controls):
    result = candidate_to_controls(parameters)
    result.update({"sculpt.eye.left.height1": controls["values"]["eye_aperture"], "sculpt.eye.right.height1": controls["values"]["eye_aperture"]})
    return result


def _id(space, parent_id, parameters, controls, calibration):
    return digest({"schemaVersion": space["schemaVersion"], "parentId": parent_id, "round": 9, "axis": "feature_calibration", "direction": "grid", "parameters": parameters, "calibrationControls": controls, "calibration": calibration})


def _validate_calibration(calibration):
    if not isinstance(calibration, dict) or set(calibration) != {"schemaVersion", "sourceMetricHashes", "responses"} or calibration["schemaVersion"] != 1:
        raise ValueError("calibration record is incomplete")
    source_hashes = calibration["sourceMetricHashes"]
    if (
        not isinstance(source_hashes, dict)
        or not source_hashes
        or any(not isinstance(candidate_id, str) or not candidate_id or not isinstance(value, str) or not re.fullmatch(r"[0-9a-f]{64}", value) for candidate_id, value in source_hashes.items())
    ):
        raise ValueError("calibration source metric hashes are invalid")
    responses = calibration["responses"]
    if not isinstance(responses, dict) or set(responses) != {"eye_aperture", "nose_length"}:
        raise ValueError("calibration responses are incomplete")
    for axis, metric in (("eye_aperture", "eyeAperture"), ("nose_length", "noseLength")):
        response = responses[axis]
        required = {"metric", "lowCandidateId", "highCandidateId", "slope", "predictedCrossing", "bracket"}
        if (
            not isinstance(response, dict) or set(response) != required or response["metric"] != metric
            or response["lowCandidateId"] not in source_hashes or response["highCandidateId"] not in source_hashes
            or response["lowCandidateId"] == response["highCandidateId"]
            or not all(isinstance(response[key], (int, float)) and math.isfinite(response[key]) for key in ("slope", "predictedCrossing"))
            or response["slope"] == 0 or response["bracket"] != [GRID[axis][0], GRID[axis][-1]]
        ):
            raise ValueError(f"calibration {axis} response is invalid")


def validate_calibration_recipe(space, parent, calibration, recipe):
    if not isinstance(recipe, dict) or set(recipe) != {"candidateId", "parentId", "round", "axis", "direction", "parameters", "calibrationControls", "calibration"}:
        raise ValueError("calibration recipe is incomplete")
    _validate_calibration(calibration)
    controls = recipe["calibrationControls"]
    values = controls.get("values") if isinstance(controls, dict) else None
    if (
        not isinstance(controls, dict) or set(controls) != {"values", "sha256"}
        or not isinstance(values, dict) or set(values) != set(GRID)
        or controls["sha256"] != digest(values)
        or any(values[key] not in GRID[key] for key in GRID)
        or recipe["calibration"] != calibration
    ):
        raise ValueError("calibration recipe controls are invalid")
    if recipe["parentId"] != parent["candidateId"] or recipe["round"] != 9 or recipe["axis"] != "feature_calibration" or recipe["direction"] != "grid":
        raise ValueError("calibration recipe lineage is invalid")
    if not isinstance(recipe["parameters"], dict) or set(recipe["parameters"]) != set(parent["parameters"]):
        raise ValueError("calibration recipe parameters are incomplete")
    for key, value in parent["parameters"].items():
        expected = values[key] if key in {"nose_length", "nose_width"} else value
        if recipe["parameters"].get(key) != expected:
            raise ValueError("calibration recipe has undeclared parameter drift")
    plain = {key: recipe[key] for key in ("candidateId", "parentId", "round", "axis", "direction", "parameters")}
    plain["candidateId"] = _candidate_id(space["schemaVersion"], plain["parentId"], plain["round"], plain["axis"], plain["direction"], plain["parameters"])
    validate_candidate(space, plain)
    if recipe["candidateId"] != _id(space, recipe["parentId"], recipe["parameters"], controls, calibration):
        raise ValueError("calibration candidate identity does not bind derivation")


def build_calibration_grid(space, parent, calibration):
    recipes = []
    for eye in GRID["eye_aperture"]:
        for length in GRID["nose_length"]:
            values = {"eye_aperture": eye, "nose_length": length, "nose_width": 0.27}
            controls = calibration_controls(values)
            parameters = deepcopy(parent["parameters"])
            parameters.update(nose_length=length, nose_width=0.27)
            recipe = {"parentId": parent["candidateId"], "round": 9, "axis": "feature_calibration", "direction": "grid", "parameters": parameters, "calibrationControls": controls, "calibration": calibration}
            recipe["candidateId"] = _id(space, recipe["parentId"], parameters, controls, calibration)
            validate_calibration_recipe(space, parent, calibration, recipe)
            recipes.append(recipe)
    if len(recipes) != 4 or len({item["candidateId"] for item in recipes}) != 4:
        raise ValueError("calibration grid must contain exactly four recipes")
    return recipes


def initialize_calibration_run(space, parent, target, calibration, run_id, run_root):
    """Write the immutable Task 9 four-recipe set before Blender is dispatched."""
    root = Path(run_root)
    recipe_root = root / "candidates"
    if (root / "run.json").exists() or recipe_root.exists():
        raise FileExistsError("calibration run already exists and will not be overwritten")
    _validate_calibration(calibration)
    recipes = build_calibration_grid(space, parent, calibration)
    recipe_root.mkdir(parents=True)
    hashes = {}
    for recipe in recipes:
        path = recipe_root / f"{recipe['candidateId']}.json"
        data = canonical(recipe).encode("utf-8") + b"\n"
        path.write_bytes(data)
        hashes[recipe["candidateId"]] = hashlib.sha256(data).hexdigest()
    record = {
        "schemaVersion": 1, "runId": run_id, "purpose": "Task 9 bounded measured feature calibration",
        "parentCandidate": parent, "targetAnnotation": target, "grid": GRID,
        "calibration": calibration, "candidateCount": len(recipes), "recipeHashes": hashes,
        "selectionRule": "Rank only by the existing feature metric after all hard gates; label the best calibration-best-for-human-review and do not machine-promote.",
        "humanAcceptance": False, "status": "initialized",
    }
    (root / "run.json").write_text(json.dumps(record, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return record
