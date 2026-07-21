"""Front-silhouette deformation around a fixed eye line."""

import bisect


def interpolate_profile(samples, height):
    heights = [sample[0] for sample in samples]
    if height <= heights[0]:
        return samples[0][1]
    if height >= heights[-1]:
        return samples[-1][1]
    upper = bisect.bisect_right(heights, height)
    low_height, low_width = samples[upper - 1]
    high_height, high_width = samples[upper]
    amount = (height - low_height) / (high_height - low_height)
    return low_width + (high_width - low_width) * amount


def smooth_profile(samples, radius):
    if radius <= 0.0:
        return list(samples)
    smoothed = []
    for height, width in samples:
        weighted = [
            (other_width, 1.0 - abs(other_height - height) / radius)
            for other_height, other_width in samples
            if abs(other_height - height) < radius
        ]
        total = sum(weight for _, weight in weighted)
        smoothed.append((height, sum(value * weight for value, weight in weighted) / total))
    return smoothed


def blend_valid_width(current_width, target_width, height, minimum_height, transition):
    if height <= minimum_height:
        return current_width
    if transition <= 0.0 or height >= minimum_height + transition:
        return target_width
    amount = (height - minimum_height) / transition
    return current_width + (target_width - current_width) * amount


def regional_profile_errors(candidate, target, regions):
    errors = {}
    for name, (minimum, maximum) in regions.items():
        samples = [(height, width) for height, width in target if minimum <= height <= maximum]
        if not samples:
            raise ValueError(f"profile region has no target samples: {name}")
        errors[name] = sum(
            abs(interpolate_profile(candidate, height) - width) for height, width in samples
        ) / len(samples)
    return errors


def visual_review_decision(regions):
    required = {"skull", "temples", "ears", "jawChin", "overall"}
    missing = required - set(regions)
    if missing:
        raise ValueError(f"missing visual head regions: {sorted(missing)}")
    invalid = {value for name, value in regions.items() if name in required} - {"pass", "reject"}
    if invalid:
        raise ValueError(f"invalid visual head decisions: {sorted(invalid)}")
    return "pass" if all(regions[name] == "pass" for name in required) else "reject"


def residual_corrected_width(target_width, candidate_width, maximum_fraction):
    ratio = target_width / candidate_width
    bounded = min(1.0 + maximum_fraction, max(1.0 - maximum_fraction, ratio))
    return target_width * bounded


def piecewise_vertical(height, current_top, current_chin, target_top, target_chin):
    if height >= 0.0:
        return height * target_top / current_top
    return height * target_chin / current_chin


def deform_head_point(point, *, center_x, eye_z, ipd, current_top, current_chin,
                      target_top, target_chin, current_half_width,
                      target_half_width, side_power, neck_fade,
                      minimum_valid_outer_height=None, valid_outer_blend=0.0,
                      minimum_vertical_height=None, vertical_blend=0.0):
    x, y, z = point
    height = (z - eye_z) / ipd
    if height < current_chin - neck_fade:
        return point
    if height < current_chin:
        influence = (height - (current_chin - neck_fade)) / neck_fade
        mapped_height = height + (target_chin - current_chin) * influence
        profile_height = target_chin
    else:
        influence = 1.0
        mapped_height = piecewise_vertical(
            height, current_top, current_chin, target_top, target_chin
        )
        profile_height = mapped_height

    if minimum_vertical_height is not None:
        if height <= minimum_vertical_height:
            vertical_influence = 0.0
        elif vertical_blend <= 0.0 or height >= minimum_vertical_height + vertical_blend:
            vertical_influence = 1.0
        else:
            vertical_influence = (height - minimum_vertical_height) / vertical_blend
        mapped_height = height + (mapped_height - height) * vertical_influence
        profile_height = mapped_height

    normalized_x = (x - center_x) / ipd
    source_width = current_half_width(height)
    desired_width = target_half_width(profile_height)
    if minimum_valid_outer_height is not None:
        desired_width = blend_valid_width(
            source_width, desired_width, profile_height,
            minimum_valid_outer_height, valid_outer_blend,
        )
    width_scale = desired_width / source_width
    side_weight = min(1.0, abs(normalized_x) / source_width) ** side_power
    adjusted_scale = 1.0 + (width_scale - 1.0) * side_weight * influence
    return (
        center_x + normalized_x * adjusted_scale * ipd,
        y,
        eye_z + mapped_height * ipd,
    )
