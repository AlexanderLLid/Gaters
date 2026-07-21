"""Compile one bounded raster-residual correction for the front head target."""

import argparse
import hashlib
import json
from pathlib import Path

from PIL import Image

from head_silhouette_fit import residual_corrected_width


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--target", required=True)
    parser.add_argument("--candidate", required=True)
    parser.add_argument("--alignment", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--minimum-height", type=float, default=-1.0)
    parser.add_argument("--maximum-correction", type=float, default=0.15)
    parser.add_argument("--target-chin", type=float, required=True)
    return parser.parse_args()


def visible_row_half_width(image, y, center_x, ipd, threshold, search_radius):
    offsets = [0] + [value for distance in range(1, search_radius + 1) for value in (-distance, distance)]
    for offset in offsets:
        row = y + offset
        if not 0 <= row < image.height:
            continue
        occupied = [x for x in range(image.width) if image.getpixel((x, row)) > threshold]
        if occupied:
            return (max(occupied) - min(occupied)) / 2.0 / ipd
    raise RuntimeError(f"candidate has no visible head pixels near row {y}")


def main():
    args = arguments()
    target_path = Path(args.target).resolve()
    candidate_path = Path(args.candidate).resolve()
    alignment_path = Path(args.alignment).resolve()
    target = json.loads(target_path.read_text(encoding="utf-8"))
    alignment = json.loads(alignment_path.read_text(encoding="utf-8"))
    image = Image.open(candidate_path).convert("L")
    center_x, eye_y = alignment["eyeCenter"]
    ipd = float(alignment["ipdPixels"])

    corrected = []
    for sample in target["samples"]:
        height = float(sample["height"])
        target_width = float(sample["halfWidth"])
        if height < args.minimum_height:
            width = target_width
        else:
            y = round(eye_y - height * ipd)
            candidate_width = visible_row_half_width(image, y, center_x, ipd, 45, 3)
            width = residual_corrected_width(
                target_width, candidate_width, args.maximum_correction
            )
        corrected.append({"height": height, "halfWidth": width})

    target["samples"] = corrected
    target["targetChin"] = args.target_chin
    target["chinPixelY"] = round(target["eyeCenterPixels"][1] - args.target_chin * target["ipdPixels"])
    target["calibration"] = {
        "method": "bounded eye-aligned raster residual",
        "sourceTarget": {"path": str(target_path), "sha256": sha256(target_path)},
        "candidate": {"path": str(candidate_path), "sha256": sha256(candidate_path)},
        "alignment": {"path": str(alignment_path), "sha256": sha256(alignment_path)},
        "minimumHeight": args.minimum_height,
        "maximumCorrection": args.maximum_correction,
    }
    Path(args.output).write_text(json.dumps(target, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"HEAD_TARGET_CALIBRATED output={Path(args.output).resolve()}")


if __name__ == "__main__":
    main()
