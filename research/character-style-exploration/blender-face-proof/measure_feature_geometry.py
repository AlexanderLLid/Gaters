"""Camera-space semantic feature receipt for one Task 8 candidate."""

import argparse
import gzip
import hashlib
import json
import sys
from pathlib import Path

import bpy
from bpy_extras.object_utils import world_to_camera_view
from mathutils import Vector

ROOT = Path(__file__).resolve().parent
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from feature_grid import body_signed_extrema, validate_feature_metric
from sculpt_integrity import validate_manifest_integrity
from sculpt_features import scene_lock


TARGETS = {
    "leftEye": ("eyes", "l-eye-height1-incr.target.gz"),
    "rightEye": ("eyes", "r-eye-height1-incr.target.gz"),
    "noseLength": ("nose", "nose-scale-vert-incr.target.gz"),
    "alare": ("nose", "nose-nostrils-width-incr.target.gz"),
}


def canonical(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"))


def sha256(value):
    return hashlib.sha256(value).hexdigest()


def arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True)
    return parser.parse_args(sys.argv[sys.argv.index("--") + 1:])


def records(path):
    with gzip.open(path, "rt", encoding="utf-8") as stream:
        return [tuple([int(parts[0])] + [float(value) for value in parts[1:]])
                for line in stream if (parts := line.split()) and not line.startswith("#")]


def topology_hash(head):
    payload = {"vertices": len(head.data.vertices), "polygons": [list(poly.vertices) for poly in head.data.polygons]}
    return sha256(canonical(payload).encode("utf-8")), payload["vertices"]


def camera_lock(camera):
    payload = {
        "camera": {"type": camera.data.type, "orthoScale": camera.data.ortho_scale, "lens": camera.data.lens,
                   "location": list(camera.location), "rotation": list(camera.rotation_euler)},
        "render": [bpy.context.scene.render.resolution_x, bpy.context.scene.render.resolution_y,
                   bpy.context.scene.render.engine, bpy.context.scene.view_settings.view_transform, bpy.context.scene.view_settings.look],
    }
    return sha256(canonical(payload).encode("utf-8")), payload


def project(camera, point):
    result = world_to_camera_view(bpy.context.scene, camera, Vector(point))
    return [result.x, result.y, result.z]


def anchor(coordinates, camera, index, role, source):
    point = coordinates[index]
    return {"role": role, "source": source, "index": index, "world": list(point), "camera": project(camera, point)}


def mixed_coordinates(head):
    keys = head.data.shape_keys
    basis = keys.key_blocks[0]
    result = [point.co.copy() for point in basis.data]
    for key in keys.key_blocks[1:]:
        if abs(key.value) > 0.000001:
            for index, point in enumerate(key.data):
                result[index] += (point.co - basis.data[index].co) * key.value
    return [head.matrix_world @ point for point in result]


