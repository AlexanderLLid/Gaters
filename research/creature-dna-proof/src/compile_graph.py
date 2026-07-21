"""Compile a small CreatureDNA recipe into a tool-neutral anatomy graph."""

from __future__ import annotations

import hashlib
import json
import math
from pathlib import Path


SCHEMA = "creature-dna/0"
GRAPH_SCHEMA = "anatomy-graph/0"
COMPILER_VERSION = "creature-dna-proof/0"
MODULE_TYPES = {"torso", "tapered_head", "tail", "leg", "wing"}


def load_recipe(path: Path) -> dict:
    with Path(path).open("r", encoding="utf-8") as handle:
        return json.load(handle)


def canonical_bytes(graph: dict) -> bytes:
    stable = json.loads(json.dumps(graph))
    stable.pop("toolchain", None)
    return json.dumps(stable, sort_keys=True, separators=(",", ":"), ensure_ascii=False).encode("utf-8")


def graph_sha256(graph: dict) -> str:
    return hashlib.sha256(canonical_bytes(graph)).hexdigest()


def compile_recipe(recipe: dict) -> dict:
    _validate_recipe_header(recipe)
    recipe_hash = hashlib.sha256(
        json.dumps(recipe, sort_keys=True, separators=(",", ":"), ensure_ascii=False).encode("utf-8")
    ).hexdigest()
    graph = {
        "schema": GRAPH_SCHEMA,
        "recipe": {"id": recipe["id"], "sha256": recipe_hash, "seed": recipe["seed"]},
        "toolchain": {"compiler": COMPILER_VERSION, "houdini": None, "edition": None},
        "modules": [],
        "joints": [],
        "bones": [],
        "sockets": [],
        "intent": {"bilateral_pairs": [], "contacts": []},
    }

    module_ids = set()
    for module in recipe["modules"]:
        module_id = _required_text(module, "id")
        module_type = _required_text(module, "type")
        if module_id in module_ids:
            raise ValueError(f"duplicate module id: {module_id}")
        if module_type not in MODULE_TYPES:
            raise ValueError(f"unsupported module type: {module_type}")
        module_ids.add(module_id)
        if module_type == "torso":
            _build_torso(graph, module)
        elif module_type == "tapered_head":
            _build_head(graph, module)
        elif module_type == "tail":
            _build_tail(graph, module)
        else:
            _build_appendages(graph, module)

    if not any(module["type"] == "torso" for module in graph["modules"]):
        raise ValueError("one torso module is required")
    return graph


def _validate_recipe_header(recipe: dict) -> None:
    if recipe.get("schema") != SCHEMA:
        raise ValueError(f"unsupported schema: {recipe.get('schema')}")
    _required_text(recipe, "id")
    if not isinstance(recipe.get("seed"), int) or isinstance(recipe.get("seed"), bool):
        raise ValueError("seed must be an integer")
    if recipe.get("units") != "m":
        raise ValueError("units must be m")
    if not isinstance(recipe.get("modules"), list) or not recipe["modules"]:
        raise ValueError("modules must be a non-empty list")


def _build_torso(graph: dict, module: dict) -> None:
    if any(item["type"] == "torso" for item in graph["modules"]):
        raise ValueError("only one torso module is supported")
    module_id = module["id"]
    length = _positive(module, "length")
    radius = _positive(module, "radius")
    graph["modules"].append({"id": module_id, "type": "torso", "instance": 0})
    _add_joint(graph, f"{module_id}.root", module_id, "root", (0.0, 0.0, 0.0), radius)
    _add_joint(graph, f"{module_id}.front", module_id, "front", (length / 2.0, 0.0, 0.0), radius * 0.8)
    _add_joint(graph, f"{module_id}.back", module_id, "back", (-length / 2.0, 0.0, 0.0), radius * 0.75)
    _add_bone(graph, f"{module_id}.root-front", f"{module_id}.root", f"{module_id}.front")
    _add_bone(graph, f"{module_id}.root-back", f"{module_id}.root", f"{module_id}.back")
    graph["sockets"].extend(
        [
            {"id": f"{module_id}.front", "joint": f"{module_id}.front", "accepts": ["tapered_head"]},
            {"id": f"{module_id}.back", "joint": f"{module_id}.back", "accepts": ["tail"]},
            {"id": f"{module_id}.root", "joint": f"{module_id}.root", "accepts": ["leg", "wing"]},
        ]
    )


