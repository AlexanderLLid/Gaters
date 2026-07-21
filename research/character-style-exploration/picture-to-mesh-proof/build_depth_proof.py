"""Build a fixed-topology relief from one front depth picture."""

import argparse
import hashlib
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
from depth_fit import fit_relief, read_pgm_depth


RESOLUTION = 256
GRID = 41
DEPTH_SCALE = 0.35


def arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True)
    return parser.parse_args(sys.argv[sys.argv.index("--") + 1:])


def depth_value(x, z):
    broad = 0.28 * math.exp(-((x / 0.65) ** 2 + (z / 0.72) ** 2))
    bump = 0.58 * math.exp(-(((x - 0.24) / 0.24) ** 2 + ((z - 0.14) / 0.29) ** 2))
    ridge = 0.16 * math.exp(-((z + 0.28 * x + 0.28) / 0.16) ** 2) * math.exp(-(x / 0.72) ** 2)
    groove = 0.17 * math.exp(-(((x + 0.28) / 0.22) ** 2 + ((z - 0.05) / 0.34) ** 2))
    return max(0.0, min(1.0, broad + bump + ridge - groove))


def target_pixels():
    return [
        [
            round(255 * depth_value(-0.9 + 1.8 * column / (RESOLUTION - 1), 0.9 - 1.8 * row / (RESOLUTION - 1)))
            for column in range(RESOLUTION)
        ]
        for row in range(RESOLUTION)
    ]


def make_grid():
    vertices = [
        (-0.9 + 1.8 * column / (GRID - 1), 0.0, -0.9 + 1.8 * row / (GRID - 1))
        for row in range(GRID)
        for column in range(GRID)
    ]
    faces = [
        (row * GRID + column, row * GRID + column + 1,
         (row + 1) * GRID + column + 1, (row + 1) * GRID + column)
        for row in range(GRID - 1)
        for column in range(GRID - 1)
    ]
    return vertices, faces


def point_camera(camera, location):
    camera.location = location
    camera.rotation_euler = (-Vector(location)).to_track_quat("-Z", "Y").to_euler()


def render(scene, camera, path, location):
    point_camera(camera, location)
    scene.render.filepath = str(path)
    bpy.ops.render.render(write_still=True)


def main():
    output = Path(arguments().output).resolve()
    if (output / "picture-to-mesh-depth.blend").exists():
        raise FileExistsError(f"refusing to overwrite {output}")
    output.mkdir(parents=True, exist_ok=True)
    (output / "renders").mkdir()

    pixels = target_pixels()
    target_pgm, target_png = output / "target-depth.pgm", output / "target-depth.png"
    write_pgm(target_pgm, pixels)
    write_png(target_png, pixels)

    clear_scene()
    vertices, faces = make_grid()
    fitted = fit_relief(vertices, read_pgm_depth(target_pgm), DEPTH_SCALE)
    mesh = bpy.data.meshes.new("DepthProofTopology")
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    form = bpy.data.objects.new("DepthProofForm", mesh)
    bpy.context.collection.objects.link(form)
    form.color = (0.42, 0.30, 0.21, 1.0)
    form.shape_key_add(name="Basis")
    fitted_key = form.shape_key_add(name="FittedDepth")
    for index, coordinate in enumerate(fitted):
        fitted_key.data[index].co = coordinate
    fitted_key.value = 1.0

    camera_data = bpy.data.cameras.new("DepthEvidenceCamera")
    camera_data.type = "ORTHO"
    camera_data.ortho_scale = 2.25
    camera = bpy.data.objects.new("DepthEvidenceCamera", camera_data)
    bpy.context.collection.objects.link(camera)
    scene = bpy.context.scene
    scene.camera = camera
    scene.render.engine = "BLENDER_WORKBENCH"
    scene.render.resolution_x = RESOLUTION
    scene.render.resolution_y = RESOLUTION
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.display.shading.light = "STUDIO"
    scene.display.shading.color_type = "OBJECT"
    scene.display.shading.show_shadows = True
    scene.display.shading.show_cavity = True
    scene.display.shading.cavity_type = "BOTH"
    scene.display.shading.background_type = "VIEWPORT"
    scene.display.shading.background_color = (0.055, 0.055, 0.055)

    render(scene, camera, output / "renders" / "front.png", (0.0, -6.0, 0.0))
    render(scene, camera, output / "renders" / "three-quarter.png", (3.6, -5.0, 2.2))
    render(scene, camera, output / "renders" / "side.png", (6.0, 0.0, 0.0))
    form["proofRole"] = "single-front-depth-relief"
    blend = output / "picture-to-mesh-depth.blend"
    bpy.ops.wm.save_as_mainfile(filepath=str(blend))

    artifacts = [target_png, blend, *(output / "renders").glob("*.png")]
    manifest = {
        "schemaVersion": 1,
        "status": "built-unverified",
        "inputType": "single-front-grayscale-depth-picture",
        "topology": {"vertexCount": len(vertices), "faceCount": len(faces), "connectivitySha256": topology_hash(faces)},
        "recipe": {"grid": GRID, "depthScale": DEPTH_SCALE, "shapeKey": "FittedDepth"},
        "tools": {
            "blender": bpy.app.version_string,
            "python": sys.version.split()[0],
            "builderSha256": sha256(Path(__file__).resolve()),
            "fitterSha256": sha256(ROOT / "depth_fit.py"),
        },
        "inputs": {"depth": {"path": target_pgm.name, "sha256": sha256(target_pgm)}},
        "artifacts": {path.relative_to(output).as_posix(): {"sha256": sha256(path)} for path in artifacts},
    }
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"PICTURE_TO_MESH_DEPTH_BUILD_OK output={output}")


if __name__ == "__main__":
    main()
