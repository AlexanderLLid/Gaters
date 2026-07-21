import math
from collections import Counter


DIRECTIONS = {
    "-x": ((-1, 0, 0), (-1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, 1.0)),
    "+x": ((1, 0, 0), (1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, 1.0)),
    "-y": ((0, -1, 0), (0.0, -1.0, 0.0), (1.0, 0.0, 0.0), (0.0, 0.0, 1.0)),
    "+y": ((0, 1, 0), (0.0, 1.0, 0.0), (1.0, 0.0, 0.0), (0.0, 0.0, 1.0)),
    "-z": ((0, 0, -1), (0.0, 0.0, -1.0), (1.0, 0.0, 0.0), (0.0, 1.0, 0.0)),
    "+z": ((0, 0, 1), (0.0, 0.0, 1.0), (1.0, 0.0, 0.0), (0.0, 1.0, 0.0)),
}


def _valid_size(value):
    return isinstance(value, list) and len(value) == 3 and all(isinstance(item, int) and item > 0 for item in value)


def _centered_start(minimum, maximum, size, offset):
    return minimum + (maximum - minimum - size) // 2 + offset


def _place_child(parent_bounds, node):
    minimum, maximum = parent_bounds
    sx, sy, sz = node["size_cells"]
    first, second = node.get("offset_cells", [0, 0])
    attach = node["attach"]
    if attach in ("-x", "+x"):
        y = _centered_start(minimum[1], maximum[1], sy, first)
        z = _centered_start(minimum[2], maximum[2], sz, second)
        x = maximum[0] if attach == "+x" else minimum[0] - sx
        return [x, y, z], [x + sx, y + sy, z + sz]
    if attach in ("-y", "+y"):
        x = _centered_start(minimum[0], maximum[0], sx, first)
        z = _centered_start(minimum[2], maximum[2], sz, second)
        y = maximum[1] if attach == "+y" else minimum[1] - sy
        return [x, y, z], [x + sx, y + sy, z + sz]
    x = _centered_start(minimum[0], maximum[0], sx, first)
    y = _centered_start(minimum[1], maximum[1], sy, second)
    z = maximum[2] if attach == "+z" else minimum[2] - sz
    return [x, y, z], [x + sx, y + sy, z + sz]


def _cells(bounds):
    minimum, maximum = bounds
    for x in range(minimum[0], maximum[0]):
        for y in range(minimum[1], maximum[1]):
            for z in range(minimum[2], maximum[2]):
                yield (x, y, z)


def _face_corners(cell, direction):
    x, y, z = cell
    return {
        "-x": [(x, y, z), (x, y, z + 1), (x, y + 1, z + 1), (x, y + 1, z)],
        "+x": [(x + 1, y, z), (x + 1, y + 1, z), (x + 1, y + 1, z + 1), (x + 1, y, z + 1)],
        "-y": [(x, y, z), (x + 1, y, z), (x + 1, y, z + 1), (x, y, z + 1)],
        "+y": [(x, y + 1, z), (x, y + 1, z + 1), (x + 1, y + 1, z + 1), (x + 1, y + 1, z)],
        "-z": [(x, y, z), (x, y + 1, z), (x + 1, y + 1, z), (x + 1, y, z)],
        "+z": [(x, y, z + 1), (x + 1, y, z + 1), (x + 1, y + 1, z + 1), (x, y + 1, z + 1)],
    }[direction]