def _build_head(graph: dict, module: dict) -> None:
    module_id = module["id"]
    parent = _parent_joint(graph, module)
    length = _positive(module, "length")
    base_radius = _positive(module, "base_radius")
    tip_radius = _positive(module, "tip_radius")
    if tip_radius >= base_radius:
        raise ValueError("tapered_head tip_radius must be less than base_radius")
    start = _joint_position(graph, parent)
    graph["modules"].append({"id": module_id, "type": "tapered_head", "instance": 0})
    stations = (
        ("base", 0.15, base_radius),
        ("mid", 0.6, base_radius + (tip_radius - base_radius) * 0.6),
        ("tip", 1.0, tip_radius),
    )
    previous = parent
    for role, fraction, radius in stations:
        joint_id = f"{module_id}.{role}"
        _add_joint(graph, joint_id, module_id, role, (start[0] + length * fraction, start[1], start[2]), radius)
        _add_bone(graph, f"{module_id}.{role}", previous, joint_id)
        previous = joint_id


def _build_tail(graph: dict, module: dict) -> None:
    module_id = module["id"]
    parent = _parent_joint(graph, module)
    length = _positive(module, "length")
    tip_radius = _positive(module, "tip_radius")
    segments = module.get("segments")
    if not isinstance(segments, int) or isinstance(segments, bool) or segments < 2:
        raise ValueError("tail segments must be an integer >= 2")
    start = _joint_position(graph, parent)
    base_radius = _joint(graph, parent)["radius_m"]
    graph["modules"].append({"id": module_id, "type": "tail", "instance": 0})
    previous = parent
    for index in range(segments):
        fraction = (index + 1) / segments
        joint_id = f"{module_id}.{index}"
        radius = base_radius + (tip_radius - base_radius) * fraction
        _add_joint(graph, joint_id, module_id, "tail", (start[0] - length * fraction, start[1], start[2]), radius)
        _add_bone(graph, f"{module_id}.{index}", previous, joint_id)
        previous = joint_id


def _build_appendages(graph: dict, module: dict) -> None:
    group_id = module["id"]
    module_type = module["type"]
    parent = _parent_joint(graph, module)
    length = _positive(module, "length")
    count = module.get("count")
    if not isinstance(count, int) or isinstance(count, bool) or count < 1:
        raise ValueError(f"{group_id} count must be a positive integer")
    placements = _placements(graph, module, count)

    instance_ids = []
    for index, placement in enumerate(placements):
        instance_id = f"{group_id}.{index}"
        instance_ids.append(instance_id)
        graph["modules"].append(
            {
                "id": instance_id,
                "type": module_type,
                "instance": index,
                "group": group_id,
                "attachment_id": placement["id"],
                "side": placement["side"],
            }
        )
        if module_type == "leg":
            _build_leg(graph, instance_id, parent, placement, length)
        else:
            _build_wing(graph, instance_id, parent, placement, length)

    if module.get("layout") == "bilateral":
        for index in range(0, len(instance_ids), 2):
            graph["intent"]["bilateral_pairs"].append([instance_ids[index], instance_ids[index + 1]])


def _placements(graph: dict, module: dict, count: int) -> list[dict]:
    layout = module.get("layout")
    if layout == "explicit":
        attachments = module.get("attachments")
        if not isinstance(attachments, list) or len(attachments) != count:
            raise ValueError("explicit attachments must match count")
        placements = []
        for attachment in attachments:
            position = attachment.get("position")
            if not isinstance(position, list) or len(position) != 3:
                raise ValueError("attachment position must contain three numbers")
            placements.append(
                {
                    "id": _required_text(attachment, "id"),
                    "position": tuple(_finite(value, "attachment position") for value in position),
                    "side": _required_text(attachment, "side"),
                }
            )
        return placements
    if layout != "bilateral":
        raise ValueError("appendage layout must be bilateral or explicit")
    if count % 2:
        raise ValueError("bilateral count must be even")

    torso = next((item for item in graph["modules"] if item["type"] == "torso"), None)
    if torso is None:
        raise ValueError("appendages require a compiled torso")
    root = _joint(graph, f"{torso['id']}.root")
    front = _joint_position(graph, f"{torso['id']}.front")
    back = _joint_position(graph, f"{torso['id']}.back")
    half_length = (front[0] - back[0]) / 2.0
    pair_count = count // 2
    xs = [0.0] if pair_count == 1 else [(-0.55 + 1.1 * i / (pair_count - 1)) * half_length for i in range(pair_count)]
    placements = []
    for pair_index, x in enumerate(xs):
        for side, sign in (("left", 1.0), ("right", -1.0)):
            placements.append(
                {
                    "id": f"pair{pair_index}_{side}",
                    "position": (x, sign * root["radius_m"] * 0.8, 0.0),
                    "side": side,
                }
            )
    return placements


