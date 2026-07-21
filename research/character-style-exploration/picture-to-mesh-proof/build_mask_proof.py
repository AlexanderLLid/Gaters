"""Build a recognizable closed mask from one grayscale front picture."""

import argparse
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Vector

ROOT = Path(__file__).resolve().parent
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from build_proof import clear_scene, sha256, topology_hash, write_pgm, write_png
from depth_fit import read_pgm_depth
from mask_fit import fit_closed_mask
from silhouette_fit import fit_fixed_topology, read_pgm_spans


RESOLUTION = 256
LEVELS = 48
SEGMENTS = 64
DEPTH_SCALE = 0.36
NEUTRAL_DEPTH = 0.42
SMOOTH_ITERATIONS = 16
SMOOTH_FACTOR = 0.35
SOURCE_WEIGHT = 0.063
VARIANTS = {
    "balanced": {"width": 1.0, "jaw": 0.86, "noseX": 0.0, "lean": 0.0},
    "broad": {"width": 1.13, "jaw": 0.96, "noseX": -0.025, "lean": 0.0},
    "crooked": {"width": 0.96, "jaw": 0.82, "noseX": 0.055, "lean": 0.035},
}


def arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True)
    parser.add_argument("--variant", choices=sorted(VARIANTS), required=True)
    return parser.parse_args(sys.argv[sys.argv.index("--") + 1:])


def gaussian(x, z, center_x, center_z, radius_x, radius_z):
    return math.exp(-(((x - center_x) / radius_x) ** 2 + ((z - center_z) / radius_z) ** 2))


def silhouette_bounds(z, settings):
    normalized = max(-1.0, min(1.0, z / 0.9))
    height = (normalized + 1.0) * 0.5
    radius = 0.14 + 0.48 * settings["width"] * math.sin(math.pi * height)
    if z < -0.18:
        lower = (z + 0.9) / 0.72
        radius *= settings["jaw"] + (1.0 - settings["jaw"]) * max(0.0, min(1.0, lower))
    center = settings["lean"] * math.sin(math.pi * (z + 0.9) / 1.8)
    return center - radius, center + radius


def mask_depth(x, z, settings):
    left, right = silhouette_bounds(z, settings)
    if x < left or x > right or not -0.9 <= z <= 0.9:
        return 0.0
    nose_x = settings["noseX"]
    value = NEUTRAL_DEPTH
    value += 0.07 * gaussian(x, z, 0.0, -0.02, 0.48, 0.72)
    value += 0.18 * gaussian(x, z, -0.23, 0.34, 0.22, 0.11)
    value += 0.16 * gaussian(x, z, 0.23, 0.34, 0.22, 0.11)
    value -= 0.24 * gaussian(x, z, -0.23, 0.19, 0.16, 0.13)
    value -= 0.22 * gaussian(x, z, 0.23, 0.19, 0.16, 0.13)
    value += 0.27 * gaussian(x, z, nose_x, 0.12, 0.095, 0.36)
    value += 0.38 * gaussian(x, z, nose_x + 0.015, -0.08, 0.16, 0.15)
    value += 0.16 * gaussian(x, z, -0.31, -0.06, 0.24, 0.22)
    value += 0.14 * gaussian(x, z, 0.31, -0.06, 0.24, 0.22)
    value -= 0.24 * gaussian(x, z, 0.0, -0.35, 0.27, 0.055)
    value += 0.05 * gaussian(x, z, 0.0, -0.29, 0.30, 0.09)
    value += 0.20 * gaussian(x, z, 0.0, -0.63, 0.28, 0.17)
    return max(0.03, min(1.0, value))


def target_pixels(settings):
    return [
        [
            round(255 * mask_depth(-1.0 + 2.0 * column / (RESOLUTION - 1), 1.0 - 2.0 * row / (RESOLUTION - 1), settings))
            for column in range(RESOLUTION)
        ]
        for row in range(RESOLUTION)
    ]


def make_mesh():
    vertices = []
    for level in range(LEVELS):
        height = level / (LEVELS - 1)
        z = -0.9 + 1.8 * height
        radius = 0.45 + 0.03 * math.sin(math.pi * height)
        for segment in range(SEGMENTS):
            angle = 2.0 * math.pi * segment / SEGMENTS
            vertices.append((radius * math.cos(angle), radius * math.sin(angle), z))
    faces = []
    for level in range(LEVELS - 1):
        for segment in range(SEGMENTS):
            following = (segment + 1) % SEGMENTS
            lower, upper = level * SEGMENTS, (level + 1) * SEGMENTS
            faces.append((lower + segment, lower + following, upper + following, upper + segment))
    faces.extend((tuple(reversed(range(SEGMENTS))), tuple((LEVELS - 1) * SEGMENTS + i for i in range(SEGMENTS))))
    return vertices, faces


def topology_edges():
    edges = []
    for level in range(LEVELS):
        for segment in range(SEGMENTS):
            edges.append((level * SEGMENTS + segment, level * SEGMENTS + (segment + 1) % SEGMENTS))
    for level in range(LEVELS - 1):
        for segment in range(SEGMENTS):
            edges.append((level * SEGMENTS + segment, (level + 1) * SEGMENTS + segment))
    return edges


