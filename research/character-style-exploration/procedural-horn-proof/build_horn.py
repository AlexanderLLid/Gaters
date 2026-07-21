"""Generate one detailed ram-style horn from explicit biological shape controls."""

from __future__ import annotations

import hashlib
import json
import math
import sys
from collections import Counter
from pathlib import Path

import hou


def load_recipe(path: Path) -> dict:
    recipe = json.loads(Path(path).read_text(encoding="utf-8"))
    if recipe.get("schema") != "horn-proof/0":
        raise ValueError("unsupported horn recipe schema")
    for key in (
        "axial_samples",
        "radial_segments",
        "turns",
        "coil_radius_base",
        "coil_decay",
        "base_radius",
        "tip_radius",
        "taper_power",
        "ridge_count",
    ):
        value = recipe.get(key)
        if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value) or value <= 0:
            raise ValueError(f"{key} must be finite and positive")
    if recipe["axial_samples"] < 8 or recipe["radial_segments"] < 8:
        raise ValueError("horn resolution must be at least 8 by 8")
    if recipe["tip_radius"] >= recipe["base_radius"]:
        raise ValueError("tip radius must be smaller than base radius")
    return recipe


def sample_centerline(recipe: dict) -> list[tuple[float, float, float]]:
    points = []
    count = recipe["axial_samples"]
    for index in range(count):
        s = index / (count - 1)
        theta = recipe["turns"] * math.tau * s
        radius = recipe["coil_radius_base"] * math.exp(-recipe["coil_decay"] * theta)
        points.append(
            (
                radius * math.cos(theta) - recipe["coil_radius_base"],
                recipe["helix_rise"] * s + recipe["asymmetry"] * math.sin(math.pi * s) * math.sin(theta * 0.5),
                radius * math.sin(theta),
            )
        )
    return points


def build(recipe: dict) -> tuple[hou.Geometry, dict]:
    centerline = sample_centerline(recipe)
    frames = _frames(centerline, recipe["twist_turns"])
    vertices = []
    colors = []
    ridge_values = []
    radial_segments = recipe["radial_segments"]

    for axial_index, (center, frame) in enumerate(zip(centerline, frames)):
        s = axial_index / (len(centerline) - 1)
        base_profile = recipe["tip_radius"] + (
            recipe["base_radius"] - recipe["tip_radius"]
        ) * (1.0 - s) ** recipe["taper_power"]
        ridge_wave = max(0.0, math.cos(math.tau * recipe["ridge_count"] * s)) ** recipe["ridge_sharpness"]
        ridge_values.append(ridge_wave)
        u, v = frame
        for radial_index in range(radial_segments):
            angle = math.tau * radial_index / radial_segments
            striation = math.cos(recipe["striation_count"] * angle + 0.35 * math.sin(math.tau * s))
            irregular = 1.0 + recipe["asymmetry"] * math.sin(angle + 0.8) * math.sin(math.pi * s)
            surface_radius = base_profile * (
                1.0
                + recipe["ridge_amplitude"] * ridge_wave
                + recipe["striation_amplitude"] * striation
            ) * irregular
            surface_radius *= _chip_factor(recipe.get("chips", []), s, angle)
            ovate = recipe["ovate_aspect"] * (1.0 - 0.08 * math.cos(angle))
            offset = _add(
                _scale(u, surface_radius * math.cos(angle)),
                _scale(v, surface_radius * ovate * math.sin(angle)),
            )
            vertices.append(_add(center, offset))
            warmth = 0.86 + 0.11 * ridge_wave + 0.03 * striation
            colors.append((0.42 * warmth, 0.285 * warmth, 0.155 * warmth))

    faces = []
    for axial_index in range(recipe["axial_samples"] - 1):
        start = axial_index * radial_segments
        following = start + radial_segments
        for radial_index in range(radial_segments):
            next_index = (radial_index + 1) % radial_segments
            faces.append(
                (
                    start + radial_index,
                    following + radial_index,
                    following + next_index,
                    start + next_index,
                )
            )
    faces.append(tuple(reversed(range(radial_segments))))
    tip_start = (recipe["axial_samples"] - 1) * radial_segments
    faces.append(tuple(tip_start + index for index in range(radial_segments)))

    summary = evaluate(vertices, faces, centerline, recipe)
    geometry = _houdini_geometry(vertices, faces, colors, recipe)
    return geometry, summary


def evaluate(vertices: list, faces: list, centerline: list, recipe: dict) -> dict:
    nonfinite = sum(not all(math.isfinite(component) for component in vertex) for vertex in vertices)
    edge_uses = Counter()
    minimum_area = math.inf
    for face in faces:
        for index, first in enumerate(face):
            second = face[(index + 1) % len(face)]
            edge_uses[tuple(sorted((first, second)))] += 1
        minimum_area = min(minimum_area, _polygon_area([vertices[index] for index in face]))
    base_vertices = vertices[: recipe["radial_segments"]]
    tip_vertices = vertices[-recipe["radial_segments"] :]
    base_mean = sum(math.dist(vertex, centerline[0]) for vertex in base_vertices) / len(base_vertices)
    tip_mean = sum(math.dist(vertex, centerline[-1]) for vertex in tip_vertices) / len(tip_vertices)
    boundary_edges = sum(count == 1 for count in edge_uses.values())
    nonmanifold_edges = sum(count != 2 for count in edge_uses.values())
    bounds = [
        [min(vertex[axis] for vertex in vertices) for axis in range(3)],
        [max(vertex[axis] for vertex in vertices) for axis in range(3)],
    ]
    passed = (
        nonfinite == 0
        and boundary_edges == 0
        and nonmanifold_edges == 0
        and minimum_area > 1e-8
        and base_mean > tip_mean * 8.0
    )
    return {
        "passed": passed,
        "point_count": len(vertices),
        "polygon_count": len(faces),
        "nonfinite_point_count": nonfinite,
        "boundary_edge_count": boundary_edges,
        "nonmanifold_edge_count": nonmanifold_edges,
        "minimum_polygon_area": minimum_area,
        "base_mean_radius": base_mean,
        "tip_mean_radius": tip_mean,
        "ridge_count": recipe["ridge_count"],
        "bounds": bounds,
        "houdini": hou.applicationVersionString(),
        "edition": str(hou.licenseCategory()),
    }


