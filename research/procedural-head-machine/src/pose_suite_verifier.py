import argparse
import json
import math
from pathlib import Path

from skin_capture_verifier import _distance_to_segment, _percentile


def verify_pose_suite(surface, skeleton, recipe, readback, tolerance=1e-6):
    failures = []
    rest_positions = readback.get("positions", [])
    modules = readback.get("modules", [])
    if rest_positions != surface.get("vertices") or modules != surface.get("vertex_modules"):
        failures.append({"rule": "POSE-SUITE-SURFACE-1", "subject": "rest surface"})
    rest_joints = {joint["name"]: joint["position"] for joint in skeleton.get("joints", [])}
    results = {}
    available = readback.get("poses", {})
    for pose in recipe.get("diagnostic_poses", []):
        pose_id = pose["id"]
        artifact = available.get(pose_id)
        if not artifact:
            failures.append({"rule": "POSE-SUITE-COVERAGE-1", "subject": pose_id})
            continue
        positions = artifact.get("positions", [])
        posed_joints = artifact.get("skeleton_positions", {})
        if len(positions) != len(rest_positions):
            failures.append({"rule": "POSE-SUITE-COVERAGE-1", "subject": pose_id})
            continue

        active = set(pose["active_modules"])
        protected = set(pose["protected_modules"])
        active_displacement = max(
            (math.dist(rest, posed) for module, rest, posed in zip(modules, rest_positions, positions) if module in active),
            default=0.0,
        )
        protected_displacement = max(
            (math.dist(rest, posed) for module, rest, posed in zip(modules, rest_positions, positions) if module in protected),
            default=0.0,
        )
        if active_displacement < pose.get("minimum_active_displacement_m", 0.05):
            failures.append({"rule": "POSE-SUITE-ACTIVE-1", "subject": pose_id})
        if protected_displacement > tolerance:
            failures.append({"rule": "POSE-SUITE-PROTECTED-1", "subject": pose_id})
        transition_displacements = {}
        for transition in pose.get("transition_modules", []):
            displacement = max(
                (math.dist(rest, posed) for module, rest, posed in zip(modules, rest_positions, positions) if module == transition["module"]),
                default=0.0,
            )
            transition_displacements[transition["module"]] = displacement
            if displacement > transition["maximum_displacement_m"]:
                failures.append({"rule": "POSE-SUITE-TRANSITION-1", "subject": pose_id})

        evaluation = pose["evaluation"]
        names = (evaluation["parent_joint"], pose["joint"], evaluation["child_joint"])
        rest_radii, posed_radii = [], []
        if all(name in rest_joints and name in posed_joints for name in names):
            pivot = rest_joints[pose["joint"]]
            for module, rest, posed in zip(modules, rest_positions, positions):
                if module == evaluation["module"] and math.dist(rest, pivot) <= evaluation["radius_m"]:
                    rest_radii.append(min(
                        _distance_to_segment(rest, rest_joints[names[0]], rest_joints[names[1]]),
                        _distance_to_segment(rest, rest_joints[names[1]], rest_joints[names[2]]),
                    ))
                    posed_radii.append(min(
                        _distance_to_segment(posed, posed_joints[names[0]], posed_joints[names[1]]),
                        _distance_to_segment(posed, posed_joints[names[1]], posed_joints[names[2]]),
                    ))
        rest_p10 = _percentile(rest_radii, 0.10)
        posed_p10 = _percentile(posed_radii, 0.10)
        ratio = posed_p10 / rest_p10 if rest_p10 > tolerance else 0.0
        if ratio < evaluation["minimum_thickness_ratio_p10"]:
            failures.append({"rule": "POSE-SUITE-THICKNESS-1", "subject": pose_id})
        results[pose_id] = {
            "active_displacement_m": active_displacement,
            "protected_displacement_m": protected_displacement,
            "transition_displacements_m": transition_displacements,
            "sample_count": len(rest_radii),
            "rest_thickness_p10_m": rest_p10,
            "posed_thickness_p10_m": posed_p10,
            "thickness_ratio_p10": ratio,
            "minimum_thickness_ratio_p10": evaluation["minimum_thickness_ratio_p10"],
        }
    return {
        "schema": "pose-suite-verification/0",
        "passed": not failures,
        "failures": failures,
        "poses": results,
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
    recipe = json.loads(args.recipe.read_text(encoding="utf-8"))
    readback = json.loads(args.readback.read_text(encoding="utf-8"))
    report = verify_pose_suite(surface, skeleton, recipe, readback)
    args.report.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"POSE_SUITE_{'PASS' if report['passed'] else 'FAIL'} report={args.report}")
    raise SystemExit(0 if report["passed"] else 1)


if __name__ == "__main__":
    main()
