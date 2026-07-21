import json
import sys
from pathlib import Path

import hou


scene_path, output_path = map(Path, sys.argv[1:3])
hou.hipFile.load(str(scene_path), suppress_save_prompt=True)
output = hou.node("/obj/PROCEDURAL_HEAD/OUT_DEFORMED")
deform = hou.node("/obj/PROCEDURAL_HEAD/SEMANTIC_HEAD_DEFORMATION")
geometry = output.geometry()
readback = {
    "schema": "head-deformation-readback/0",
    "backend": "houdini-sop",
    "version": hou.applicationVersionString(),
    "deformation_id": deform.userData("deformation_id"),
    "graph": {"operation": "widen_region", "amount": deform.parm("amount").eval()},
    "vertices": [list(point.position()) for point in geometry.points()],
    "faces": [[vertex.point().number() for vertex in primitive.vertices()] for primitive in geometry.prims()],
    "regions": [
        {name: point.attribValue(name) for name in ("skull", "face", "jaw", "chin")}
        for point in geometry.points()
    ],
}
output_path.write_text(json.dumps(readback, indent=2, sort_keys=True) + "\n", encoding="utf-8")
print(f"HOUDINI_DEFORMATION_READBACK_PASS output={output_path}")
