"""Deterministic Task 7 macro-grid recipes and front-geometry scoring."""

from copy import deepcopy
import hashlib
import json
import math
from pathlib import Path

from face_search import _candidate_id, validate_candidate


GRID = {
    "head_width": (-0.08, 0.10),
    "head_height": (0.00, -0.20),
    "jaw_taper": (0.085, 0.00),
}
RATIO_KEYS = ("cheekWidth", "jawWidth", "mouthWidth", "eyeToChinHeight", "lowerFaceTaper")


def _canonical(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"))


def build_grid(space, retained, run_id):
    """Return the eight bounded, complete recipes; no adaptive search state exists."""
    values = [GRID[axis] for axis in GRID]
    recipes = []
    for width in values[0]:
        for height in values[1]:
            for taper in values[2]:
                parameters = deepcopy(retained["parameters"])
                parameters.update(head_width=width, head_height=height, jaw_taper=taper)
                candidate = {
                    "parentId": retained["candidateId"],
                    "round": 7,
                    "axis": "macro_grid",
                    "direction": "grid",
                    "parameters": parameters,
                }
                candidate["candidateId"] = _candidate_id(
                    space["schemaVersion"], candidate["parentId"], candidate["round"],
                    candidate["axis"], candidate["direction"], parameters,
                )
                validate_candidate(space, candidate)
                recipes.append(candidate)
    if len(recipes) != 8 or len({item["candidateId"] for item in recipes}) != 8:
        raise ValueError("macro grid must contain exactly eight unique candidates")
    return recipes


def contour_extrema(points, fraction, half_band, eye_z, chin_z):
    """Select visible facial-contour x extrema from one normalized vertical band."""
    target_z = eye_z - fraction * (eye_z - chin_z)
    eligible = [
        point for point in points
        if point.get("isBody", True) and not point.get("isEar", False) and abs(point["z"] - target_z) <= half_band
    ]
    if not eligible:
        raise ValueError("contour band has no non-ear surface vertices")
    return {"left": min(eligible, key=lambda point: point["x"]),
            "right": max(eligible, key=lambda point: point["x"]),
            "targetZ": target_z}


def mouth_corner_indices(horizontal_displacements):
    """Return the stable left/right corners from the strongest signed mouth target moves."""
    negative = [item for item in horizontal_displacements if item[1] < 0]
    positive = [item for item in horizontal_displacements if item[1] > 0]
    if not negative or not positive:
        raise ValueError("mouth target has no signed horizontal corner displacement")
    return min(negative, key=lambda item: item[1])[0], max(positive, key=lambda item: item[1])[0]


def validate_metric_record(target, record, camera_hash, vertex_count, topology_hash):
    if record.get("targetImage", {}).get("sha256") != target.get("image", {}).get("sha256"):
        raise ValueError("target image hash changed")
    if record.get("camera", {}).get("sha256") != camera_hash:
        raise ValueError("camera hash changed")
    topology = record.get("topology", {})
    if topology.get("vertexCount") != vertex_count or topology.get("sha256") != topology_hash:
        raise ValueError("topology changed")
    anchors = record.get("anchors")
    required = {"leftEye", "rightEye", "leftCheek", "rightCheek", "leftJaw", "rightJaw", "leftMouth", "rightMouth", "chin"}
    if not isinstance(anchors, dict) or required - set(anchors) or any(not anchors[name] for name in required):
        raise ValueError("metric anchors are incomplete")
    ratios = record.get("ratios")
    if not isinstance(ratios, dict) or set(ratios) != set(RATIO_KEYS):
        raise ValueError("metric ratios are incomplete")
    if any(not isinstance(ratios[key], (int, float)) or not math.isfinite(ratios[key]) for key in RATIO_KEYS):
        raise ValueError("metric ratios must be finite")
    if not 0.65 <= ratios["mouthWidth"] <= 1.45:
        raise ValueError("metric mouth width is outside anatomical IPD range")


def score_metrics(target, record):
    """Unweighted L1 error over only manually annotated front-supported ratios."""
    target_ratios = target["ratios"]
    ratios = record["ratios"]
    if set(target_ratios) != set(RATIO_KEYS):
        raise ValueError("target ratios are incomplete")
    return sum(abs(ratios[key] - target_ratios[key]) for key in RATIO_KEYS)


def decide_nomination(retained, candidates, minimum_margin):
    if not isinstance(minimum_margin, (int, float)) or minimum_margin <= 0:
        raise ValueError("minimum improvement margin must be positive")
    best = min(candidates, key=lambda item: (item["score"], item["candidateId"]))
    improvement = retained["score"] - best["score"]
    if improvement < minimum_margin:
        return {"status": "retained", "candidateId": retained["candidateId"], "improvement": improvement,
                "minimumMargin": minimum_margin}
    return {"status": "nominated", "candidateId": best["candidateId"], "improvement": improvement,
            "minimumMargin": minimum_margin}


def initialize_run(space, retained, target, run_id, run_root):
    """Create the authoritative immutable recipe set before any Blender build starts."""
    run_root = Path(run_root)
    recipe_root = run_root / "candidates"
    if (run_root / "run.json").exists() or recipe_root.exists():
        raise FileExistsError("macro run already exists and will not be overwritten")
    recipes = build_grid(space, retained, run_id)
    recipe_root.mkdir(parents=True)
    recipe_hashes = {}
    for recipe in recipes:
        path = recipe_root / f"{recipe['candidateId']}.json"
        data = _canonical(recipe).encode("utf-8") + b"\n"
        path.write_bytes(data)
        recipe_hashes[recipe["candidateId"]] = hashlib.sha256(data).hexdigest()
    record = {
        "schemaVersion": 1, "runId": run_id, "purpose": "Task 7 deterministic macro geometry grid",
        "retainedCandidate": retained, "targetAnnotation": target,
        "grid": GRID, "candidateCount": len(recipes), "recipeHashes": recipe_hashes,
        "minimumImprovementMargin": 0.02,
        "selectionRule": "Nominate only the lowest deterministic front-geometry score that beats retained by the declared margin and passes fresh 3D hard checks.",
        "humanAcceptance": False,
        "status": "initialized",
    }
    (run_root / "run.json").write_text(json.dumps(record, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return record
