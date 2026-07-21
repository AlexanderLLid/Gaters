import json
import sys
from pathlib import Path

import hou


socket_run, output_dir = map(Path, sys.argv[1:3])
composition = json.loads((socket_run / "composed-mesh.json").read_text(encoding="utf-8"))
receipt = json.loads((socket_run / "receipt.json").read_text(encoding="utf-8"))
output_dir.mkdir(parents=True, exist_ok=True)

geometry = hou.Geometry()
geometry.addAttrib(hou.attribType.Point, "module", "")
geometry.addAttrib(hou.attribType.Prim, "module", "")
region_names = tuple(composition["regions"][0])
for name in region_names:
    geometry.addAttrib(hou.attribType.Point, name, 0.0)
points = []
for position, module, regions in zip(
    composition["vertices"], composition["vertex_modules"], composition["regions"]
):
    point = geometry.createPoint()
    point.setPosition(position)
    point.setAttribValue("module", module)
    for name, value in regions.items():
        point.setAttribValue(name, value)
    points.append(point)
for face, module in zip(composition["faces"], composition["face_modules"]):
    polygon = geometry.createPolygon(is_closed=True)
    for index in face:
        polygon.addVertex(points[index])
    polygon.setAttribValue("module", module)
hou.hipFile.clear(suppress_save_prompt=True)
container = hou.node("/obj").createNode("geo", "COMPOSED_CHARACTER")
for child in container.children():
    child.destroy()
stash = container.createNode("stash", "COMPOSED_MESH")
stash.parm("stash").set(geometry)
stash.setUserData("composition_sha256", receipt["composition_sha256"])
stash.setUserData("socket", json.dumps(composition["socket"], sort_keys=True))
stash.setUserData("region_names", json.dumps(region_names))
output = container.createNode("null", "OUT_COMPOSED")
output.setInput(0, stash)
output.setDisplayFlag(True)
output.setRenderFlag(True)
container.layoutChildren()
scene_path = output_dir / "composed-character.hipnc"
hou.hipFile.save(str(scene_path))
print(f"HOUDINI_SOCKET_BUILD_PASS scene={scene_path}")
