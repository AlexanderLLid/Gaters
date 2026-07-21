import argparse
import json
import math
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


BACKGROUND = (22, 24, 27)
PANEL = (31, 34, 39)
EDGE = (13, 15, 18)
COLORS = {
    "torso": (117, 139, 158),
    "head": (174, 133, 96),
    "left_arm": (104, 148, 126),
    "right_arm": (104, 148, 126),
    "left_leg": (81, 119, 105),
    "right_leg": (81, 119, 105),
}


def _font(size):
    try:
        return ImageFont.truetype("arial.ttf", size)
    except OSError:
        return ImageFont.load_default()


def _normal(a, b, c):
    ab = [b[index] - a[index] for index in range(3)]
    ac = [c[index] - a[index] for index in range(3)]
    return (
        ab[1] * ac[2] - ab[2] * ac[1],
        ab[2] * ac[0] - ab[0] * ac[2],
        ab[0] * ac[1] - ab[1] * ac[0],
    )


def _lit_color(base, normal):
    magnitude = math.sqrt(sum(value * value for value in normal)) or 1.0
    unit = [value / magnitude for value in normal]
    light = (0.35, -0.55, 0.76)
    light_magnitude = math.sqrt(sum(value * value for value in light))
    diffuse = max(0.0, sum(unit[index] * light[index] / light_magnitude for index in range(3)))
    factor = 0.45 + 0.55 * diffuse
    return tuple(round(value * factor) for value in base)


def _panel(draw, body, box, yaw_degrees, label, show_edges, solid_color=None):
    solid_color = body.get("_preview_solid_color", solid_color)
    left, top, right, bottom = box
    draw.rounded_rectangle(box, radius=18, fill=PANEL)
    yaw = math.radians(yaw_degrees)
    camera = (math.sin(yaw), -math.cos(yaw), 0.0)
    screen_right = (math.cos(yaw), math.sin(yaw), 0.0)
    projected = []
    for x, y, z in body["vertices"]:
        projected.append((x * screen_right[0] + y * screen_right[1], z, x * camera[0] + y * camera[1]))
    min_x, max_x = min(point[0] for point in projected), max(point[0] for point in projected)
    min_z, max_z = min(point[1] for point in projected), max(point[1] for point in projected)
    width, height = right - left - 80, bottom - top - 130
    scale = min(width / (max_x - min_x), height / (max_z - min_z))
    center_x = (min_x + max_x) / 2
    center_z = (min_z + max_z) / 2
    origin_x = (left + right) / 2
    origin_z = (top + bottom) / 2 + 25

    def screen(index):
        x, z, _ = projected[index]
        return (origin_x + (x - center_x) * scale, origin_z - (z - center_z) * scale)

    visible = []
    for face, module in zip(body["faces"], body["face_modules"]):
        normal = _normal(*(body["vertices"][index] for index in face[:3]))
        facing = sum(normal[index] * camera[index] for index in range(3))
        if facing > 1e-10:
            depth = sum(projected[index][2] for index in face) / len(face)
            visible.append((depth, face, module, normal))
    for _, face, module, normal in sorted(visible):
        points = [screen(index) for index in face]
        color = _lit_color(solid_color or COLORS.get(module, (140, 140, 140)), normal)
        if show_edges:
            draw.polygon(points, fill=color, outline=EDGE, width=2)
        else:
            draw.polygon(points, fill=color)

    ground_y = origin_z - (0 - center_z) * scale
    draw.line((left + 32, ground_y, right - 32, ground_y), fill=(76, 81, 88), width=2)
    draw.text((left + 24, top + 18), label, fill=(238, 240, 243), font=_font(25))
    draw.text((left + 24, bottom - 48), f"yaw {yaw_degrees}°  |  actual accepted mesh", fill=(164, 170, 178), font=_font(17))


def render(
    body,
    output,
    title="BODYPLAN v0 — STRUCTURAL HUMANOID",
    subtitle="Generated geometry preview • no AI-painted detail • no anatomy claim",
    show_edges=True,
    solid_color=None,
):
    if solid_color is not None:
        body = {**body, "_preview_solid_color": solid_color}
    scale = 1
    image = Image.new("RGB", (1600 * scale, 900 * scale), BACKGROUND)
    draw = ImageDraw.Draw(image)
    draw.text((54 * scale, 34 * scale), title, fill=(244, 245, 247), font=_font(30 * scale))
    draw.text((54 * scale, 76 * scale), subtitle, fill=(165, 171, 180), font=_font(18 * scale))
    _panel(draw, body, (50 * scale, 120 * scale, 785 * scale, 840 * scale), 0, "A — FRONT", show_edges)
    _panel(draw, body, (815 * scale, 120 * scale, 1550 * scale, 840 * scale), 35, "B — THREE-QUARTER", show_edges)
    image = image.resize((1600, 900), Image.Resampling.LANCZOS)
    output.parent.mkdir(parents=True, exist_ok=True)
    image.save(output)
    pixels = image.getdata()
    non_background = sum(pixel != BACKGROUND for pixel in pixels)
    if non_background < 100000:
        raise RuntimeError("BODY_PLAN_PREVIEW_EMPTY")
    print(f"BODY_PLAN_PREVIEW_PASS output={output} non_background_pixels={non_background}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("body_run", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--title", default="BODYPLAN v0 — STRUCTURAL HUMANOID")
    parser.add_argument("--subtitle", default="Generated geometry preview • no AI-painted detail • no anatomy claim")
    args = parser.parse_args()
    body = json.loads((args.body_run / "composed-mesh.json").read_text(encoding="utf-8"))
    render(body, args.output, args.title, args.subtitle)


if __name__ == "__main__":
    main()