def main():
    output = Path(arguments().output).resolve()
    recipe = validate_manifest_integrity(output)
    manifest = json.loads((output / "manifest.json").read_text(encoding="utf-8"))
    if bpy.data.filepath != str(output / "face-sculpt.blend"):
        raise RuntimeError("feature metric must run from the freshly reopened candidate blend")
    head, camera = bpy.data.objects.get("FaceProofHead"), bpy.data.objects.get("FaceProofCamera")
    left_eye, right_eye = bpy.data.objects.get("SculptEyeL"), bpy.data.objects.get("SculptEyeR")
    if not all((head, camera, left_eye, right_eye)):
        raise RuntimeError("feature metric source objects are missing")
    if scene_lock(head, camera) != manifest["sceneLock"]:
        raise RuntimeError("fresh reopened scene lock does not match manifest")
    from bl_ext.blender_org.mpfb.services import LocationService
    target_root = Path(LocationService.get_mpfb_data("targets"))
    source_records = {name: records(target_root / directory / filename) for name, (directory, filename) in TARGETS.items()}
    coordinates = mixed_coordinates(head)
    if any(index >= len(coordinates) for items in source_records.values() for index, *_ in items):
        raise RuntimeError("stable MPFB target vertices do not match candidate topology")
    body_group = head.vertex_groups.get("body")
    body_indices = {vertex.index for vertex in head.data.vertices if body_group and any(link.group == body_group.index and link.weight > 0.5 for link in vertex.groups)}
    if not body_indices:
        raise RuntimeError("head body vertex group is missing")
    try:
        left_up, left_down = body_signed_extrema(source_records["leftEye"], body_indices, 2)
        right_up, right_down = body_signed_extrema(source_records["rightEye"], body_indices, 2)
        nasion, subnasale = body_signed_extrema(source_records["noseLength"], body_indices, 2)
        right_alare, left_alare = body_signed_extrema(source_records["alare"], body_indices, 1)
    except ValueError as error:
        raise RuntimeError(str(error)) from error
    anchors = {
        "leftUpperLid": anchor(coordinates, camera, left_up, "upper lid margin", "l-eye-height1-incr signed vertical maximum"),
        "leftLowerLid": anchor(coordinates, camera, left_down, "lower lid margin", "l-eye-height1-incr signed vertical minimum"),
        "rightUpperLid": anchor(coordinates, camera, right_up, "upper lid margin", "r-eye-height1-incr signed vertical maximum"),
        "rightLowerLid": anchor(coordinates, camera, right_down, "lower lid margin", "r-eye-height1-incr signed vertical minimum"),
        "nasion": anchor(coordinates, camera, nasion, "nose root", "nose-scale-vert-incr signed vertical maximum"),
        "subnasale": anchor(coordinates, camera, subnasale, "nose base", "nose-scale-vert-incr signed vertical minimum"),
        "leftAlare": anchor(coordinates, camera, left_alare, "left alare", "nose-nostrils-width-incr signed horizontal minimum"),
        "rightAlare": anchor(coordinates, camera, right_alare, "right alare", "nose-nostrils-width-incr signed horizontal maximum"),
        "leftEyeCenter": {"role": "left eye center", "source": "SculptEyeL center", "world": list(left_eye.location), "camera": project(camera, left_eye.location)},
        "rightEyeCenter": {"role": "right eye center", "source": "SculptEyeR center", "world": list(right_eye.location), "camera": project(camera, right_eye.location)},
    }
    eye_distance = abs(anchors["rightEyeCenter"]["camera"][0] - anchors["leftEyeCenter"]["camera"][0])
    if eye_distance <= 0:
        raise RuntimeError("eye center distance is invalid")
    target = json.loads((ROOT / "target-feature-geometry.json").read_text(encoding="utf-8"))
    camera_hash, lock = camera_lock(camera)
    scene_lock_record = manifest["sceneLock"]
    topo_hash, vertex_count = topology_hash(head)
    ratios = {
        "eyeAperture": ((abs(anchors["leftUpperLid"]["camera"][1] - anchors["leftLowerLid"]["camera"][1]) + abs(anchors["rightUpperLid"]["camera"][1] - anchors["rightLowerLid"]["camera"][1])) / 2) / eye_distance,
        "noseLength": abs(anchors["nasion"]["camera"][1] - anchors["subnasale"]["camera"][1]) / eye_distance,
        "alarWidth": abs(anchors["rightAlare"]["camera"][0] - anchors["leftAlare"]["camera"][0]) / eye_distance,
    }
    record = {"schemaVersion": 1, "candidateId": recipe["candidateId"], "targetImage": target["image"],
              "targetAnnotation": {"path": "target-feature-geometry.json", "sha256": sha256((ROOT / "target-feature-geometry.json").read_bytes())},
              "camera": {"sha256": camera_hash, "settings": lock["camera"]}, "sceneLock": scene_lock_record,
              "topology": {"vertexCount": vertex_count, "sha256": topo_hash}, "anchors": anchors,
              "targetFiles": {name: {"path": f"{directory}/{filename}", "sha256": sha256((target_root / directory / filename).read_bytes()),
                                           "recordCount": len(source_records[name]), "eligibleBodyVertexCount": sum(record[0] in body_indices for record in source_records[name])}
                              for name, (directory, filename) in TARGETS.items()}, "eligibleBodyVertexCount": len(body_indices),
              "ratios": ratios, "scoreInputs": list(ratios),
              "method": "Fixed signed extrema from paired MPFB eye-height, nose-scale-vertical, and nostril-width targets, projected through the saved front camera; no raster appearance is scored."}
    validate_feature_metric(target, record, camera_hash, scene_lock_record["sha256"], vertex_count, topo_hash)
    path = output / "feature-geometry.json"
    path.write_text(json.dumps(record, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"FEATURE_GEOMETRY_OK candidate={recipe['candidateId']} metrics={path}")


if __name__ == "__main__":
    main()
