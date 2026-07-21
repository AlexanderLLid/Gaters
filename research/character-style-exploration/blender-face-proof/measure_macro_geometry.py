"""Blender-side, camera-space geometry receipt for one Task 7 candidate."""

import argparse
import gzip
import hashlib
import json
import math
import sys
from pathlib import Path

import bpy
from bpy_extras.object_utils import world_to_camera_view
from mathutils import Vector


ROOT = Path(__file__).resolve().parent
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from sculpt_integrity import validate_manifest_integrity
from macro_grid import contour_extrema, mouth_corner_indices


TARGET_FILES = {
    "jaw": ("chin", "chin-width-incr.target.gz"),
    "mouth": ("mouth", "mouth-scale-horiz-incr.target.gz"),
}


def canonical(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"))


def sha256_bytes(value):
    return hashlib.sha256(value).hexdigest()


def arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True)
    return parser.parse_args(sys.argv[sys.argv.index("--") + 1 :])


def target_records(path):
    with gzip.open(path, "rt", encoding="utf-8") as stream:
        return [tuple([int(parts[0])] + [float(value) for value in parts[1:]])
                for line in stream if (parts := line.split()) and not line.startswith("#")]


def target_indices(path):
    return sorted({record[0] for record in target_records(path)})


def mixed_coordinates(head):
    keys = head.data.shape_keys
    if not keys or not keys.key_blocks:
        return [vertex.co.copy() for vertex in head.data.vertices]
    basis = keys.key_blocks[0]
    result = [point.co.copy() for point in basis.data]
    for key in keys.key_blocks[1:]:
        if abs(key.value) > 0.000001:
            for index, point in enumerate(key.data):
                result[index] += (point.co - basis.data[index].co) * key.value
    return result


def camera_hash(camera):
    payload = {
        "type": camera.data.type, "orthoScale": camera.data.ortho_scale, "lens": camera.data.lens,
        "location": list(camera.location), "rotation": list(camera.rotation_euler),
        "resolution": [bpy.context.scene.render.resolution_x, bpy.context.scene.render.resolution_y],
    }
    return sha256_bytes(canonical(payload).encode("utf-8")), payload


def topology_hash(head):
    payload = {"vertices": len(head.data.vertices), "polygons": [list(poly.vertices) for poly in head.data.polygons]}
    return sha256_bytes(canonical(payload).encode("utf-8")), payload["vertices"]


def project(camera, point):
    result = world_to_camera_view(bpy.context.scene, camera, Vector(point))
    return [result.x, result.y, result.z]


def extremum(points, direction):
    return min(points, key=lambda item: item["camera"][0 if direction == "left" else 1]) if direction in {"left", "chin"} else max(points, key=lambda item: item["camera"][0])


def split_points(points, left):
    center = sum(point["world"][0] for point in points) / len(points)
    result = [point for point in points if (point["world"][0] < center) == left]
    if not result:
        raise RuntimeError("stable target anchor did not split into both sides")
    return result


def region(points, indices, side=None, chin=False, split=False):
    selected = [{"index": index, "world": list(points[index]), "camera": None} for index in indices]
    for point in selected:
        point["camera"] = project(CAMERA, point["world"])
    if split:
        selected = split_points(selected, side == "left")
    representative = extremum(selected, "chin" if chin else side)
    return {"indices": [point["index"] for point in selected], "raw": selected, **representative}


def point_anchor(points, index):
    point = {"index": index, "world": list(points[index])}
    point["camera"] = project(CAMERA, point["world"])
    return {"indices": [index], "raw": [point], "world": point["world"], "camera": point["camera"]}


def contour_anchor(points, extreme):
    point = {"index": extreme["index"], "world": extreme["world"]}
    point["camera"] = project(CAMERA, point["world"])
    return {"indices": [point["index"]], "raw": [point], "world": point["world"], "camera": point["camera"]}


