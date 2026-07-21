"""Extract a normalized front head-width profile from the approved bald target."""

import argparse
import hashlib
import json
from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parent


def extract_profile(image, *, threshold, eye_center, ipd, chin_y, step):
    spans = {}
    for y in range(chin_y + 1):
        occupied = [
            x for x in range(image.width)
            if sum(image.getpixel((x, y))[:3]) / 3.0 > threshold
        ]
        if occupied:
            spans[y] = (min(occupied), max(occupied))
    if not spans:
        raise ValueError("target head silhouette is empty")
    top = min(spans)
    rows = list(range(top, chin_y + 1, step))
    if rows[-1] != chin_y:
        rows.append(chin_y)
    samples = []
    for y in rows:
        if y not in spans:
            continue
        left, right = spans[y]
        samples.append({
            "height": (eye_center[1] - y) / ipd,
            "halfWidth": ((eye_center[0] - left) + (right - eye_center[0])) / 2.0 / ipd,
        })
    return {
        "targetTop": (eye_center[1] - top) / ipd,
        "targetChin": (eye_center[1] - chin_y) / ipd,
        "samples": samples,
    }


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--image", required=True)
    parser.add_argument("--landmarks", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--threshold", type=float, default=45.0)
    parser.add_argument("--step", type=int, default=4)
    args = parser.parse_args()
    image_path = Path(args.image).resolve()
    landmark_path = Path(args.landmarks).resolve()
    target = json.loads(landmark_path.read_text(encoding="utf-8"))
    points = target["points"]
    eye_center = (
        (points["leftEye"][0] + points["rightEye"][0]) / 2.0,
        (points["leftEye"][1] + points["rightEye"][1]) / 2.0,
    )
    ipd = abs(points["rightEye"][0] - points["leftEye"][0])
    profile = extract_profile(
        Image.open(image_path).convert("RGB"), threshold=args.threshold,
        eye_center=eye_center, ipd=ipd, chin_y=int(points["chin"][1]), step=args.step,
    )
    profile.update({
        "schemaVersion": 1,
        "image": {"path": str(image_path), "sha256": sha256(image_path)},
        "landmarks": {"path": str(landmark_path), "sha256": sha256(landmark_path)},
        "threshold": args.threshold,
        "stepPixels": args.step,
        "eyeCenterPixels": eye_center,
        "ipdPixels": ipd,
        "chinPixelY": int(points["chin"][1]),
    })
    Path(args.output).write_text(json.dumps(profile, indent=2, sort_keys=True) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
