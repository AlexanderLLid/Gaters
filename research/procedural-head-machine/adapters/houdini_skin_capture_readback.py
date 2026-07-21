import json
import sys
from pathlib import Path

import hou


scene_path, output_path = map(Path, sys.argv[1:3])
hou.hipFile.load(str(scene_path), suppress_save_prompt=True)
container = hou.node("/obj/SKIN_CAPTURE")
geometry = hou.node("/obj/SKIN_CAPTURE/OUT_CAPTURED_SKIN").geometry()
pose_nodes = json.loads(container.userData("pose_nodes"))
poses = {}
for pose_id, nodes in pose_nodes.items():
    posed_geometry = container.node(nodes["surface"]).geometry()
    posed_skeleton = container.node(nodes["skeleton"]).geometry()
    poses[pose_id] = {
        "positions": [list(point.position()) for point in posed_geometry.points()],
        "skeleton_positions": {point.attribValue("name"): list(point.position()) for point in posed_skeleton.points()},
    }
first_pose = next(iter(poses.values()))
point_names = {attrib.name() for attrib in geometry.pointAttribs()}
global_names = {attrib.name() for attrib in geometry.globalAttribs()}
readback = {
    "schema": "skin-capture-readback/0",
    "backend": "houdini-kinefx",
    "version": hou.applicationVersionString(),
    "positions": [list(point.position()) for point in geometry.points()],
    "faces": [[vertex.point().number() for vertex in primitive.vertices()] for primitive in geometry.prims()],
    "modules": [point.attribValue("module") for point in geometry.points()],
    "capture_indices": [list(point.attribValue("boneCapture_index")) for point in geometry.points()] if "boneCapture_index" in point_names else [],
    "capture_weights": [list(point.attribValue("boneCapture_data")) for point in geometry.points()] if "boneCapture_data" in point_names else [],
    "capture_paths": list(geometry.attribValue("boneCapture_pCaptPath")) if "boneCapture_pCaptPath" in global_names else [],
    "posed_positions": first_pose["positions"],
    "posed_skeleton_positions": first_pose["skeleton_positions"],
    "poses": poses,
    "point_attributes": sorted(point_names),
    "global_attributes": sorted(global_names),
    "source_guide_sha256": container.userData("source_guide_sha256"),
    "skeleton_sha256": container.userData("skeleton_sha256"),
    "recipe": json.loads(container.userData("recipe")),
}
output_path.write_text(json.dumps(readback, indent=2, sort_keys=True) + "\n", encoding="utf-8")
print(f"HOUDINI_SKIN_CAPTURE_READBACK_PASS output={output_path} paths={len(readback['capture_paths'])}")
