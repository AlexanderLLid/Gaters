"""Fresh-load verifier for one direct style-macro deformation."""

import argparse
import hashlib
import json
import math
import sys
from pathlib import Path

import bpy
from bpy_extras.object_utils import world_to_camera_view

ROOT = Path(__file__).resolve().parent
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from sculpt_clay import mixed_coordinates


RATIOS = ("cheekWidth", "jawWidth", "mouthWidth", "eyeToChinHeight", "lowerFaceTaper")


def arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True)
    return parser.parse_args(sys.argv[sys.argv.index("--") + 1:])


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def topology_hash(head):
    payload = {
        "vertices": len(head.data.vertices),
        "polygons": [list(polygon.vertices) for polygon in head.data.polygons],
    }
    data = json.dumps(payload, sort_keys=True, separators=(",", ":")).encode("utf-8")
    return hashlib.sha256(data).hexdigest()


def project(camera, point):
    result = world_to_camera_view(bpy.context.scene, camera, point)
    return (result.x, result.y)


def macro_ratios(head, camera, baseline):
    coordinates = [head.matrix_world @ point for point in mixed_coordinates(head)]
    anchors = {}
    for name in ("leftCheek", "rightCheek", "leftJaw", "rightJaw", "leftMouth", "rightMouth", "chin"):
        recorded = baseline["anchors"][name]
        index = recorded["index"] if "index" in recorded else recorded["indices"][0]
        anchors[name] = project(camera, coordinates[index])
    left_eye, right_eye = bpy.data.objects.get("SculptEyeL"), bpy.data.objects.get("SculptEyeR")
    anchors["leftEye"] = project(camera, left_eye.location)
    anchors["rightEye"] = project(camera, right_eye.location)
    eye_distance = abs(anchors["rightEye"][0] - anchors["leftEye"][0])
    width = lambda left, right: abs(anchors[right][0] - anchors[left][0]) / eye_distance
    cheek = width("leftCheek", "rightCheek")
    jaw = width("leftJaw", "rightJaw")
    return {
        "cheekWidth": cheek,
        "jawWidth": jaw,
        "mouthWidth": width("leftMouth", "rightMouth"),
        "eyeToChinHeight": abs((anchors["leftEye"][1] + anchors["rightEye"][1]) * 0.5 - anchors["chin"][1]) / eye_distance,
        "lowerFaceTaper": jaw / cheek,
    }


def score(measured, target):
    return sum(abs(float(measured[name]) - float(target[name])) for name in RATIOS)


def eye_opening_widths(head, camera):
    detail = json.loads((ROOT / "target-detail-geometry.json").read_text(encoding="utf-8"))
    feature = json.loads((ROOT / "target-feature-geometry.json").read_text(encoding="utf-8"))
    coordinates = [head.matrix_world @ point for point in mixed_coordinates(head)]
    indices = detail["stableVertexIndices"]["indices"]
    left_eye, right_eye = bpy.data.objects.get("SculptEyeL"), bpy.data.objects.get("SculptEyeR")
    ipd = abs(project(camera, right_eye.location)[0] - project(camera, left_eye.location)[0])
    candidate = {
        "left": abs(project(camera, coordinates[indices["leftEyeInnerCorner"]])[0]
                    - project(camera, coordinates[indices["leftEyeOuterCorner"]])[0]) / ipd,
        "right": abs(project(camera, coordinates[indices["rightEyeOuterCorner"]])[0]
                     - project(camera, coordinates[indices["rightEyeInnerCorner"]])[0]) / ipd,
    }
    points = detail["consensus"]["targetPoints"]
    target_ipd = abs(feature["points"]["rightEyeCenter"][0] - feature["points"]["leftEyeCenter"][0])
    target = {
        "left": abs(points["leftEyeInnerCorner"][0] - points["leftEyeOuterCorner"][0]) / target_ipd,
        "right": abs(points["rightEyeOuterCorner"][0] - points["rightEyeInnerCorner"][0]) / target_ipd,
    }
    return candidate, target


def eye_aperture(head, camera, baseline_macro_path):
    feature_path = Path(baseline_macro_path).parent / "feature-geometry.json"
    baseline_feature = json.loads(feature_path.read_text(encoding="utf-8"))
    target_feature = json.loads((ROOT / "target-feature-geometry.json").read_text(encoding="utf-8"))
    coordinates = [head.matrix_world @ point for point in mixed_coordinates(head)]
    anchors = baseline_feature["anchors"]
    left_eye, right_eye = bpy.data.objects.get("SculptEyeL"), bpy.data.objects.get("SculptEyeR")
    ipd = abs(project(camera, right_eye.location)[0] - project(camera, left_eye.location)[0])

    def vertical_distance(upper, lower):
        upper_y = project(camera, coordinates[anchors[upper]["index"]])[1]
        lower_y = project(camera, coordinates[anchors[lower]["index"]])[1]
        return abs(upper_y - lower_y)

    measured = (
        vertical_distance("leftUpperLid", "leftLowerLid")
        + vertical_distance("rightUpperLid", "rightLowerLid")
    ) / 2.0 / ipd
    return measured, float(target_feature["ratios"]["eyeAperture"])


