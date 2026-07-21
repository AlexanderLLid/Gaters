import argparse
import json
from collections import Counter
from pathlib import Path

from PIL import Image, ImageDraw

import render_body_plan_preview as preview


def render(readback, verification, output):
    image = Image.new("RGB", (1800, 1260), preview.BACKGROUND)
    draw = ImageDraw.Draw(image)
    draw.text((46, 28), "GENERATED HOUDINI MOTION - ACTUAL DEFORMED MESH", fill=(244, 245, 247), font=preview._font(30))
    draw.text((46, 72), "Versioned curves -> bounded skeleton frames -> KineFX Joint Deform", fill=(165, 171, 180), font=preview._font(18))
    boxes = [(40, 120, 590, 650), (625, 120, 1175, 650), (1210, 120, 1760, 650), (315, 690, 865, 1220), (935, 690, 1485, 1220)]
    selected = [0, 3, 6, 9, 12]
    face_modules = [Counter(readback["modules"][index] for index in face).most_common(1)[0][0] for face in readback["faces"]]
    for box, index in zip(boxes, selected):
        frame = readback["frames"][index]
        state = "REST" if index in (0, len(readback["frames"]) - 1) else ("PEAK" if index == 6 else "TRANSITION")
        label = f"FRAME {index + 1:02d} | {frame['time_seconds']:.3f}s | {state}"
        body = {"vertices": frame["positions"], "faces": readback["faces"], "face_modules": face_modules}
        preview._panel(draw, body, box, 25, label, False)
    status = "PASS" if verification["passed"] else "FAIL"
    draw.text((1475, 36), status, fill=(82, 204, 137) if status == "PASS" else (235, 90, 90), font=preview._font(28))
    output.parent.mkdir(parents=True, exist_ok=True)
    image.save(output)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("readback", type=Path)
    parser.add_argument("verification", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    render(json.loads(args.readback.read_text(encoding="utf-8")), json.loads(args.verification.read_text(encoding="utf-8")), args.output)
    print(f"MOTION_SEQUENCE_PREVIEW_PASS output={args.output}")


if __name__ == "__main__":
    main()
