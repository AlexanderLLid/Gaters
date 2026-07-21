import math
from collections import Counter, deque


STEPS = {
    "-x": (-1, 0, 0), "+x": (1, 0, 0),
    "-y": (0, -1, 0), "+y": (0, 1, 0),
    "-z": (0, 0, -1), "+z": (0, 0, 1),
}


def _failure(rule, subject):
    return {"rule": rule, "subject": subject}


def _triangle_area(a, b, c):
    ab = [b[index] - a[index] for index in range(3)]
    ac = [c[index] - a[index] for index in range(3)]
    cross = [
        ab[1] * ac[2] - ab[2] * ac[1],
        ab[2] * ac[0] - ab[0] * ac[2],
        ab[0] * ac[1] - ab[1] * ac[0],
    ]
    return math.sqrt(sum(value * value for value in cross)) / 2


def _components(cells):
    remaining = set(cells)
    count = 0
    while remaining:
        count += 1
        queue = deque([remaining.pop()])
        while queue:
            cell = queue.popleft()
            for step in STEPS.values():
                neighbor = tuple(cell[axis] + step[axis] for axis in range(3))
                if neighbor in remaining:
                    remaining.remove(neighbor)
                    queue.append(neighbor)
    return count


def _mirror_error(source, target):
    expected = {(-cell[0] - 1, cell[1], cell[2]) for cell in source}
    if not expected or not target:
        return float("inf")
    return float(max(min(sum(abs(a - b) for a, b in zip(point, candidate)) for candidate in target) for point in expected))


def verify_body_plan(plan, body, tolerance=1e-9):
    failures = []
    vertices = body.get("vertices", [])
    faces = body.get("faces", [])
    node_ids = {node["id"] for node in plan["nodes"]}
    node_by_id = {node["id"]: node for node in plan["nodes"]}

    if body.get("schema") != "body-plan-mesh/0" or body.get("body_plan_id") != plan["id"]:
        failures.append(_failure("BODY-PLAN-SCHEMA-1", "body"))

    finite = all(
        isinstance(vertex, list)
        and len(vertex) == 3
        and all(isinstance(value, (int, float)) and math.isfinite(value) for value in vertex)
        for vertex in vertices
    )
    valid_faces = []
    for face in faces:
        valid = (
            isinstance(face, list)
            and len(face) == 4
            and len(set(face)) == 4
            and all(isinstance(index, int) and 0 <= index < len(vertices) for index in face)
        )
        if valid and finite:
            area = _triangle_area(vertices[face[0]], vertices[face[1]], vertices[face[2]])
            area += _triangle_area(vertices[face[0]], vertices[face[2]], vertices[face[3]])
            valid = area > 1e-14
        if valid:
            valid_faces.append(face)
    edges = Counter()
    for face in valid_faces:
        for index, start in enumerate(face):
            edges[tuple(sorted((start, face[(index + 1) % 4])))] += 1
    manifold = len(valid_faces) == len(faces) and bool(edges) and all(count == 2 for count in edges.values())
    if not manifold:
        failures.append(_failure("BODY-PLAN-MANIFOLD-1", "mesh"))

    cell_records = body.get("cells", [])
    cells = {}
    records_valid = True
    for record in cell_records:
        try:
            position = tuple(record["position"])
            module = record["module"]
            records_valid = records_valid and len(position) == 3 and all(isinstance(value, int) for value in position) and module in node_ids and position not in cells
            cells[position] = module
        except (KeyError, TypeError):
            records_valid = False
    components = _components(cells) if cells else 0
    expected_boundary_faces = sum(
        1
        for cell in cells
        for step in STEPS.values()
        if tuple(cell[axis] + step[axis] for axis in range(3)) not in cells
    )
    if not records_valid or components != 1 or expected_boundary_faces != len(faces):
        failures.append(_failure("BODY-PLAN-CONNECTIVITY-1", "cells"))

    connections = {(item.get("parent"), item.get("child")): item for item in body.get("connections", [])}
    contacts_valid = len(connections) == len(plan["nodes"]) - 1
    for node in plan["nodes"]:
        if "parent" not in node:
            continue
        step = STEPS[node["attach"]]
        opposite = tuple(-value for value in step)
        actual = sum(
            1 for cell, module in cells.items()
            if module == node["id"]
            and cells.get(tuple(cell[axis] + opposite[axis] for axis in range(3))) == node["parent"]
        )
        receipt = connections.get((node["parent"], node["id"]), {})
        contacts_valid = contacts_valid and actual > 0 and receipt.get("attach") == node["attach"] and receipt.get("contact_faces") == actual
    if not contacts_valid:
        failures.append(_failure("BODY-PLAN-CONTACT-1", "connections"))

    mirror_errors = []
    for node in plan["nodes"]:
        if "mirror_of" not in node:
            continue
        source = {cell for cell, module in cells.items() if module == node["mirror_of"]}
        target = {cell for cell, module in cells.items() if module == node["id"]}
        mirror_errors.append(max(_mirror_error(source, target), _mirror_error(target, source)))
    max_mirror_error = max(mirror_errors, default=float("inf"))
    if not math.isfinite(max_mirror_error) or max_mirror_error > 0:
        failures.append(_failure("BODY-PLAN-MIRROR-1", "declared pairs"))

    regions = body.get("regions", [])
    semantics_valid = (
        len(body.get("placements", {})) == len(node_ids)
        and set(body.get("placements", {})) == node_ids
        and len(body.get("face_modules", [])) == len(faces)
        and all(module in node_ids for module in body.get("face_modules", []))
        and len(body.get("vertex_modules", [])) == len(vertices)
        and all(set(label.split("|")) <= node_ids for label in body.get("vertex_modules", []))
        and len(regions) == len(vertices)
        and all(
            set(weights) == {"core", "head", "limb"}
            and all(isinstance(value, (int, float)) and math.isfinite(value) and 0 <= value <= 1 for value in weights.values())
            and abs(sum(weights.values()) - 1.0) <= tolerance
            for weights in regions
        )
    )
    if not semantics_valid:
        failures.append(_failure("BODY-PLAN-SEMANTIC-1", "labels"))

    width = max((vertex[0] for vertex in vertices), default=float("nan")) - min((vertex[0] for vertex in vertices), default=float("nan"))
    height = max((vertex[2] for vertex in vertices), default=float("nan")) - min((vertex[2] for vertex in vertices), default=float("nan"))
    return {
        "schema": "body-plan-verification/0",
        "passed": not failures,
        "failures": failures,
        "metrics": {
            "vertices": len(vertices),
            "faces": len(faces),
            "cells": len(cells),
            "connected_components": components,
            "manifold_edges": len(edges),
            "max_mirror_error_cells": max_mirror_error,
            "width_m": width,
            "height_m": height,
        },
    }
