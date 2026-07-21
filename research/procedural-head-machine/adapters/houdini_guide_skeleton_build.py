import json
import sys
from pathlib import Path

import hou


skeleton_run, output_dir = map(Path, sys.argv[1:3])
skeleton = json.loads((skeleton_run / "skeleton.json").read_text(encoding="utf-8"))
receipt = json.loads((skeleton_run / "receipt.json").read_text(encoding="utf-8"))
output_dir.mkdir(parents=True, exist_ok=True)

geometry = hou.Geometry()
geometry.addAttrib(hou.attribType.Point, "name", "")
geometry.addAttrib(hou.attribType.Point, "guide_parent", "")
transform_attrib = geometry.addAttrib(hou.attribType.Point, "transform", (1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0))
points = {}
for joint in skeleton["joints"]:
    point = geometry.createPoint()
    point.setPosition(joint["position"])
    point.setAttribValue("name", joint["name"])
    point.setAttribValue("guide_parent", joint["parent"] or "")
    flat = joint["basis"]["aim"] + joint["basis"]["up"] + joint["basis"]["side"]
    point.setAttribValue(transform_attrib, tuple(flat))
    points[joint["name"]] = point
for bone in skeleton["bones"]:
    line = geometry.createPolygon(is_closed=False)
    line.addVertex(points[bone["parent"]])
    line.addVertex(points[bone["child"]])

hou.hipFile.clear(suppress_save_prompt=True)
container = hou.node("/obj").createNode("geo", "GUIDE_SKELETON")
for child in container.children():
    child.destroy()
stash = container.createNode("stash", "GUIDE_SKELETON_POINTS")
stash.parm("stash").set(geometry)
stash.setUserData("source_guide_sha256", skeleton["source_guide_sha256"])
stash.setUserData("skeleton_sha256", receipt["skeleton_sha256"])
output = container.createNode("null", "OUT_GUIDE_SKELETON")
output.setInput(0, stash)
output.setDisplayFlag(True)
output.setRenderFlag(True)
container.layoutChildren()
scene_path = output_dir / "guide-skeleton.hipnc"
hou.hipFile.save(str(scene_path))
print(f"HOUDINI_GUIDE_SKELETON_BUILD_PASS scene={scene_path} joints={len(skeleton['joints'])} bones={len(skeleton['bones'])}")