def write_artifacts(recipe_path: Path, output_dir: Path) -> dict:
    recipe = load_recipe(recipe_path)
    geometry, summary = build(recipe)
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    recipe_bytes = json.dumps(recipe, sort_keys=True, separators=(",", ":")).encode("utf-8")
    summary["recipe_id"] = recipe["id"]
    summary["recipe_sha256"] = hashlib.sha256(recipe_bytes).hexdigest()

    hou.hipFile.clear(suppress_save_prompt=True)
    container = hou.node("/obj").createNode("geo", "BIOLOGICAL_HORN")
    for child in container.children():
        child.destroy()
    stash = container.createNode("stash", "HORN_SURFACE")
    stash.parm("stash").set(geometry)
    output = container.createNode("null", "OUT_HORN")
    output.setInput(0, stash)
    output.setDisplayFlag(True)
    output.setRenderFlag(True)
    container.setUserData("recipe_id", recipe["id"])
    container.setUserData("recipe_sha256", summary["recipe_sha256"])
    container.layoutChildren()

    scene_path = output_dir / "horn-detail.hipnc"
    hou.hipFile.save(str(scene_path))
    geometry.saveToFile(str(output_dir / "horn-detail.obj"))
    (output_dir / "horn-recipe.json").write_text(json.dumps(recipe, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    (output_dir / "horn-summary.json").write_text(json.dumps(summary, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if not summary["passed"]:
        raise RuntimeError("horn geometry failed its topology policy")
    print(
        f"BIOLOGICAL_HORN_PASS points={summary['point_count']} "
        f"polygons={summary['polygon_count']} scene={scene_path}"
    )
    return summary


def _frames(centerline, twist_turns):
    tangents = []
    for index in range(len(centerline)):
        before = centerline[max(0, index - 1)]
        after = centerline[min(len(centerline) - 1, index + 1)]
        tangents.append(_normalize(_subtract(after, before)))
    initial_reference = (0.0, 1.0, 0.0)
    u = _normalize(_cross(tangents[0], initial_reference))
    frames = []
    for index, tangent in enumerate(tangents):
        projected = _subtract(u, _scale(tangent, _dot(u, tangent)))
        if _length(projected) < 1e-7:
            projected = _cross(tangent, (0.0, 0.0, 1.0))
        u = _normalize(projected)
        v = _normalize(_cross(tangent, u))
        twist = math.tau * twist_turns * index / (len(tangents) - 1)
        rotated_u = _add(_scale(u, math.cos(twist)), _scale(v, math.sin(twist)))
        rotated_v = _add(_scale(v, math.cos(twist)), _scale(u, -math.sin(twist)))
        frames.append((rotated_u, rotated_v))
    return frames


def _chip_factor(chips, s, angle):
    factor = 1.0
    for chip in chips:
        axial = abs(s - chip["position"]) / chip["length"]
        angular = abs(math.atan2(math.sin(angle - chip["angle"]), math.cos(angle - chip["angle"]))) / chip["width"]
        if axial < 1.0 and angular < 1.0:
            influence = (1.0 - axial * axial) * (1.0 - angular * angular)
            factor *= 1.0 - chip["depth"] * influence * influence
    return factor


def _houdini_geometry(vertices, faces, colors, recipe):
    geometry = hou.Geometry()
    geometry.addAttrib(hou.attribType.Point, "Cd", (1.0, 1.0, 1.0))
    geometry.addAttrib(hou.attribType.Point, "growth", 0.0)
    geometry.addAttrib(hou.attribType.Prim, "feature", "horn_sheath")
    points = []
    for index, (position, color) in enumerate(zip(vertices, colors)):
        point = geometry.createPoint()
        point.setPosition(position)
        point.setAttribValue("Cd", color)
        point.setAttribValue("growth", (index // recipe["radial_segments"]) / (recipe["axial_samples"] - 1))
        points.append(point)
    for face in faces:
        polygon = geometry.createPolygon(is_closed=True)
        for index in face:
            polygon.addVertex(points[index])
    return geometry


def _polygon_area(points):
    origin = points[0]
    return sum(
        _length(_cross(_subtract(points[index], origin), _subtract(points[index + 1], origin))) * 0.5
        for index in range(1, len(points) - 1)
    )


def _add(a, b):
    return tuple(a[index] + b[index] for index in range(3))


def _subtract(a, b):
    return tuple(a[index] - b[index] for index in range(3))


def _scale(value, factor):
    return tuple(component * factor for component in value)


def _dot(a, b):
    return sum(a[index] * b[index] for index in range(3))


def _cross(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])


def _length(value):
    return math.sqrt(_dot(value, value))


def _normalize(value):
    length = _length(value)
    if length < 1e-12:
        raise ValueError("cannot normalize a zero-length vector")
    return tuple(component / length for component in value)


if __name__ == "__main__":
    if len(sys.argv) != 3:
        raise SystemExit("usage: hython build_horn.py RECIPE OUTPUT_DIR")
    write_artifacts(Path(sys.argv[1]), Path(sys.argv[2]))
