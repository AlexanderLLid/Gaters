"""Generate the solver-independent physical contract for a humanoid body."""


PROFILE_VERSION = 1


PART_MASS_RATIOS = (
    ("pelvis", 0.14),
    ("spine", 0.12),
    ("chest", 0.20),
    ("head", 0.08),
    ("upper_arm_l", 0.03),
    ("lower_arm_l", 0.02),
    ("upper_arm_r", 0.03),
    ("lower_arm_r", 0.02),
    ("thigh_l", 0.11),
    ("shin_l", 0.05),
    ("foot_l", 0.02),
    ("thigh_r", 0.11),
    ("shin_r", 0.05),
    ("foot_r", 0.02),
)


JOINTS = (
    ("pelvis", "spine", "spine", (-20, 20), (-15, 15), (-20, 20), 700.0),
    ("spine", "chest", "chest", (-20, 20), (-15, 15), (-20, 20), 800.0),
    ("chest", "head", "neck", (-30, 30), (-25, 25), (-50, 50), 360.0),
    ("chest", "upper_arm_l", "upper_arm_l", (-75, 75), (-90, 90), (-110, 110), 240.0),
    ("upper_arm_l", "lower_arm_l", "lower_arm_l", (-5, 5), (-5, 5), (0, 145), 200.0),
    ("chest", "upper_arm_r", "upper_arm_r", (-75, 75), (-90, 90), (-110, 110), 240.0),
    ("upper_arm_r", "lower_arm_r", "lower_arm_r", (-5, 5), (-5, 5), (0, 145), 200.0),
    ("pelvis", "thigh_l", "thigh_l", (-45, 60), (-35, 35), (-45, 45), 700.0),
    ("thigh_l", "shin_l", "shin_l", (-5, 5), (-5, 5), (0, 150), 600.0),
    ("shin_l", "foot_l", "foot_l", (-20, 30), (-15, 15), (-20, 20), 900.0),
    ("pelvis", "thigh_r", "thigh_r", (-45, 60), (-35, 35), (-45, 45), 700.0),
    ("thigh_r", "shin_r", "shin_r", (-5, 5), (-5, 5), (0, 150), 600.0),
    ("shin_r", "foot_r", "foot_r", (-20, 30), (-15, 15), (-20, 20), 900.0),
)


def generate_physical_profile(body, anchored_feet=True):
    """Return deterministic masses, collision roles, and joint safety limits."""
    height = float(body["heightMeters"])
    if not 1.2 <= height <= 2.4:
        raise ValueError("heightMeters must be between 1.2 and 2.4")
    total_mass = round(75.0 * (height / 1.8) ** 3, 6)
    parts = []
    assigned = 0.0
    for index, (bone, ratio) in enumerate(PART_MASS_RATIOS):
        mass = (
            round(total_mass - assigned, 6)
            if index == len(PART_MASS_RATIOS) - 1
            else round(total_mass * ratio, 6)
        )
        assigned = round(assigned + mass, 6)
        parts.append(
            {
                "bone": bone,
                "massKg": mass,
                "collisionShape": "BOX",
                "linearDamping": 0.35,
                "angularDamping": 0.65,
                "anchored": anchored_feet and bone in {"foot_l", "foot_r"},
            }
        )

    joints = []
    for parent, child, anchor, x_limits, y_limits, z_limits, stiffness in JOINTS:
        joints.append(
            {
                "name": f"joint.{parent}.{child}",
                "parent": parent,
                "child": child,
                "anchorBone": anchor,
                "type": "GENERIC_SPRING",
                "linearLimitsMeters": {"x": [0.0, 0.0], "y": [0.0, 0.0], "z": [0.0, 0.0]},
                "angularLimitsDegrees": {
                    "x": list(x_limits),
                    "y": list(y_limits),
                    "z": list(z_limits),
                },
                "angularSpringStiffness": stiffness,
                "angularSpringDamping": 25.0,
            }
        )

    return {
        "schemaVersion": 1,
        "profileVersion": PROFILE_VERSION,
        "totalMassKg": total_mass,
        "parts": parts,
        "joints": joints,
    }
