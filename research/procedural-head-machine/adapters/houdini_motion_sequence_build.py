import hashlib
import json
import sys
from pathlib import Path

import hou


source_scene, motion_run, output_dir = map(Path, sys.argv[1:4])
motion = json.loads((motion_run / "motion.json").read_text(encoding="utf-8"))
receipt_motion_sha256 = json.loads((motion_run / "receipt.json").read_text(encoding="utf-8"))["motion_sha256"]
motion_sha256 = hashlib.sha256(json.dumps(motion, sort_keys=True, separators=(",", ":"), allow_nan=False).encode("utf-8")).hexdigest()
if motion_sha256 != receipt_motion_sha256:
    raise ValueError("MOTION_SEQUENCE_MOTION_PROVENANCE")
source_scene_sha256 = hashlib.sha256(source_scene.read_bytes()).hexdigest()
output_dir.mkdir(parents=True, exist_ok=True)

hou.hipFile.load(str(source_scene), suppress_save_prompt=True)
container = hou.node("/obj/SKIN_CAPTURE")
if not container:
    raise ValueError("MOTION_SEQUENCE_SOURCE_INVALID")
captured = container.node("LIMIT_AND_NORMALIZE_WEIGHTS")
rest_skeleton = container.node("REST_SKELETON")
if not captured or not rest_skeleton:
    raise ValueError("MOTION_SEQUENCE_SOURCE_INVALID")
rest_geometry = rest_skeleton.geometry()
bones = [
    (primitive.vertices()[0].point().attribValue("name"), primitive.vertices()[1].point().attribValue("name"))
    for primitive in rest_geometry.prims()
    if len(primitive.vertices()) == 2
]


def build_skeleton_geometry(joints):
    geometry = hou.Geometry()
    geometry.addAttrib(hou.attribType.Point, "name", "")
    transform = geometry.addAttrib(hou.attribType.Point, "transform", (1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0))
    points = {}
    for joint in joints:
        point = geometry.createPoint()
        point.setPosition(joint["position"])
        point.setAttribValue("name", joint["name"])
        point.setAttribValue(transform, tuple(joint["basis"]["aim"] + joint["basis"]["up"] + joint["basis"]["side"]))
        points[joint["name"]] = point
    for parent, child in bones:
        line = geometry.createPolygon(is_closed=False)
        line.addVertex(points[parent])
        line.addVertex(points[child])
    return geometry


frame_nodes = []
for frame in motion["frames"]:
    node = container.createNode("stash", f"MOTION_SKELETON_{frame['index']:03d}")
    node.parm("stash").set(build_skeleton_geometry(frame["joints"]))
    frame_nodes.append(node)

sequence = container.createNode("switch", "GENERATED_SKELETON_SEQUENCE")
for index, node in enumerate(frame_nodes):
    sequence.setInput(index, node)
sequence.parm("input").setExpression(f"clamp($F - 1, 0, {len(frame_nodes) - 1})", hou.exprLanguage.Hscript)
deform = container.createNode("kinefx::jointdeform", "GENERATED_JOINT_DEFORM")
deform.setInput(0, captured)
deform.setInput(1, rest_skeleton)
deform.setInput(2, sequence)
output = container.createNode("null", "OUT_GENERATED_MOTION")
output.setInput(0, deform)
output.setDisplayFlag(True)
output.setRenderFlag(True)
container.setUserData("motion_sha256", motion_sha256)
container.setUserData("source_skin_scene_sha256", source_scene_sha256)
container.setUserData("motion_frame_count", str(len(frame_nodes)))
hou.playbar.setFrameRange(1, len(frame_nodes))
hou.playbar.setPlaybackRange(1, len(frame_nodes))
hou.setFrame(1)
container.layoutChildren()
output.geometry()
scene_path = output_dir / "generated-motion.hipnc"
hou.hipFile.save(str(scene_path))
print(f"HOUDINI_MOTION_BUILD_PASS scene={scene_path} frames={len(frame_nodes)}")
