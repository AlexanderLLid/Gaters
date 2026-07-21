import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "src"))

import hou
from pose_skeleton import apply_pose


surface_run, skeleton_run, recipe_path, output_dir = map(Path, sys.argv[1:5])
surface = json.loads((surface_run / "readback.json").read_text(encoding="utf-8"))
skeleton = json.loads((skeleton_run / "skeleton.json").read_text(encoding="utf-8"))
skeleton_receipt = json.loads((skeleton_run / "receipt.json").read_text(encoding="utf-8"))
recipe = json.loads(recipe_path.read_text(encoding="utf-8"))
output_dir.mkdir(parents=True, exist_ok=True)

skin_geometry = hou.Geometry()
skin_geometry.addAttrib(hou.attribType.Point, "module", "")
skin_points = []
for position, module in zip(surface["vertices"], surface["vertex_modules"]):
    point = skin_geometry.createPoint()
    point.setPosition(position)
    point.setAttribValue("module", module)
    skin_points.append(point)
for face in surface["faces"]:
    polygon = skin_geometry.createPolygon(is_closed=True)
    for index in face:
        polygon.addVertex(skin_points[index])

def build_skeleton_geometry(joints):
    geometry = hou.Geometry()
    geometry.addAttrib(hou.attribType.Point, "name", "")
    transform = geometry.addAttrib(hou.attribType.Point, "transform", (1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0))
    points = {}
    for joint in joints:
        point = geometry.createPoint()
        point.setPosition(joint["position"])
        point.setAttribValue("name", joint["name"])
        flat = joint["basis"]["aim"] + joint["basis"]["up"] + joint["basis"]["side"]
        point.setAttribValue(transform, tuple(flat))
        points[joint["name"]] = point
    for bone in skeleton["bones"]:
        line = geometry.createPolygon(is_closed=False)
        line.addVertex(points[bone["parent"]])
        line.addVertex(points[bone["child"]])
    return geometry


rest_skeleton_geometry = build_skeleton_geometry(skeleton["joints"])
poses = recipe.get("diagnostic_poses") or [{"id": "diagnostic_pose", **recipe["diagnostic_pose"]}]

hou.hipFile.clear(suppress_save_prompt=True)
container = hou.node("/obj").createNode("geo", "SKIN_CAPTURE")
for child in container.children():
    child.destroy()
skin_source = container.createNode("stash", "REST_SKIN")
skin_source.parm("stash").set(skin_geometry)
skeleton_source = container.createNode("stash", "REST_SKELETON")
skeleton_source.parm("stash").set(rest_skeleton_geometry)
method = recipe["method"]
if method == "houdini-joint-capture-proximity":
    capture = container.createNode("kinefx::jointcaptureproximity", "GENERATE_CAPTURE_WEIGHTS")
elif method == "houdini-joint-capture-biharmonic":
    capture = container.createNode("kinefx::jointcapturebiharmonic", "GENERATE_CAPTURE_WEIGHTS")
else:
    raise ValueError("SKIN_CAPTURE_METHOD_UNSUPPORTED")
capture.setInput(0, skin_source)
capture.setInput(1, skeleton_source)
if method == "houdini-joint-capture-proximity":
    capture.parm("maxinfluences").set(recipe["max_influences"])
    capture.parm("dropoff").set(recipe["dropoff"])
    capture.parm("normweights").set(1 if recipe["normalize"] else 0)
else:
    capture.parm("maxiter").set(recipe["max_iterations"])
correct = container.createNode("capturecorrect", "LIMIT_AND_NORMALIZE_WEIGHTS")
correct.setInput(0, capture)
correct.parm("limitregions").set(1)
correct.parm("maxregions").set(recipe["max_influences"])
correct.parm("renormalize").set(1 if recipe["normalize"] else 0)
unpack = container.createNode("captureattribunpack", "UNPACK_FOR_VERIFICATION")
unpack.setInput(0, correct)
output = container.createNode("null", "OUT_CAPTURED_SKIN")
output.setInput(0, unpack)
pose_nodes = {}
posed_outputs = []
for index, pose in enumerate(poses):
    posed_source = container.createNode("stash", f"POSED_SKELETON_{index}")
    posed_source.parm("stash").set(build_skeleton_geometry(apply_pose(skeleton, pose)))
    deform = container.createNode("kinefx::jointdeform", f"JOINT_DEFORM_{index}")
    deform.setInput(0, correct)
    deform.setInput(1, skeleton_source)
    deform.setInput(2, posed_source)
    posed_output = container.createNode("null", f"OUT_POSED_SKIN_{index}")
    posed_output.setInput(0, deform)
    pose_nodes[pose["id"]] = {"skeleton": posed_source.name(), "surface": posed_output.name()}
    posed_outputs.append(posed_output)
posed_outputs[-1].setDisplayFlag(True)
posed_outputs[-1].setRenderFlag(True)
container.setUserData("source_guide_sha256", skeleton["source_guide_sha256"])
container.setUserData("skeleton_sha256", skeleton_receipt["skeleton_sha256"])
container.setUserData("recipe", json.dumps(recipe, sort_keys=True))
container.setUserData("pose_nodes", json.dumps(pose_nodes, sort_keys=True))
container.layoutChildren()
output.geometry()
for posed_output in posed_outputs:
    posed_output.geometry()
scene_path = output_dir / "captured-mannequin.hipnc"
hou.hipFile.save(str(scene_path))
print(f"HOUDINI_SKIN_CAPTURE_BUILD_PASS scene={scene_path} points={len(skin_points)}")
