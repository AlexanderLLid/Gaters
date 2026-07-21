import argparse
import json
import math
from pathlib import Path

from smooth_body_verifier import verify_smooth_body


def measure_chain_radii(surface, guide, module, landmarks, axis, band):
    radial_axes = [index for index in range(3) if index != axis]
    result = {}
    for name in landmarks:
        center = guide["landmarks"][name]
        samples = [
            vertex for vertex, label in zip(surface.get("vertices", []), surface.get("vertex_modules", []))
            if label == module and abs(vertex[axis] - center[axis]) <= band
        ]
        result[name] = max(
            (math.sqrt(sum((vertex[index] - center[index]) ** 2 for index in radial_axes)) for vertex in samples),
            default=float("inf"),
        )
    return result


def _measure_radius_at(surface, module, center, axis, band):
    radial_axes = [index for index in range(3) if index != axis]
    samples = [
        vertex for vertex, label in zip(surface.get("vertices", []), surface.get("vertex_modules", []))
        if label == module and abs(vertex[axis] - center[axis]) <= band
    ]
    return max(
        (math.sqrt(sum((vertex[index] - center[index]) ** 2 for index in radial_axes)) for vertex in samples),
        default=float("inf"),
    )


def verify_anatomical_surface(body, guide, recipe, surface):
    base = verify_smooth_body(body, recipe, surface)
    failures = list(base["failures"])
    if surface.get("guide_metadata") != guide:
        failures.append({"rule": "ANATOMICAL-SURFACE-PROVENANCE-1", "subject": "guide"})

    band = recipe["voxel_size_m"] * 2.5
    measured = {}
    tapers = []
    for side in ("left", "right"):
        arm = measure_chain_radii(surface, guide, f"{side}_arm", (f"{side}_elbow", f"{side}_wrist"), 0, band)
        knee_name, ankle_name = f"{side}_knee", f"{side}_ankle"
        knee, ankle = guide["landmarks"][knee_name], guide["landmarks"][ankle_name]
        calf_low = [knee[index] + 0.65 * (ankle[index] - knee[index]) for index in range(3)]
        leg = {
            knee_name: _measure_radius_at(surface, f"{side}_leg", knee, 2, band),
            f"{side}_calf_low": _measure_radius_at(surface, f"{side}_leg", calf_low, 2, band),
        }
        measured.update(arm)
        measured.update(leg)
        tapers.extend([
            arm[f"{side}_elbow"] - arm[f"{side}_wrist"],
            leg[knee_name] - leg[f"{side}_calf_low"],
        ])
    minimum_taper = min(tapers, default=float("-inf"))
    if not all(math.isfinite(value) for value in measured.values()) or minimum_taper < recipe["minimum_distal_taper_m"]:
        failures.append({"rule": "ANATOMICAL-SURFACE-TAPER-1", "subject": "limbs"})

    return {
        "schema": "anatomical-surface-verification/0",
        "passed": not failures,
        "failures": failures,
        "metrics": {
            **base["metrics"],
            "minimum_distal_taper_m": minimum_taper,
            "measured_joint_radii_m": measured,
        },
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("body_run", type=Path)
    parser.add_argument("guide_run", type=Path)
    parser.add_argument("recipe", type=Path)
    parser.add_argument("readback", type=Path)
    parser.add_argument("report", type=Path)
    args = parser.parse_args()
    body = json.loads((args.body_run / "composed-mesh.json").read_text(encoding="utf-8"))
    guide = json.loads((args.guide_run / "guide.json").read_text(encoding="utf-8"))
    recipe = json.loads(args.recipe.read_text(encoding="utf-8"))
    surface = json.loads(args.readback.read_text(encoding="utf-8"))
    report = verify_anatomical_surface(body, guide, recipe, surface)
    args.report.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"ANATOMICAL_SURFACE_{'PASS' if report['passed'] else 'FAIL'} report={args.report}")
    raise SystemExit(0 if report["passed"] else 1)


if __name__ == "__main__":
    main()
