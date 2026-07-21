"""Fit the complete front head profile while preserving source depth."""

import argparse
import gzip
import hashlib
import json
import sys
from pathlib import Path

import bpy
from bpy_extras.object_utils import world_to_camera_view

ROOT = Path(__file__).resolve().parent
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from head_silhouette_fit import deform_head_point, interpolate_profile, smooth_profile
from sculpt_clay import mixed_coordinates, render_views


def arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--recipe", required=True)
    parser.add_argument("--output", required=True)
    return parser.parse_args(sys.argv[sys.argv.index("--") + 1:])


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def topology_hash(head):
    payload = json.dumps(
        [list(polygon.vertices) for polygon in head.data.polygons], separators=(",", ":")
    ).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def body_indices(head):
    group = head.vertex_groups.get("body")
    if group is None:
        raise RuntimeError("head body vertex group is missing")
    return {
        vertex.index for vertex in head.data.vertices
        if any(link.group == group.index and link.weight > 0.5 for link in vertex.groups)
    }


def target_indices(path):
    with gzip.open(path, "rt", encoding="utf-8") as stream:
        return {
            int(parts[0]) for line in stream
            if (parts := line.split()) and not line.startswith("#")
        }


def profile_samples(points, indices, center_x, eye_z, ipd, bin_size, bottom, top):
    bins = {}
    for index in indices:
        point = points[index]
        height = (point.z - eye_z) / ipd
        if not bottom <= height <= top:
            continue
        bucket = round(height / bin_size)
        width = abs(point.x - center_x) / ipd
        bins[bucket] = max(bins.get(bucket, 0.0), width)
    return sorted((bucket * bin_size, width) for bucket, width in bins.items())


