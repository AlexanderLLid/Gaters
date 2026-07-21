import json
import sys
from pathlib import Path

import hou


scene_path, output_path = map(Path, sys.argv[1:3])
hou.hipFile.load(str(scene_path), suppress_save_prompt=True)
output = hou.node("/obj/GUIDE_SKELETON/OUT_GUIDE_SKELETON")
geometry = output.geometry()
stash = hou.node("/obj/GUIDE_SKELETON/GUIDE_SKELETON_POINTS")
names = [point.attribValue("name") for point in geometry.points()]
readback = {
    "schema": "guide-skeleton-adapter-readback/0",
    "backend": "houdini-kinefx",
    "version": hou.applicationVersionString(),
    "joint_names": names,
    "parents": [point.attribValue("guide_parent") for point in geometry.points()],
    "positions": [list(point.position()) for point in geometry.points()],
    "transforms": [list(point.attribValue("transform")) for point in geometry.points()],
    "edges": [[vertex.point().attribValue("name") for vertex in primitive.vertices()] for primitive in geometry.prims()],
    "source_guide_sha256": stash.userData("source_guide_sha256"),
    "skeleton_sha256": stash.userData("skeleton_sha256"),
}
output_path.write_text(json.dumps(readback, indent=2, sort_keys=True) + "\n", encoding="utf-8")
print(f"HOUDINI_GUIDE_SKELETON_READBACK_PASS output={output_path}")
