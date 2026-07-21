import json
import sys
from pathlib import Path

import hou


scene_path, output_path = map(Path, sys.argv[1:3])
hou.hipFile.load(str(scene_path), suppress_save_prompt=True)
container = hou.node("/obj/SMOOTH_BODY")
output = hou.node("/obj/SMOOTH_BODY/OUT_SMOOTH_BODY")
geometry = output.geometry()
readback = {
    "schema": "smooth-body-readback/0",
    "backend": "houdini",
    "version": hou.applicationVersionString(),
    "vertices": [list(point.position()) for point in geometry.points()],
    "faces": [[vertex.point().number() for vertex in primitive.vertices()] for primitive in geometry.prims()],
    "regions": [
        {name: point.attribValue(name) for name in ("core", "head", "limb")}
        for point in geometry.points()
    ],
    "vertex_modules": [point.attribValue("module") for point in geometry.points()],
    "face_modules": [primitive.attribValue("module") for primitive in geometry.prims()],
    "body_metadata": json.loads(container.userData("body_metadata")),
    "parameters": json.loads(container.userData("parameters")),
    "native_graph": [node.name() for node in container.children()],
}
if container.userData("guide_metadata") is not None:
    readback["guide_metadata"] = json.loads(container.userData("guide_metadata"))
output_path.write_text(json.dumps(readback, indent=2, sort_keys=True) + "\n", encoding="utf-8")
print(f"HOUDINI_SMOOTH_BODY_READBACK_PASS output={output_path}")
