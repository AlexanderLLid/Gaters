import argparse
import json
from collections import defaultdict
from pathlib import Path

import render_body_plan_preview as preview


JOINT_COLORS = {
    "pelvis": (122, 128, 138), "waist": (142, 149, 160), "chest": (164, 174, 188), "head_center": (181, 142, 103),
    "left_shoulder": (70, 133, 113), "left_elbow": (78, 158, 128), "left_wrist": (97, 190, 151),
    "left_hip": (57, 115, 99), "left_knee": (67, 142, 117), "left_ankle": (86, 171, 136),
    "right_shoulder": (174, 105, 61), "right_elbow": (208, 126, 70), "right_wrist": (237, 153, 89),
    "right_hip": (145, 88, 54), "right_knee": (186, 108, 61), "right_ankle": (220, 135, 76),
}


def render(readback, output):
    face_modules = []
    for face in readback["faces"]:
        totals = defaultdict(float)
        for vertex in face:
            for index, weight in zip(readback["capture_indices"][vertex], readback["capture_weights"][vertex]):
                totals[readback["capture_paths"][index]] += weight
        face_modules.append(max(totals, key=totals.get))
    preview.COLORS.update(JOINT_COLORS)
    preview.render(
        {"vertices": readback["positions"], "faces": readback["faces"], "face_modules": face_modules},
        Path(output),
        "PROXIMITY SKIN WEIGHTS v0 - ACTUAL CAPTURED MESH",
        "Colour = dominant joint / max 2 influences / no deformation-quality claim",
        show_edges=False,
    )


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("readback", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    render(json.loads(args.readback.read_text(encoding="utf-8")), args.output)


if __name__ == "__main__":
    main()