def presentation_report(head):
    brows = bpy.data.objects.get("FaceProofHead.eyebrow003")
    lips = head.data.materials.get("StyleLip")
    lip_index = head.data.materials.find("StyleLip")
    visible_materials = [
        material for obj in bpy.context.scene.objects
        if obj.type == "MESH" and not obj.hide_render
        for material in obj.data.materials if material is not None
    ]
    return {
        "browsVisible": brows is not None and not brows.hide_render,
        "proceduralPupils": all(bpy.data.objects.get(f"StylePupil{side}") for side in ("L", "R")),
        "lipMaterialAssigned": lips is not None and any(
            polygon.material_index == lip_index for polygon in head.data.polygons
        ),
        "visibleMaterialsUseNoImages": not any(
            material.node_tree is not None
            and any(node.type == "TEX_IMAGE" for node in material.node_tree.nodes)
            for material in visible_materials
        ),
    }


def main():
    output = Path(arguments().output).resolve()
    blend = output / "face-style-macro.blend"
    if Path(bpy.data.filepath).resolve() != blend:
        raise RuntimeError("style macro verifier opened the wrong blend")
    manifest = json.loads((output / "manifest.json").read_text(encoding="utf-8"))
    if manifest.get("status") != "built-unverified":
        raise RuntimeError("style macro manifest status is invalid")
    for name, expected in manifest["artifacts"].items():
        if sha256(output / name) != expected:
            raise RuntimeError(f"style macro artifact hash mismatch: {name}")
    head = bpy.data.objects.get("FaceProofHead")
    camera = bpy.data.objects.get("FaceProofCamera")
    if head is None or camera is None:
        raise RuntimeError("style macro verification objects are missing")
    key = head.data.shape_keys.key_blocks.get("StyleMacroDirect") if head.data.shape_keys else None
    if key is None or abs(key.value - 1.0) > 0.000001:
        raise RuntimeError("active StyleMacroDirect shape key is missing")
    basis = head.data.shape_keys.key_blocks[0]
    profile_depth = max(abs(point.co.y - basis.data[index].co.y) for index, point in enumerate(key.data))
    finite = all(math.isfinite(value) for point in mixed_coordinates(head) for value in point)
    baseline = json.loads(Path(manifest["baselineMacro"]["path"]).read_text(encoding="utf-8"))
    target = json.loads(Path(manifest["targetGeometry"]["path"]).read_text(encoding="utf-8"))
    measured = macro_ratios(head, camera, baseline)
    baseline_score = score(baseline["ratios"], target["ratios"])
    candidate_score = score(measured, target["ratios"])
    eye_widths, target_eye_widths = eye_opening_widths(head, camera)
    aperture, target_aperture = eye_aperture(head, camera, manifest["baselineMacro"]["path"])
    topology_unchanged = topology_hash(head) == baseline["topology"]["sha256"]
    renders_valid = True
    for name in ("front.png", "three-quarter.png", "profile.png"):
        image = bpy.data.images.load(str(output / name), check_existing=False)
        renders_valid = renders_valid and tuple(image.size) == (768, 768)
        bpy.data.images.remove(image)
    gates = {
        "topology": topology_unchanged,
        "profileDepth": profile_depth <= 0.000001,
        "boundedDisplacement": manifest["maximumDisplacement"] <= 0.01,
        "finiteCoordinates": finite,
        "isolatedArtProof": not any(obj.type == "ARMATURE" for obj in bpy.context.scene.objects) and not bpy.data.actions,
        "renders": renders_valid,
        "macroImprovement": baseline_score - candidate_score >= 0.10,
    }
    promoted = all(gates.values())
    report = {
        "schemaVersion": 1,
        "decision": "promote-macro-control" if promoted else "reject",
        "gates": gates,
        "topologyUnchanged": topology_unchanged,
        "profileDepthPreserved": profile_depth <= 0.000001,
        "maximumProfileDepthChange": profile_depth,
        "maximumDisplacement": manifest["maximumDisplacement"],
        "baselineRatios": baseline["ratios"],
        "candidateRatios": measured,
        "targetRatios": target["ratios"],
        "baselineScore": baseline_score,
        "candidateScore": candidate_score,
        "improvement": baseline_score - candidate_score,
        "eyeOpeningWidths": eye_widths,
        "targetEyeOpeningWidths": target_eye_widths,
        "eyeAperture": aperture,
        "targetEyeAperture": target_aperture,
        "presentation": presentation_report(head),
        "visualDecision": "pending-human",
        "evidence": {"blendSha256": sha256(blend), "manifestSha256": sha256(output / "manifest.json"),
                     "verifierSha256": sha256(Path(__file__).resolve())},
    }
    (output / "verification.json").write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if not promoted:
        raise RuntimeError(json.dumps(report, sort_keys=True))
    print(f"STYLE_MACRO_VERIFY_OK improvement={report['improvement']:.8f} output={output}")


if __name__ == "__main__":
    main()
