import argparse
import json
import math
from collections import Counter, deque
from pathlib import Path


REQUIRED_GRAPH = {
    "MODULE_PRIMITIVES",
    "SURFACE_VDB",
    "SURFACE_SMOOTH",
    "SURFACE_POLYGONS",
    "SURFACE_REDUCE",
    "SURFACE_LABELS",
    "OUT_SMOOTH_BODY",
}
PARAMETERS = ("voxel_size_m", "smooth_iterations", "adaptivity", "module_overlap_m", "target_faces")
METADATA = ("body_plan_id", "placements", "connections", "cells", "mirrors")


def _failure(rule, subject):
    return {"rule": rule, "subject": subject}


def _cross(a, b):
    return (
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0],
    )


def _components(vertex_count, faces):
    adjacency = [set() for _ in range(vertex_count)]
    used = set()
    for face in faces:
        used.update(face)
        for index, start in enumerate(face):
            end = face[(index + 1) % len(face)]
            adjacency[start].add(end)
            adjacency[end].add(start)
    count = 0
    remaining = set(used)
    while remaining:
        count += 1
        queue = deque([remaining.pop()])
        while queue:
            for neighbor in adjacency[queue.popleft()]:
                if neighbor in remaining:
                    remaining.remove(neighbor)
                    queue.append(neighbor)
    return count


def verify_smooth_body(body, recipe, surface, tolerance=1e-7):
    failures = []
    vertices = surface.get("vertices", [])
    faces = surface.get("faces", [])
    if surface.get("schema") != "smooth-body-readback/0":
        failures.append(_failure("SMOOTH-BODY-SCHEMA-1", "surface"))

    finite = all(
        isinstance(vertex, list)
        and len(vertex) == 3
        and all(isinstance(value, (int, float)) and math.isfinite(value) for value in vertex)
        for vertex in vertices
    )
    valid_faces = []
    non_axial = 0
    edges = Counter()
    for face in faces:
        valid = (
            isinstance(face, list)
            and len(face) in (3, 4)
            and len(face) == len(set(face))
            and all(isinstance(index, int) and 0 <= index < len(vertices) for index in face)
        )
        if valid and finite:
            a, b, c = (vertices[index] for index in face[:3])
            normal = _cross([b[i] - a[i] for i in range(3)], [c[i] - a[i] for i in range(3)])
            magnitude = math.sqrt(sum(value * value for value in normal))
            valid = magnitude > 1e-14
            if valid:
                unit = [abs(value) / magnitude for value in normal]
                non_axial += sum(value > 0.05 for value in unit) >= 2
        if valid:
            valid_faces.append(face)
            for index, start in enumerate(face):
                edges[tuple(sorted((start, face[(index + 1) % len(face)])))] += 1
    components = _components(len(vertices), valid_faces) if valid_faces else 0
    manifold = (
        len(valid_faces) == len(faces)
        and bool(edges)
        and all(count == 2 for count in edges.values())
        and components == 1
    )
    if not manifold:
        failures.append(_failure("SMOOTH-BODY-MANIFOLD-1", "mesh"))

    non_axial_fraction = non_axial / len(valid_faces) if valid_faces else 0.0
    if non_axial_fraction < recipe["minimum_non_axial_face_fraction"]:
        failures.append(_failure("SMOOTH-BODY-SMOOTHNESS-1", "face normals"))
    if len(faces) > recipe["maximum_faces"]:
        failures.append(_failure("SMOOTH-BODY-BUDGET-1", "faces"))

    body_metadata = surface.get("body_metadata", {})
    if any(body_metadata.get(key) != body.get(key) for key in METADATA):
        failures.append(_failure("SMOOTH-BODY-PROVENANCE-1", "body plan"))
    parameters = surface.get("parameters", {})
    graph_valid = REQUIRED_GRAPH.issubset(surface.get("native_graph", [])) and all(
        parameters.get(key) == recipe[key] for key in PARAMETERS
    )
    if not graph_valid:
        failures.append(_failure("SMOOTH-BODY-GRAPH-1", "native graph"))

    module_ids = set(body.get("placements", {}))
    regions = surface.get("regions", [])
    vertex_modules = surface.get("vertex_modules", [])
    face_modules = surface.get("face_modules", [])
    semantics_valid = (
        len(vertex_modules) == len(vertices)
        and set(vertex_modules) == module_ids
        and len(face_modules) == len(faces)
        and set(face_modules) <= module_ids
        and len(regions) == len(vertices)
        and all(
            set(weights) == {"core", "head", "limb"}
            and all(isinstance(value, (int, float)) and math.isfinite(value) and 0 <= value <= 1 for value in weights.values())
            and abs(sum(weights.values()) - 1.0) <= tolerance
            for weights in regions
        )
    )
    if not semantics_valid:
        failures.append(_failure("SMOOTH-BODY-SEMANTIC-1", "labels"))

    max_bounds_error = float("inf")
    if vertices and body.get("vertices"):
        expected = [
            (min(vertex[axis] for vertex in body["vertices"]), max(vertex[axis] for vertex in body["vertices"]))
            for axis in range(3)
        ]
        actual = [
            (min(vertex[axis] for vertex in vertices), max(vertex[axis] for vertex in vertices))
            for axis in range(3)
        ]
        max_bounds_error = max(abs(a - b) for pair_a, pair_b in zip(expected, actual) for a, b in zip(pair_a, pair_b))
    if not math.isfinite(max_bounds_error) or max_bounds_error > recipe["bounds_tolerance_m"]:
        failures.append(_failure("SMOOTH-BODY-BOUNDS-1", "dimensions"))

    return {
        "schema": "smooth-body-verification/0",
        "passed": not failures,
        "failures": failures,
        "metrics": {
            "vertices": len(vertices),
            "faces": len(faces),
            "manifold_edges": len(edges),
            "connected_components": components,
            "non_axial_face_fraction": non_axial_fraction,
            "max_bounds_error_m": max_bounds_error,
        },
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("body_run", type=Path)
    parser.add_argument("recipe", type=Path)
    parser.add_argument("readback", type=Path)
    parser.add_argument("report", type=Path)
    args = parser.parse_args()
    body = json.loads((args.body_run / "composed-mesh.json").read_text(encoding="utf-8"))
    recipe = json.loads(args.recipe.read_text(encoding="utf-8"))
    surface = json.loads(args.readback.read_text(encoding="utf-8"))
    report = verify_smooth_body(body, recipe, surface)
    args.report.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"SMOOTH_BODY_{'PASS' if report['passed'] else 'FAIL'} report={args.report}")
    raise SystemExit(0 if report["passed"] else 1)


if __name__ == "__main__":
    main()
