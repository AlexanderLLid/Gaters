import json
import sys
from pathlib import Path

import hou


scene_path, output_path = map(Path, sys.argv[1:3])
hou.hipFile.load(str(scene_path), suppress_save_prompt=True)
output = hou.node("/obj/COMPOSED_CHARACTER/OUT_COMPOSED")
geometry = output.geometry()
stash = hou.node("/obj/COMPOSED_CHARACTER/COMPOSED_MESH")
region_names = json.loads(stash.userData("region_names"))
readback = {
    "schema": "socket-adapter-readback/0",
    "backend": "houdini",
    "version": hou.applicationVersionString(),
    "vertices": [list(point.position()) for point in geometry.points()],
    "faces": [[vertex.point().number() for vertex in primitive.vertices()] for primitive in geometry.prims()],
    "regions": [
        {name: point.attribValue(name) for name in region_names}
        for point in geometry.points()
    ],
    "vertex_modules": [point.attribValue("module") for point in geometry.points()],
    "face_modules": [primitive.attribValue("module") for primitive in geometry.prims()],
    "socket": json.loads(stash.userData("socket")),
    "composition_sha256": stash.userData("composition_sha256"),
}
output_path.write_text(json.dumps(readback, indent=2, sort_keys=True) + "\n", encoding="utf-8")
print(f"HOUDINI_SOCKET_READBACK_PASS output={output_path}")
