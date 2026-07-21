"""Independent mathematical checks for raw AnatomyGraph data."""

from __future__ import annotations

import hashlib
import json
import math


def verify(recipe: dict, graph: dict) -> dict:
    failures = []

    def fail(rule: str, subject: str) -> None:
        item = {"rule": rule, "subject": subject}
        if item not in failures:
            failures.append(item)

    _check_schema(recipe, graph, fail)
    _check_names(graph, fail)
    joints = {joint.get("id"): joint for joint in graph.get("joints", []) if isinstance(joint, dict)}
    _check_parents(graph, joints, fail)
    _check_lengths(graph, joints, fail)
    _check_bounds(recipe, graph, fail)
    _check_connectivity(graph, joints, fail)
    _check_counts(recipe, graph, fail)
    _check_mirrors(graph, fail)
    _check_tapers(recipe, joints, fail)

    failures.sort(key=lambda item: (item["rule"], item["subject"]))
    return {
        "passed": not failures,
        "failures": failures,
        "metrics": {
            "module_count": len(graph.get("modules", [])),
            "joint_count": len(graph.get("joints", [])),
            "bone_count": len(graph.get("bones", [])),
            "failure_count": len(failures),
        },
    }


def _check_schema(recipe, graph, fail) -> None:
    if recipe.get("schema") != "creature-dna/0" or graph.get("schema") != "anatomy-graph/0":
        fail("GRAPH-SCHEMA-1", "schema")
    expected_hash = hashlib.sha256(
        json.dumps(recipe, sort_keys=True, separators=(",", ":"), ensure_ascii=False).encode("utf-8")
    ).hexdigest()
    identity = graph.get("recipe", {})
    if identity.get("id") != recipe.get("id") or identity.get("sha256") != expected_hash or identity.get("seed") != recipe.get("seed"):
        fail("GRAPH-SCHEMA-1", "recipe")


def _check_names(graph, fail) -> None:
    for collection in ("modules", "joints", "bones", "sockets"):
        identifiers = [item.get("id") for item in graph.get(collection, []) if isinstance(item, dict)]
        if any(not isinstance(identifier, str) or not identifier for identifier in identifiers):
            fail("GRAPH-NAME-1", collection)
        duplicates = {identifier for identifier in identifiers if identifiers.count(identifier) > 1}
        for identifier in duplicates:
            fail("GRAPH-NAME-1", identifier)


def _check_parents(graph, joints, fail) -> None:
    for bone in graph.get("bones", []):
        if bone.get("parent_joint") not in joints:
            fail("GRAPH-PARENT-1", str(bone.get("parent_joint")))
        if bone.get("child_joint") not in joints:
            fail("GRAPH-PARENT-1", str(bone.get("child_joint")))
    for socket in graph.get("sockets", []):
        if socket.get("joint") not in joints:
            fail("GRAPH-PARENT-1", str(socket.get("joint")))
    for contact in graph.get("intent", {}).get("contacts", []):
        if contact not in joints:
            fail("GRAPH-PARENT-1", str(contact))


def _check_lengths(graph, joints, fail) -> None:
    for bone in graph.get("bones", []):
        parent = joints.get(bone.get("parent_joint"))
        child = joints.get(bone.get("child_joint"))
        if parent is None or child is None:
            continue
        first = parent.get("position_m", [])
        second = child.get("position_m", [])
        if not _vector(first) or not _vector(second):
            fail("GRAPH-LENGTH-1", str(bone.get("id")))
            continue
        distance = math.dist(first, second)
        if not math.isfinite(distance) or distance <= 1e-6:
            fail("GRAPH-LENGTH-1", str(bone.get("id")))


def _check_bounds(recipe, graph, fail) -> None:
    dimensions = []
    for module in recipe.get("modules", []):
        for key in ("length", "radius", "base_radius", "tip_radius"):
            value = module.get(key)
            if _number(value) and value > 0:
                dimensions.append(float(value))
    bound = max(1.0, sum(dimensions) * 2.0)
    for joint in graph.get("joints", []):
        position = joint.get("position_m", [])
        radius = joint.get("radius_m")
        if not _vector(position) or any(abs(value) > bound for value in position):
            fail("GRAPH-BOUNDS-1", str(joint.get("id")))
        if not _number(radius) or radius <= 0.0 or radius > bound:
            fail("GRAPH-BOUNDS-1", str(joint.get("id")))


