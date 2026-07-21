import argparse
import json
import math
from pathlib import Path


def _distance_to_segment(point, start, end):
    segment = [end[index] - start[index] for index in range(3)]
    offset = [point[index] - start[index] for index in range(3)]
    length_squared = sum(value * value for value in segment)
    factor = max(0.0, min(1.0, sum(offset[index] * segment[index] for index in range(3)) / length_squared))
    closest = [start[index] + factor * segment[index] for index in range(3)]
    return math.dist(point, closest)


def _percentile(values, fraction):
    ordered = sorted(values)
    return ordered[int((len(ordered) - 1) * fraction)] if ordered else 0.0


def verify_skin_capture(surface, skeleton, skeleton_sha256, recipe, readback, tolerance=1e-6):
    failures = []
    positions = readback.get("positions", [])
    max_position_delta = float("inf")
    if len(positions) == len(surface.get("vertices", [])):
        max_position_delta = max((math.dist(expected, actual) for expected, actual in zip(surface["vertices"], positions)), default=0.0)
    if not math.isfinite(max_position_delta) or max_position_delta > tolerance or readback.get("modules") != surface.get("vertex_modules"):
        failures.append({"rule": "SKIN-CAPTURE-SURFACE-1", "subject": "rest surface"})

    paths = readback.get("capture_paths", [])
    indices = readback.get("capture_indices", [])
    weights = readback.get("capture_weights", [])
    count_ok = len(indices) == len(positions) == len(weights) and len(paths) == len(skeleton.get("joints", []))
    max_sum_error = float("inf")
    max_influences = 0
    invalid_index = False
    if count_ok:
        max_sum_error = 0.0
        for point_indices, point_weights in zip(indices, weights):
            if len(point_indices) != len(point_weights):
                invalid_index = True
                continue
            invalid_index = invalid_index or any(not isinstance(index, int) or index < 0 or index >= len(paths) for index in point_indices)
            active = sum(weight > tolerance for weight in point_weights)
            max_influences = max(max_influences, active)
            max_sum_error = max(max_sum_error, abs(sum(point_weights) - 1.0))
    if not count_ok or invalid_index:
        failures.append({"rule": "SKIN-CAPTURE-COVERAGE-1", "subject": "capture arrays"})
    if not math.isfinite(max_sum_error) or max_sum_error > tolerance:
        failures.append({"rule": "SKIN-CAPTURE-NORMALIZATION-1", "subject": "weights"})
    if max_influences > recipe.get("max_influences", 0):
        failures.append({"rule": "SKIN-CAPTURE-INFLUENCE-1", "subject": "weights"})

    cross_side_weight = 0.0
    if count_ok and not invalid_index:
        for module, point_indices, point_weights in zip(readback["modules"], indices, weights):
            if not module.startswith(("left_", "right_")):
                continue
            opposite = "right_" if module.startswith("left_") else "left_"
            cross_side_weight = max(cross_side_weight, sum(weight for index, weight in zip(point_indices, point_weights) if paths[index].startswith(opposite)))
    if cross_side_weight > tolerance:
        failures.append({"rule": "SKIN-CAPTURE-SIDE-1", "subject": "left/right isolation"})

    if (
        readback.get("skeleton_sha256") != skeleton_sha256
        or readback.get("source_guide_sha256") != skeleton.get("source_guide_sha256")
        or readback.get("recipe") != recipe
    ):
        failures.append({"rule": "SKIN-CAPTURE-PROVENANCE-1", "subject": "inputs"})

    posed_positions = readback.get("posed_positions", [])
    active_displacement = 0.0
    protected_displacement = 0.0
    if len(posed_positions) == len(positions):
        for module, rest, posed in zip(readback["modules"], positions, posed_positions):
            displacement = math.dist(rest, posed)
            if module == "left_arm":
                active_displacement = max(active_displacement, displacement)
            else:
                protected_displacement = max(protected_displacement, displacement)
    if active_displacement < 0.10:
        failures.append({"rule": "SKIN-CAPTURE-POSE-ACTIVE-1", "subject": "left arm"})
    if not math.isfinite(protected_displacement) or protected_displacement > tolerance:
        failures.append({"rule": "SKIN-CAPTURE-POSE-PROTECTED-1", "subject": "unposed modules"})

    joints = {joint["name"]: joint["position"] for joint in skeleton.get("joints", [])}
    posed_joints = readback.get("posed_skeleton_positions", {})
    rest_radii, posed_radii = [], []
    elbow = joints.get("left_elbow")
    if elbow and all(name in joints and name in posed_joints for name in ("left_shoulder", "left_elbow", "left_wrist")):
        radius = recipe.get("elbow_evaluation_radius_m", 0.16)
        for module, rest, posed in zip(readback["modules"], positions, posed_positions):
            if module == "left_arm" and math.dist(rest, elbow) <= radius:
                rest_radii.append(min(
                    _distance_to_segment(rest, joints["left_shoulder"], joints["left_elbow"]),
                    _distance_to_segment(rest, joints["left_elbow"], joints["left_wrist"]),
                ))
                posed_radii.append(min(
                    _distance_to_segment(posed, posed_joints["left_shoulder"], posed_joints["left_elbow"]),
                    _distance_to_segment(posed, posed_joints["left_elbow"], posed_joints["left_wrist"]),
                ))
    rest_thickness_p10 = _percentile(rest_radii, 0.10)
    posed_thickness_p10 = _percentile(posed_radii, 0.10)
    bend_thickness_ratio = posed_thickness_p10 / rest_thickness_p10 if rest_thickness_p10 > tolerance else 0.0

    return {
        "schema": "skin-capture-verification/0",
        "passed": not failures,
        "failures": failures,
        "metrics": {
            "vertex_count": len(positions),
            "capture_region_count": len(paths),
            "max_influences": max_influences,
            "max_weight_sum_error": max_sum_error,
            "max_cross_side_weight": cross_side_weight,
            "max_position_delta_m": max_position_delta,
            "active_limb_displacement_m": active_displacement,
            "protected_module_displacement_m": protected_displacement,
            "elbow_sample_count": len(rest_radii),
            "elbow_rest_thickness_p10_m": rest_thickness_p10,
            "elbow_posed_thickness_p10_m": posed_thickness_p10,
            "elbow_thickness_ratio_p10": bend_thickness_ratio,
        },
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("surface_run", type=Path)
    parser.add_argument("skeleton_run", type=Path)
    parser.add_argument("recipe", type=Path)
    parser.add_argument("readback", type=Path)
    parser.add_argument("report", type=Path)
    args = parser.parse_args()
    surface = json.loads((args.surface_run / "readback.json").read_text(encoding="utf-8"))
    skeleton = json.loads((args.skeleton_run / "skeleton.json").read_text(encoding="utf-8"))
    skeleton_receipt = json.loads((args.skeleton_run / "receipt.json").read_text(encoding="utf-8"))
    recipe = json.loads(args.recipe.read_text(encoding="utf-8"))
    readback = json.loads(args.readback.read_text(encoding="utf-8"))
    report = verify_skin_capture(surface, skeleton, skeleton_receipt["skeleton_sha256"], recipe, readback)
    args.report.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"SKIN_CAPTURE_{'PASS' if report['passed'] else 'FAIL'} report={args.report}")
    raise SystemExit(0 if report["passed"] else 1)


if __name__ == "__main__":
    main()
