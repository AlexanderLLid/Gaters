"""Generate one deterministic balance-recovery step from an impact event."""

import math


RECOVERY_VERSION = 1


def physical_structure_passed(checks):
    return all(passed for name, passed in checks.items() if name != "reactionSettled")


def rounded(values):
    return [round(float(value), 6) for value in values]


def plan_recovery(event, stance, fps=30):
    if event.get("type") != "impact":
        raise ValueError("recovery event type must be impact")
    direction = event.get("direction")
    strength = event.get("strength")
    if not isinstance(direction, list) or len(direction) != 3:
        raise ValueError("impact direction must contain three components")
    if not isinstance(strength, (int, float)) or not 0.0 <= strength <= 1.0:
        raise ValueError("impact strength must be between 0 and 1")
    if not isinstance(fps, int) or fps <= 0:
        raise ValueError("fps must be a positive integer")
    for foot in ("foot_l", "foot_r"):
        if foot not in stance or len(stance[foot]) != 3:
            raise ValueError(f"stance must contain {foot}")

    frames = {
        "release": round(7 * fps / 30) + 1,
        "apex": round(15 * fps / 30) + 1,
        "plant": round(23 * fps / 30) + 1,
        "settled": fps * 3 + 1,
    }
    if strength < 0.35:
        support_center = rounded(
            [
                (stance["foot_l"][0] + stance["foot_r"][0]) * 0.5,
                (stance["foot_l"][1] + stance["foot_r"][1]) * 0.5,
            ]
        )
        return {
            "schemaVersion": 1,
            "recoveryVersion": RECOVERY_VERSION,
            "mode": "hold",
            "stepFoot": None,
            "stepLengthMeters": 0.0,
            "liftMeters": 0.0,
            "targetMeters": None,
            "supportCenterMeters": support_center,
            "frames": frames,
            "waypoints": [],
        }

    horizontal = [float(direction[0]), float(direction[1])]
    magnitude = math.hypot(*horizontal)
    if magnitude <= 0.000001:
        raise ValueError("impact must have a horizontal direction")
    horizontal = [component / magnitude for component in horizontal]
    step_foot = "foot_l" if horizontal[0] > 0.0 or (
        abs(horizontal[0]) <= 0.000001 and horizontal[1] >= 0.0
    ) else "foot_r"
    start = [float(value) for value in stance[step_foot]]
    step_length = round(min(0.40, max(0.18, 0.14 + 0.26 * float(strength))), 6)
    lift = round(min(0.14, max(0.06, 0.06 + 0.06 * float(strength))), 6)
    target = rounded(
        [
            start[0] + horizontal[0] * step_length,
            start[1] + horizontal[1] * step_length,
            start[2],
        ]
    )
    apex = rounded(
        [
            (start[0] + target[0]) * 0.5,
            (start[1] + target[1]) * 0.5,
            start[2] + lift,
        ]
    )
    final_feet = {foot: list(values) for foot, values in stance.items()}
    final_feet[step_foot] = target
    support_center = rounded(
        [
            (final_feet["foot_l"][0] + final_feet["foot_r"][0]) * 0.5,
            (final_feet["foot_l"][1] + final_feet["foot_r"][1]) * 0.5,
        ]
    )
    return {
        "schemaVersion": 1,
        "recoveryVersion": RECOVERY_VERSION,
        "mode": "step",
        "stepFoot": step_foot,
        "stepLengthMeters": step_length,
        "liftMeters": lift,
        "targetMeters": target,
        "supportCenterMeters": support_center,
        "frames": frames,
        "waypoints": [
            {"frame": frames["release"], "positionMeters": rounded(start)},
            {"frame": frames["apex"], "positionMeters": apex},
            {"frame": frames["plant"], "positionMeters": target},
        ],
    }
