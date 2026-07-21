import math
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


BACKGROUND = (22, 24, 27)
PANEL = (31, 34, 39)
LEFT = (91, 173, 142)
RIGHT = (224, 151, 92)
CORE = (190, 197, 207)


def _font(size):
    try:
        return ImageFont.truetype("arial.ttf", size)
    except OSError:
        return ImageFont.load_default()


def _color(name):
    if name.startswith("left_"):
        return LEFT
    if name.startswith("right_"):
        return RIGHT
    return CORE


def _panel(image, skeleton, box, yaw_degrees, label, annotate=False):
    draw = ImageDraw.Draw(image)
    left, top, right, bottom = box
    draw.rounded_rectangle(box, radius=18, fill=PANEL)
    yaw = math.radians(yaw_degrees)
    screen_right = (math.cos(yaw), math.sin(yaw), 0.0)
    joints = {joint["name"]: joint for joint in skeleton["joints"]}

    def project(point):
        return point[0] * screen_right[0] + point[1] * screen_right[1], point[2]

    projected = [project(joint["position"]) for joint in joints.values()]
    min_x, max_x = min(point[0] for point in projected), max(point[0] for point in projected)
    min_z, max_z = min(point[1] for point in projected), max(point[1] for point in projected)
    scale = min((right - left - 110) / (max_x - min_x), (bottom - top - 150) / (max_z - min_z))
    center_x, center_z = (min_x + max_x) / 2, (min_z + max_z) / 2
    origin_x, origin_z = (left + right) / 2, (top + bottom) / 2 + 30

    def screen(point):
        x, z = project(point)
        return origin_x + (x - center_x) * scale, origin_z - (z - center_z) * scale

    for bone in skeleton["bones"]:
        parent, child = joints[bone["parent"]], joints[bone["child"]]
        draw.line((*screen(parent["position"]), *screen(child["position"])), fill=_color(child["name"]), width=8)
    for joint in joints.values():
        x, y = screen(joint["position"])
        color = _color(joint["name"])
        draw.ellipse((x - 8, y - 8, x + 8, y + 8), fill=color, outline=(14, 15, 17), width=2)
        if annotate and joint["name"] in {"pelvis", "chest", "head_center", "left_elbow", "left_wrist", "left_knee", "left_ankle"}:
            draw.text((x + 12, y - 8), joint["name"].replace("left_", ""), fill=(226, 229, 233), font=_font(15))
    ground = screen([0, 0, min(joint["position"][2] for joint in joints.values())])[1]
    draw.line((left + 32, ground, right - 32, ground), fill=(76, 81, 88), width=2)
    draw.text((left + 24, top + 18), label, fill=(238, 240, 243), font=_font(25))


def render(skeleton, output):
    image = Image.new("RGB", (1600, 900), BACKGROUND)
    draw = ImageDraw.Draw(image)
    draw.text((54, 34), "GUIDE SKELETON v0 - GENERATED FROM THE SURFACE GUIDE", fill=(244, 245, 247), font=_font(30))
    draw.text((54, 76), "16 joints / 15 bones / exact shared landmarks / no rig or skinning claim", fill=(165, 171, 180), font=_font(18))
    _panel(image, skeleton, (50, 120, 785, 840), 0, "A - FRONT SKELETON", annotate=True)
    _panel(image, skeleton, (815, 120, 1550, 840), 35, "B - THREE-QUARTER SKELETON")
    output = Path(output)
    output.parent.mkdir(parents=True, exist_ok=True)
    image.save(output)
    print(f"GUIDE_SKELETON_PREVIEW_PASS output={output}")
