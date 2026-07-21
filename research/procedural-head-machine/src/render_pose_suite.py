import argparse
import json
from collections import Counter
from pathlib import Path

from PIL import Image, ImageDraw

import render_body_plan_preview as preview


def render(readback, verification, output):
    image = Image.new("RGB", (1800, 1260), preview.BACKGROUND)
    draw = ImageDraw.Draw(image)
    draw.text((46, 28), "GENERATED MAJOR-JOINT POSE SUITE - ACTUAL DEFORMED MESH", fill=(244, 245, 247), font=preview._font(30))
    draw.text((46, 72), "One biharmonic capture / five data-driven poses / thickness measured at each joint", fill=(165, 171, 180), font=preview._font(18))
    boxes = [(40, 120, 590, 650), (625, 120, 1175, 650), (1210, 120, 1760, 650), (315, 690, 865, 1220), (935, 690, 1485, 1220)]
    face_modules = [Counter(readback["modules"][index] for index in face).most_common(1)[0][0] for face in readback["faces"]]
    failed = {failure["subject"] for failure in verification["failures"]}
    for box, (pose_id, artifact) in zip(boxes, readback["poses"].items()):
        metric = verification["poses"][pose_id]
        status = "FAIL" if pose_id in failed else "PASS"
        label = f"{pose_id.replace('_', ' ').upper()} | {status} | {metric['thickness_ratio_p10']:.1%}"
        body = {"vertices": artifact["positions"], "faces": readback["faces"], "face_modules": face_modules}
        preview._panel(draw, body, box, 35 if "twist" in pose_id or "knee" in pose_id or "hip" in pose_id else 0, label, False)
    output = Path(output)
    output.parent.mkdir(parents=True, exist_ok=True)
    image.save(output)
    print(f"POSE_SUITE_PREVIEW_PASS output={output}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("readback", type=Path)
    parser.add_argument("verification", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    render(
        json.loads(args.readback.read_text(encoding="utf-8")),
        json.loads(args.verification.read_text(encoding="utf-8")),
        args.output,
    )


if __name__ == "__main__":
    main()
