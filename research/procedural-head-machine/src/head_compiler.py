import math


REGION_CENTERS = {
    "chin": -0.85,
    "jaw": -0.45,
    "face": 0.15,
    "skull": 0.75,
}


def _region_weights(vertical):
    raw = {
        name: max(0.0, 1.0 - abs(vertical - center) / 0.7)
        for name, center in REGION_CENTERS.items()
    }
    total = sum(raw.values())
    return {name: value / total for name, value in raw.items()}


def _shape_scale(vertical, jaw_width, chin_taper):
    jaw_influence = max(0.0, 1.0 - abs(vertical + 0.45) / 0.45)
    jaw_scale = 1.0 + (jaw_width - 1.0) * jaw_influence
    chin_influence = max(0.0, min(1.0, (-vertical - 0.65) / 0.35))
    chin_scale = 1.0 - (1.0 - chin_taper) * chin_influence
    return jaw_scale * chin_scale, chin_scale


def compile_recipe(recipe):
    if recipe.get("schema") != "head-recipe/0":
        raise ValueError("HEAD-SCHEMA-1")

    width = float(recipe["width_m"])
    height = float(recipe["height_m"])
    depth = float(recipe["depth_m"])
    jaw_width = float(recipe["jaw_width"])
    chin_taper = float(recipe["chin_taper"])
    rings = int(recipe["rings"])
    segments = int(recipe["segments"])
    if (
        min(width, height, depth, jaw_width, chin_taper) <= 0
        or jaw_width > 1
        or chin_taper > 1
        or rings < 4
        or segments < 8
    ):
        raise ValueError("HEAD-INPUT-1")

    half_width, half_height, half_depth = width / 2, height / 2, depth / 2
    vertices = [[0.0, 0.0, half_height]]
    regions = [_region_weights(1.0)]

    for ring in range(1, rings):
        polar = math.pi * ring / rings
        vertical = math.cos(polar)
        radial = math.sin(polar)
        width_scale, depth_scale = _shape_scale(vertical, jaw_width, chin_taper)
        for segment in range(segments):
            angle = 2 * math.pi * segment / segments
            vertices.append([
                round(half_width * radial * math.cos(angle) * width_scale, 12),
                round(half_depth * radial * math.sin(angle) * depth_scale, 12),
                round(half_height * vertical, 12),
            ])
            regions.append(_region_weights(vertical))

    bottom = len(vertices)
    vertices.append([0.0, 0.0, -half_height])
    regions.append(_region_weights(-1.0))

    faces = []
    first_ring = 1
    for segment in range(segments):
        next_segment = (segment + 1) % segments
        faces.append([0, first_ring + segment, first_ring + next_segment])

    ring_count = rings - 1
    for ring in range(ring_count - 1):
        current = first_ring + ring * segments
        following = current + segments
        for segment in range(segments):
            next_segment = (segment + 1) % segments
            faces.append([
                current + segment,
                current + next_segment,
                following + next_segment,
                following + segment,
            ])

    last_ring = first_ring + (ring_count - 1) * segments
    for segment in range(segments):
        next_segment = (segment + 1) % segments
        faces.append([bottom, last_ring + next_segment, last_ring + segment])

    return {
        "schema": "head-mesh/0",
        "recipe_id": str(recipe["id"]),
        "vertices": vertices,
        "faces": faces,
        "regions": regions,
    }
