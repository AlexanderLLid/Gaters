"""Fresh-reopen verifier for the picture-to-mesh proof."""

import argparse
import hashlib
import json
import math
import sys
from pathlib import Path

import bpy

ROOT = Path(__file__).resolve().parent
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))


def arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True)
    return parser.parse_args(sys.argv[sys.argv.index("--") + 1:])


def topology_hash(polygons):
    faces = [tuple(polygon.vertices) for polygon in polygons]
    return hashlib.sha256(json.dumps(faces, separators=(",", ":")).encode("ascii")).hexdigest()


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def pgm_mask(path):
    tokens = []
    for line in path.read_text(encoding="ascii").splitlines():
        tokens.extend(line.split("#", 1)[0].split())
    width, height, maximum = map(int, tokens[1:4])
    values = list(map(int, tokens[4:]))
    return width, height, [value > maximum // 2 for value in values]


def png_mask(path):
    image = bpy.data.images.load(str(path), check_existing=False)
    width, height = image.size
    channels = image.channels
    pixels = image.pixels[:]
    mask = []
    for row in range(height):
        source_row = height - 1 - row
        for column in range(width):
            start = (source_row * width + column) * channels
            mask.append(max(pixels[start:start + min(3, channels)]) > 0.5)
    bpy.data.images.remove(image)
    return width, height, mask


def iou(target_path, render_path):
    target_width, target_height, target = pgm_mask(target_path)
    render_width, render_height, rendered = png_mask(render_path)
    if (target_width, target_height) != (render_width, render_height):
        raise ValueError("target and render dimensions differ")
    intersection = sum(a and b for a, b in zip(target, rendered))
    union = sum(a or b for a, b in zip(target, rendered))
    return intersection / union if union else 0.0


def main():
    output = Path(arguments().output).resolve()
    blend = output / "picture-to-mesh.blend"
    if Path(bpy.data.filepath).resolve() != blend:
        raise RuntimeError("verification must freshly reopen the generated blend")
    manifest = json.loads((output / "manifest.json").read_text(encoding="utf-8"))
    form = bpy.data.objects.get("PictureToMeshForm")
    fitted_key = form.data.shape_keys.key_blocks.get("Fitted") if form and form.data.shape_keys else None
    if form is None or fitted_key is None:
        raise RuntimeError("proof mesh or fitted shape key is missing")

    expected = manifest["topology"]
    topology_unchanged = (
        len(form.data.vertices) == expected["vertexCount"]
        and len(fitted_key.data) == expected["vertexCount"]
        and len(form.data.polygons) == expected["faceCount"]
        and topology_hash(form.data.polygons) == expected["connectivitySha256"]
    )
    basis = form.data.shape_keys.key_blocks["Basis"]
    depth_preserved = all(abs(a.co.y - b.co.y) < 1e-9 for a, b in zip(basis.data, fitted_key.data))
    fitted_key.value = 1.0
    bpy.context.view_layer.update()
    evaluated = form.evaluated_get(bpy.context.evaluated_depsgraph_get())
    evaluated_mesh = evaluated.to_mesh()
    finite = all(math.isfinite(component) for vertex in evaluated_mesh.vertices for component in vertex.co)
    minimum_area = min(polygon.area for polygon in evaluated_mesh.polygons)
    evaluated.to_mesh_clear()

    baseline = {"frontIoU": iou(output / "target-front.pgm", output / "baseline" / "front.png")}
    fitted = {"frontIoU": iou(output / "target-front.pgm", output / "fitted" / "front.png")}
    gates = {
        "frontFit": fitted["frontIoU"] >= 0.88 and fitted["frontIoU"] >= baseline["frontIoU"] + 0.15,
        "topology": topology_unchanged,
        "depthPreserved": depth_preserved,
        "finiteCoordinates": finite,
        "nonDegenerateFaces": minimum_area > 1e-6,
    }
    report = {
        "schemaVersion": 1,
        "promoted": all(gates.values()),
        "baseline": baseline,
        "fitted": fitted,
        "topologyUnchanged": topology_unchanged,
        "depthPreserved": depth_preserved,
        "minimumFaceArea": minimum_area,
        "gates": gates,
        "evidence": {
            "verifierSha256": sha256(Path(__file__).resolve()),
            "manifestSha256": sha256(output / "manifest.json"),
            "blendSha256": sha256(blend),
        },
        "decision": "promote-fixed-topology-silhouette-fit" if all(gates.values()) else "reject",
    }
    (output / "verification.json").write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if not report["promoted"]:
        raise RuntimeError(json.dumps(report, sort_keys=True))
    print(f"PICTURE_TO_MESH_VERIFY_OK output={output}")


if __name__ == "__main__":
    main()
