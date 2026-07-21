"""Materialize a verified AnatomyGraph as a self-contained Houdini guide scene."""

from __future__ import annotations

import json
import sys
from pathlib import Path

import hou


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from run_proof import run_recipe


COLORS = {
    "torso": (0.55, 0.55, 0.55),
    "tapered_head": (0.95, 0.55, 0.2),
    "tail": (0.4, 0.75, 0.35),
    "leg": (0.25, 0.55, 0.95),
    "wing": (0.75, 0.35, 0.9),
}


def build(recipe_path: Path, output_dir: Path) -> Path:
    output_dir = Path(output_dir)
    result = run_recipe(Path(recipe_path), output_dir)
    if not result["passed"]:
        raise RuntimeError("CreatureDNA graph failed independent verification")

    graph_path = output_dir / "anatomy-graph.json"
    graph = json.loads(graph_path.read_text(encoding="utf-8"))
    license_name = str(hou.licenseCategory())
    graph["toolchain"].update(
        {
            "houdini": hou.applicationVersionString(),
            "edition": license_name,
        }
    )
    graph_path.write_text(json.dumps(graph, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    geometry = _make_geometry(graph)
    hou.hipFile.clear(suppress_save_prompt=True)
    container = hou.node("/obj").createNode("geo", "CREATURE_DNA")
    for child in container.children():
        child.destroy()
    container.setUserData("creature_dna_recipe", graph["recipe"]["id"])
    container.setUserData("creature_dna_graph_sha256", result["graph_sha256"])

    stash = container.createNode("stash", "DNA_GUIDES")
    stash.parm("stash").set(geometry)
    output = container.createNode("null", "OUT_GUIDES")
    output.setInput(0, stash)
    output.setDisplayFlag(True)
    output.setRenderFlag(True)
    container.layoutChildren()

    scene_path = output_dir / f"creature-guide.{_scene_extension(license_name)}"
    hou.hipFile.save(str(scene_path))
    print(
        f"HOUDINI_GUIDE_PASS recipe={graph['recipe']['id']} "
        f"points={len(graph['joints'])} curves={len(graph['bones'])} scene={scene_path}"
    )
    return scene_path


def _make_geometry(graph: dict) -> hou.Geometry:
    geometry = hou.Geometry()
    geometry.addAttrib(hou.attribType.Point, "joint_id", "")
    geometry.addAttrib(hou.attribType.Point, "module_id", "")
    geometry.addAttrib(hou.attribType.Point, "role", "")
    geometry.addAttrib(hou.attribType.Point, "side", "")
    geometry.addAttrib(hou.attribType.Point, "radius_m", 0.0)
    geometry.addAttrib(hou.attribType.Point, "Cd", (1.0, 1.0, 1.0))
    geometry.addAttrib(hou.attribType.Prim, "bone_id", "")
    geometry.addAttrib(hou.attribType.Prim, "parent_joint", "")
    geometry.addAttrib(hou.attribType.Prim, "child_joint", "")
    module_types = {module["id"]: module["type"] for module in graph["modules"]}
    points = {}

    for joint in graph["joints"]:
        point = geometry.createPoint()
        point.setPosition(joint["position_m"])
        point.setAttribValue("joint_id", joint["id"])
        point.setAttribValue("module_id", joint["module"])
        point.setAttribValue("role", joint["role"])
        point.setAttribValue("side", joint["side"])
        point.setAttribValue("radius_m", joint["radius_m"])
        point.setAttribValue("Cd", COLORS[module_types[joint["module"]]])
        points[joint["id"]] = point

    for bone in graph["bones"]:
        curve = geometry.createPolygon(is_closed=False)
        curve.addVertex(points[bone["parent_joint"]])
        curve.addVertex(points[bone["child_joint"]])
        curve.setAttribValue("bone_id", bone["id"])
        curve.setAttribValue("parent_joint", bone["parent_joint"])
        curve.setAttribValue("child_joint", bone["child_joint"])
    return geometry


def _scene_extension(license_name: str) -> str:
    lowered = license_name.lower()
    if "apprentice" in lowered:
        return "hipnc"
    if "indie" in lowered:
        return "hiplc"
    return "hip"


if __name__ == "__main__":
    if len(sys.argv) != 3:
        raise SystemExit("usage: hython build_guides.py RECIPE OUTPUT_DIR")
    build(Path(sys.argv[1]), Path(sys.argv[2]))