def compile_body_plan(plan):
    nodes = plan.get("nodes", [])
    valid = (
        plan.get("schema") == "body-plan/0"
        and isinstance(plan.get("cell_size_m"), (int, float))
        and math.isfinite(plan["cell_size_m"])
        and plan["cell_size_m"] > 0
        and len(nodes) >= 2
        and len({node.get("id") for node in nodes}) == len(nodes)
        and all(_valid_size(node.get("size_cells")) and node.get("role") in {"core", "head", "limb"} for node in nodes)
    )
    roots = [node for node in nodes if "parent" not in node]
    if not valid or len(roots) != 1:
        raise ValueError("BODY-PLAN-INPUT-1")

    placements = {}
    occupied = {}
    node_by_id = {node["id"]: node for node in nodes}
    for node in nodes:
        if "parent" not in node:
            sx, sy, sz = node["size_cells"]
            minimum = [-sx // 2, -sy // 2, 0]
            bounds = [minimum, [minimum[0] + sx, minimum[1] + sy, sz]]
        else:
            if node.get("parent") not in placements or node.get("attach") not in DIRECTIONS:
                raise ValueError("BODY-PLAN-INPUT-1")
            bounds = _place_child(placements[node["parent"]]["bounds_cells"], node)
        module_cells = list(_cells(bounds))
        if any(cell in occupied for cell in module_cells):
            raise ValueError("BODY-PLAN-OVERLAP-1")
        for cell in module_cells:
            occupied[cell] = node["id"]
        placements[node["id"]] = {"role": node["role"], "bounds_cells": bounds}

    connections = []
    for node in nodes:
        if "parent" not in node:
            continue
        step, tangent, u, v = DIRECTIONS[node["attach"]]
        opposite = tuple(-value for value in step)
        child_cells = [cell for cell, module in occupied.items() if module == node["id"]]
        contacts = [cell for cell in child_cells if occupied.get(tuple(cell[axis] + opposite[axis] for axis in range(3))) == node["parent"]]
        if not contacts:
            raise ValueError("BODY-PLAN-CONTACT-1")
        centers = [[cell[axis] + 0.5 + opposite[axis] * 0.5 for axis in range(3)] for cell in contacts]
        origin = [round(sum(center[axis] for center in centers) / len(centers) * plan["cell_size_m"], 12) for axis in range(3)]
        connections.append({
            "parent": node["parent"],
            "child": node["id"],
            "attach": node["attach"],
            "contact_faces": len(contacts),
            "frame": {"origin": origin, "tangent": list(tangent), "u": list(u), "v": list(v)},
        })

    vertex_ids = {}
    grid_vertices = []
    faces = []
    face_modules = []
    incident_modules = {}
    for cell, module in sorted(occupied.items()):
        for direction, (step, _, _, _) in DIRECTIONS.items():
            neighbor = tuple(cell[axis] + step[axis] for axis in range(3))
            if neighbor in occupied:
                continue
            face = []
            for corner in _face_corners(cell, direction):
                if corner not in vertex_ids:
                    vertex_ids[corner] = len(grid_vertices)
                    grid_vertices.append(corner)
                index = vertex_ids[corner]
                face.append(index)
                incident_modules.setdefault(index, []).append(module)
            faces.append(face)
            face_modules.append(module)

    roles = {node["id"]: node["role"] for node in nodes}
    vertex_modules = ["|".join(sorted(set(incident_modules[index]))) for index in range(len(grid_vertices))]
    regions = []
    for index in range(len(grid_vertices)):
        counts = Counter(roles[module] for module in incident_modules[index])
        total = sum(counts.values())
        regions.append({role: counts.get(role, 0) / total for role in ("core", "head", "limb")})
    scale = plan["cell_size_m"]
    vertices = [[round(value * scale, 12) for value in vertex] for vertex in grid_vertices]
    return {
        "schema": "body-plan-mesh/0",
        "body_plan_id": plan["id"],
        "cell_size_m": scale,
        "vertices": vertices,
        "faces": faces,
        "regions": regions,
        "vertex_modules": vertex_modules,
        "face_modules": face_modules,
        "placements": placements,
        "connections": connections,
        "cells": [{"position": list(cell), "module": module} for cell, module in sorted(occupied.items())],
        "mirrors": [
            {"source": node["mirror_of"], "target": node["id"], "plane": "x=0"}
            for node in nodes if "mirror_of" in node
        ],
    }
