"""Minimum front-depth-image to fixed-topology relief fitter."""

import math


def read_pgm_depth(path):
    tokens = []
    for line in path.read_text(encoding="ascii").splitlines():
        tokens.extend(line.split("#", 1)[0].split())
    if not tokens or tokens[0] != "P2":
        raise ValueError("expected an ASCII PGM (P2) image")
    width, height, maximum = map(int, tokens[1:4])
    values = list(map(int, tokens[4:]))
    if width < 2 or height < 2 or maximum < 1 or len(values) != width * height:
        raise ValueError("invalid PGM depth image")
    rows = [
        [value / maximum for value in values[row * width:(row + 1) * width]]
        for row in range(height)
    ]
    return width, height, rows


def sample_depth(image, u, v):
    width, height, rows = image
    x = max(0.0, min(1.0, u)) * (width - 1)
    y = (1.0 - max(0.0, min(1.0, v))) * (height - 1)
    x0, y0 = int(math.floor(x)), int(math.floor(y))
    x1, y1 = min(width - 1, x0 + 1), min(height - 1, y0 + 1)
    tx, ty = x - x0, y - y0
    top = rows[y0][x0] * (1.0 - tx) + rows[y0][x1] * tx
    bottom = rows[y1][x0] * (1.0 - tx) + rows[y1][x1] * tx
    return top * (1.0 - ty) + bottom * ty


def fit_relief(vertices, image, depth_scale):
    if not vertices:
        raise ValueError("mesh has no vertices")
    minimum_x, maximum_x = min(v[0] for v in vertices), max(v[0] for v in vertices)
    minimum_z, maximum_z = min(v[2] for v in vertices), max(v[2] for v in vertices)
    if minimum_x == maximum_x or minimum_z == maximum_z:
        raise ValueError("mesh has no image-plane area")
    return [
        (
            x,
            y - depth_scale * sample_depth(
                image,
                (x - minimum_x) / (maximum_x - minimum_x),
                (z - minimum_z) / (maximum_z - minimum_z),
            ),
            z,
        )
        for x, y, z in vertices
    ]
