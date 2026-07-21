import copy
import math


def _normalize(vector):
    length = math.sqrt(sum(value * value for value in vector))
    if length <= 1e-12:
        raise ValueError("POSE-AXIS-1")
    return [value / length for value in vector]


def _rotate(vector, axis, angle):
    cosine, sine = math.cos(angle), math.sin(angle)
    dot = sum(vector[index] * axis[index] for index in range(3))
    cross = [
        axis[1] * vector[2] - axis[2] * vector[1],
        axis[2] * vector[0] - axis[0] * vector[2],
        axis[0] * vector[1] - axis[1] * vector[0],
    ]
    return [
        vector[index] * cosine + cross[index] * sine + axis[index] * dot * (1.0 - cosine)
        for index in range(3)
    ]


def apply_pose(skeleton, pose):
    joints = {joint["name"]: joint for joint in skeleton["joints"]}
    names = {pose.get("joint"), *pose.get("descendants", [])}
    if None in names or not names.issubset(joints):
        raise ValueError("POSE-JOINT-1")
    pivot_joint = joints[pose["joint"]]
    named_axes = {"x": [1.0, 0.0, 0.0], "y": [0.0, 1.0, 0.0], "z": [0.0, 0.0, 1.0]}
    axis_value = pose.get("axis")
    if axis_value == "joint_aim":
        axis = pivot_joint["basis"]["aim"]
    elif isinstance(axis_value, str):
        axis = named_axes.get(axis_value)
    else:
        axis = axis_value
    if not isinstance(axis, list) or len(axis) != 3:
        raise ValueError("POSE-AXIS-1")
    axis = _normalize(axis)
    angle = math.radians(pose["degrees"])
    pivot = pivot_joint["position"]
    result = []
    for source in skeleton["joints"]:
        joint = copy.deepcopy(source)
        if joint["name"] in pose["descendants"]:
            offset = [joint["position"][index] - pivot[index] for index in range(3)]
            rotated = _rotate(offset, axis, angle)
            joint["position"] = [pivot[index] + rotated[index] for index in range(3)]
        if joint["name"] in names:
            joint["basis"] = {key: _rotate(value, axis, angle) for key, value in joint["basis"].items()}
        result.append(joint)
    return result
