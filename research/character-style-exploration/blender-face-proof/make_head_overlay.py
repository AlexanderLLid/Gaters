"""Create an eye-aligned, head-only silhouette comparison."""

import argparse
import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


def arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--candidate", required=True)
    parser.add_argument("--candidate-alignment", required=True)
    parser.add_argument("--target-profile", required=True)
    parser.add_argument("--output", required=True)
    return parser.parse_args()


def row_mask(image, threshold, top, bottom):
    gray = image.convert("L")
    mask = Image.new("1", image.size)
    source = gray.load()
    result = mask.load()
    for y in range(max(0, top), min(image.height, bottom + 1)):
        occupied = [x for x in range(image.width) if source[x, y] > threshold]
        if occupied:
            left, right = min(occupied), max(occupied)
            for x in range(left, right + 1):
                result[x, y] = 1
    return mask


def profile_mask(size, samples, eye_center, ipd, top, bottom):
    ordered = sorted((float(item["height"]), float(item["halfWidth"])) for item in samples)
    mask = Image.new("1", size)
    result = mask.load()
    for y in range(max(0, top), min(size[1], bottom + 1)):
        height = (eye_center[1] - y) / ipd
        nearest = min(ordered, key=lambda item: abs(item[0] - height))
        half_width = nearest[1] * ipd
        left = max(0, round(eye_center[0] - half_width))
        right = min(size[0] - 1, round(eye_center[0] + half_width))
        for x in range(left, right + 1):
            result[x, y] = 1
    return mask


def main():
    args = arguments()
    candidate = Image.open(args.candidate).convert("RGB")
    candidate_alignment = json.loads(Path(args.candidate_alignment).read_text(encoding="utf-8"))
    target_profile = json.loads(Path(args.target_profile).read_text(encoding="utf-8"))
    target = Image.open(target_profile["image"]["path"]).convert("RGB")

    candidate_center = candidate_alignment["eyeCenter"]
    candidate_ipd = candidate_alignment["ipdPixels"]
    target_center = target_profile["eyeCenterPixels"]
    target_ipd = target_profile["ipdPixels"]
    scale = candidate_ipd / target_ipd
    scaled = target.resize((round(target.width * scale), round(target.height * scale)), Image.Resampling.LANCZOS)
    aligned_target = Image.new("RGB", candidate.size)
    offset = (
        round(candidate_center[0] - target_center[0] * scale),
        round(candidate_center[1] - target_center[1] * scale),
    )
    aligned_target.paste(scaled, offset)

    top = round(candidate_center[1] - float(target_profile["targetTop"]) * candidate_ipd) - 2
    bottom = round(candidate_center[1] - float(target_profile["targetChin"]) * candidate_ipd) + 2
    target_mask = profile_mask(candidate.size, target_profile["samples"], candidate_center,
                               candidate_ipd, top, bottom)
    candidate_mask = row_mask(candidate, 45, top, bottom)

    half_width = round(candidate_ipd * 1.75)
    crop = (
        round(candidate_center[0] - half_width),
        top - 18,
        round(candidate_center[0] + half_width),
        bottom + 18,
    )
    target_head = aligned_target.crop(crop)
    candidate_head = candidate.crop(crop)
    output_path = Path(args.output)
    target_head.save(output_path.with_name("a-target-head-aligned.png"))
    candidate_head.save(output_path.with_name("b-generated-head-aligned.png"))
    wipe = target_head.copy()
    middle = wipe.width // 2
    wipe.paste(candidate_head.crop((middle, 0, wipe.width, wipe.height)), (middle, 0))
    wipe_draw = ImageDraw.Draw(wipe)
    wipe_draw.line((middle, 0, middle, wipe.height), fill=(255, 224, 64), width=2)
    wipe_draw.text((8, 8), "A TARGET", fill=(39, 196, 214), font=ImageFont.load_default())
    wipe_draw.text((middle + 8, 8), "B GENERATED", fill=(225, 73, 143), font=ImageFont.load_default())
    wipe.save(output_path.with_name("c-center-wipe.png"))

    frames = []
    for image, label, color in (
        (target_head, "A TARGET", (39, 196, 214)),
        (candidate_head, "B GENERATED", (225, 73, 143)),
    ):
        frame = image.copy()
        frame_draw = ImageDraw.Draw(frame)
        frame_draw.rectangle((6, 6, 112, 26), fill=(26, 27, 30))
        frame_draw.text((10, 10), label, fill=color, font=ImageFont.load_default())
        frames.append(frame)
    frames[0].save(output_path.with_name("d-aligned-blink.gif"), save_all=True,
                   append_images=frames[1:], duration=900, loop=0, disposal=2)
    target_crop = target_mask.crop(crop)
    candidate_crop = candidate_mask.crop(crop)
    canvas = Image.new("RGB", target_crop.size, (26, 27, 30))
    pixels = canvas.load()
    a = target_crop.load()
    b = candidate_crop.load()
    for y in range(canvas.height):
        for x in range(canvas.width):
            if a[x, y] and b[x, y]:
                pixels[x, y] = (156, 158, 163)
            elif a[x, y]:
                pixels[x, y] = (39, 196, 214)
            elif b[x, y]:
                pixels[x, y] = (225, 73, 143)

    scale_out = 2
    canvas = canvas.resize((canvas.width * scale_out, canvas.height * scale_out), Image.Resampling.NEAREST)
    presentation = Image.new("RGB", (canvas.width, canvas.height + 90), (26, 27, 30))
    presentation.paste(canvas, (0, 90))
    canvas = presentation
    draw = ImageDraw.Draw(canvas)
    font = ImageFont.load_default()
    draw.text((24, 22), "A TARGET ONLY", fill=(39, 196, 214), font=font)
    draw.text((24, 50), "B GENERATED ONLY", fill=(225, 73, 143), font=font)
    draw.text((255, 22), "OVERLAP", fill=(156, 158, 163), font=font)
    Path(args.output).parent.mkdir(parents=True, exist_ok=True)
    canvas.save(args.output)
    print(f"HEAD_OVERLAY_OK output={args.output}")


if __name__ == "__main__":
    main()
