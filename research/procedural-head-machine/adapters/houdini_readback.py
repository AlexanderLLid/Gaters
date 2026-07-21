import json
import sys
from pathlib import Path

import hou


scene_path, output_path = map(Path, sys.argv[1:3])
hou.hipFile.load(str(scene_path), suppress_save_prompt=True)
output = hou.node("/obj/PROCEDURAL_HEAD/OUT_HEAD")
geometry = output.geometry()
stash = hou.node("/obj/PROCEDURAL_HEAD/HEAD_MESH")
readback = {
    "schema": "head-adapter-readback/0",
    "backend": "houdini",
    "version": hou.applicationVersionString(),
    "vertices": [list(point.position()) for point in geometry.points()],
    "faces": [[vertex.point().number() for vertex in primitive.vertices()] for primitive in geometry.prims()],
    "regions": [
        {name: point.attribValue(name) for name in ("skull", "face", "jaw", "chin")}
        for point in geometry.points()
    ],
    "source_mesh_sha256": stash.userData("source_mesh_sha256"),
    "source_topology_sha256": stash.userData("source_topology_sha256"),
}
output_path.write_text(json.dumps(readback, indent=2, sort_keys=True) + "\n", encoding="utf-8")
print(f"HOUDINI_HEAD_READBACK_PASS output={output_path}")
