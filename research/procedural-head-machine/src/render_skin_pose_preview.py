import argparse
import json
from collections import Counter
from pathlib import Path

from render_body_plan_preview import render


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("readback", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    readback = json.loads(args.readback.read_text(encoding="utf-8"))
    face_modules = [Counter(readback["modules"][index] for index in face).most_common(1)[0][0] for face in readback["faces"]]
    render(
        {"vertices": readback["posed_positions"], "faces": readback["faces"], "face_modules": face_modules},
        args.output,
        "DIAGNOSTIC ELBOW POSE v0 - ACTUAL DEFORMED MESH",
        "55 degree generated joint pose / protected modules fixed / no deformation-quality claim",
        show_edges=False,
    )


if __name__ == "__main__":
    main()
