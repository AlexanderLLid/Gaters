import argparse
import json
import math
from pathlib import Path


def verify_guide_skeleton_adapter(skeleton, readback, skeleton_sha256, tolerance=1e-7):
    failures = []
    joints = skeleton.get("joints", [])
    expected_names = [joint["name"] for joint in joints]
    expected_parents = [joint["parent"] or "" for joint in joints]
    if readback.get("joint_names") != expected_names:
        failures.append({"rule": "GUIDE-SKELETON-ADAPTER-NAMES-1", "subject": "joint names"})

    positions = readback.get("positions", [])
    max_position_delta = float("inf")
    if len(positions) == len(joints):
        max_position_delta = max((math.dist(joint["position"], position) for joint, position in zip(joints, positions)), default=0.0)
    if not math.isfinite(max_position_delta) or max_position_delta > tolerance:
        failures.append({"rule": "GUIDE-SKELETON-ADAPTER-POSITION-1", "subject": "joint positions"})

    expected_transforms = [joint["basis"]["aim"] + joint["basis"]["up"] + joint["basis"]["side"] for joint in joints]
    transforms = readback.get("transforms", [])
    max_transform_delta = float("inf")
    if len(transforms) == len(expected_transforms) and all(len(value) == 9 for value in transforms):
        max_transform_delta = max(
            (abs(expected - actual) for expected_matrix, actual_matrix in zip(expected_transforms, transforms) for expected, actual in zip(expected_matrix, actual_matrix)),
            default=0.0,
        )
    if not math.isfinite(max_transform_delta) or max_transform_delta > tolerance:
        failures.append({"rule": "GUIDE-SKELETON-ADAPTER-TRANSFORM-1", "subject": "joint transforms"})

    expected_edges = [[bone["parent"], bone["child"]] for bone in skeleton.get("bones", [])]
    if readback.get("parents") != expected_parents or readback.get("edges") != expected_edges:
        failures.append({"rule": "GUIDE-SKELETON-ADAPTER-HIERARCHY-1", "subject": "joint hierarchy"})
    if readback.get("source_guide_sha256") != skeleton.get("source_guide_sha256") or readback.get("skeleton_sha256") != skeleton_sha256:
        failures.append({"rule": "GUIDE-SKELETON-ADAPTER-PROVENANCE-1", "subject": "source hashes"})

    return {
        "schema": "guide-skeleton-adapter-verification/0",
        "passed": not failures,
        "failures": failures,
        "metrics": {
            "joint_count": len(readback.get("joint_names", [])),
            "bone_count": len(readback.get("edges", [])),
            "max_position_delta_m": max_position_delta,
            "max_transform_delta": max_transform_delta,
        },
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("skeleton_run", type=Path)
    parser.add_argument("readback", type=Path)
    parser.add_argument("report", type=Path)
    args = parser.parse_args()
    skeleton = json.loads((args.skeleton_run / "skeleton.json").read_text(encoding="utf-8"))
    receipt = json.loads((args.skeleton_run / "receipt.json").read_text(encoding="utf-8"))
    readback = json.loads(args.readback.read_text(encoding="utf-8"))
    report = verify_guide_skeleton_adapter(skeleton, readback, receipt["skeleton_sha256"])
    args.report.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"GUIDE_SKELETON_ADAPTER_{'PASS' if report['passed'] else 'FAIL'} report={args.report}")
    raise SystemExit(0 if report["passed"] else 1)


if __name__ == "__main__":
    main()