def _check_connectivity(graph, joints, fail) -> None:
    if not joints:
        fail("GRAPH-CONNECT-1", "root")
        return
    root = next((joint["id"] for joint in graph.get("joints", []) if joint.get("role") == "root"), None)
    if root is None:
        fail("GRAPH-CONNECT-1", "root")
        return
    edges = {joint_id: set() for joint_id in joints}
    for bone in graph.get("bones", []):
        parent = bone.get("parent_joint")
        child = bone.get("child_joint")
        if parent in edges and child in edges:
            edges[parent].add(child)
            edges[child].add(parent)
    reached = {root}
    frontier = [root]
    while frontier:
        current = frontier.pop()
        for neighbor in edges[current] - reached:
            reached.add(neighbor)
            frontier.append(neighbor)
    for joint_id in joints.keys() - reached:
        fail("GRAPH-CONNECT-1", joint_id)


def _check_counts(recipe, graph, fail) -> None:
    expected = {}
    for module in recipe.get("modules", []):
        module_type = module.get("type")
        expected[module_type] = expected.get(module_type, 0) + (module.get("count", 1) if module_type in {"leg", "wing"} else 1)
    actual = {}
    for module in graph.get("modules", []):
        module_type = module.get("type")
        actual[module_type] = actual.get(module_type, 0) + 1
    for module_type in sorted(set(expected) | set(actual)):
        if expected.get(module_type, 0) != actual.get(module_type, 0):
            fail("GRAPH-COUNT-1", str(module_type))


def _check_mirrors(graph, fail) -> None:
    modules = {module.get("id"): module for module in graph.get("modules", [])}
    joints_by_module = {}
    for joint in graph.get("joints", []):
        joints_by_module.setdefault(joint.get("module"), {})[joint.get("role")] = joint
    for pair in graph.get("intent", {}).get("bilateral_pairs", []):
        if not isinstance(pair, list) or len(pair) != 2 or pair[0] not in modules or pair[1] not in modules:
            fail("GRAPH-MIRROR-1", str(pair))
            continue
        left = joints_by_module.get(pair[0], {})
        right = joints_by_module.get(pair[1], {})
        if set(left) != set(right):
            fail("GRAPH-MIRROR-1", "/".join(pair))
            continue
        for role in left:
            a = left[role].get("position_m", [])
            b = right[role].get("position_m", [])
            if not _vector(a) or not _vector(b):
                fail("GRAPH-MIRROR-1", f"{'/'.join(pair)}.{role}")
                continue
            mirrored = abs(a[0] - b[0]) <= 1e-5 and abs(a[1] + b[1]) <= 1e-5 and abs(a[2] - b[2]) <= 1e-5
            same_radius = abs(float(left[role].get("radius_m", math.inf)) - float(right[role].get("radius_m", -math.inf))) <= 1e-5
            if not mirrored or not same_radius:
                fail("GRAPH-MIRROR-1", f"{'/'.join(pair)}.{role}")


def _check_tapers(recipe, joints, fail) -> None:
    for module in recipe.get("modules", []):
        if module.get("type") != "tapered_head":
            continue
        module_id = module.get("id")
        radii = []
        for role in ("base", "mid", "tip"):
            joint = joints.get(f"{module_id}.{role}")
            if joint is None or not _number(joint.get("radius_m")):
                fail("GRAPH-TAPER-1", str(module_id))
                break
            radii.append(float(joint["radius_m"]))
        else:
            if not radii[0] > radii[1] > radii[2] > 0.0:
                fail("GRAPH-TAPER-1", str(module_id))


def _number(value) -> bool:
    return not isinstance(value, bool) and isinstance(value, (int, float)) and math.isfinite(value)


def _vector(value) -> bool:
    return isinstance(value, list) and len(value) == 3 and all(_number(item) for item in value)
