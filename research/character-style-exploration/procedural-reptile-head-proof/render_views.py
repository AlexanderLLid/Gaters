"""Render the fixed Apprentice evaluation views from the saved Houdini scene."""

from __future__ import annotations

import json
import sys
from pathlib import Path

import hou
from PIL import Image, ImageDraw, ImageFont


VIEWS = (
    ("CAM_GAMEPLAY", "A-gameplay.png", "A - STANDARD THIRD-PERSON DISTANCE"),
    ("CAM_CLOSE", "B-gameplay-close.png", "B - GAMEPLAY CLOSE DISTANCE"),
    ("CAM_INSPECT", "C-inspection.png", "C - INSPECTION CLOSE-UP"),
)


def render_views(scene_path: Path, output_dir: Path) -> None:
    scene_path = Path(scene_path).resolve()
    output_dir = Path(output_dir).resolve()
    output_dir.mkdir(parents=True, exist_ok=True)
    hou.hipFile.load(str(scene_path), suppress_save_prompt=True)

    driver = hou.node("/out/REPTILE_MANTRA")
    if driver is None:
        driver = hou.node("/out").createNode("ifd", "REPTILE_MANTRA")
    _set(driver, "trange", 0)
    _set(driver, "override_camerares", 1)
    _set(driver, "res_fraction", "specific")
    _set(driver, "res_overridex", 1280)
    _set(driver, "res_overridey", 720)
    _set(driver, "vm_renderengine", "pbrraytrace")

    rendered = []
    for camera_name, filename, label in VIEWS:
        path = output_dir / filename
        driver.parm("camera").set(f"/obj/{camera_name}")
        driver.parm("vm_picture").set(path.as_posix())
        driver.render(verbose=False)
        if not path.is_file():
            raise RuntimeError(f"Houdini OpenGL did not create {path}")
        _label(path, label)
        rendered.append(filename)

    evaluation = {
        "schema": "art-evaluation/0",
        "candidate": scene_path.stem,
        "renderer": "Houdini Mantra Apprentice",
        "renders": rendered,
        "labels": [view[2] for view in VIEWS],
        "humanAcceptance": False,
        "visibleFailure": "",
    }
    (output_dir / "evaluation.json").write_text(
        json.dumps(evaluation, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    print("REPTILE_HEAD_RENDER_PASS " + " ".join(rendered))


def _set(node: hou.Node, name: str, value) -> None:
    parm = node.parm(name)
    if parm is not None:
        parm.set(value)


def _label(path: Path, text: str) -> None:
    with Image.open(path) as source:
        image = source.convert("RGB")
    draw = ImageDraw.Draw(image, "RGBA")
    draw.rectangle((0, 0, image.width, 54), fill=(15, 17, 16, 225))
    font = ImageFont.load_default(size=22)
    draw.text((22, 15), text, font=font, fill=(238, 235, 219, 255))
    image.save(path)


if __name__ == "__main__":
    if len(sys.argv) != 3:
        raise SystemExit("usage: hython render_views.py SCENE OUTPUT_DIR")
    render_views(Path(sys.argv[1]), Path(sys.argv[2]))
