"""Reopen a Houdini guide scene and compare its cooked geometry with AnatomyGraph."""

from __future__ import annotations

import json
import math
import sys
from pathlib import Path

import hou


def inspect(scene_path: Path, graph_path: Path, report_path: Path) -> dict:
    graph = json.loads(Path(graph_path).read_text(encoding="utf-8"))
    hou.hipFile.load(str(scene_path), suppress_save_prompt=True)
    node = hou.node("/obj/CREATURE_DNA/OUT_GUIDES")
    failures = []
    if node is None:
        failures.append({"rule": "HOUDINI-NODE-1", "subject": "/obj/CREATURE_DNA/OUT_GUIDES"})
        points = []
        primitives = []
    else:
        geometry = node.geometry()
        points = geometry.points()
        primitives = geometry.prims()

    expected_joints = {joint["id"]: joint for joint in graph["joints"]}
    actual_joints = {point.attribValue("joint_id"): point for point in points}
    if set(actual_joints) != set(expected_joints):
        failures.append({"rule": "HOUDINI-JOINTS-1", "subject": "joint_ids"})
    for joint_id in set(actual_joints) & set(expected_joints):
        actual = tuple(actual_joints[joint_id].position())
        expected = expected_joints[joint_id]["position_m"]
        if math.dist(actual, expected) > 1e-6:
            failures.append({"rule": "HOUDINI-POSITION-1", "subject": joint_id})

    expected_bones = {bone["id"]: bone for bone in graph["bones"]}
    actual_bones = {primitive.attribValue("bone_id"): primitive for primitive in primitives}
    if set(actual_bones) != set(expected_bones):
        failures.append({"rule": "HOUDINI-BONES-1", "subject": "bone_ids"})
    for bone_id in set(actual_bones) & set(expected_bones):
        primitive = actual_bones[bone_id]
        expected = expected_bones[bone_id]
        endpoints = [vertex.point().attribValue("joint_id") for vertex in primitive.vertices()]
        if endpoints != [expected["parent_joint"], expected["child_joint"]]:
            failures.append({"rule": "HOUDINI-ENDPOINTS-1", "subject": bone_id})

    report = {
        "passed": not failures,
        "failures": sorted(failures, key=lambda item: (item["rule"], item["subject"])),
        "point_count": len(points),
        "curve_count": len(primitives),
        "expected_joint_count": len(expected_joints),
        "expected_bone_count": len(expected_bones),
        "houdini": hou.applicationVersionString(),
        "edition": str(hou.licenseCategory()),
    }
    Path(report_path).write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    state = "PASS" if report["passed"] else "FAIL"
    print(f"HOUDINI_SCENE_{state} points={len(points)} curves={len(primitives)}")
    return report


if __name__ == "__main__":
    if len(sys.argv) != 4:
        raise SystemExit("usage: hython inspect_guides.py SCENE GRAPH REPORT")
    result = inspect(Path(sys.argv[1]), Path(sys.argv[2]), Path(sys.argv[3]))
    raise SystemExit(0 if result["passed"] else 1)
