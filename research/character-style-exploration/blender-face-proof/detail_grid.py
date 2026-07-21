"""Immutable Task 10 visible-detail recipes and landmark-only scoring."""

from copy import deepcopy
import hashlib
import json
import math
from pathlib import Path

from face_search import _candidate_id, validate_candidate
from feature_calibration import calibration_effective_controls, validate_calibration_recipe


PARENT_ID = "9032fd4dc9db2de64564b3f19e32ca5e6199881df111c2fa2d37feb1c7cc12f8"
GRID = {
    "eye_outer_corner_down": (0.08,),
    "mouth_width": (0.24, 0.36),
    "upper_lip_height": (0.08, 0.18),
    "nose_point_down": (0.15, 0.25),
}
DETAIL_KEYS = tuple(GRID)
LANDMARK_KEYS = (
    "leftEyeOuterCorner", "leftEyeInnerCorner", "rightEyeInnerCorner", "rightEyeOuterCorner",
    "mouthLeftCorner", "mouthRightCorner", "upperLipTopCenter", "mouthSeamCenter",
    "lowerLipBottomCenter", "noseTipLowestCenter",
)
RATIO_KEYS = ("eyeOuterCornerTilt", "mouthCommissureWidth", "upperLipHeight", "lowerLipHeight", "noseTipLowestHeight")
MACRO_KEYS = ("cheekWidth", "jawWidth", "eyeToChinHeight", "lowerFaceTaper")
DIRECT_DETAIL_SHAPE_KEYS = {"mouth.width": "mouth.width", "nose.pointDown": "nose.pointDown"}


