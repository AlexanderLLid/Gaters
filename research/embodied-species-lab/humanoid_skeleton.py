"""Pure mechanical skeleton contract shared by Blender generators and tests."""


FOOT_PLACEMENT_ROLES = (
    "foot_l",
    "ball_l",
    "ik_foot_l",
    "foot_r",
    "ball_r",
    "ik_foot_r",
)


def humanoid_bones(body):
    height = float(body["heightMeters"])
    shoulder = float(body["shoulderWidthMeters"])
    hip = float(body["hipWidthMeters"])
    scale = height / 1.8
    left_x = hip / 2.0
    right_x = -left_x

    def bone(name, parent, head, tail, deform=True):
        return {
            "name": name,
            "parent": parent,
            "head": head,
            "tail": tail,
            "deform": deform,
        }

    return [
        bone("root", None, (0.0, 0.0, 0.0), (0.0, 0.0, 0.12 * scale)),
        bone("ik_foot_l", "root", (left_x, 0.0, 0.10 * scale), (left_x, -0.24 * scale, 0.07 * scale), False),
        bone("ik_foot_r", "root", (right_x, 0.0, 0.10 * scale), (right_x, -0.24 * scale, 0.07 * scale), False),
        bone("pelvis", "root", (0.0, 0.0, 0.82 * scale), (0.0, 0.0, 1.00 * scale)),
        bone("spine", "pelvis", (0.0, 0.0, 1.00 * scale), (0.0, 0.0, 1.28 * scale)),
        bone("chest", "spine", (0.0, 0.0, 1.28 * scale), (0.0, 0.0, 1.50 * scale)),
        bone("neck", "chest", (0.0, 0.0, 1.50 * scale), (0.0, 0.0, 1.62 * scale)),
        bone("head", "neck", (0.0, 0.0, 1.62 * scale), (0.0, 0.0, 1.80 * scale)),
        bone("upper_arm_l", "chest", (shoulder / 2.0, 0.0, 1.45 * scale), (shoulder / 2.0 + 0.27 * scale, 0.0, 1.30 * scale)),
        bone("lower_arm_l", "upper_arm_l", (shoulder / 2.0 + 0.27 * scale, 0.0, 1.30 * scale), (shoulder / 2.0 + 0.52 * scale, 0.0, 1.15 * scale)),
        bone("upper_arm_r", "chest", (-shoulder / 2.0, 0.0, 1.45 * scale), (-shoulder / 2.0 - 0.27 * scale, 0.0, 1.30 * scale)),
        bone("lower_arm_r", "upper_arm_r", (-shoulder / 2.0 - 0.27 * scale, 0.0, 1.30 * scale), (-shoulder / 2.0 - 0.52 * scale, 0.0, 1.15 * scale)),
        bone("thigh_l", "pelvis", (left_x, 0.0, 0.88 * scale), (left_x, 0.0, 0.48 * scale)),
        bone("shin_l", "thigh_l", (left_x, 0.0, 0.48 * scale), (left_x, 0.0, 0.10 * scale)),
        bone("foot_l", "shin_l", (left_x, 0.0, 0.10 * scale), (left_x, -0.24 * scale, 0.07 * scale)),
        bone("ball_l", "foot_l", (left_x, -0.18 * scale, 0.0775 * scale), (left_x, -0.28 * scale, 0.065 * scale)),
        bone("thigh_r", "pelvis", (right_x, 0.0, 0.88 * scale), (right_x, 0.0, 0.48 * scale)),
        bone("shin_r", "thigh_r", (right_x, 0.0, 0.48 * scale), (right_x, 0.0, 0.10 * scale)),
        bone("foot_r", "shin_r", (right_x, 0.0, 0.10 * scale), (right_x, -0.24 * scale, 0.07 * scale)),
        bone("ball_r", "foot_r", (right_x, -0.18 * scale, 0.0775 * scale), (right_x, -0.28 * scale, 0.065 * scale)),
    ]
