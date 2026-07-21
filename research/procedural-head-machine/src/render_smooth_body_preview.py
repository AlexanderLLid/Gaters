import argparse
import json
from pathlib import Path

from render_body_plan_preview import render


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("readback", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    surface = json.loads(args.readback.read_text(encoding="utf-8"))
    render(
        surface,
        args.output,
        "SMOOTH BODY SURFACE v0 — ACTUAL GENERATED MESH",
        "Houdini VDB surface • accepted structural checks • no anatomy or art-style claim",
        show_edges=False,
    )


if __name__ == "__main__":
    main()
