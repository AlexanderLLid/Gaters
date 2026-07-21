import math
from collections import Counter


def _failure(rule, subject):
    return {"rule": rule, "subject": subject}


def _dot(a, b):
    return sum(x * y for x, y in zip(a, b))


def _length(vector):
    return math.sqrt(_dot(vector, vector))


def _triangle_area(a, b, c):
    ab = [b[index] - a[index] for index in range(3)]
    ac = [c[index] - a[index] for index in range(3)]
    cross = [
        ab[1] * ac[2] - ab[2] * ac[1],
        ab[2] * ac[0] - ab[0] * ac[2],
        ab[0] * ac[1] - ab[1] * ac[0],
    ]
    return _length(cross) / 2


def verify_mirrored_branches(parent, recipe, composed, tolerance=1e-9):
    failures = []
    vertices = composed.get("vertices", [])
    faces = composed.get("faces", [])
    socket = composed.get("socket", {})

    if composed.get("schema") != "composed-character-mesh/0" or composed.get("socket_id") != recipe["id"]:
        failures.append(_failure("BRANCH-SCHEMA-1", "composition"))

    if vertices[: len(parent["vertices"])] != parent["vertices"] or faces[: len(parent["faces"])] != parent["faces"]:
        failures.append(_failure("BRANCH-PRESERVE-1", "parent"))

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
    if len(valid_faces) != len(faces) or not edges or any(count != 2 for count in edges.values()):
        failures.append(_failure("BRANCH-MANIFOLD-1", "mesh"))

    expected_interfaces = {item["name"]: item["interface"] for item in parent["sockets"]}
    if socket.get("interfaces") != expected_interfaces:
        failures.append(_failure("BRANCH-SEAM-1", "interfaces"))

    expected_frames = {
        item["name"]: {key: item[key] for key in ("origin", "tangent", "u", "v")}
        for item in parent["sockets"]
    }
    frames = socket.get("frames", {})
    frames_valid = frames == expected_frames
    for frame in frames.values() if isinstance(frames, dict) else []:
        axes = [frame.get(name, []) for name in ("tangent", "u", "v")]
        frames_valid = frames_valid and all(abs(_length(axis) - 1.0) <= tolerance for axis in axes)
        frames_valid = frames_valid and all(abs(_dot(axes[a], axes[b])) <= tolerance for a, b in ((0, 1), (0, 2), (1, 2)))
    if not frames_valid:
        failures.append(_failure("BRANCH-FRAME-1", "frames"))

    mirror_errors = []
    for pair in socket.get("mirror_pairs", []):
        if not isinstance(pair, list) and not isinstance(pair, tuple):
            continue
        if len(pair) != 2 or any(not isinstance(index, int) or index < 0 or index >= len(vertices) for index in pair):
            continue
        left, right = (vertices[index] for index in pair)
        mirror_errors.append(math.dist(left, [-right[0], right[1], right[2]]))
    expected_pair_count = 4 + recipe["branch_rings"] * 4 + 1
    max_mirror_error = max(mirror_errors, default=float("inf"))
    if len(mirror_errors) != expected_pair_count or not math.isfinite(max_mirror_error) or max_mirror_error > tolerance:
        failures.append(_failure("BRANCH-MIRROR-1", "branches"))

    realized_lengths = []
    caps = socket.get("caps", {})
    if frames_valid and set(caps) == {"left", "right"}:
        for name in ("left", "right"):
            cap = vertices[caps[name]]
            frame = frames[name]
            delta = [cap[axis] - frame["origin"][axis] for axis in range(3)]
            realized_lengths.append(_dot(delta, frame["tangent"]))
    max_length_error = max((abs(value - recipe["branch_length_m"]) for value in realized_lengths), default=float("inf"))
    if len(realized_lengths) != 2 or not math.isfinite(max_length_error) or max_length_error > tolerance:
        failures.append(_failure("BRANCH-LENGTH-1", "branches"))

    regions = composed.get("regions", [])
    semantics_valid = (
        len(regions) == len(vertices)
        and all(
            set(weights) == {"core", "branch"}
            and all(math.isfinite(value) and 0 <= value <= 1 for value in weights.values())
            and abs(sum(weights.values()) - 1.0) <= tolerance
            for weights in regions
        )
        and len(composed.get("vertex_modules", [])) == len(vertices)
        and len(composed.get("face_modules", [])) == len(faces)
    )
    if not semantics_valid:
        failures.append(_failure("BRANCH-SEMANTIC-1", "labels"))

    return {
        "schema": "mirrored-branch-verification/0",
        "passed": not failures,
        "failures": failures,
        "metrics": {
            "vertices": len(vertices),
            "faces": len(faces),
            "manifold_edges": len(edges),
            "max_mirror_error_m": max_mirror_error,
            "max_length_error_m": max_length_error,
        },
    }