def _build_leg(graph: dict, module_id: str, parent: str, placement: dict, length: float) -> None:
    x, y, z = placement["position"]
    side = placement["side"]
    outward = 0.0 if side == "center" else (1.0 if side == "left" else -1.0)
    joints = (
        ("hip", (x, y, z), length * 0.16),
        ("knee", (x + length * 0.08, y + outward * length * 0.08, z - length * 0.45), length * 0.12),
        ("ankle", (x, y + outward * length * 0.04, z - length * 0.88), length * 0.08),
        ("toe", (x + length * 0.18, y + outward * length * 0.04, z - length), length * 0.06),
    )
    previous = parent
    for role, position, radius in joints:
        joint_id = f"{module_id}.{role}"
        _add_joint(graph, joint_id, module_id, role, position, radius, side)
        _add_bone(graph, f"{module_id}.{role}", previous, joint_id)
        previous = joint_id
    graph["intent"]["contacts"].append(f"{module_id}.toe")


def _build_wing(graph: dict, module_id: str, parent: str, placement: dict, length: float) -> None:
    x, y, z = placement["position"]
    side = placement["side"]
    outward = 1.0 if side == "left" else -1.0
    joints = (
        ("shoulder", (x, y, z + length * 0.08), length * 0.11),
        ("elbow", (x - length * 0.08, y + outward * length * 0.42, z + length * 0.28), length * 0.08),
        ("wrist", (x + length * 0.06, y + outward * length * 0.76, z + length * 0.16), length * 0.05),
        ("tip", (x + length * 0.2, y + outward * length, z), length * 0.025),
    )
    previous = parent
    for role, position, radius in joints:
        joint_id = f"{module_id}.{role}"
        _add_joint(graph, joint_id, module_id, role, position, radius, side)
        _add_bone(graph, f"{module_id}.{role}", previous, joint_id)
        previous = joint_id


def _add_joint(graph: dict, joint_id: str, module: str, role: str, position, radius: float, side: str = "center") -> None:
    graph["joints"].append(
        {
            "id": joint_id,
            "module": module,
            "role": role,
            "position_m": [round(float(value), 6) for value in position],
            "radius_m": round(float(radius), 6),
            "side": side,
        }
    )


def _add_bone(graph: dict, bone_id: str, parent_joint: str, child_joint: str) -> None:
    graph["bones"].append({"id": bone_id, "parent_joint": parent_joint, "child_joint": child_joint})


def _parent_joint(graph: dict, module: dict) -> str:
    parent = _required_text(module, "parent")
    _joint(graph, parent)
    return parent


def _joint(graph: dict, joint_id: str) -> dict:
    joint = next((item for item in graph["joints"] if item["id"] == joint_id), None)
    if joint is None:
        raise ValueError(f"missing parent joint: {joint_id}")
    return joint


def _joint_position(graph: dict, joint_id: str) -> tuple[float, float, float]:
    return tuple(_joint(graph, joint_id)["position_m"])


def _required_text(value: dict, key: str) -> str:
    result = value.get(key)
    if not isinstance(result, str) or not result.strip():
        raise ValueError(f"{key} must be non-empty text")
    return result


def _positive(value: dict, key: str) -> float:
    result = _finite(value.get(key), key)
    if result <= 0.0:
        raise ValueError(f"{key} must be positive")
    return result


def _finite(value, name: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value):
        raise ValueError(f"{name} must be finite")
    return float(value)
