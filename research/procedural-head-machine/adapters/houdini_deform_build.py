import json
import sys
from pathlib import Path

import hou


source_run, command_path, output_dir = map(Path, sys.argv[1:4])
mesh_data = json.loads((source_run / "mesh.json").read_text(encoding="utf-8"))
region_data = json.loads((source_run / "regions.json").read_text(encoding="utf-8"))
command = json.loads(command_path.read_text(encoding="utf-8"))
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
deform = container.createNode("attribwrangle", "SEMANTIC_HEAD_DEFORMATION")
deform.setInput(0, stash)
templates = deform.parmTemplateGroup()
templates.append(hou.FloatParmTemplate("amount", "Amount", 1, default_value=(command["amount"],)))
deform.setParmTemplateGroup(templates)
deform.parm("amount").set(command["amount"])
deform.parm("snippet").set(
    f'float mask = max(f@{command["region"]} - f@{command["preserve"]}, 0.0); '
    '@P.x *= 1.0 + chf("amount") * mask;'
)
deform.setUserData("deformation_id", command["id"])
output = container.createNode("null", "OUT_DEFORMED")
output.setInput(0, deform)
output.setDisplayFlag(True)
output.setRenderFlag(True)
container.layoutChildren()
scene_path = output_dir / "deformed-head.hipnc"
hou.hipFile.save(str(scene_path))
print(f"HOUDINI_DEFORMATION_BUILD_PASS scene={scene_path}")
