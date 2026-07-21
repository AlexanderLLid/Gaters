"""Map Task 10 consensus candidate pixels once to frontmost Task 9 body vertices."""

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

from detail_grid import LANDMARK_KEYS, canonical, select_frontmost_landmark_vertex


PARENT_ID = "9032fd4dc9db2de64564b3f19e32ca5e6199881df111c2fa2d37feb1c7cc12f8"
PARENT_OUTPUT = ROOT / "Derived" / "search-runs" / "feature-calibration-20260720-100000" / "round-9" / PARENT_ID


def arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True)
    return parser.parse_args(sys.argv[sys.argv.index("--") + 1:])


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


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


def main():
    output = Path(arguments().output).resolve()
    if output.exists():
        raise FileExistsError(f"Refusing to overwrite landmark receipt: {output}")
    if Path(bpy.data.filepath).resolve() != (PARENT_OUTPUT / "face-sculpt.blend").resolve():
        raise RuntimeError("landmark mapping must run from the reopened Task 9 review candidate")
    head, camera = bpy.data.objects.get("FaceProofHead"), bpy.data.objects.get("FaceProofCamera")
    if head is None or camera is None:
        raise RuntimeError("detail landmark mapping source objects are missing")
    group = head.vertex_groups.get("body")
    if group is None:
        raise RuntimeError("head body group is missing")
    body_indices = [vertex.index for vertex in head.data.vertices if any(link.group == group.index and link.weight > 0.5 for link in vertex.groups)]
    if not body_indices:
        raise RuntimeError("head body group has no vertices")
    target_path = ROOT / "target-detail-geometry.json"
    target = json.loads(target_path.read_text(encoding="utf-8"))
    points = target["consensus"]["candidatePoints"]
    if set(points) != set(LANDMARK_KEYS):
        raise RuntimeError("detail consensus candidate points are incomplete")
    coordinates = mixed_coordinates(head)
    projected = {}
    for index in body_indices:
        view = world_to_camera_view(bpy.context.scene, camera, head.matrix_world @ coordinates[index])
        projected[index] = (view.x * 768, (1 - view.y) * 768, view.z)
    indices, distances = {}, {}
    for name, pixel in points.items():
        eligible = [{"index": index, "pixelDistance": ((xy[0] - pixel[0]) ** 2 + (xy[1] - pixel[1]) ** 2) ** 0.5, "cameraDepth": xy[2]}
                    for index, xy in projected.items() if xy[2] > 0]
        selected = select_frontmost_landmark_vertex(eligible)
        indices[name], distances[name] = selected["index"], {"pixelDistance": selected["pixelDistance"], "cameraDepth": selected["cameraDepth"]}
    payload = {"schemaVersion": 1, "candidateId": PARENT_ID,
               "candidateBlend": {"path": str((PARENT_OUTPUT / "face-sculpt.blend").resolve()), "sha256": sha256(PARENT_OUTPUT / "face-sculpt.blend")},
               "targetDetail": {"path": target_path.name, "sha256": sha256(target_path)},
               "toolVersions": {"mapScript": sha256(Path(__file__).resolve()), "detailContract": sha256(ROOT / "detail_grid.py")},
               "method": "Consensus Task 9 front pixels projected to body vertices once; select the camera-frontmost vertex within the nearest two-pixel-expanded, six-pixel-capped neighborhood. Indices are reviewed/frozen outputs and are never remapped per detail candidate.",
               "eligibleBodyVertexCount": len(body_indices), "indices": indices, "distances": distances}
    payload["sha256"] = hashlib.sha256(canonical(payload).encode("utf-8")).hexdigest()
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"DETAIL_LANDMARK_MAP_OK receipt={output}")


if __name__ == "__main__":
    main()
