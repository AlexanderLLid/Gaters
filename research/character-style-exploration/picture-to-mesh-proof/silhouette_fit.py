"""Minimum fixed-topology fitter for paired orthographic silhouette masks."""

import math


def read_pgm_spans(path, threshold=0.5):
    tokens = []
    for line in path.read_text(encoding="ascii").splitlines():
        tokens.extend(line.split("#", 1)[0].split())
    if not tokens or tokens[0] != "P2":
        raise ValueError("expected an ASCII PGM (P2) image")
    width, height, maximum = map(int, tokens[1:4])
    pixels = list(map(int, tokens[4:]))
    if len(pixels) != width * height or width < 2 or maximum < 1:
        raise ValueError("invalid PGM dimensions or pixels")

    rows = []
    for row in range(height):
        occupied = [column for column in range(width) if pixels[row * width + column] > maximum * threshold]
        if occupied:
            normalize = lambda column: column * 2.0 / (width - 1) - 1.0
            rows.append((normalize(occupied[0]), normalize(occupied[-1])))
        else:
            rows.append(None)
    populated = [index for index, span in enumerate(rows) if span is not None]
    if not populated:
        raise ValueError("silhouette is empty")
    return list(reversed(rows[min(populated):max(populated) + 1]))


def _sample(spans, height):
    position = max(0.0, min(1.0, height)) * (len(spans) - 1)
    low = int(math.floor(position))
    high = min(len(spans) - 1, low + 1)
    blend = position - low
    if spans[low] is None or spans[high] is None:
        raise ValueError("silhouette contains an empty interior row")
    return tuple(spans[low][axis] * (1.0 - blend) + spans[high][axis] * blend for axis in (0, 1))


def fit_fixed_topology(vertices, front_spans):
    if not vertices:
        raise ValueError("mesh has no vertices")
    minimum_z = min(vertex[2] for vertex in vertices)
    maximum_z = max(vertex[2] for vertex in vertices)
    if maximum_z == minimum_z:
        raise ValueError("mesh has no height")

    fitted = []
    for x, y, z in vertices:
        height = (z - minimum_z) / (maximum_z - minimum_z)
        front_left, front_right = _sample(front_spans, height)
        angle = math.atan2(y, x)
        front_center, front_radius = (front_left + front_right) * 0.5, (front_right - front_left) * 0.5
        new_x = front_center + front_radius * math.cos(angle)
        fitted.append(tuple(0.0 if abs(value) < 1e-12 else value for value in (new_x, y, z)))
    return fitted
