"""Fresh-reopen verifier for recognizable closed-mask fitting."""

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

from verify_proof import png_mask


def arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True)
    return parser.parse_args(sys.argv[sys.argv.index("--") + 1:])


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def topology_hash(polygons):
    faces = [tuple(polygon.vertices) for polygon in polygons]
    return hashlib.sha256(json.dumps(faces, separators=(",", ":")).encode("ascii")).hexdigest()


def target_mask(path):
    tokens = []
    for line in path.read_text(encoding="ascii").splitlines():
        tokens.extend(line.split("#", 1)[0].split())
    width, height, maximum = map(int, tokens[1:4])
    values = list(map(int, tokens[4:]))
    return width, height, [value > maximum * 0.02 for value in values]


def silhouette_iou(target_path, render_path):
    target_width, target_height, target = target_mask(target_path)
    render_width, render_height, rendered = png_mask(render_path)
    if (target_width, target_height) != (render_width, render_height):
        raise ValueError("mask dimensions differ")
    intersection = sum(a and b for a, b in zip(target, rendered))
    union = sum(a or b for a, b in zip(target, rendered))
    return intersection / union


def probe_delta(silhouette, fitted, x, z):
    candidates = [
        (index, (point.co.x - x) ** 2 + (point.co.z - z) ** 2)
        for index, point in enumerate(silhouette.data)
        if point.co.y < 0.0
    ]
    minimum = min(distance for _, distance in candidates)
    nearby = [index for index, distance in candidates if distance <= minimum + 0.0015]
    index = min(nearby, key=lambda candidate: silhouette.data[candidate].co.y)
    return silhouette.data[index].co.y - fitted.data[index].co.y


def main():
    output = Path(arguments().output).resolve()
    blend = output / "picture-to-mesh-mask.blend"
    if Path(bpy.data.filepath).resolve() != blend:
        raise RuntimeError("verification must freshly reopen the mask blend")
    manifest = json.loads((output / "manifest.json").read_text(encoding="utf-8"))
    form = bpy.data.objects.get("MaskProofForm")
    keys = form.data.shape_keys.key_blocks if form and form.data.shape_keys else None
    if keys is None or keys.get("SilhouetteOnly") is None or keys.get("FittedMask") is None:
        raise RuntimeError("mask proof shape keys are missing")
    basis, silhouette, fitted = keys["Basis"], keys["SilhouetteOnly"], keys["FittedMask"]
    expected = manifest["topology"]
    topology_unchanged = (
        len(fitted.data) == expected["vertexCount"] and len(form.data.polygons) == expected["faceCount"]
        and topology_hash(form.data.polygons) == expected["connectivitySha256"]
    )
    rear_preserved = all(
        (a.co - b.co).length < 1e-9
        for a, b in zip(silhouette.data, fitted.data)
        if a.co.y >= 0.0
    )
    settings = manifest["variantSettings"]
    nose_x = settings["noseX"]
    deltas = {
        "nose": probe_delta(silhouette, fitted, nose_x, -0.08),
        "leftBrow": probe_delta(silhouette, fitted, -0.23, 0.34),
        "rightBrow": probe_delta(silhouette, fitted, 0.23, 0.34),
        "leftEye": probe_delta(silhouette, fitted, -0.23, 0.19),
        "rightEye": probe_delta(silhouette, fitted, 0.23, 0.19),
        "leftCheek": probe_delta(silhouette, fitted, -0.31, -0.06),
        "rightCheek": probe_delta(silhouette, fitted, 0.31, -0.06),
        "mouth": probe_delta(silhouette, fitted, 0.0, -0.35),
        "chin": probe_delta(silhouette, fitted, 0.0, -0.63),
    }
    relations = {
        "noseAboveCheeks": deltas["nose"] > (deltas["leftCheek"] + deltas["rightCheek"]) * 0.5 + 0.025,
        "leftBrowAboveEye": deltas["leftBrow"] > deltas["leftEye"] + 0.025,
        "rightBrowAboveEye": deltas["rightBrow"] > deltas["rightEye"] + 0.025,
        "leftCheekAboveEye": deltas["leftCheek"] > deltas["leftEye"] + 0.02,
        "rightCheekAboveEye": deltas["rightCheek"] > deltas["rightEye"] + 0.02,
        "chinAboveMouthGroove": deltas["chin"] > deltas["mouth"] + 0.02,
    }
    maximum_stretch = 1.0
    for edge in form.data.edges:
        first, second = edge.vertices
        base_length = (basis.data[first].co - basis.data[second].co).length
        fit_length = (fitted.data[first].co - fitted.data[second].co).length
        maximum_stretch = max(maximum_stretch, fit_length / base_length)
    fitted.value = 1.0
    bpy.context.view_layer.update()
    evaluated = form.evaluated_get(bpy.context.evaluated_depsgraph_get())
    evaluated_mesh = evaluated.to_mesh()
    finite = all(math.isfinite(component) for vertex in evaluated_mesh.vertices for component in vertex.co)
    minimum_area = min(polygon.area for polygon in evaluated_mesh.polygons)
    evaluated.to_mesh_clear()
    front_iou = silhouette_iou(output / "target-mask.pgm", output / "renders" / "front-mask.png")
    gates = {
        "frontSilhouette": front_iou >= 0.95,
        "depthRelations": all(relations.values()),
        "topology": topology_unchanged,
        "rearRelief": rear_preserved,
        "finiteCoordinates": finite,
        "nonDegenerateFaces": minimum_area > 1e-6,
        "boundedEdgeStretch": maximum_stretch <= 1.75,
    }
    promoted = all(gates.values())
    report = {
        "schemaVersion": 1,
        "variant": manifest["variant"],
        "promoted": promoted,
        "decision": "promote-recognizable-mask-fit" if promoted else "reject",
        "frontSilhouetteIoU": front_iou,
        "topologyUnchanged": topology_unchanged,
        "rearReliefPreserved": rear_preserved,
        "featureDepthDeltas": deltas,
        "depthRelations": relations,
        "maximumEdgeStretch": maximum_stretch,
        "minimumFaceArea": minimum_area,
        "gates": gates,
        "evidence": {"verifierSha256": sha256(Path(__file__).resolve()),
                     "manifestSha256": sha256(output / "manifest.json"), "blendSha256": sha256(blend)},
    }
    (output / "verification.json").write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if not promoted:
        raise RuntimeError(json.dumps(report, sort_keys=True))
    print(f"PICTURE_TO_MESH_MASK_VERIFY_OK variant={manifest['variant']} output={output}")


if __name__ == "__main__":
    main()
