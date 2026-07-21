"""Fresh-load verifier for complete front head-profile fitting."""

import argparse
import hashlib
import json
import math
import sys
from pathlib import Path

import bpy

ROOT = Path(__file__).resolve().parent
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from build_head_silhouette import body_indices, profile_samples, topology_hash
from head_silhouette_fit import regional_profile_errors, smooth_profile, visual_review_decision
from sculpt_clay import mixed_coordinates


def arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True)
    return parser.parse_args(sys.argv[sys.argv.index("--") + 1:])


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def score(points, head, target, parameters, center_x, eye_z, ipd, excluded_indices):
    indices = body_indices(head) - excluded_indices
    chin_index = int(parameters["chinVertexIndex"])
    top = max((points[index].z - eye_z) / ipd for index in indices)
    chin = (points[chin_index].z - eye_z) / ipd
    minimum_height = float(parameters.get("minimumValidOuterHeight", -1.0))
    samples = profile_samples(
        points, indices, center_x, eye_z, ipd, float(parameters["profileBinSize"]),
        minimum_height, max(top, float(target["targetTop"])),
    )
    target_samples = sorted(
        (float(sample["height"]), float(sample["halfWidth"]))
        for sample in target["samples"]
    )
    smoothing = float(parameters.get("smoothingRadius", 0.0))
    samples = smooth_profile(samples, smoothing)
    target_samples = smooth_profile(target_samples, smoothing)
    regions = {"skull": (0.45, max(top, float(target["targetTop"])))}
    if minimum_height < 0.45:
        regions["temples"] = (minimum_height, 0.45)
    if minimum_height < -0.35:
        regions["faceSides"] = (minimum_height, -0.35)
    regional = regional_profile_errors(samples, target_samples, regions)
    total = sum(regional.values()) + abs(top - float(target["targetTop"]))
    return total, {"top": top, "chinDiagnosticOnly": chin, "regionalErrors": regional}


def main():
    output = Path(arguments().output).resolve()
    blend = output / "face-head-silhouette.blend"
    if Path(bpy.data.filepath).resolve() != blend:
        raise RuntimeError("head silhouette verifier opened the wrong blend")
    manifest = json.loads((output / "manifest.json").read_text(encoding="utf-8"))
    for name, expected in manifest["artifacts"].items():
        if sha256(output / name) != expected:
            raise RuntimeError(f"head silhouette artifact hash mismatch: {name}")
    head = bpy.data.objects.get("FaceProofHead")
    left_eye, right_eye = bpy.data.objects.get("SculptEyeL"), bpy.data.objects.get("SculptEyeR")
    key = head.data.shape_keys.key_blocks.get("HeadSilhouetteFit") if head and head.data.shape_keys else None
    if key is None or abs(key.value - 1.0) > 0.000001:
        raise RuntimeError("active HeadSilhouetteFit shape key is missing")
    parameters = manifest["recipe"]["parameters"]
    target = json.loads(Path(manifest["targetProfile"]["path"]).read_text(encoding="utf-8"))
    center_x = (left_eye.location.x + right_eye.location.x) / 2.0
    eye_z = (left_eye.location.z + right_eye.location.z) / 2.0
    ipd = abs(right_eye.location.x - left_eye.location.x)
    key.value = 0.0
    bpy.context.view_layer.update()
    baseline = mixed_coordinates(head)
    key.value = 1.0
    bpy.context.view_layer.update()
    fitted = mixed_coordinates(head)
    excluded_indices = set(manifest.get("excludedEarIndices", []))
    baseline_score, baseline_parts = score(baseline, head, target, parameters, center_x, eye_z, ipd, excluded_indices)
    fitted_score, fitted_parts = score(fitted, head, target, parameters, center_x, eye_z, ipd, excluded_indices)
    maximum_depth = max(abs(after.y - before.y) for before, after in zip(baseline, fitted))
    topology_unchanged = topology_hash(head) == manifest["sourceTopology"]["sha256"]
    finite = all(math.isfinite(value) for point in fitted for value in point)
    regional_improvement = all(
        fitted_parts["regionalErrors"][name] <= baseline_parts["regionalErrors"][name]
        for name in fitted_parts["regionalErrors"]
    )
    gates = {
        "validOuterProfileImprovement": fitted_score <= baseline_score * 0.85,
        "everyRegionImproves": regional_improvement,
        "topology": topology_unchanged,
        "depthPreserved": maximum_depth <= 0.000001,
        "boundedDisplacement": manifest["maximumDisplacement"] <= 0.025,
        "finiteCoordinates": finite,
        "neckContinuous": manifest["recipe"]["parameters"]["neckFade"] >= 0.4,
    }
    mechanical_pass = all(gates.values())
    review_path = output / "head-shape-review.json"
    visual_decision = "required"
    if review_path.is_file():
        review = json.loads(review_path.read_text(encoding="utf-8"))
        if review.get("reviewType") != "gaters.head-shape-review.v1":
            raise RuntimeError("unsupported head shape review")
        for name in ("a-target-head-aligned.png", "b-generated-head-aligned.png",
                     "c-center-wipe.png", "d-aligned-blink.gif"):
            if not (output / name).is_file() or sha256(output / name) != review.get("evidence", {}).get(name):
                raise RuntimeError(f"head shape review evidence mismatch: {name}")
        visual_decision = visual_review_decision(review.get("regions", {}))
    promotion_eligible = mechanical_pass and visual_decision == "pass"
    if promotion_eligible:
        decision = "promote-head-shape"
    elif mechanical_pass and visual_decision == "reject":
        decision = "visual-reject"
    elif mechanical_pass:
        decision = "mechanical-pass-awaiting-visual-review"
    else:
        decision = "reject"
    report = {
        "schemaVersion": 1,
        "decision": decision,
        "promotionEligible": promotion_eligible,
        "gates": gates,
        "baselineScore": baseline_score,
        "fittedScore": fitted_score,
        "improvementFraction": 1.0 - fitted_score / baseline_score,
        "baseline": baseline_parts,
        "fitted": fitted_parts,
        "regionalErrors": {
            "baseline": baseline_parts["regionalErrors"],
            "fitted": fitted_parts["regionalErrors"],
        },
        "automaticScoreCoverage": {
            "minimumNormalizedHeight": float(parameters.get("minimumValidOuterHeight", -1.0)),
            "included": list(fitted_parts["regionalErrors"]),
            "excluded": "ears-jaw-chin-neck",
        },
        "target": {"top": target["targetTop"], "chin": target["targetChin"]},
        "maximumDepthChange": maximum_depth,
        "maximumDisplacement": manifest["maximumDisplacement"],
        "topologyUnchanged": topology_unchanged,
        "visualDecision": visual_decision,
        "evidence": {"blendSha256": sha256(blend), "manifestSha256": sha256(output / "manifest.json"),
                     "verifierSha256": sha256(Path(__file__).resolve())},
    }
    (output / "verification.json").write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if not mechanical_pass:
        raise RuntimeError(json.dumps(report, sort_keys=True))
    print(f"HEAD_SILHOUETTE_VERIFIED decision={decision} improvement={report['improvementFraction']:.6f} output={output}")


if __name__ == "__main__":
    main()