def point_camera(camera, location):
    camera.location = location
    camera.rotation_euler = (-Vector(location)).to_track_quat("-Z", "Y").to_euler()


def render(scene, camera, path, location):
    point_camera(camera, location)
    scene.render.filepath = str(path)
    bpy.ops.render.render(write_still=True)


def main():
    args = arguments()
    output = Path(args.output).resolve()
    if (output / "picture-to-mesh-mask.blend").exists():
        raise FileExistsError(f"refusing to overwrite {output}")
    output.mkdir(parents=True, exist_ok=True)
    (output / "renders").mkdir()
    settings = VARIANTS[args.variant]
    pixels = target_pixels(settings)
    target_pgm, target_png = output / "target-mask.pgm", output / "target-mask.png"
    write_pgm(target_pgm, pixels)
    write_png(target_png, pixels)

    clear_scene()
    vertices, faces = make_mesh()
    spans = read_pgm_spans(target_pgm, threshold=0.02)
    depth_image = read_pgm_depth(target_pgm)
    silhouette = fit_fixed_topology(vertices, spans)
    fitted = fit_closed_mask(
        vertices, spans, depth_image, DEPTH_SCALE, NEUTRAL_DEPTH,
        edges=topology_edges(), smooth_iterations=SMOOTH_ITERATIONS, smooth_factor=SMOOTH_FACTOR,
        source_weight=SOURCE_WEIGHT,
    )
    mesh = bpy.data.meshes.new("MaskProofTopology")
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    form = bpy.data.objects.new("MaskProofForm", mesh)
    bpy.context.collection.objects.link(form)
    basis = form.shape_key_add(name="Basis", from_mix=False)
    silhouette_key = form.shape_key_add(name="SilhouetteOnly", from_mix=False)
    fitted_key = form.shape_key_add(name="FittedMask", from_mix=False)
    for index in range(len(vertices)):
        silhouette_key.data[index].co = silhouette[index]
        fitted_key.data[index].co = fitted[index]
    silhouette_key.value = 0.0
    fitted_key.value = 1.0

    camera_data = bpy.data.cameras.new("MaskEvidenceCamera")
    camera_data.type = "ORTHO"
    camera_data.ortho_scale = 2.0
    camera = bpy.data.objects.new("MaskEvidenceCamera", camera_data)
    bpy.context.collection.objects.link(camera)
    scene = bpy.context.scene
    scene.camera = camera
    scene.render.engine = "BLENDER_WORKBENCH"
    scene.render.resolution_x = RESOLUTION
    scene.render.resolution_y = RESOLUTION
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.display.shading.color_type = "OBJECT"
    scene.display.shading.background_type = "VIEWPORT"

    form.color = (1.0, 1.0, 1.0, 1.0)
    scene.display.shading.background_color = (0.0, 0.0, 0.0)
    scene.display.shading.light = "FLAT"
    scene.display.shading.show_shadows = False
    scene.display.shading.show_cavity = False
    render(scene, camera, output / "renders" / "front-mask.png", (0.0, -6.0, 0.0))

    form.color = (0.38, 0.27, 0.18, 1.0)
    scene.display.shading.background_color = (0.055, 0.055, 0.055)
    scene.display.shading.light = "STUDIO"
    scene.display.shading.show_shadows = True
    scene.display.shading.show_cavity = True
    scene.display.shading.cavity_type = "BOTH"
    render(scene, camera, output / "renders" / "front.png", (0.0, -6.0, 0.0))
    render(scene, camera, output / "renders" / "three-quarter.png", (3.8, -5.0, 1.0))
    render(scene, camera, output / "renders" / "side.png", (6.0, 0.0, 0.0))

    form["proofRole"] = "single-front-recognizable-mask"
    blend = output / "picture-to-mesh-mask.blend"
    bpy.ops.wm.save_as_mainfile(filepath=str(blend))
    artifacts = [target_png, blend, *(output / "renders").glob("*.png")]
    manifest = {
        "schemaVersion": 1,
        "status": "built-unverified",
        "inputType": "single-front-grayscale-silhouette-and-relative-depth",
        "variant": args.variant,
        "variantSettings": settings,
        "topology": {"vertexCount": len(vertices), "faceCount": len(faces), "connectivitySha256": topology_hash(faces)},
        "recipe": {"levels": LEVELS, "segments": SEGMENTS, "depthScale": DEPTH_SCALE,
                   "neutralDepth": NEUTRAL_DEPTH, "smoothIterations": SMOOTH_ITERATIONS,
                   "smoothFactor": SMOOTH_FACTOR, "sourceWeight": SOURCE_WEIGHT},
        "tools": {"blender": bpy.app.version_string, "python": sys.version.split()[0],
                  "builderSha256": sha256(Path(__file__).resolve()), "fitterSha256": sha256(ROOT / "mask_fit.py")},
        "inputs": {"front": {"path": target_pgm.name, "sha256": sha256(target_pgm)}},
        "artifacts": {path.relative_to(output).as_posix(): {"sha256": sha256(path)} for path in artifacts},
    }
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"PICTURE_TO_MESH_MASK_BUILD_OK variant={args.variant} output={output}")


if __name__ == "__main__":
    main()
