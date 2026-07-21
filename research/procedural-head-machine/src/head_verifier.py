import math
from collections import Counter


REGIONS = {"skull", "face", "jaw", "chin"}


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


def verify(recipe, mesh):
    failures = []
    if mesh.get("schema") != "head-mesh/0" or mesh.get("recipe_id") != str(recipe.get("id")):
        failures.append(_failure("HEAD-SCHEMA-1", "mesh"))

    vertices = mesh.get("vertices", [])
    faces = mesh.get("faces", [])
    regions = mesh.get("regions", [])
    finite = all(
        isinstance(vertex, list)
        and len(vertex) == 3
        and all(isinstance(value, (int, float)) and math.isfinite(value) for value in vertex)
        for vertex in vertices
    )
    if not finite:
        failures.append(_failure("HEAD-FINITE-1", "vertices"))

    valid_faces = []
    for index, face in enumerate(faces):
        valid = (
            isinstance(face, list)
            and len(face) in (3, 4)
            and len(face) == len(set(face))
            and all(isinstance(vertex_id, int) and 0 <= vertex_id < len(vertices) for vertex_id in face)
        )
        if valid and finite:
            area = _triangle_area(vertices[face[0]], vertices[face[1]], vertices[face[2]])
            if len(face) == 4:
                area += _triangle_area(vertices[face[0]], vertices[face[2]], vertices[face[3]])
            valid = area > 1e-14
        if valid:
            valid_faces.append(face)
        else:
            failures.append(_failure("HEAD-FACE-1", index))

    edge_counts = Counter()
    for face in valid_faces:
        for index, start in enumerate(face):
            edge_counts[tuple(sorted((start, face[(index + 1) % len(face)])))] += 1
    if len(valid_faces) != len(faces) or not edge_counts or any(count != 2 for count in edge_counts.values()):
        failures.append(_failure("HEAD-MANIFOLD-1", "edges"))

    regions_valid = len(regions) == len(vertices)
    if regions_valid:
        for weights in regions:
            regions_valid = (
                isinstance(weights, dict)
                and set(weights) == REGIONS
                and all(isinstance(value, (int, float)) and math.isfinite(value) and 0 <= value <= 1 for value in weights.values())
                and abs(sum(weights.values()) - 1.0) <= 1e-9
            )
            if not regions_valid:
                break
    if not regions_valid:
        failures.append(_failure("HEAD-REGION-1", "regions"))

    realized = None
    if finite and vertices:
        axes = list(zip(*vertices))
        realized = [max(axis) - min(axis) for axis in axes]
        declared = [float(recipe["width_m"]), float(recipe["depth_m"]), float(recipe["height_m"])]
        if any(abs(actual - expected) > 1e-9 for actual, expected in zip(realized, declared)):
            failures.append(_failure("HEAD-BOUNDS-1", "dimensions"))

    return {
        "schema": "head-verification/0",
        "passed": not failures,
        "failures": failures,
        "metrics": {
            "vertices": len(vertices),
            "faces": len(faces),
            "realized_dimensions_m": realized,
        },
    }
