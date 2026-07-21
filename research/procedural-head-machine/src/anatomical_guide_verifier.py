import argparse
import json
import math
from collections import deque
from pathlib import Path


REQUIRED_LANDMARKS = {
    "pelvis", "waist", "chest", "neck_base", "head_base", "head_center",
    "left_shoulder", "left_elbow", "left_wrist", "right_shoulder", "right_elbow", "right_wrist",
    "left_hip", "left_knee", "left_ankle", "right_hip", "right_knee", "right_ankle",
}
METADATA = ("body_plan_id", "placements", "connections", "cells", "mirrors")


def _failure(rule, subject):
    return {"rule": rule, "subject": subject}


def verify_anatomical_guide(body, recipe, guide, tolerance=1e-9):
    failures = []
    landmarks = guide.get("landmarks", {})
    if guide.get("schema") != "anatomical-guide/0" or guide.get("id") != recipe["id"] or not REQUIRED_LANDMARKS.issubset(landmarks):
        failures.append(_failure("ANATOMY-SCHEMA-1", "guide"))

    finite = all(
        isinstance(point, list)
        and len(point) == 3
        and all(isinstance(value, (int, float)) and math.isfinite(value) for value in point)
        for point in landmarks.values()
    )
    body_min = [min(vertex[axis] for vertex in body["vertices"]) - 0.1 for axis in range(3)]
    body_max = [max(vertex[axis] for vertex in body["vertices"]) + 0.1 for axis in range(3)]
    contained = finite and all(
        all(body_min[axis] <= point[axis] <= body_max[axis] for axis in range(3))
        for point in landmarks.values()
    )
    if not contained:
        failures.append(_failure("ANATOMY-BOUNDS-1", "landmarks"))

    symmetry_errors = []
    for pair in guide.get("symmetry_pairs", []):
        if len(pair) != 2 or any(name not in landmarks for name in pair):
            continue
        left, right = (landmarks[name] for name in pair)
        symmetry_errors.append(math.dist(left, [-right[0], right[1], right[2]]))
    max_symmetry_error = max(symmetry_errors, default=float("inf"))
    if len(symmetry_errors) != 6 or not math.isfinite(max_symmetry_error) or max_symmetry_error > tolerance:
        failures.append(_failure("ANATOMY-SYMMETRY-1", "paired landmarks"))

    order_valid = (
        landmarks.get("pelvis", [0, 0, 0])[2]
        < landmarks.get("waist", [0, 0, 0])[2]
        < landmarks.get("chest", [0, 0, 0])[2]
        < landmarks.get("neck_base", [0, 0, 0])[2]
        < landmarks.get("head_center", [0, 0, 0])[2]
    )
    for side, sign in (("left", -1), ("right", 1)):
        shoulder = landmarks.get(f"{side}_shoulder", [0, 0, 0])
        elbow = landmarks.get(f"{side}_elbow", [0, 0, 0])
        wrist = landmarks.get(f"{side}_wrist", [0, 0, 0])
        order_valid = order_valid and sign * shoulder[0] < sign * elbow[0] < sign * wrist[0]
        order_valid = order_valid and landmarks.get(f"{side}_hip", [0, 0, 0])[2] > landmarks.get(f"{side}_knee", [0, 0, 0])[2] > landmarks.get(f"{side}_ankle", [0, 0, 0])[2]
    if not order_valid:
        failures.append(_failure("ANATOMY-ORDER-1", "joint chains"))

    skeleton = guide.get("skeleton_segments", [])
    adjacency = {name: set() for name in landmarks}
    skeleton_valid = bool(skeleton)
    for segment in skeleton:
        start, end = segment.get("start"), segment.get("end")
        skeleton_valid = skeleton_valid and start in landmarks and end in landmarks and start != end
        if start in landmarks and end in landmarks:
            adjacency[start].add(end)
            adjacency[end].add(start)
    used = {name for name, neighbors in adjacency.items() if neighbors}
    if used:
        reached = set()
        queue = deque([next(iter(used))])
        while queue:
            current = queue.popleft()
            if current in reached:
                continue
            reached.add(current)
            queue.extend(adjacency[current] - reached)
        skeleton_valid = skeleton_valid and reached == used
    if not skeleton_valid:
        failures.append(_failure("ANATOMY-GRAPH-1", "skeleton segments"))

    surface_valid = bool(guide.get("surface_segments")) and bool(guide.get("surface_ellipsoids"))
    modules = set(body["placements"])
    for segment in guide.get("surface_segments", []):
        surface_valid = surface_valid and (
            segment.get("start") in landmarks
            and segment.get("end") in landmarks
            and segment.get("module") in modules
            and segment.get("role") in {"core", "head", "limb"}
            and isinstance(segment.get("start_radius_m"), (int, float))
            and isinstance(segment.get("end_radius_m"), (int, float))
            and segment["start_radius_m"] > 0
            and segment["end_radius_m"] > 0
            and segment.get("samples") == recipe["segment_samples"]
        )
    for ellipsoid in guide.get("surface_ellipsoids", []):
        surface_valid = surface_valid and (
            ellipsoid.get("module") in modules
            and ellipsoid.get("role") in {"core", "head", "limb"}
            and len(ellipsoid.get("center", [])) == 3
            and len(ellipsoid.get("radii", [])) == 3
            and all(isinstance(value, (int, float)) and math.isfinite(value) and value > 0 for value in ellipsoid["radii"])
        )
    if not surface_valid:
        failures.append(_failure("ANATOMY-SURFACE-1", "volume guides"))

    metadata = guide.get("body_metadata", {})
    if any(metadata.get(key) != body.get(key) for key in METADATA):
        failures.append(_failure("ANATOMY-PROVENANCE-1", "body plan"))

    return {
        "schema": "anatomical-guide-verification/0",
        "passed": not failures,
        "failures": failures,
        "metrics": {
            "landmarks": len(landmarks),
            "skeleton_segments": len(skeleton),
            "surface_segments": len(guide.get("surface_segments", [])),
            "surface_ellipsoids": len(guide.get("surface_ellipsoids", [])),
            "max_symmetry_error_m": max_symmetry_error,
        },
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("body_run", type=Path)
    parser.add_argument("recipe", type=Path)
    parser.add_argument("guide", type=Path)
    parser.add_argument("report", type=Path)
    args = parser.parse_args()
    body = json.loads((args.body_run / "composed-mesh.json").read_text(encoding="utf-8"))
    recipe = json.loads(args.recipe.read_text(encoding="utf-8"))
    guide = json.loads(args.guide.read_text(encoding="utf-8"))
    report = verify_anatomical_guide(body, recipe, guide)
    args.report.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"ANATOMICAL_GUIDE_{'PASS' if report['passed'] else 'FAIL'} report={args.report}")
    raise SystemExit(0 if report["passed"] else 1)


if __name__ == "__main__":
    main()