def main():
    global CAMERA
    output = Path(arguments().output).resolve()
    recipe = validate_manifest_integrity(output)
    if bpy.data.filepath != str(output / "face-sculpt.blend"):
        raise RuntimeError("macro metric must run from the freshly reopened candidate blend")
    head = bpy.data.objects.get("FaceProofHead")
    CAMERA = bpy.data.objects.get("FaceProofCamera")
    left_eye, right_eye = bpy.data.objects.get("SculptEyeL"), bpy.data.objects.get("SculptEyeR")
    if not all((head, CAMERA, left_eye, right_eye)):
        raise RuntimeError("macro metric source objects are missing")
    from bl_ext.blender_org.mpfb.services import LocationService
    target_root = Path(LocationService.get_mpfb_data("targets"))
    sets = {name: target_indices(target_root / directory / filename) for name, (directory, filename) in TARGET_FILES.items()}
    ear_indices = set().union(*(target_indices(path) for path in (target_root / "ears").glob("*.target.gz")))
    coordinates = [head.matrix_world @ point for point in mixed_coordinates(head)]
    if any(index >= len(coordinates) for indices in sets.values() for index in indices) or any(index >= len(coordinates) for index in ear_indices):
        raise RuntimeError("stable MPFB target vertices do not match candidate topology")
    target = json.loads((ROOT / "target-front-geometry.json").read_text(encoding="utf-8"))
    bands = target["contourBands"]
    chin = region(coordinates, sets["jaw"], chin=True)
    body_group = head.vertex_groups.get("body")
    if body_group is None:
        raise RuntimeError("head body vertex group is missing")
    body_indices = {
        vertex.index for vertex in head.data.vertices
        if any(link.group == body_group.index and link.weight > 0.5 for link in vertex.groups)
    }
    if not body_indices:
        raise RuntimeError("head body vertex group has no eligible contour vertices")
    eye_z = (left_eye.location.z + right_eye.location.z) * 0.5
    chin_z = chin["world"][2]
    band_half_width = abs(eye_z - chin_z) * bands["halfBandOfEyeChin"]
    surface = [
        {"index": index, "x": point.x, "z": point.z, "world": list(point), "isEar": index in ear_indices, "isBody": True}
        for index, point in enumerate(coordinates) if index in body_indices
    ]
    cheek = contour_extrema(surface, bands["cheekFraction"], band_half_width, eye_z, chin_z)
    jaw = contour_extrema(surface, bands["jawFraction"], band_half_width, eye_z, chin_z)
    mouth_records = target_records(target_root / "mouth" / "mouth-scale-horiz-incr.target.gz")
    left_mouth, right_mouth = mouth_corner_indices([(record[0], record[1]) for record in mouth_records])
    anchors = {
        "leftEye": {"source": "SculptEyeL center", "world": list(left_eye.location), "camera": project(CAMERA, left_eye.location)},
        "rightEye": {"source": "SculptEyeR center", "world": list(right_eye.location), "camera": project(CAMERA, right_eye.location)},
        "leftCheek": contour_anchor(coordinates, cheek["left"]),
        "rightCheek": contour_anchor(coordinates, cheek["right"]),
        "leftJaw": contour_anchor(coordinates, jaw["left"]),
        "rightJaw": contour_anchor(coordinates, jaw["right"]),
        "leftMouth": point_anchor(coordinates, left_mouth),
        "rightMouth": point_anchor(coordinates, right_mouth),
        "chin": chin,
    }
    eye_distance = abs(anchors["rightEye"]["camera"][0] - anchors["leftEye"]["camera"][0])
    if not math.isfinite(eye_distance) or eye_distance <= 0:
        raise RuntimeError("eye separation is invalid")
    width = lambda left, right: abs(anchors[right]["camera"][0] - anchors[left]["camera"][0]) / eye_distance
    cheek_width, jaw_width = width("leftCheek", "rightCheek"), width("leftJaw", "rightJaw")
    metric_path = output / "macro-geometry.json"
    cam_hash, cam_payload = camera_hash(CAMERA)
    topo_hash, vertex_count = topology_hash(head)
    record = {
        "schemaVersion": 1, "candidateId": recipe["candidateId"],
        "targetImage": target["image"], "camera": {"sha256": cam_hash, "settings": cam_payload},
        "topology": {"vertexCount": vertex_count, "sha256": topo_hash}, "anchors": anchors,
        "contourBands": {**bands, "halfBandWorld": band_half_width, "eligibleBodyVertexCount": len(body_indices), "excludedEarVertexCount": len(ear_indices)},
        "ratios": {"cheekWidth": cheek_width, "jawWidth": jaw_width, "mouthWidth": width("leftMouth", "rightMouth"),
                   "eyeToChinHeight": abs((anchors["leftEye"]["camera"][1] + anchors["rightEye"]["camera"][1]) / 2 - anchors["chin"]["camera"][1]) / eye_distance,
                   "lowerFaceTaper": jaw_width / cheek_width},
        "scoreInputs": ["cheekWidth", "jawWidth", "mouthWidth", "eyeToChinHeight", "lowerFaceTaper"],
        "method": "Full non-ear head-surface contour extrema in target-matched normalized cheek/jaw bands, stable maximum-signed-displacement mouth corners, and SculptEye centers projected through the saved orthographic front camera; no raster pixels are scored.",
    }
    metric_path.write_text(json.dumps(record, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"MACRO_GEOMETRY_OK candidate={recipe['candidateId']} metrics={metric_path}")


if __name__ == "__main__":
    main()
