import math
from collections import Counter


REGIONS = {"skull", "face", "jaw", "chin", "neck"}


def _failure(rule, subject):
    return {"rule": rule, "subject": subject}


def _distance(a, b):
    return math.dist(a, b)


def _triangle_area(a, b, c):
    ab = [b[index] - a[index] for index in range(3)]
    ac = [c[index] - a[index] for index in range(3)]
    cross = [
        ab[1] * ac[2] - ab[2] * ac[1],
        ab[2] * ac[0] - ab[0] * ac[2],
        ab[0] * ac[1] - ab[1] * ac[0],
    ]
    return math.sqrt(sum(value * value for value in cross)) / 2


def verify_socket(source_mesh, socket, composed, tolerance=1e-9):
    failures = []
    vertices = composed.get("vertices", [])
    faces = composed.get("faces", [])
    interface = socket["interface_vertices"]
    previous = socket["previous_vertices"]
    socket_result = composed.get("socket", {})
    first = socket_result.get("first_neck_ring", [])
    last = socket_result.get("last_neck_ring", [])

    if composed.get("schema") != "composed-character-mesh/0" or composed.get("socket_id") != socket["id"]:
        failures.append(_failure("SOCKET-SCHEMA-1", "composition"))

    interface_end = max(interface)
    expected_faces = [
        face for face in source_mesh["faces"]
        if all(vertex <= interface_end for vertex in face)
    ]
    preserved = (
        vertices[: interface_end + 1] == source_mesh["vertices"][: interface_end + 1]
        and faces[: len(expected_faces)] == expected_faces
    )
    if not preserved:
        failures.append(_failure("SOCKET-PRESERVE-1", "head"))

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
            and len(face) in (3, 4)
            and len(face) == len(set(face))
            and all(isinstance(index, int) and 0 <= index < len(vertices) for index in face)
        )
        if valid and finite:
            area = _triangle_area(vertices[face[0]], vertices[face[1]], vertices[face[2]])
            if len(face) == 4:
                area += _triangle_area(vertices[face[0]], vertices[face[2]], vertices[face[3]])
            valid = area > 1e-14
        if valid:
            valid_faces.append(face)
    edges = Counter()
    for face in valid_faces:
        for index, start in enumerate(face):
            edges[tuple(sorted((start, face[(index + 1) % len(face)])))] += 1
    manifold = len(valid_faces) == len(faces) and bool(edges) and all(count == 2 for count in edges.values())
    if not manifold:
        failures.append(_failure("SOCKET-MANIFOLD-1", "mesh"))

    seam_valid = (
        socket_result.get("interface_vertices") == interface
        and len(first) == len(interface)
        and all(_distance(vertices[seam], vertices[next_ring]) > tolerance for seam, next_ring in zip(interface, first))
    )
    if not seam_valid:
        failures.append(_failure("SOCKET-SEAM-1", "interface"))

    tangent_angles = []
    if seam_valid:
        for before_id, seam_id, after_id in zip(previous, interface, first):
            incoming = [vertices[seam_id][axis] - vertices[before_id][axis] for axis in range(3)]
            outgoing = [vertices[after_id][axis] - vertices[seam_id][axis] for axis in range(3)]
            incoming_length = math.sqrt(sum(value * value for value in incoming))
            outgoing_length = math.sqrt(sum(value * value for value in outgoing))
            cosine = sum(a * b for a, b in zip(incoming, outgoing)) / (incoming_length * outgoing_length)
            tangent_angles.append(math.acos(max(-1.0, min(1.0, cosine))))
    max_tangent_angle = max(tangent_angles, default=float("inf"))
    if not math.isfinite(max_tangent_angle) or max_tangent_angle > 1e-7:
        failures.append(_failure("SOCKET-TANGENT-1", "interface"))

    realized_length = float("inf")
    if len(last) == len(interface):
        interface_z = sum(vertices[index][2] for index in interface) / len(interface)
        last_z = sum(vertices[index][2] for index in last) / len(last)
        realized_length = interface_z - last_z
    if not math.isfinite(realized_length) or abs(realized_length - socket["neck_length_m"]) > tolerance:
        failures.append(_failure("SOCKET-LENGTH-1", "neck"))

    regions = composed.get("regions", [])
    regions_valid = len(regions) == len(vertices) and all(
        set(weights) == REGIONS
        and all(math.isfinite(value) and 0 <= value <= 1 for value in weights.values())
        and abs(sum(weights.values()) - 1.0) <= tolerance
        for weights in regions
    )
    modules_valid = (
        len(composed.get("vertex_modules", [])) == len(vertices)
        and len(composed.get("face_modules", [])) == len(faces)
    )
    if not regions_valid or not modules_valid:
        failures.append(_failure("SOCKET-SEMANTIC-1", "labels"))

    return {
        "schema": "module-socket-verification/0",
        "passed": not failures,
        "failures": failures,
        "metrics": {
            "vertices": len(vertices),
            "faces": len(faces),
            "manifold_edges": len(edges),
            "max_tangent_angle_rad": max_tangent_angle,
            "realized_neck_length_m": realized_length,
        },
    }