def main():
    args = arguments()
    recipe_path = Path(args.recipe).resolve()
    recipe = json.loads(recipe_path.read_text(encoding="utf-8"))
    if set(recipe) != {"schemaVersion", "controlSet", "sourceBlend", "targetProfile", "parameters"}:
        raise ValueError("head silhouette recipe fields changed")
    if recipe["schemaVersion"] != 1 or recipe["controlSet"] != "gaters.head-silhouette-fit.v1":
        raise ValueError("unsupported head silhouette recipe")
    source = (ROOT / recipe["sourceBlend"]).resolve()
    target_path = (ROOT / recipe["targetProfile"]).resolve()
    if Path(bpy.data.filepath).resolve() != source:
        raise RuntimeError("head silhouette builder opened the wrong source blend")
    output = Path(args.output).resolve()
    if output.exists() and any(output.iterdir()):
        raise FileExistsError(f"refusing to overwrite head silhouette evidence: {output}")
    output.mkdir(parents=True, exist_ok=True)

    head = bpy.data.objects.get("FaceProofHead")
    camera = bpy.data.objects.get("FaceProofCamera")
    left_eye, right_eye = bpy.data.objects.get("SculptEyeL"), bpy.data.objects.get("SculptEyeR")
    if not all((head, camera, left_eye, right_eye)):
        raise RuntimeError("head silhouette source objects are missing")
    parameters = recipe["parameters"]
    required_parameters = {"chinVertexIndex", "sidePower", "neckFade", "profileBinSize"}
    optional_parameters = {"smoothingRadius", "minimumValidOuterHeight", "validOuterBlend",
                           "minimumVerticalHeight", "verticalBlend", "chinHeightIncrease",
                           "chinWidthDecrease"}
    if not required_parameters <= set(parameters) or set(parameters) - required_parameters - optional_parameters:
        raise ValueError("head silhouette parameters changed")
    if head.data.shape_keys.key_blocks.get("HeadSilhouetteFit") is not None:
        raise RuntimeError("HeadSilhouetteFit already exists")

    from bl_ext.blender_org.mpfb.services import LocationService, TargetService
    target_root = Path(LocationService.get_mpfb_data("targets"))
    applied_targets = {}
    for parameter, filename, key_name in (
        ("chinHeightIncrease", "chin-height-incr.target.gz", "HeadAlignChinHeight"),
        ("chinWidthDecrease", "chin-width-decr.target.gz", "HeadAlignChinWidth"),
    ):
        weight = float(parameters.get(parameter, 0.0))
        if weight:
            path = target_root / "chin" / filename
            TargetService.load_target(head, str(path), weight=weight, name=key_name)
            applied_targets[parameter] = {"weight": weight, "path": str(path), "sha256": sha256(path)}

    target = json.loads(target_path.read_text(encoding="utf-8"))
    target_samples = sorted(
        (float(sample["height"]), float(sample["halfWidth"]))
        for sample in target["samples"]
    )
    points = mixed_coordinates(head)
    indices = body_indices(head)
    ear_indices = set().union(*(
        target_indices(path) for path in (target_root / "ears").glob("*.target.gz")
    ))
    if not ear_indices or max(ear_indices) >= len(points):
        raise RuntimeError("stable MPFB ear vertices do not match candidate topology")
    deformation_indices = indices - ear_indices
    center_x = (left_eye.location.x + right_eye.location.x) / 2.0
    eye_z = (left_eye.location.z + right_eye.location.z) / 2.0
    ipd = abs(right_eye.location.x - left_eye.location.x)
    chin_index = int(parameters["chinVertexIndex"])
    if chin_index not in indices:
        raise RuntimeError("semantic chin vertex is outside the head body")
    current_top = max((points[index].z - eye_z) / ipd for index in indices)
    current_chin = (points[chin_index].z - eye_z) / ipd
    source_samples = profile_samples(
        points, deformation_indices, center_x, eye_z, ipd, float(parameters["profileBinSize"]),
        current_chin, current_top,
    )
    smoothing = float(parameters.get("smoothingRadius", 0.0))
    source_samples = smooth_profile(source_samples, smoothing)
    target_samples = smooth_profile(target_samples, smoothing)
    current_width = lambda height: max(0.02, interpolate_profile(source_samples, height))
    target_width = lambda height: max(0.02, interpolate_profile(target_samples, height))

    fitted = [point.copy() for point in points]
    maximum = 0.0
    maximum_depth = 0.0
    for index in deformation_indices:
        changed = deform_head_point(
            tuple(points[index]), center_x=center_x, eye_z=eye_z, ipd=ipd,
            current_top=current_top, current_chin=current_chin,
            target_top=float(target["targetTop"]), target_chin=float(target["targetChin"]),
            current_half_width=current_width, target_half_width=target_width,
            side_power=float(parameters["sidePower"]), neck_fade=float(parameters["neckFade"]),
            minimum_valid_outer_height=(float(parameters["minimumValidOuterHeight"])
                                        if "minimumValidOuterHeight" in parameters else None),
            valid_outer_blend=float(parameters.get("validOuterBlend", 0.0)),
            minimum_vertical_height=(float(parameters["minimumVerticalHeight"])
                                     if "minimumVerticalHeight" in parameters else None),
            vertical_blend=float(parameters.get("verticalBlend", 0.0)),
        )
        fitted[index][:] = changed
        delta = fitted[index] - points[index]
        maximum = max(maximum, delta.length)
        maximum_depth = max(maximum_depth, abs(delta.y))

    key = head.shape_key_add(name="HeadSilhouetteFit", from_mix=False)
    basis = head.data.shape_keys.key_blocks[0]
    for index, (before, after) in enumerate(zip(points, fitted)):
        key.data[index].co = basis.data[index].co + (after - before)
    key.value = 1.0
    head["head_silhouette_recipe"] = json.dumps(parameters, sort_keys=True)

    blend = output / "face-head-silhouette.blend"
    bpy.ops.wm.save_as_mainfile(filepath=str(blend))
    render_views(camera, (center_x, -0.01, camera.location.z), center_x, output)
    bpy.ops.wm.save_as_mainfile(filepath=str(blend))
    width, height = bpy.context.scene.render.resolution_x, bpy.context.scene.render.resolution_y
    def eye_pixels(eye):
        point = world_to_camera_view(bpy.context.scene, camera, eye.location)
        return [point.x * width, (1.0 - point.y) * height]
    left_pixels, right_pixels = eye_pixels(left_eye), eye_pixels(right_eye)
    alignment = output / "front-alignment.json"
    alignment.write_text(json.dumps({
        "schemaVersion": 1,
        "imageSize": [width, height],
        "leftEye": left_pixels,
        "rightEye": right_pixels,
        "eyeCenter": [(left_pixels[0] + right_pixels[0]) / 2.0,
                      (left_pixels[1] + right_pixels[1]) / 2.0],
        "ipdPixels": abs(right_pixels[0] - left_pixels[0]),
    }, indent=2) + "\n", encoding="utf-8")
    artifacts = [blend, alignment, *(output / name for name in ("front.png", "three-quarter.png", "profile.png"))]
    manifest = {
        "schemaVersion": 1,
        "status": "built-unverified",
        "shapeKey": key.name,
        "sourceBlend": {"path": str(source), "sha256": sha256(source)},
        "targetProfile": {"path": str(target_path), "sha256": sha256(target_path)},
        "recipe": {"path": str(recipe_path), "sha256": sha256(recipe_path), "parameters": parameters},
        "semanticAnchors": {"chinVertexIndex": chin_index, "currentTop": current_top, "currentChin": current_chin},
        "sourceTopology": {"vertexCount": len(head.data.vertices), "faceCount": len(head.data.polygons), "sha256": topology_hash(head)},
        "maximumDisplacement": maximum,
        "maximumDepthChange": maximum_depth,
        "excludedEarVertexCount": len(ear_indices),
        "excludedEarIndices": sorted(ear_indices),
        "maximumEarDisplacement": max((fitted[index] - points[index]).length for index in ear_indices),
        "appliedMpfbTargets": applied_targets,
        "artifacts": {path.name: sha256(path) for path in artifacts},
        "toolVersions": {"blender": bpy.app.version_string, "python": sys.version.split()[0],
                         "builderSha256": sha256(Path(__file__).resolve()),
                         "fitterSha256": sha256(ROOT / "head_silhouette_fit.py")},
    }
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"HEAD_SILHOUETTE_BUILD_OK output={output} maxDisplacement={maximum:.8f}")


if __name__ == "__main__":
    main()
