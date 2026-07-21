import math


def _sub(a, b):
    return [a[index] - b[index] for index in range(3)]


def _dot(a, b):
    return sum(left * right for left, right in zip(a, b))


def _cross(a, b):
    return [
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0],
    ]


def _normalize(vector):
    length = math.sqrt(_dot(vector, vector))
    if length <= 1e-12:
        raise ValueError("SKELETON-FRAME-1")
    return [value / length for value in vector]


def _basis(aim, preferred_up):
    aim = _normalize(aim)
    projected_up = [preferred_up[index] - _dot(preferred_up, aim) * aim[index] for index in range(3)]
    if _dot(projected_up, projected_up) <= 1e-12:
        fallback = [1.0, 0.0, 0.0] if abs(aim[0]) < 0.9 else [0.0, 0.0, 1.0]
        projected_up = [fallback[index] - _dot(fallback, aim) * aim[index] for index in range(3)]
    up = _normalize(projected_up)
    side = _normalize(_cross(aim, up))
    return {name: [round(value, 12) for value in axis] for name, axis in (("aim", aim), ("up", up), ("side", side))}


def compile_guide_skeleton(guide, recipe, source_guide_sha256):
    if guide.get("schema") != "anatomical-guide/0" or recipe.get("schema") != "guide-skeleton-recipe/0":
        raise ValueError("SKELETON-INPUT-1")
    required = set(recipe.get("required_joints", []))
    root = recipe.get("root_joint")
    landmarks = guide.get("landmarks", {})
    if not required or root not in required or not required.issubset(landmarks):
        raise ValueError("SKELETON-GRAPH-1")

    parents = {root: None}
    children = {name: [] for name in required}
    segment_ids = {}
    for segment in guide.get("skeleton_segments", []):
        start, end = segment.get("start"), segment.get("end")
        if start not in required or end not in required or end in parents:
            raise ValueError("SKELETON-GRAPH-1")
        parents[end] = start
        children[start].append(end)
        segment_ids[(start, end)] = segment["id"]
    if set(parents) != required:
        raise ValueError("SKELETON-GRAPH-1")

    order = []
    queue = [root]
    while queue:
        current = queue.pop(0)
        order.append(current)
        queue.extend(children[current])
    if set(order) != required:
        raise ValueError("SKELETON-GRAPH-1")

    preferred = recipe.get("primary_children", {})
    joints = []
    for name in order:
        child = preferred.get(name)
        if child is not None and child not in children[name]:
            raise ValueError("SKELETON-GRAPH-1")
        if child is None and children[name]:
            child = sorted(children[name])[0]
        if child is not None:
            aim = _sub(landmarks[child], landmarks[name])
        elif parents[name] is not None:
            aim = _sub(landmarks[name], landmarks[parents[name]])
        else:
            raise ValueError("SKELETON-FRAME-1")
        joints.append({
            "name": name,
            "parent": parents[name],
            "position": landmarks[name],
            "basis": _basis(aim, recipe["orientation_up_axis"]),
        })

    return {
        "schema": "guide-skeleton/0",
        "id": recipe["id"],
        "guide_id": guide["id"],
        "source_guide_sha256": source_guide_sha256,
        "root_joint": root,
        "joints": joints,
        "bones": [
            {"id": segment_ids[(parent, child)], "parent": parent, "child": child}
            for child, parent in parents.items() if parent is not None
        ],
        "symmetry_pairs": [pair for pair in guide["symmetry_pairs"] if pair[0] in required and pair[1] in required],
    }
