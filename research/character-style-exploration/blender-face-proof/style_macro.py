"""Pure bounded macro deformation for the anatomical face style experiment."""


def bounded_parameter(parameters, name, default):
    value = float(parameters.get(name, default))
    if not 0.0 <= value <= 1.0:
        raise ValueError(f"{name} must be between 0 and 1")
    return value


def resolve_mouth_width(parameters, current):
    value = float(parameters.get("mouthWidthValue", current))
    if not 0.0 <= value <= 1.0:
        raise ValueError("mouthWidthValue must be between 0 and 1")
    return value


def resolve_mouth_narrow(parameters):
    value = float(parameters.get("mouthNarrowValue", 0.0))
    if not 0.0 <= value <= 1.0:
        raise ValueError("mouthNarrowValue must be between 0 and 1")
    return value


def resolve_eye_scale_decrease(parameters):
    value = float(parameters.get("eyeScaleDecrease", 0.0))
    if not 0.0 <= value <= 1.0:
        raise ValueError("eyeScaleDecrease must be between 0 and 1")
    return value


def resolve_eye_scale_decreases(parameters):
    fallback = resolve_eye_scale_decrease(parameters)
    values = (
        float(parameters.get("eyeScaleDecreaseLeft", fallback)),
        float(parameters.get("eyeScaleDecreaseRight", fallback)),
    )
    if any(not 0.0 <= value <= 1.0 for value in values):
        raise ValueError("eye scale decreases must be between 0 and 1")
    return values


def resolve_eye_height(parameters, current):
    value = float(parameters.get("eyeHeightValue", current))
    if not 0.0 <= value <= 1.0:
        raise ValueError("eyeHeightValue must be between 0 and 1")
    return value


def resolve_eye_narrows(parameters, current_left, current_right):
    values = (
        float(parameters.get("eyeNarrowLeft", current_left)),
        float(parameters.get("eyeNarrowRight", current_right)),
    )
    if any(not 0.0 <= value <= 1.0 for value in values):
        raise ValueError("eye narrow values must be between 0 and 1")
    return values


def _lerp(first, second, amount):
    return first + (second - first) * amount


def _lower_scale(fraction, cheek_scale, jaw_scale, cheek_fraction, jaw_fraction):
    if fraction <= cheek_fraction:
        return _lerp(1.0, cheek_scale, fraction / cheek_fraction)
    if fraction <= jaw_fraction:
        return _lerp(
            cheek_scale,
            jaw_scale,
            (fraction - cheek_fraction) / (jaw_fraction - cheek_fraction),
        )
    return jaw_scale


def deform_point(point, *, center_x, eye_z, chin_z, top_z, side_radius,
                 upper_height_scale, lower_height_scale, cheek_scale, jaw_scale,
                 cheek_fraction, jaw_fraction, lateral_power):
    x, y, z = point
    if z < chin_z or z > top_z:
        return point
    original_z = z
    if z >= eye_z:
        z = eye_z + (z - eye_z) * upper_height_scale
        lateral_scale = 1.0
    else:
        fraction = (eye_z - original_z) / (eye_z - chin_z)
        z = eye_z + (z - eye_z) * lower_height_scale
        lateral_scale = _lower_scale(
            fraction, cheek_scale, jaw_scale, cheek_fraction, jaw_fraction
        )
    distance = abs(x - center_x)
    side_weight = min(1.0, distance / side_radius) ** lateral_power
    x = center_x + (x - center_x) * (1.0 + (lateral_scale - 1.0) * side_weight)
    return (x, y, z)