def canonical(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"))


def digest(value):
    return hashlib.sha256(canonical(value).encode("utf-8")).hexdigest()


def file_hash(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def detail_controls(values):
    return {"values": values, "sha256": digest(values)}


def direct_detail_shape_key_values(controls):
    """Return absolute existing-key assignments for controls that must not stack."""
    if not isinstance(controls, dict) or any(not isinstance(controls.get(name), (int, float)) for name in DIRECT_DETAIL_SHAPE_KEYS):
        raise ValueError("direct detail controls are incomplete")
    return {shape_key: controls[name] for name, shape_key in DIRECT_DETAIL_SHAPE_KEYS.items()}


def target_binding(target_path):
    path = Path(target_path)
    return {"path": path.name, "sha256": file_hash(path)}


def task9_source_receipts(root):
    root = Path(root) / "Derived" / "search-runs" / "feature-calibration-20260720-100000" / "round-9" / PARENT_ID
    return {"featureGeometry": file_hash(root / "feature-geometry.json"), "macroGeometry": file_hash(root / "macro-geometry.json"), "parentManifest": file_hash(root / "manifest.json")}


def select_frontmost_landmark_vertex(candidates):
    """Choose the camera-frontmost vertex from the nearest small pixel neighborhood."""
    if not candidates or any(not isinstance(item.get("pixelDistance"), (int, float)) or not isinstance(item.get("cameraDepth"), (int, float)) for item in candidates):
        raise ValueError("landmark mapping candidates are invalid")
    nearest = min(item["pixelDistance"] for item in candidates)
    # Preserve the nearest candidate even when no body vertex falls within six pixels;
    # otherwise admit up to two additional pixels, capped at a six-pixel neighborhood.
    limit = max(nearest, min(6.0, nearest + 2.0))
    neighborhood = [item for item in candidates if item["pixelDistance"] <= limit]
    return min(neighborhood, key=lambda item: (item["cameraDepth"], item["pixelDistance"], item["index"]))


def validate_detail_target(target):
    if not isinstance(target, dict) or target.get("schemaVersion") != 1:
        raise ValueError("detail target schema is invalid")
    consensus = target.get("consensus", {})
    points = consensus.get("targetPoints")
    candidate_points = consensus.get("candidatePoints")
    if not isinstance(points, dict) or not isinstance(candidate_points, dict) or set(points) != set(LANDMARK_KEYS) or set(candidate_points) != set(LANDMARK_KEYS):
        raise ValueError("detail target landmarks are incomplete")
    stable = target.get("stableVertexIndices")
    if not isinstance(stable, dict) or set(stable) != {"indices", "receipt"}:
        raise ValueError("detail target landmark map is not frozen")
    indices = stable["indices"]
    receipt = stable["receipt"]
    if not isinstance(indices, dict) or set(indices) != set(LANDMARK_KEYS) or any(not isinstance(value, int) or value < 0 for value in indices.values()):
        raise ValueError("detail target landmark indices are invalid")
    if not isinstance(receipt, dict) or set(receipt) != {"path", "sha256"} or not isinstance(receipt["path"], str) or not isinstance(receipt["sha256"], str) or len(receipt["sha256"]) != 64:
        raise ValueError("detail target landmark receipt is invalid")
    ratios = target.get("metric", {}).get("ratios")
    if not isinstance(ratios, dict) or set(ratios) != set(RATIO_KEYS) or any(not isinstance(value, (int, float)) or not math.isfinite(value) for value in ratios.values()):
        raise ValueError("detail target ratios are invalid")


def detail_effective_controls(parent, controls):
    result = calibration_effective_controls(parent["parameters"], parent["calibrationControls"])
    values = controls["values"]
    result.update({
        "sculpt.eye.left.corner1.down": values["eye_outer_corner_down"],
        "sculpt.eye.right.corner1.down": values["eye_outer_corner_down"],
        "mouth.width": values["mouth_width"],
        "sculpt.upperlip.height.incr": values["upper_lip_height"],
        "nose.pointDown": values["nose_point_down"],
    })
    return result


def _detail_id(space, parent, controls, target, source_receipts):
    return digest({"schemaVersion": space["schemaVersion"], "parentId": parent["candidateId"], "round": 10,
                   "axis": "detail_grid", "direction": "grid", "parameters": parent["parameters"],
                   "calibrationControls": parent["calibrationControls"], "detailControls": controls,
                   "targetDetail": target, "sourceReceipts": source_receipts})


def validate_detail_recipe(space, parent, target, source_receipts, recipe):
    required = {"candidateId", "parentId", "round", "axis", "direction", "parameters", "calibrationControls", "detailControls", "targetDetail", "sourceReceipts"}
    if not isinstance(recipe, dict) or set(recipe) != required:
        raise ValueError("detail recipe is incomplete")
    validate_calibration_recipe(space, json.loads((Path(__file__).resolve().parent / "Runs" / "macro-grid-20260720-071000" / "candidates" / "eaec583dac7c9adf295510e82a06dd721dceb2aea371ced31bfdcaa88b884d5b.json").read_text()), parent["calibration"], parent)
    if recipe["parentId"] != parent["candidateId"] or parent["candidateId"] != PARENT_ID or recipe["round"] != 10 or recipe["axis"] != "detail_grid" or recipe["direction"] != "grid":
        raise ValueError("detail recipe lineage is invalid")
    if recipe["parameters"] != parent["parameters"] or recipe["calibrationControls"] != parent["calibrationControls"]:
        raise ValueError("detail recipe changed Task 7 or Task 9 controls")
    controls = recipe["detailControls"]
    values = controls.get("values") if isinstance(controls, dict) else None
    if not isinstance(controls, dict) or set(controls) != {"values", "sha256"} or not isinstance(values, dict) or set(values) != set(DETAIL_KEYS) or controls["sha256"] != digest(values) or any(values[key] not in GRID[key] for key in DETAIL_KEYS):
        raise ValueError("detail controls are invalid")
    if recipe["targetDetail"] != target or recipe["sourceReceipts"] != source_receipts:
        raise ValueError("detail recipe provenance does not match")
    plain = {key: recipe[key] for key in ("candidateId", "parentId", "round", "axis", "direction", "parameters")}
    plain["candidateId"] = _candidate_id(space["schemaVersion"], plain["parentId"], plain["round"], plain["axis"], plain["direction"], plain["parameters"])
    validate_candidate(space, plain)
    if recipe["candidateId"] != _detail_id(space, parent, controls, target, source_receipts):
        raise ValueError("detail candidate identity does not bind its contract")


def build_detail_grid(space, parent, target, source_receipts):
    recipes = []
    for mouth in GRID["mouth_width"]:
        for lip in GRID["upper_lip_height"]:
            for nose in GRID["nose_point_down"]:
                values = {"eye_outer_corner_down": 0.08, "mouth_width": mouth, "upper_lip_height": lip, "nose_point_down": nose}
                controls = detail_controls(values)
                recipe = {"parentId": parent["candidateId"], "round": 10, "axis": "detail_grid", "direction": "grid",
                          "parameters": deepcopy(parent["parameters"]), "calibrationControls": deepcopy(parent["calibrationControls"]),
                          "detailControls": controls, "targetDetail": target, "sourceReceipts": source_receipts}
                recipe["candidateId"] = _detail_id(space, parent, controls, target, source_receipts)
                validate_detail_recipe(space, parent, target, source_receipts, recipe)
                recipes.append(recipe)
    if len(recipes) != 8 or len({recipe["candidateId"] for recipe in recipes}) != 8:
        raise ValueError("detail grid must contain exactly eight recipes")
    return recipes


def initialize_detail_run(space, parent, target_path, run_id, run_root, source_receipts):
    """Write the immutable eight-recipe detail grid after the landmark map is frozen."""
    target_path = Path(target_path)
    target_payload = json.loads(target_path.read_text(encoding="utf-8"))
    validate_detail_target(target_payload)
    binding = target_binding(target_path)
    root = Path(run_root)
    recipe_root = root / "candidates"
    if (root / "run.json").exists() or recipe_root.exists():
        raise FileExistsError("detail run already exists and will not be overwritten")
    recipes = build_detail_grid(space, parent, binding, source_receipts)
    recipe_root.mkdir(parents=True)
    hashes = {}
    for recipe in recipes:
        path = recipe_root / f"{recipe['candidateId']}.json"
        data = canonical(recipe).encode("utf-8") + b"\n"
        path.write_bytes(data)
        hashes[recipe["candidateId"]] = hashlib.sha256(data).hexdigest()
    record = {"schemaVersion": 1, "runId": run_id, "purpose": "Task 10 visible landmark detail grid",
              "parentCandidate": parent, "targetDetail": binding, "sourceReceipts": source_receipts,
              "grid": GRID, "candidateCount": len(recipes), "recipeHashes": hashes,
              "selectionRule": "Rank only frozen visible landmark ratios; macro mouthWidth is excluded from scoring and gates. Label the winner review-only.",
              "humanAcceptance": False, "status": "initialized"}
    (root / "run.json").write_text(json.dumps(record, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return record


def _detail_ratios(camera, ipd, eye_y):
    if not math.isfinite(ipd) or ipd <= 0:
        raise ValueError("horizontal IPD is invalid")
    return {
        "eyeOuterCornerTilt": (((camera["leftEyeOuterCorner"][1] - camera["leftEyeInnerCorner"][1]) + (camera["rightEyeOuterCorner"][1] - camera["rightEyeInnerCorner"][1])) / 2) / ipd,
        "mouthCommissureWidth": abs(camera["mouthRightCorner"][0] - camera["mouthLeftCorner"][0]) / ipd,
        "upperLipHeight": abs(camera["mouthSeamCenter"][1] - camera["upperLipTopCenter"][1]) / ipd,
        "lowerLipHeight": abs(camera["lowerLipBottomCenter"][1] - camera["mouthSeamCenter"][1]) / ipd,
        "noseTipLowestHeight": abs(camera["noseTipLowestCenter"][1] - eye_y) / ipd,
    }


def detail_ratios(anchors, horizontal_ipd=None, eye_center_y=None):
    if not isinstance(anchors, dict) or set(anchors) != set(LANDMARK_KEYS):
        raise ValueError("detail anchors are incomplete")
    camera = {key: anchors[key]["camera"] for key in LANDMARK_KEYS}
    if any(not isinstance(value, list) or len(value) < 2 for value in camera.values()):
        raise ValueError("detail anchor camera coordinates are invalid")
    left_center = (camera["leftEyeOuterCorner"][0] + camera["leftEyeInnerCorner"][0]) / 2
    right_center = (camera["rightEyeInnerCorner"][0] + camera["rightEyeOuterCorner"][0]) / 2
    ipd = abs(right_center - left_center) if horizontal_ipd is None else horizontal_ipd
    eye_y = sum(camera[key][1] for key in ("leftEyeOuterCorner", "leftEyeInnerCorner", "rightEyeInnerCorner", "rightEyeOuterCorner")) / 4 if eye_center_y is None else eye_center_y
    return _detail_ratios(camera, ipd, eye_y)


def target_detail_ratios(detail_points, feature_points):
    """Derive Task 10 target ratios from detail points and the frozen feature IPD."""
    if set(detail_points) != set(LANDMARK_KEYS) or not {"leftEyeCenter", "rightEyeCenter"} <= set(feature_points):
        raise ValueError("target detail points are incomplete")
    ipd = abs(feature_points["rightEyeCenter"][0] - feature_points["leftEyeCenter"][0])
    eye_y = (feature_points["leftEyeCenter"][1] + feature_points["rightEyeCenter"][1]) / 2
    camera = {key: [detail_points[key][0], detail_points[key][1], 1.0] for key in LANDMARK_KEYS}
    return _detail_ratios(camera, ipd, eye_y)


def detail_score(target, record):
    return sum(abs(record["ratios"][key] - target["metric"]["ratios"][key]) for key in RATIO_KEYS)


def detail_macro_regression_gate(baseline, candidates, tolerance):
    if not isinstance(tolerance, (int, float)) or not math.isfinite(tolerance) or tolerance < 0:
        raise ValueError("macro tolerance is invalid")
    base = baseline.get("ratios", {})
    if set(base) < set(MACRO_KEYS):
        raise ValueError("baseline macro receipt is incomplete")
    maximum = 0.0
    for candidate in candidates:
        ratios = candidate.get("ratios", {})
        if set(ratios) < set(MACRO_KEYS):
            raise ValueError("candidate macro receipt is incomplete")
        maximum = max(maximum, *(abs(ratios[key] - base[key]) for key in MACRO_KEYS))
    return {"keys": list(MACRO_KEYS), "tolerance": tolerance, "maximumObservedDelta": maximum, "passed": maximum <= tolerance}
