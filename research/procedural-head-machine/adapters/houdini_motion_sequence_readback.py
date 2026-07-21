import json
import sys
from pathlib import Path

import hou


scene_path, motion_path, output_path = map(Path, sys.argv[1:4])
motion = json.loads(motion_path.read_text(encoding="utf-8"))
hou.hipFile.load(str(scene_path), suppress_save_prompt=True)
container = hou.node("/obj/SKIN_CAPTURE")
rest = container.node("OUT_CAPTURED_SKIN").geometry()
surface_node = container.node("OUT_GENERATED_MOTION")
skeleton_node = container.node("GENERATED_SKELETON_SEQUENCE")
frames = []
for frame in motion["frames"]:
    hou.setFrame(frame["index"] + 1)
    surface = surface_node.geometry()
    skeleton = skeleton_node.geometry()
    frames.append({
        "index": frame["index"],
        "time_seconds": frame["time_seconds"],
        "positions": [list(point.position()) for point in surface.points()],
        "skeleton_positions": {point.attribValue("name"): list(point.position()) for point in skeleton.points()},
    })
readback = {
    "schema": "houdini-motion-readback/0",
    "backend": "houdini-kinefx",
    "version": hou.applicationVersionString(),
    "positions": [list(point.position()) for point in rest.points()],
    "faces": [[vertex.point().number() for vertex in primitive.vertices()] for primitive in rest.prims()],
    "modules": [point.attribValue("module") for point in rest.points()],
    "frames": frames,
    "motion_sha256": container.userData("motion_sha256"),
    "source_skin_scene_sha256": container.userData("source_skin_scene_sha256"),
    "frame_range": list(hou.playbar.frameRange()),
}
output_path.write_text(json.dumps(readback, indent=2, sort_keys=True) + "\n", encoding="utf-8")
print(f"HOUDINI_MOTION_READBACK_PASS output={output_path} frames={len(frames)}")
