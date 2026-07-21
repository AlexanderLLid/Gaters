import math


REQUIRED_MODULES = {"torso", "head", "left_arm", "right_arm", "left_leg", "right_leg"}


def _bounds(body, module):
    minimum, maximum = body["placements"][module]["bounds_cells"]
    scale = body["cell_size_m"]
    return [value * scale for value in minimum], [value * scale for value in maximum]


def _center(minimum, maximum):
    return [(minimum[axis] + maximum[axis]) / 2 for axis in range(3)]


def _point(x, y, z):
    return [round(x, 12), round(y, 12), round(z, 12)]


def _segment(identifier, module, role, start, end, start_radius, end_radius, samples):
    return {
        "id": identifier,
        "module": module,
        "role": role,
        "start": start,
        "end": end,
        "start_radius_m": round(start_radius, 12),
        "end_radius_m": round(end_radius, 12),
        "samples": samples,
    }


def compile_anatomical_guide(body, recipe):
    valid = (
        body.get("schema") == "body-plan-mesh/0"
        and REQUIRED_MODULES.issubset(body.get("placements", {}))
        and recipe.get("schema") == "anatomical-guide-recipe/0"
        and isinstance(recipe.get("segment_samples"), int)
        and recipe["segment_samples"] >= 3
    )
    if not valid:
        raise ValueError("ANATOMY-BODY-1")

    torso_min, torso_max = _bounds(body, "torso")
    head_min, head_max = _bounds(body, "head")
    torso_center = _center(torso_min, torso_max)
    head_center = _center(head_min, head_max)
    torso_height = torso_max[2] - torso_min[2]
    torso_half_width = (torso_max[0] - torso_min[0]) / 2
    torso_half_depth = (torso_max[1] - torso_min[1]) / 2

    pelvis_z = torso_min[2] + recipe["torso_sections"]["pelvis"]["z_fraction"] * torso_height
    waist_z = torso_min[2] + recipe["torso_sections"]["abdomen"]["z_fraction"] * torso_height
    chest_z = torso_min[2] + recipe["torso_sections"]["chest"]["z_fraction"] * torso_height
    explicit_terminals = {
        "neck", "left_hand", "right_hand", "left_foot", "right_foot"
    }.issubset(body["placements"])
    neck_min, neck_max = _bounds(body, "neck") if explicit_terminals else (None, None)
    neck_z = neck_min[2] if explicit_terminals else torso_min[2] + 0.96 * torso_height
    head_base_z = head_min[2] + 0.10 * (head_max[2] - head_min[2])
    landmarks = {
        "pelvis": _point(0, 0, pelvis_z),
        "waist": _point(0, 0, waist_z),
        "chest": _point(0, 0, chest_z),
        "neck_base": _point(0, 0, neck_z),
        "head_base": _point(0, 0, head_base_z),
        "head_center": _point(*head_center),
        "head_top": _point(head_center[0], head_center[1], head_max[2]),
    }

    for side, sign in (("left", -1), ("right", 1)):
        arm_min, arm_max = _bounds(body, f"{side}_arm")
        arm_center = _center(arm_min, arm_max)
        shoulder_x = sign * torso_half_width * 0.88
        outer_x = arm_min[0] if sign < 0 else arm_max[0]
        elbow_x = shoulder_x + recipe["arm_elbow_fraction"] * (outer_x - shoulder_x)
        wrist_x = shoulder_x + recipe["arm_wrist_fraction"] * (outer_x - shoulder_x)
        landmarks[f"{side}_shoulder"] = _point(shoulder_x, 0, arm_center[2])
        landmarks[f"{side}_elbow"] = _point(elbow_x, 0, arm_center[2] - 0.025 * torso_height)
        landmarks[f"{side}_wrist"] = _point(wrist_x, 0, arm_center[2] - 0.05 * torso_height)

        leg_min, leg_max = _bounds(body, f"{side}_leg")
        leg_center = _center(leg_min, leg_max)
        hip_z = torso_min[2] + 0.06 * torso_height
        bottom_z = leg_min[2]
        knee_z = hip_z + recipe["leg_knee_fraction"] * (bottom_z - hip_z)
        ankle_z = hip_z + recipe["leg_ankle_fraction"] * (bottom_z - hip_z)
        landmarks[f"{side}_hip"] = _point(leg_center[0], 0, hip_z)
        landmarks[f"{side}_knee"] = _point(leg_center[0], 0, knee_z)
        landmarks[f"{side}_ankle"] = _point(leg_center[0], 0, ankle_z)

    symmetry_pairs = [
        [f"left_{part}", f"right_{part}"]
        for part in ("shoulder", "elbow", "wrist", "hip", "knee", "ankle")
    ]
    skeleton_segments = [
        {"id": "spine_lower", "start": "pelvis", "end": "waist"},
        {"id": "spine_upper", "start": "waist", "end": "chest"},
        {"id": "neck", "start": "chest", "end": "head_center"},
    ]

    surface_segments = []
    arm_thickness = min(
        body["placements"]["left_arm"]["bounds_cells"][1][1] - body["placements"]["left_arm"]["bounds_cells"][0][1],
        body["placements"]["left_arm"]["bounds_cells"][1][2] - body["placements"]["left_arm"]["bounds_cells"][0][2],
    ) * body["cell_size_m"] / 2
    leg_thickness = min(
        body["placements"]["left_leg"]["bounds_cells"][1][0] - body["placements"]["left_leg"]["bounds_cells"][0][0],
        body["placements"]["left_leg"]["bounds_cells"][1][1] - body["placements"]["left_leg"]["bounds_cells"][0][1],
    ) * body["cell_size_m"] / 2
    arm_radii = [arm_thickness * value for value in recipe["arm_radius_profile"]]
    leg_radii = [leg_thickness * value for value in recipe["leg_radius_profile"]]
    samples = recipe["segment_samples"]
    for side in ("left", "right"):
        surface_segments.extend([
            _segment(f"{side}_upper_arm", f"{side}_arm", "limb", f"{side}_shoulder", f"{side}_elbow", arm_radii[0], arm_radii[1], samples),
            _segment(f"{side}_lower_arm", f"{side}_arm", "limb", f"{side}_elbow", f"{side}_wrist", arm_radii[1], arm_radii[2], samples),
            _segment(f"{side}_thigh", f"{side}_leg", "limb", f"{side}_hip", f"{side}_knee", leg_radii[0], leg_radii[1], samples),
            _segment(f"{side}_calf", f"{side}_leg", "limb", f"{side}_knee", f"{side}_ankle", leg_radii[1], leg_radii[2], samples),
        ])
        skeleton_segments.extend([
            {"id": f"{side}_clavicle", "start": "chest", "end": f"{side}_shoulder"},
            {"id": f"{side}_upper_arm", "start": f"{side}_shoulder", "end": f"{side}_elbow"},
            {"id": f"{side}_lower_arm", "start": f"{side}_elbow", "end": f"{side}_wrist"},
            {"id": f"{side}_pelvis", "start": "pelvis", "end": f"{side}_hip"},
            {"id": f"{side}_thigh", "start": f"{side}_hip", "end": f"{side}_knee"},
            {"id": f"{side}_calf", "start": f"{side}_knee", "end": f"{side}_ankle"},
        ])

    surface_ellipsoids = []
    for name, section in recipe["torso_sections"].items():
        surface_ellipsoids.append({
            "id": name,
            "module": "torso",
            "role": "core",
            "center": _point(torso_center[0], torso_center[1], torso_min[2] + section["z_fraction"] * torso_height),
            "radii": _point(
                torso_half_width * section["width_fraction"],
                torso_half_depth * section["depth_fraction"],
                torso_height * section["height_fraction"] / 2,
            ),
        })
    head_half = [(head_max[axis] - head_min[axis]) / 2 for axis in range(3)]
    head_height = head_max[2] - head_min[2]
    for name, section in recipe["head_sections"].items():
        surface_ellipsoids.append({
            "id": name,
            "module": "head",
            "role": "head",
            "center": _point(head_center[0], head_center[1], head_min[2] + section["z_fraction"] * head_height),
            "radii": _point(*(head_half[axis] * section["radii_fraction"][axis] for axis in range(3))),
        })
    neck_radius = (
        min(neck_max[0] - neck_min[0], neck_max[1] - neck_min[1]) * 0.46
        if explicit_terminals else head_half[0] * recipe["neck_radius_fraction"]
    )
    surface_segments.append(_segment(
        "neck", "neck" if explicit_terminals else "torso", "core",
        "neck_base", "head_base", neck_radius, neck_radius * 0.94, samples,
    ))
    for side in ("left", "right"):
        wrist = landmarks[f"{side}_wrist"]
        ankle = landmarks[f"{side}_ankle"]
        shoulder = landmarks[f"{side}_shoulder"]
        surface_ellipsoids.append({
            "id": f"{side}_deltoid", "module": f"{side}_arm", "role": "limb",
            "center": _point(shoulder[0], shoulder[1], shoulder[2]),
            "radii": _point(*(arm_thickness * value for value in recipe["deltoid_radius_scale"])),
        })
        if explicit_terminals:
            hand_min, hand_max = _bounds(body, f"{side}_hand")
            foot_min, foot_max = _bounds(body, f"{side}_foot")
            hand_center, foot_center = _center(hand_min, hand_max), _center(foot_min, foot_max)
            hand_radii = [(hand_max[axis] - hand_min[axis]) * 0.46 for axis in range(3)]
            foot_radii = [(foot_max[axis] - foot_min[axis]) * 0.46 for axis in range(3)]
        else:
            hand_center = [wrist[0] + (-1 if side == "left" else 1) * arm_radii[2] * 0.45, wrist[1], wrist[2]]
            hand_radii = [arm_radii[2] * 1.35, arm_radii[2] * 0.72, arm_radii[2] * 0.82]
            foot_center = [ankle[0], ankle[1] - leg_radii[2] * 0.75, ankle[2] - leg_radii[2] * 0.20]
            foot_radii = [leg_radii[2] * 1.05, leg_radii[2] * 1.85, leg_radii[2] * 0.75]
        hand_radii = [value * scale for value, scale in zip(hand_radii, recipe["hand_radius_scale"])]
        foot_radii = [value * scale for value, scale in zip(foot_radii, recipe["foot_radius_scale"])]
        surface_ellipsoids.extend([
            {
                "id": f"{side}_hand", "module": f"{side}_hand" if explicit_terminals else f"{side}_arm", "role": "limb",
                "center": _point(*hand_center), "radii": _point(*hand_radii),
            },
            {
                "id": f"{side}_foot", "module": f"{side}_foot" if explicit_terminals else f"{side}_leg", "role": "limb",
                "center": _point(*foot_center), "radii": _point(*foot_radii),
            },
        ])

    return {
        "schema": "anatomical-guide/0",
        "id": recipe["id"],
        "body_plan_id": body["body_plan_id"],
        "landmarks": landmarks,
        "symmetry_pairs": symmetry_pairs,
        "skeleton_segments": skeleton_segments,
        "surface_segments": surface_segments,
        "surface_ellipsoids": surface_ellipsoids,
        "body_metadata": {key: body[key] for key in ("body_plan_id", "placements", "connections", "cells", "mirrors")},
    }
