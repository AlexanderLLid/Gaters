import copy
import math


def _sample(keys, time_seconds):
    for left, right in zip(keys, keys[1:]):
        if time_seconds <= right["time_seconds"]:
            factor = (time_seconds - left["time_seconds"]) / (right["time_seconds"] - left["time_seconds"])
            return left["degrees"] + factor * (right["degrees"] - left["degrees"])
    return keys[-1]["degrees"]


def _max_joint_delta(expected, actual):
    if len(expected) != len(actual):
        return float("inf")
    maximum = 0.0
    for left, right in zip(expected, actual):
        vectors = [(left.get("position", []), right.get("position", []))]
        vectors.extend((left.get("basis", {}).get(axis, []), right.get("basis", {}).get(axis, [])) for axis in ("aim", "up", "side"))
        for expected_vector, actual_vector in vectors:
            if len(expected_vector) != 3 or len(actual_vector) != 3 or not all(isinstance(value, (int, float)) and math.isfinite(value) for value in expected_vector + actual_vector):
                return float("inf")
            maximum = max(maximum, math.dist(expected_vector, actual_vector))
    return maximum


def _normalize(vector):
    length = math.sqrt(sum(value * value for value in vector))
    return [value / length for value in vector]


def _rotate(vector, axis, radians):
    x, y, z = axis
    cosine, sine = math.cos(radians), math.sin(radians)
    matrix = (
        (cosine + x * x * (1 - cosine), x * y * (1 - cosine) - z * sine, x * z * (1 - cosine) + y * sine),
        (y * x * (1 - cosine) + z * sine, cosine + y * y * (1 - cosine), y * z * (1 - cosine) - x * sine),
        (z * x * (1 - cosine) - y * sine, z * y * (1 - cosine) + x * sine, cosine + z * z * (1 - cosine)),
    )
    return [sum(matrix[row][column] * vector[column] for column in range(3)) for row in range(3)]


def _replay_pose(skeleton, control, degrees):
    result = copy.deepcopy(skeleton)
    joints = {joint["name"]: joint for joint in result["joints"]}
    pivot_joint = joints[control["joint"]]
    named = {"x": [1.0, 0.0, 0.0], "y": [0.0, 1.0, 0.0], "z": [0.0, 0.0, 1.0]}
    axis = pivot_joint["basis"]["aim"] if control["axis"] == "joint_aim" else named.get(control["axis"], control["axis"]) if isinstance(control["axis"], str) else control["axis"]
    axis = _normalize(axis)
    pivot = pivot_joint["position"]
    affected = {control["joint"], *control["descendants"]}
    for joint in result["joints"]:
        if joint["name"] in control["descendants"]:
            offset = [joint["position"][index] - pivot[index] for index in range(3)]
            rotated = _rotate(offset, axis, math.radians(degrees))
            joint["position"] = [pivot[index] + rotated[index] for index in range(3)]
        if joint["name"] in affected:
            joint["basis"] = {name: _rotate(vector, axis, math.radians(degrees)) for name, vector in joint["basis"].items()}
    return result


def verify_motion(skeleton, limits, recipe, motion, skeleton_sha256, limits_sha256, tolerance=1e-8):
    failures = []
    frames = motion.get("frames", [])
    expected_count = round(recipe["duration_seconds"] * recipe["fps"]) + 1
    if len(frames) != expected_count or motion.get("fps") != recipe["fps"] or motion.get("duration_seconds") != recipe["duration_seconds"]:
        failures.append({"rule": "MOTION-TIME-1", "subject": "frame schedule"})
    controls = limits.get("controls", {})
    tracks = recipe.get("tracks", [])
    max_replay_delta = 0.0
    max_joint_motion = 0.0
    max_frame_delta = 0.0
    previous_values = None
    if len(frames) == expected_count:
        for index, frame in enumerate(frames):
            time_seconds = index / recipe["fps"]
            if frame.get("index") != index or abs(frame.get("time_seconds", float("inf")) - time_seconds) > tolerance:
                failures.append({"rule": "MOTION-TIME-1", "subject": f"frame {index}"})
                continue
            expected_values = {track["control"]: _sample(track["keys"], time_seconds) for track in tracks}
            values = frame.get("controls", {})
            if set(values) != set(expected_values) or any(abs(values[key] - expected_values[key]) > tolerance for key in expected_values):
                failures.append({"rule": "MOTION-CURVE-1", "subject": f"frame {index}"})
                continue
            if any(not controls[key]["minimum_degrees"] <= value <= controls[key]["maximum_degrees"] for key, value in values.items()):
                failures.append({"rule": "MOTION-LIMIT-1", "subject": f"frame {index}"})
            if previous_values is not None:
                max_frame_delta = max(max_frame_delta, max(abs(values[key] - previous_values[key]) for key in values))
            previous_values = values
            replay = copy.deepcopy(skeleton)
            if any(abs(value) > tolerance for value in values.values()):
                for track in tracks:
                    replay = _replay_pose(replay, controls[track["control"]], values[track["control"]])
            max_replay_delta = max(max_replay_delta, _max_joint_delta(replay["joints"], frame.get("joints", [])))
            max_joint_motion = max(max_joint_motion, _max_joint_delta(skeleton["joints"], frame.get("joints", [])))
    if max_replay_delta > tolerance:
        failures.append({"rule": "MOTION-REPLAY-1", "subject": "skeleton frames"})
    if max_frame_delta > recipe["maximum_frame_delta_degrees"] + tolerance:
        failures.append({"rule": "MOTION-CONTINUITY-1", "subject": "control curves"})
    if max_joint_motion <= 0.1:
        failures.append({"rule": "MOTION-ACTIVE-1", "subject": "skeleton"})
    if not frames or frames[0].get("joints") != skeleton.get("joints") or frames[-1].get("joints") != skeleton.get("joints"):
        failures.append({"rule": "MOTION-LOOP-1", "subject": "rest endpoints"})
    if motion.get("schema") != "motion-frames/0" or motion.get("skeleton_id") != skeleton.get("id") or motion.get("source_skeleton_sha256") != skeleton_sha256 or motion.get("source_joint_limits_sha256") != limits_sha256:
        failures.append({"rule": "MOTION-PROVENANCE-1", "subject": "inputs"})
    return {
        "schema": "motion-verification/0",
        "passed": not failures,
        "failures": failures,
        "frame_count": len(frames),
        "maximum_replay_delta_m": max_replay_delta,
        "maximum_joint_motion_m": max_joint_motion,
        "maximum_frame_delta_degrees": max_frame_delta,
    }
