"""Render a labeled diagnostic PNG from a Houdini-exported OBJ proxy."""

from __future__ import annotations

import math
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


def load_obj(path: Path):
    vertices = []
    faces = []
    for line in Path(path).read_text(encoding="utf-8").splitlines():
        if line.startswith("v "):
            vertices.append(tuple(map(float, line.split()[1:4])))
        elif line.startswith("f "):
            faces.append([int(part.split("/")[0]) - 1 for part in line.split()[1:]])
    return vertices, faces


def render(vertices, faces, output: Path):
    scale = 2
    panel_width, height = 620 * scale, 700 * scale
    image = Image.new("RGB", (panel_width * 2, height), (20, 22, 24))
    draw = ImageDraw.Draw(image)
    views = [
        ("A - THREE-QUARTER", (8.0, -10.0, 6.0)),
        ("B - FRONT", (8.0, 0.0, 2.5)),
    ]
    for panel, (label, camera) in enumerate(views):
        projected, depths = _project(vertices, camera, panel_width, height)
        ordered = sorted(range(len(faces)), key=lambda index: sum(depths[i] for i in faces[index]) / len(faces[index]), reverse=True)
        for face_index in ordered:
            face = faces[face_index]
            points = [projected[index] for index in face]
            brightness = _brightness([vertices[index] for index in face])
            color = tuple(int(channel * brightness) for channel in (128, 145, 119))
            shifted = [(x + panel * panel_width, y) for x, y in points]
            draw.polygon(shifted, fill=color, outline=(34, 39, 34))
        x0 = panel * panel_width
        draw.rectangle((x0, 0, x0 + panel_width - 1, height - 1), outline=(72, 76, 78), width=2 * scale)
        draw.text((x0 + 24 * scale, 22 * scale), label, fill=(232, 232, 226), font=ImageFont.load_default())
    draw.text((24 * scale, height - 34 * scale), "HOUDINI CAPSULE PROXY V0 - STRUCTURE ONLY", fill=(190, 194, 187), font=ImageFont.load_default())
    image.resize((panel_width, height // scale), Image.Resampling.LANCZOS).save(output)


def _project(vertices, camera, width, height):
    target = tuple(sum(vertex[index] for vertex in vertices) / len(vertices) for index in range(3))
    forward = _normalize(_subtract(target, camera))
    right = _normalize(_cross(forward, (0.0, 0.0, 1.0)))
    up = _cross(right, forward)
    raw = [(sum((vertex[i] - target[i]) * right[i] for i in range(3)), sum((vertex[i] - target[i]) * up[i] for i in range(3))) for vertex in vertices]
    depths = [math.dist(vertex, camera) for vertex in vertices]
    min_x, max_x = min(x for x, _ in raw), max(x for x, _ in raw)
    min_y, max_y = min(y for _, y in raw), max(y for _, y in raw)
    fit = min((width - 100) / max(max_x - min_x, 1e-6), (height - 120) / max(max_y - min_y, 1e-6))
    center_x = (min_x + max_x) / 2.0
    center_y = (min_y + max_y) / 2.0
    projected = [
        (width / 2 + (x - center_x) * fit, height / 2 - (y - center_y) * fit)
        for x, y in raw
    ]
    return projected, depths


def _brightness(points):
    if len(points) < 3:
        return 0.6
    normal = _normalize(_cross(_subtract(points[1], points[0]), _subtract(points[2], points[0])))
    light = _normalize((0.4, -0.5, 1.0))
    return 0.36 + 0.64 * abs(sum(normal[index] * light[index] for index in range(3)))


def _subtract(a, b):
    return tuple(a[index] - b[index] for index in range(3))


def _cross(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])


def _normalize(value):
    length = math.sqrt(sum(component * component for component in value))
    return tuple(component / max(length, 1e-9) for component in value)


if __name__ == "__main__":
    if len(sys.argv) != 3:
        raise SystemExit("usage: python render_proxy_preview.py INPUT.obj OUTPUT.png")
    vertices, faces = load_obj(Path(sys.argv[1]))
    if len(vertices) < 1000 or len(faces) < 1000:
        raise SystemExit("proxy mesh is below the diagnostic minimum")
    render(vertices, faces, Path(sys.argv[2]))
