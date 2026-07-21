import math


def _side_neutral(value):
    if isinstance(value, str):
        return value.replace("left_", "side_").replace("right_", "side_")
    return [_side_neutral(item) for item in value]


def _axis_key(axis):
    return axis if isinstance(axis, str) else tuple(float(value) for value in axis)


def verify_joint_limits(skeleton, recipe, limits, pose_suite, skeleton_sha256):
    failures = []
    joint_names = {joint["name"] for joint in skeleton.get("joints", [])}
    controls = limits.get("controls", {})
    expected_ids = {control["id"] for control in recipe.get("controls", [])}
    if set(controls) != expected_ids:
        failures.append({"rule": "JOINT-LIMIT-COVERAGE-1", "subject": "control ids"})
    expected_controls = {control["id"]: {key: value for key, value in control.items() if key != "id"} for control in recipe.get("controls", [])}
    if controls != expected_controls:
        failures.append({"rule": "JOINT-LIMIT-RECIPE-1", "subject": "compiled controls"})
    if limits.get("mirror_pairs") != recipe.get("mirror_pairs", []):
        failures.append({"rule": "JOINT-LIMIT-MIRROR-1", "subject": "mirror table"})

    for control_id, control in controls.items():
        names = {control.get("joint"), *control.get("descendants", [])}
        low, high = control.get("minimum_degrees"), control.get("maximum_degrees")
        finite_range = all(isinstance(value, (int, float)) and math.isfinite(value) for value in (low, high))
        if None in names or not names.issubset(joint_names):
            failures.append({"rule": "JOINT-LIMIT-JOINT-1", "subject": control_id})
        if not finite_range or not low <= 0 <= high:
            failures.append({"rule": "JOINT-LIMIT-RANGE-1", "subject": control_id})

    for left_id, right_id in limits.get("mirror_pairs", []):
        left, right = controls.get(left_id), controls.get(right_id)
        comparable = left and right and {
            "joint": _side_neutral(left["joint"]),
            "descendants": _side_neutral(left["descendants"]),
            "axis": _axis_key(left["axis"]),
            "minimum_degrees": left["minimum_degrees"],
            "maximum_degrees": left["maximum_degrees"],
        } == {
            "joint": _side_neutral(right["joint"]),
            "descendants": _side_neutral(right["descendants"]),
            "axis": _axis_key(right["axis"]),
            "minimum_degrees": right["minimum_degrees"],
            "maximum_degrees": right["maximum_degrees"],
        }
        if not comparable:
            failures.append({"rule": "JOINT-LIMIT-MIRROR-1", "subject": f"{left_id}/{right_id}"})

    for pose in pose_suite.get("diagnostic_poses", []):
        control = controls.get(pose["id"])
        if not control or control["joint"] != pose["joint"] or _axis_key(control["axis"]) != _axis_key(pose["axis"]) or not control["minimum_degrees"] <= pose["degrees"] <= control["maximum_degrees"]:
            failures.append({"rule": "JOINT-LIMIT-POSE-SUITE-1", "subject": pose["id"]})

    if limits.get("schema") != "joint-limits/0" or limits.get("skeleton_id") != skeleton.get("id") or limits.get("source_skeleton_sha256") != skeleton_sha256:
        failures.append({"rule": "JOINT-LIMIT-PROVENANCE-1", "subject": "skeleton"})
    return {
        "schema": "joint-limit-verification/0",
        "passed": not failures,
        "failures": failures,
        "control_count": len(controls),
        "pose_suite_count": len(pose_suite.get("diagnostic_poses", [])),
    }
