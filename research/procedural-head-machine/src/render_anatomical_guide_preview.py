import math
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


BACKGROUND = (22, 24, 27)
PANEL = (31, 34, 39)
COLORS = {
    "core": (117, 139, 158, 160),
    "head": (174, 133, 96, 175),
    "limb": (95, 137, 116, 180),
}


def _font(size):
    try:
        return ImageFont.truetype("arial.ttf", size)
    except OSError:
        return ImageFont.load_default()


def _panel(image, guide, box, yaw_degrees, label, annotate=False):
    layer = Image.new("RGBA", image.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer)
    left, top, right, bottom = box
    draw.rounded_rectangle(box, radius=18, fill=PANEL + (255,))
    yaw = math.radians(yaw_degrees)
    screen_right = (math.cos(yaw), math.sin(yaw), 0.0)

    def project(point):
        return point[0] * screen_right[0] + point[1] * screen_right[1], point[2]

    extent_points = list(guide["landmarks"].values())
    for ellipsoid in guide["surface_ellipsoids"]:
        center, radii = ellipsoid["center"], ellipsoid["radii"]
        extent_points.extend([
            [center[0] - radii[0], center[1] - radii[1], center[2] - radii[2]],
            [center[0] + radii[0], center[1] + radii[1], center[2] + radii[2]],
        ])
    projected = [project(point) for point in extent_points]
    min_x, max_x = min(point[0] for point in projected), max(point[0] for point in projected)
    min_z, max_z = min(point[1] for point in projected), max(point[1] for point in projected)
    scale = min((right - left - 100) / (max_x - min_x), (bottom - top - 140) / (max_z - min_z))
    center_x, center_z = (min_x + max_x) / 2, (min_z + max_z) / 2
    origin_x, origin_z = (left + right) / 2, (top + bottom) / 2 + 30

    def screen(point):
        x, z = project(point)
        return origin_x + (x - center_x) * scale, origin_z - (z - center_z) * scale

    for ellipsoid in guide["surface_ellipsoids"]:
        center, radii = ellipsoid["center"], ellipsoid["radii"]
        x, z = screen(center)
        rx = math.sqrt((radii[0] * math.cos(yaw)) ** 2 + (radii[1] * math.sin(yaw)) ** 2) * scale
        rz = radii[2] * scale
        draw.ellipse((x - rx, z - rz, x + rx, z + rz), fill=COLORS[ellipsoid["role"]])
    for segment in guide["surface_segments"]:
        start = screen(guide["landmarks"][segment["start"]])
        end = screen(guide["landmarks"][segment["end"]])
        width = max(3, round((segment["start_radius_m"] + segment["end_radius_m"]) * scale))
        draw.line((*start, *end), fill=COLORS[segment["role"]], width=width)
    for segment in guide["skeleton_segments"]:
        draw.line((*screen(guide["landmarks"][segment["start"]]), *screen(guide["landmarks"][segment["end"]])), fill=(20, 22, 25, 220), width=3)
    for name, point in guide["landmarks"].items():
        x, y = screen(point)
        radius = 5 if name in {"pelvis", "chest", "neck_base", "head_center"} else 4
        draw.ellipse((x - radius, y - radius, x + radius, y + radius), fill=(239, 241, 244, 255), outline=(20, 22, 25, 255), width=2)
        if annotate and name in {"neck_base", "left_elbow", "left_wrist", "left_knee", "left_ankle"}:
            draw.text((x + 8, y - 7), name.replace("left_", ""), fill=(220, 224, 229, 255), font=_font(14))
    ground = screen([0, 0, min(point[2] for point in guide["landmarks"].values())])[1]
    draw.line((left + 32, ground, right - 32, ground), fill=(76, 81, 88, 255), width=2)
    draw.text((left + 24, top + 18), label, fill=(238, 240, 243, 255), font=_font(25))
    image.alpha_composite(layer)


def render(guide, output):
    image = Image.new("RGBA", (1600, 900), BACKGROUND + (255,))
    draw = ImageDraw.Draw(image)
    draw.text((54, 34), "ANATOMICAL GUIDE v0 — GENERATED FROM BODYPLAN", fill=(244, 245, 247, 255), font=_font(30))
    draw.text((54, 76), "Landmarks, joint chains, tapered radii and surface volumes • no anatomy-quality claim", fill=(165, 171, 180, 255), font=_font(18))
    _panel(image, guide, (50, 120, 785, 840), 0, "A — FRONT GUIDE", annotate=True)
    _panel(image, guide, (815, 120, 1550, 840), 35, "B — THREE-QUARTER GUIDE")
    output = Path(output)
    output.parent.mkdir(parents=True, exist_ok=True)
    image.convert("RGB").save(output)
    print(f"ANATOMICAL_GUIDE_PREVIEW_PASS output={output}")
