import argparse
import json
import math
from pathlib import Path


def _dot(a, b):
    return sum(left * right for left, right in zip(a, b))


def _cross(a, b):
    return [
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0],
    ]


def verify_guide_skeleton(guide, recipe, skeleton, source_guide_sha256, tolerance=1e-8):
    failures = []
    joints = skeleton.get("joints", [])
    by_name = {joint.get("name"): joint for joint in joints}
    required = set(recipe.get("required_joints", []))
    if set(by_name) != required or len(joints) != len(required):
        failures.append({"rule": "GUIDE-SKELETON-JOINTS-1", "subject": "joints"})

    expected_parents = {recipe.get("root_joint"): None}
    for segment in guide.get("skeleton_segments", []):
        if segment.get("start") in required and segment.get("end") in required:
            expected_parents[segment["end"]] = segment["start"]
    hierarchy_ok = set(expected_parents) == required and all(
        by_name.get(name, {}).get("parent") == parent for name, parent in expected_parents.items()
    )
    if not hierarchy_ok:
        failures.append({"rule": "GUIDE-SKELETON-HIERARCHY-1", "subject": "parents"})

    max_position_delta = 0.0
    positions_ok = True
    for name in required:
        try:
            delta = math.dist(by_name[name]["position"], guide["landmarks"][name])
            max_position_delta = max(max_position_delta, delta)
            positions_ok = positions_ok and math.isfinite(delta) and delta <= tolerance
        except (KeyError, TypeError, ValueError):
            positions_ok = False
            max_position_delta = float("inf")
    if not positions_ok:
        failures.append({"rule": "GUIDE-SKELETON-POSITION-1", "subject": "joint positions"})

    max_frame_error = 0.0
    frames_ok = True
    for joint in joints:
        try:
            aim, up, side = (joint["basis"][name] for name in ("aim", "up", "side"))
            errors = [
                abs(_dot(aim, aim) - 1), abs(_dot(up, up) - 1), abs(_dot(side, side) - 1),
                abs(_dot(aim, up)), abs(_dot(aim, side)), abs(_dot(up, side)),
                math.dist(_cross(aim, up), side),
            ]
            max_frame_error = max(max_frame_error, *errors)
            frames_ok = frames_ok and all(math.isfinite(value) and value <= tolerance for value in errors)
        except (KeyError, TypeError, ValueError):
            frames_ok = False
            max_frame_error = float("inf")
    if not frames_ok:
        failures.append({"rule": "GUIDE-SKELETON-FRAME-1", "subject": "joint frames"})

    if skeleton.get("source_guide_sha256") != source_guide_sha256 or skeleton.get("guide_id") != guide.get("id"):
        failures.append({"rule": "GUIDE-SKELETON-PROVENANCE-1", "subject": "guide"})

    return {
        "schema": "guide-skeleton-verification/0",
        "passed": not failures,
        "failures": failures,
        "metrics": {
            "joint_count": len(joints),
            "bone_count": len(skeleton.get("bones", [])),
            "max_position_delta_m": max_position_delta,
            "max_frame_error": max_frame_error,
        },
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("guide", type=Path)
    parser.add_argument("recipe", type=Path)
    parser.add_argument("skeleton", type=Path)
    parser.add_argument("source_guide_sha256")
    parser.add_argument("report", type=Path)
    args = parser.parse_args()
    guide = json.loads(args.guide.read_text(encoding="utf-8"))
    recipe = json.loads(args.recipe.read_text(encoding="utf-8"))
    skeleton = json.loads(args.skeleton.read_text(encoding="utf-8"))
    report = verify_guide_skeleton(guide, recipe, skeleton, args.source_guide_sha256)
    args.report.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"GUIDE_SKELETON_{'PASS' if report['passed'] else 'FAIL'} report={args.report}")
    raise SystemExit(0 if report["passed"] else 1)


if __name__ == "__main__":
    main()
