import json
import sys
from pathlib import Path

import hou


source_run, output_dir = map(Path, sys.argv[1:3])
mesh_data = json.loads((source_run / "mesh.json").read_text(encoding="utf-8"))
region_data = json.loads((source_run / "regions.json").read_text(encoding="utf-8"))
receipt = json.loads((source_run / "receipt.json").read_text(encoding="utf-8"))
output_dir.mkdir(parents=True, exist_ok=True)

geometry = hou.Geometry()
for name in ("skull", "face", "jaw", "chin"):
    geometry.addAttrib(hou.attribType.Point, name, 0.0)
points = []
for index, position in enumerate(mesh_data["vertices"]):
    point = geometry.createPoint()
    point.setPosition(position)
    for name, weight in region_data["weights"][index].items():
        point.setAttribValue(name, weight)
    points.append(point)
for face in mesh_data["faces"]:
    polygon = geometry.createPolygon(is_closed=True)
    for index in face:
        polygon.addVertex(points[index])

hou.hipFile.clear(suppress_save_prompt=True)
container = hou.node("/obj").createNode("geo", "PROCEDURAL_HEAD")
for child in container.children():
    child.destroy()
stash = container.createNode("stash", "HEAD_MESH")
stash.parm("stash").set(geometry)
stash.setUserData("source_mesh_sha256", receipt["mesh_sha256"])
stash.setUserData("source_topology_sha256", receipt["topology_sha256"])
output = container.createNode("null", "OUT_HEAD")
output.setInput(0, stash)
output.setDisplayFlag(True)
output.setRenderFlag(True)
container.layoutChildren()
scene_path = output_dir / "head.hipnc"
hou.hipFile.save(str(scene_path))
print(f"HOUDINI_HEAD_BUILD_PASS scene={scene_path}")
