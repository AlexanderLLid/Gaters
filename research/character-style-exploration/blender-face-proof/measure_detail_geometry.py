"""Fresh-load Task 10 receipt from frozen visible-landmark body vertices."""

import argparse
import hashlib
import json
import sys
from pathlib import Path

import bpy
from bpy_extras.object_utils import world_to_camera_view

ROOT = Path(__file__).resolve().parent
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from detail_grid import LANDMARK_KEYS, detail_ratios, file_hash, validate_detail_target
from sculpt_integrity import validate_manifest_integrity
from measure_macro_geometry import mixed_coordinates, topology_hash, camera_hash


def arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True)
    return parser.parse_args(sys.argv[sys.argv.index("--") + 1:])


def project(camera, point):
    view = world_to_camera_view(bpy.context.scene, camera, point)
    return [view.x, view.y, view.z]


def main():
    output = Path(arguments().output).resolve()
    recipe = validate_manifest_integrity(output)
    if Path(bpy.data.filepath).resolve() != (output / "face-sculpt.blend").resolve():
        raise RuntimeError("detail metric must run from the freshly reopened candidate blend")
    head, camera = bpy.data.objects.get("FaceProofHead"), bpy.data.objects.get("FaceProofCamera")
    left_eye, right_eye = bpy.data.objects.get("SculptEyeL"), bpy.data.objects.get("SculptEyeR")
    if head is None or camera is None or left_eye is None or right_eye is None:
        raise RuntimeError("detail metric source objects are missing")
    target_path = ROOT / "target-detail-geometry.json"
    target = json.loads(target_path.read_text(encoding="utf-8"))
    validate_detail_target(target)
    if recipe["targetDetail"] != {"path": target_path.name, "sha256": file_hash(target_path)}:
        raise RuntimeError("detail target binding changed")
    stable = target["stableVertexIndices"]
    coordinates = [head.matrix_world @ point for point in mixed_coordinates(head)]
    indices = stable["indices"]
    if any(index >= len(coordinates) for index in indices.values()):
        raise RuntimeError("frozen detail vertex does not match candidate topology")
    anchors = {name: {"index": indices[name], "world": list(coordinates[indices[name]]), "camera": project(camera, coordinates[indices[name]])} for name in LANDMARK_KEYS}
    left_eye_camera, right_eye_camera = project(camera, left_eye.location), project(camera, right_eye.location)
    ratios = detail_ratios(anchors, horizontal_ipd=abs(right_eye_camera[0] - left_eye_camera[0]), eye_center_y=(left_eye_camera[1] + right_eye_camera[1]) / 2)
    cam_hash, cam_payload = camera_hash(camera)
    topo_hash, vertex_count = topology_hash(head)
    manifest = json.loads((output / "manifest.json").read_text(encoding="utf-8"))
    record = {"schemaVersion": 1, "candidateId": recipe["candidateId"], "targetImage": target["image"],
              "targetAnnotation": recipe["targetDetail"], "landmarkMap": stable,
              "camera": {"sha256": cam_hash, "settings": cam_payload}, "sceneLock": manifest["sceneLock"],
              "topology": {"vertexCount": vertex_count, "sha256": topo_hash}, "anchors": anchors,
              "toolVersions": {"detailMeasure": file_hash(Path(__file__).resolve()), "macroMeasure": file_hash(ROOT / "measure_macro_geometry.py"), "detailContract": file_hash(ROOT / "detail_grid.py")},
              "ratios": ratios, "scoreInputs": list(ratios),
              "method": "Frozen Task 9 frontmost body vertices only, projected through the saved front camera. Visible commissures replace the rejected macro mouth target extrema."}
    path = output / "detail-geometry.json"
    path.write_text(json.dumps(record, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"DETAIL_GEOMETRY_OK candidate={recipe['candidateId']} metrics={path}")


if __name__ == "__main__":
    main()
