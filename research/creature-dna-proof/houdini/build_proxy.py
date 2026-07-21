"""Build a crude capsule creature proxy from a verified AnatomyGraph."""

from __future__ import annotations

import json
import math
import sys
from pathlib import Path

import hou


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from run_proof import run_recipe


COLORS = {
    "torso": (0.35, 0.38, 0.32),
    "tapered_head": (0.48, 0.34, 0.22),
    "tail": (0.29, 0.42, 0.25),
    "leg": (0.28, 0.34, 0.3),
    "wing": (0.36, 0.25, 0.42),
}


def build(recipe_path: Path, output_dir: Path) -> Path:
    output_dir = Path(output_dir)
    result = run_recipe(recipe_path, output_dir)
    if not result["passed"]:
        raise RuntimeError("CreatureDNA graph failed verification")
    graph = json.loads((output_dir / "anatomy-graph.json").read_text(encoding="utf-8"))
    geometry = _proxy_geometry(graph)

    hou.hipFile.clear(suppress_save_prompt=True)
    container = hou.node("/obj").createNode("geo", "CREATURE_DNA")
    for child in container.children():
        child.destroy()
    stash = container.createNode("stash", "CAPSULE_PROXY")
    stash.parm("stash").set(geometry)
    output = container.createNode("null", "OUT_PROXY")
    output.setInput(0, stash)
    output.setDisplayFlag(True)
    output.setRenderFlag(True)
    container.layoutChildren()

    scene_path = output_dir / "creature-proxy.hipnc"
    hou.hipFile.save(str(scene_path))
    geometry.saveToFile(str(output_dir / "creature-proxy.obj"))
    summary = {
        "passed": len(geometry.points()) > 1000 and len(geometry.prims()) > 1000,
        "point_count": len(geometry.points()),
        "polygon_count": len(geometry.prims()),
        "source_joint_count": len(graph["joints"]),
        "source_bone_count": len(graph["bones"]),
        "houdini": hou.applicationVersionString(),
        "edition": str(hou.licenseCategory()),
    }
    (output_dir / "proxy-summary.json").write_text(
        json.dumps(summary, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    if not summary["passed"]:
        raise RuntimeError("proxy geometry did not meet the minimum output contract")
    print(
        f"HOUDINI_PROXY_PASS points={summary['point_count']} "
        f"polygons={summary['polygon_count']} scene={scene_path}"
    )
    return scene_path


def _proxy_geometry(graph: dict) -> hou.Geometry:
    geometry = hou.Geometry()
    geometry.addAttrib(hou.attribType.Prim, "Cd", (1.0, 1.0, 1.0))
    geometry.addAttrib(hou.attribType.Prim, "module_id", "")
    joints = {joint["id"]: joint for joint in graph["joints"]}
    module_types = {module["id"]: module["type"] for module in graph["modules"]}

    for joint in graph["joints"]:
        color = COLORS[module_types[joint["module"]]]
        _add_sphere(geometry, joint["position_m"], joint["radius_m"] * 0.94, color, joint["module"])

    for bone in graph["bones"]:
        parent = joints[bone["parent_joint"]]
        child = joints[bone["child_joint"]]
        same_module = parent["module"] == child["module"]
        start_radius = parent["radius_m"] if same_module else child["radius_m"]
        color = COLORS[module_types[child["module"]]]
        _add_frustum(
            geometry,
            parent["position_m"],
            child["position_m"],
            max(0.025, start_radius * 0.82),
            max(0.02, child["radius_m"] * 0.82),
            color,
            child["module"],
        )

    for module in graph["modules"]:
        if module["type"] == "wing":
            _add_wing_membrane(geometry, module["id"], joints, COLORS["wing"])
    return geometry


def _add_sphere(geometry, center, radius, color, module_id, segments=10, rings=6):
    top = _point(geometry, (center[0], center[1], center[2] + radius))
    rows = []
    for ring in range(1, rings):
        angle = math.pi * ring / rings
        row = []
        for segment in range(segments):
            around = math.tau * segment / segments
            row.append(
                _point(
                    geometry,
                    (
                        center[0] + radius * math.sin(angle) * math.cos(around),
                        center[1] + radius * math.sin(angle) * math.sin(around),
                        center[2] + radius * math.cos(angle),
                    ),
                )
            )
        rows.append(row)
    bottom = _point(geometry, (center[0], center[1], center[2] - radius))
    for segment in range(segments):
        next_segment = (segment + 1) % segments
        _face(geometry, [top, rows[0][segment], rows[0][next_segment]], color, module_id)
        for row in range(len(rows) - 1):
            _face(
                geometry,
                [rows[row][segment], rows[row + 1][segment], rows[row + 1][next_segment], rows[row][next_segment]],
                color,
                module_id,
            )
        _face(geometry, [rows[-1][next_segment], rows[-1][segment], bottom], color, module_id)


def _add_frustum(geometry, start, end, start_radius, end_radius, color, module_id, sides=10):
    direction = _normalize(_subtract(end, start))
    helper = (0.0, 0.0, 1.0) if abs(direction[2]) < 0.9 else (0.0, 1.0, 0.0)
    axis_a = _normalize(_cross(direction, helper))
    axis_b = _cross(direction, axis_a)
    start_ring = []
    end_ring = []
    for side in range(sides):
        angle = math.tau * side / sides
        radial = _add(_scale(axis_a, math.cos(angle)), _scale(axis_b, math.sin(angle)))
        start_ring.append(_point(geometry, _add(start, _scale(radial, start_radius))))
        end_ring.append(_point(geometry, _add(end, _scale(radial, end_radius))))
    for side in range(sides):
        next_side = (side + 1) % sides
        _face(geometry, [start_ring[side], end_ring[side], end_ring[next_side], start_ring[next_side]], color, module_id)
    _face(geometry, list(reversed(start_ring)), color, module_id)
    _face(geometry, end_ring, color, module_id)


def _add_wing_membrane(geometry, module_id, joints, color):
    shoulder = joints[f"{module_id}.shoulder"]["position_m"]
    elbow = joints[f"{module_id}.elbow"]["position_m"]
    wrist = joints[f"{module_id}.wrist"]["position_m"]
    tip = joints[f"{module_id}.tip"]["position_m"]
    points = [_point(geometry, position) for position in (shoulder, elbow, wrist, tip)]
    _face(geometry, [points[0], points[1], points[2]], color, module_id)
    _face(geometry, [points[0], points[2], points[3]], color, module_id)


def _point(geometry, position):
    point = geometry.createPoint()
    point.setPosition(position)
    return point


def _face(geometry, points, color, module_id):
    polygon = geometry.createPolygon(is_closed=True)
    for point in points:
        polygon.addVertex(point)
    polygon.setAttribValue("Cd", color)
    polygon.setAttribValue("module_id", module_id)


def _add(a, b):
    return tuple(a[index] + b[index] for index in range(3))


def _subtract(a, b):
    return tuple(a[index] - b[index] for index in range(3))


def _scale(value, factor):
    return tuple(component * factor for component in value)


def _cross(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])


def _normalize(value):
    length = math.sqrt(sum(component * component for component in value))
    if length <= 1e-9:
        raise ValueError("cannot build proxy for zero-length bone")
    return tuple(component / length for component in value)


if __name__ == "__main__":
    if len(sys.argv) != 3:
        raise SystemExit("usage: hython build_proxy.py RECIPE OUTPUT_DIR")
    build(Path(sys.argv[1]), Path(sys.argv[2]))
