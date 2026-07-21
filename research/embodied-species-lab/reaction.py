"""Pure event-to-motion recipe synthesis for the embodied-species lab."""

import math
import random


GENERATOR_VERSION = 1
MAX_ROOT_DISPLACEMENT_METERS = 0.35
BONES = (
    "pelvis",
    "spine",
    "chest",
    "head",
    "upper_arm_l",
    "upper_arm_r",
    "thigh_l",
    "thigh_r",
)


def _rounded_vector(values):
    return [round(float(value), 6) for value in values]


def _scaled(values, scale):
    return _rounded_vector(value * scale for value in values)


def _neutral_rotations():
    return {bone: [0.0, 0.0, 0.0] for bone in BONES}


def synthesize_reaction(event, seed, fps):
    """Return a bounded one-second reaction recipe for one event and seed."""
    if event.get("type") != "impact":
        raise ValueError("the first lab machine supports impact events only")
    direction = event.get("direction", [])
    strength = event.get("strength")
    if len(direction) != 3 or not all(isinstance(value, (int, float)) for value in direction):
        raise ValueError("impact direction must contain three numeric components")
    if not isinstance(strength, (int, float)) or not 0.0 <= strength <= 1.0:
        raise ValueError("impact strength must be between zero and one")
    if not isinstance(fps, int) or fps <= 0:
        raise ValueError("fps must be a positive integer")

    length = math.sqrt(sum(float(value) ** 2 for value in direction))
    if length == 0.0:
        raise ValueError("impact direction cannot be zero")
    unit = [float(value) / length for value in direction]
    rng = random.Random(int(seed))
    lateral_bias = rng.uniform(-1.0, 1.0)
    twist_bias = rng.uniform(-1.0, 1.0)
    displacement = min(MAX_ROOT_DISPLACEMENT_METERS, 0.08 + 0.26 * float(strength))
    peak_root = [
        unit[0] * displacement,
        unit[1] * displacement + lateral_bias * 0.05 * float(strength),
        max(0.0, unit[2]) * displacement * 0.25,
    ]
    peak_twist = 8.0 + 22.0 * float(strength)
    peak_lean = 10.0 + 28.0 * float(strength)
    side = 1.0 if unit[1] + lateral_bias * 0.25 >= 0.0 else -1.0

    peak_rotations = _neutral_rotations()
    peak_rotations.update({
        "pelvis": _rounded_vector((unit[1] * peak_lean * 0.35, -unit[0] * peak_lean * 0.3, side * peak_twist * 0.25)),
        "spine": _rounded_vector((unit[1] * peak_lean * 0.55, -unit[0] * peak_lean * 0.55, twist_bias * peak_twist * 0.55)),
        "chest": _rounded_vector((unit[1] * peak_lean, -unit[0] * peak_lean, twist_bias * peak_twist)),
        "head": _rounded_vector((-unit[1] * peak_lean * 0.25, unit[0] * peak_lean * 0.2, -twist_bias * peak_twist * 0.25)),
        "upper_arm_l": _rounded_vector((side * peak_lean * 0.45, 0.0, -peak_twist * 0.7)),
        "upper_arm_r": _rounded_vector((-side * peak_lean * 0.45, 0.0, peak_twist * 0.7)),
        "thigh_l": _rounded_vector((0.0, side * peak_lean * 0.18, 0.0)),
        "thigh_r": _rounded_vector((0.0, -side * peak_lean * 0.18, 0.0)),
    })

    keyframes = []
    for frame, amount in ((1, 0.0), (4, 0.35), (12, 1.0), (22, 0.4), (31, 0.0)):
        rotations = {
            bone: _scaled(rotation, amount)
            for bone, rotation in peak_rotations.items()
        }
        keyframes.append({
            "frame": frame,
            "rootLocationMeters": _scaled(peak_root, amount),
            "boneEulerDegrees": rotations,
        })

    return {
        "schemaVersion": 1,
        "generatorVersion": GENERATOR_VERSION,
        "eventId": event.get("eventId", "impact"),
        "seed": int(seed),
        "fps": fps,
        "durationSeconds": 1.0,
        "variation": {
            "lateralBias": round(lateral_bias, 6),
            "twistBias": round(twist_bias, 6),
        },
        "limits": {"maximumRootDisplacementMeters": MAX_ROOT_DISPLACEMENT_METERS},
        "keyframes": keyframes,
    }
