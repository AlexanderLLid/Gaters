"""Render labeled diagnostic views from the Houdini horn OBJ."""

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
    supersample = 2
    panel_width = 600 * supersample
    height = 760 * supersample
    image = Image.new("RGB", (panel_width * 3, height), (18, 19, 20))
    draw = ImageDraw.Draw(image)
    font_path = Path(r"C:\Windows\Fonts\segoeui.ttf")
    label_font = ImageFont.truetype(str(font_path), 21 * supersample)
    note_font = ImageFont.truetype(str(font_path), 13 * supersample)
    center = _mean(vertices)
    detail_center = _mean(vertices[: max(40, len(vertices) // 3)])
    views = (
        ("A  SIDE PROFILE", (0.0, -7.0, 0.8), center, 0.92),
        ("B  THREE-QUARTER", (5.0, -7.0, 3.5), center, 0.9),
        ("C  GROWTH RIDGES", (4.0, -7.0, 2.5), detail_center, 2.05),
    )
    for panel, (label, camera, target, zoom) in enumerate(views):
        projected, depths = _project(vertices, camera, target, panel_width, height, zoom)
        ordered = sorted(
            range(len(faces)),
            key=lambda index: sum(depths[vertex] for vertex in faces[index]) / len(faces[index]),
            reverse=True,
        )
        x_offset = panel * panel_width
        for face_index in ordered:
            face = faces[face_index]
            points = [(projected[index][0] + x_offset, projected[index][1]) for index in face]
            brightness = _brightness([vertices[index] for index in face])
            color = tuple(int(channel * brightness) for channel in (176, 122, 66))
            draw.polygon(points, fill=color, outline=(54, 39, 27))
        draw.rectangle(
            (x_offset, 0, x_offset + panel_width - 1, height - 1),
            outline=(77, 78, 76),
            width=2 * supersample,
        )
        draw.text((x_offset + 26 * supersample, 24 * supersample), label, fill=(237, 232, 220), font=label_font)

    draw.text(
        (26 * supersample, height - 34 * supersample),
        "PROCEDURAL RAM HORN V0  |  LOG-SPIRAL PATH + POWER-LAW TAPER + KERATIN RIDGES",
        fill=(185, 180, 169),
        font=note_font,
    )
    image.resize((panel_width * 3 // supersample, height // supersample), Image.Resampling.LANCZOS).save(output)


def _project(vertices, camera, target, width, height, zoom):
    forward = _normalize(_subtract(target, camera))
    right = _normalize(_cross(forward, (0.0, 0.0, 1.0)))
    up = _cross(right, forward)
    coordinates = [
        (
            _dot(_subtract(vertex, target), right),
            _dot(_subtract(vertex, target), up),
        )
        for vertex in vertices
    ]
    depths = [math.dist(vertex, camera) for vertex in vertices]
    min_x, max_x = min(x for x, _ in coordinates), max(x for x, _ in coordinates)
    min_y, max_y = min(y for _, y in coordinates), max(y for _, y in coordinates)
    fit = min((width - 110) / max(max_x - min_x, 1e-6), (height - 130) / max(max_y - min_y, 1e-6)) * zoom
    projected = [
        (width / 2 + x * fit, height / 2 - y * fit)
        for x, y in coordinates
    ]
    return projected, depths


def _brightness(points):
    if len(points) < 3:
        return 0.7
    normal = _normalize(_cross(_subtract(points[1], points[0]), _subtract(points[2], points[0])))
    light = _normalize((0.4, -0.65, 1.0))
    return 0.34 + 0.72 * abs(_dot(normal, light))


def _mean(points):
    return tuple(sum(point[axis] for point in points) / len(points) for axis in range(3))


def _subtract(a, b):
    return tuple(a[index] - b[index] for index in range(3))


def _dot(a, b):
    return sum(a[index] * b[index] for index in range(3))


def _cross(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])


def _normalize(value):
    length = math.sqrt(_dot(value, value))
    if length < 1e-12:
        return (0.0, 0.0, 1.0)
    return tuple(component / length for component in value)


if __name__ == "__main__":
    if len(sys.argv) != 3:
        raise SystemExit("usage: python render_preview.py INPUT.obj OUTPUT.png")
    vertices, faces = load_obj(Path(sys.argv[1]))
    if len(vertices) < 1000 or len(faces) < 1000:
        raise SystemExit("horn mesh is below the diagnostic detail floor")
    render(vertices, faces, Path(sys.argv[2]))
