"""Fresh-reopen verifier for the single-picture depth proof."""

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

from depth_fit import read_pgm_depth, sample_depth


def arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True)
    return parser.parse_args(sys.argv[sys.argv.index("--") + 1:])


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def topology_hash(polygons):
    faces = [tuple(polygon.vertices) for polygon in polygons]
    return hashlib.sha256(json.dumps(faces, separators=(",", ":")).encode("ascii")).hexdigest()


def main():
    output = Path(arguments().output).resolve()
    blend = output / "picture-to-mesh-depth.blend"
    if Path(bpy.data.filepath).resolve() != blend:
        raise RuntimeError("verification must freshly reopen the generated depth blend")
    manifest = json.loads((output / "manifest.json").read_text(encoding="utf-8"))
    form = bpy.data.objects.get("DepthProofForm")
    keys = form.data.shape_keys.key_blocks if form and form.data.shape_keys else None
    if keys is None or keys.get("FittedDepth") is None:
        raise RuntimeError("depth proof mesh or shape key is missing")
    basis, fitted = keys["Basis"], keys["FittedDepth"]
    topology = manifest["topology"]
    topology_unchanged = (
        len(basis.data) == topology["vertexCount"]
        and len(fitted.data) == topology["vertexCount"]
        and len(form.data.polygons) == topology["faceCount"]
        and topology_hash(form.data.polygons) == topology["connectivitySha256"]
    )
    image_plane_unchanged = all(
        abs(a.co.x - b.co.x) < 1e-9 and abs(a.co.z - b.co.z) < 1e-9
        for a, b in zip(basis.data, fitted.data)
    )

    image = read_pgm_depth(output / "target-depth.pgm")
    minimum_x, maximum_x = min(p.co.x for p in basis.data), max(p.co.x for p in basis.data)
    minimum_z, maximum_z = min(p.co.z for p in basis.data), max(p.co.z for p in basis.data)
    depth_scale = manifest["recipe"]["depthScale"]
    squared_errors = []
    for original, result in zip(basis.data, fitted.data):
        expected = original.co.y - depth_scale * sample_depth(
            image,
            (original.co.x - minimum_x) / (maximum_x - minimum_x),
            (original.co.z - minimum_z) / (maximum_z - minimum_z),
        )
        squared_errors.append((result.co.y - expected) ** 2)
    depth_rmse = math.sqrt(sum(squared_errors) / len(squared_errors))

    maximum_stretch = 1.0
    for edge in form.data.edges:
        first, second = edge.vertices
        base_length = (basis.data[first].co - basis.data[second].co).length
        fit_length = (fitted.data[first].co - fitted.data[second].co).length
        maximum_stretch = max(maximum_stretch, fit_length / base_length)

    keys["FittedDepth"].value = 1.0
    bpy.context.view_layer.update()
    evaluated = form.evaluated_get(bpy.context.evaluated_depsgraph_get())
    evaluated_mesh = evaluated.to_mesh()
    finite = all(math.isfinite(component) for vertex in evaluated_mesh.vertices for component in vertex.co)
    minimum_area = min(polygon.area for polygon in evaluated_mesh.polygons)
    evaluated.to_mesh_clear()
    gates = {
        "depthRmse": depth_rmse <= 0.002,
        "topology": topology_unchanged,
        "imagePlaneCoordinates": image_plane_unchanged,
        "finiteCoordinates": finite,
        "nonDegenerateFaces": minimum_area > 1e-6,
        "boundedEdgeStretch": maximum_stretch <= 1.5,
    }
    promoted = all(gates.values())
    report = {
        "schemaVersion": 1,
        "promoted": promoted,
        "decision": "promote-single-picture-depth-relief" if promoted else "reject",
        "depthRmse": depth_rmse,
        "topologyUnchanged": topology_unchanged,
        "imagePlaneCoordinatesUnchanged": image_plane_unchanged,
        "minimumFaceArea": minimum_area,
        "maximumEdgeStretch": maximum_stretch,
        "gates": gates,
        "evidence": {
            "verifierSha256": sha256(Path(__file__).resolve()),
            "manifestSha256": sha256(output / "manifest.json"),
            "blendSha256": sha256(blend),
        },
    }
    (output / "verification.json").write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if not promoted:
        raise RuntimeError(json.dumps(report, sort_keys=True))
    print(f"PICTURE_TO_MESH_DEPTH_VERIFY_OK output={output}")


if __name__ == "__main__":
    main()
