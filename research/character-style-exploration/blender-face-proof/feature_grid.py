"""Deterministic Task 8 feature-grid recipes and front-geometry scoring."""

from copy import deepcopy
import hashlib
import json
import math

from face_search import _candidate_id, candidate_to_controls, validate_candidate


FEATURE_GRID = {
    "eye_aperture": (0.00, 0.12),
    "nose_length": (0.30, 0.37),
    "nose_width": (0.26, 0.33),
}
FEATURE_KEYS = tuple(FEATURE_GRID)
RATIO_KEYS = ("eyeAperture", "noseLength", "alarWidth")


def _canonical(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"))


def _hash(value):
    return hashlib.sha256(_canonical(value).encode("utf-8")).hexdigest()


def _feature_controls(values):
    return {"values": values, "sha256": _hash(values)}


def _feature_candidate_id(space, parent_id, parameters, controls):
    return _hash({
        "schemaVersion": space["schemaVersion"], "parentId": parent_id, "round": 8,
        "axis": "feature_grid", "direction": "grid", "parameters": parameters,
        "featureControls": controls,
    })


def validate_feature_recipe(space, parent, recipe):
    """Reject any recipe whose identity or non-feature parent vector drifts."""
    required = {"candidateId", "parentId", "round", "axis", "direction", "parameters", "featureControls"}
    if not isinstance(recipe, dict) or set(recipe) != required:
        raise ValueError("feature recipe fields are incomplete")
    controls = recipe["featureControls"]
    if not isinstance(controls, dict) or set(controls) != {"values", "sha256"}:
        raise ValueError("featureControls record is incomplete")
    values = controls["values"]
    if not isinstance(values, dict) or set(values) != set(FEATURE_KEYS) or controls["sha256"] != _hash(values):
        raise ValueError("featureControls hash does not match values")
    if any(values[key] not in FEATURE_GRID[key] for key in FEATURE_KEYS):
        raise ValueError("featureControls values are outside the fixed grid")
    if recipe["parentId"] != parent["candidateId"] or recipe["round"] != 8 or recipe["axis"] != "feature_grid" or recipe["direction"] != "grid":
        raise ValueError("feature recipe lineage is invalid")
    parameters = recipe["parameters"]
    base = {key: parameters[key] for key in parent["parameters"]} if isinstance(parameters, dict) and set(parameters) == set(parent["parameters"]) else None
    if base is None:
        raise ValueError("feature recipe parameters are incomplete")
    for key, value in parent["parameters"].items():
        expected = values[key] if key in {"nose_length", "nose_width"} else value
        if parameters[key] != expected:
            raise ValueError("feature recipe contains an undeclared control difference")
    plain = {key: recipe[key] for key in required - {"featureControls"}}
    plain["candidateId"] = _candidate_id(space["schemaVersion"], plain["parentId"], plain["round"], plain["axis"], plain["direction"], parameters)
    validate_candidate(space, plain)
    if recipe["candidateId"] != _feature_candidate_id(space, recipe["parentId"], parameters, controls):
        raise ValueError("feature candidate identity does not bind featureControls")
    return recipe


def build_feature_grid(space, parent):
    recipes = []
    for aperture in FEATURE_GRID["eye_aperture"]:
        for length in FEATURE_GRID["nose_length"]:
            for width in FEATURE_GRID["nose_width"]:
                values = {"eye_aperture": aperture, "nose_length": length, "nose_width": width}
                controls = _feature_controls(values)
                parameters = deepcopy(parent["parameters"])
                parameters.update(nose_length=length, nose_width=width)
                recipe = {
                    "parentId": parent["candidateId"], "round": 8, "axis": "feature_grid", "direction": "grid",
                    "parameters": parameters, "featureControls": controls,
                }
                recipe["candidateId"] = _feature_candidate_id(space, recipe["parentId"], parameters, controls)
                validate_feature_recipe(space, parent, recipe)
                recipes.append(recipe)
    if len(recipes) != 8 or len({recipe["candidateId"] for recipe in recipes}) != 8:
        raise ValueError("feature grid must contain exactly eight unique recipes")
    return recipes


def initialize_feature_run(space, parent, target, run_id, run_root):
    """Write the immutable eight-recipe set before any Blender command is dispatched."""
    from pathlib import Path

    root = Path(run_root)
    recipe_root = root / "candidates"
    if (root / "run.json").exists() or recipe_root.exists():
        raise FileExistsError("feature run already exists and will not be overwritten")
    recipes = build_feature_grid(space, parent)
    recipe_root.mkdir(parents=True)
    hashes = {}
    for recipe in recipes:
        path = recipe_root / f"{recipe['candidateId']}.json"
        data = _canonical(recipe).encode("utf-8") + b"\n"
        path.write_bytes(data)
        hashes[recipe["candidateId"]] = hashlib.sha256(data).hexdigest()
    replay = next(recipe for recipe in recipes if recipe["featureControls"]["values"] == {
        "eye_aperture": 0.0, "nose_length": 0.30, "nose_width": 0.26,
    })
    record = {
        "schemaVersion": 1, "runId": run_id, "purpose": "Task 8 deterministic feature geometry grid",
        "parentCandidate": parent, "targetAnnotation": target, "grid": FEATURE_GRID,
        "candidateCount": len(recipes), "recipeHashes": hashes, "allLowReplay": replay,
        "minimumImprovementMargin": 0.02,
        "annotationUncertaintyMargin": target.get("totalUncertaintyRatio", 0.0),
        "selectionRule": "Nominate only a feature-only geometry score improving the all-low replay by the declared margin while preserving Task 7 macro ratios and all fresh hard gates.",
        "humanAcceptance": False, "status": "initialized",
    }
    (root / "run.json").write_text(json.dumps(record, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return record


def feature_effective_controls(parameters, feature_controls):
    """The complete legacy control vector plus the one declared aperture pair."""
    controls = candidate_to_controls(parameters)
    aperture = feature_controls["values"]["eye_aperture"]
    controls.update({"sculpt.eye.left.height1": aperture, "sculpt.eye.right.height1": aperture})
    return controls


def body_signed_extrema(records, body_indices, component):
    """Choose a semantic signed target role from visible head body vertices only."""
    eligible = [record for record in records if record[0] in body_indices]
    if not eligible:
        raise ValueError("target has no body vertices")
    positive = max(eligible, key=lambda record: record[component])
    negative = min(eligible, key=lambda record: record[component])
    if positive[component] <= 0 or negative[component] >= 0:
        raise ValueError("target body vertices do not expose paired signed extrema")
    return positive[0], negative[0]


def validate_feature_metric(target, record, camera_hash, scene_lock_hash, vertex_count, topology_hash):
    if record.get("targetImage", {}).get("sha256") != target.get("image", {}).get("sha256"):
        raise ValueError("target image hash changed")
    if record.get("camera", {}).get("sha256") != camera_hash or record.get("sceneLock", {}).get("sha256") != scene_lock_hash:
        raise ValueError("scene lock changed")
    topology = record.get("topology", {})
    if topology.get("vertexCount") != vertex_count or topology.get("sha256") != topology_hash:
        raise ValueError("topology changed")
    anchors = record.get("anchors")
    required = {"leftUpperLid", "leftLowerLid", "rightUpperLid", "rightLowerLid", "nasion", "subnasale", "leftAlare", "rightAlare", "leftEyeCenter", "rightEyeCenter"}
    if not isinstance(anchors, dict) or required - set(anchors) or any(not anchors[key] for key in required):
        raise ValueError("feature anchors are incomplete")
    ratios = record.get("ratios")
    if not isinstance(ratios, dict) or set(ratios) != set(RATIO_KEYS) or any(not isinstance(ratios[key], (int, float)) or not math.isfinite(ratios[key]) for key in RATIO_KEYS):
        raise ValueError("feature ratios must be finite and complete")


def feature_score(target, record):
    ratios = target["ratios"]
    if set(ratios) != set(RATIO_KEYS):
        raise ValueError("feature target ratios are incomplete")
    return sum(abs(record["ratios"][key] - ratios[key]) for key in RATIO_KEYS)


def macro_regression_gate(baseline_record, candidate_records, tolerance):
    """Reproduce the Task 8 macro gate from metric receipts, not a supplied boolean."""
    if not isinstance(tolerance, (int, float)) or not math.isfinite(tolerance) or tolerance < 0:
        raise ValueError("macro regression tolerance must be finite and non-negative")
    baseline = baseline_record.get("ratios")
    if not isinstance(baseline, dict) or not baseline:
        raise ValueError("baseline macro ratios are missing")
    maximum = 0.0
    for record in candidate_records:
        ratios = record.get("ratios")
        if not isinstance(ratios, dict) or set(ratios) != set(baseline):
            raise ValueError("candidate macro ratios do not match baseline")
        for key, value in ratios.items():
            if not isinstance(value, (int, float)) or not math.isfinite(value):
                raise ValueError("candidate macro ratios must be finite")
            maximum = max(maximum, abs(value - baseline[key]))
    return {"tolerance": tolerance, "maximumObservedDelta": maximum, "passed": maximum <= tolerance}


def decide_feature_nomination(baseline, candidates, minimum_margin, macro_passed, hard_gates_passed, annotation_uncertainty=0.0):
    best = min(candidates, key=lambda candidate: (candidate["score"], candidate["candidateId"]))
    improvement = baseline["score"] - best["score"]
    required_margin = max(minimum_margin, annotation_uncertainty)
    status = "nominated" if improvement >= required_margin and macro_passed and hard_gates_passed else "retained"
    return {"status": status, "candidateId": best["candidateId"] if status == "nominated" else baseline["candidateId"],
            "improvement": improvement, "minimumMargin": minimum_margin, "annotationUncertaintyMargin": annotation_uncertainty, "requiredMargin": required_margin,
            "macroPassed": macro_passed, "hardGatesPassed": hard_gates_passed}
