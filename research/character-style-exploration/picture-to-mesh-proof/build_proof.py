"""Build the fixed-topology picture-to-mesh silhouette proof in Blender."""

import argparse
import hashlib
import json
import math
import struct
import sys
import zlib
from pathlib import Path

import bpy
from mathutils import Vector

ROOT = Path(__file__).resolve().parent
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from silhouette_fit import fit_fixed_topology, read_pgm_spans


RESOLUTION = 256
LEVELS = 32
SEGMENTS = 48


def arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True)
    return parser.parse_args(sys.argv[sys.argv.index("--") + 1:])


def profile(height):
    arch = math.sin(math.pi * height)
    radius = 0.24 + 0.34 * arch ** 0.72 + 0.07 * math.exp(-((height - 0.32) / 0.13) ** 2)
    center = 0.065 * math.sin(2.0 * math.pi * height) * arch
    return center - radius, center + radius


def target_pixels():
    result = []
    for row in range(RESOLUTION):
        z = 1.0 - 2.0 * row / (RESOLUTION - 1)
        if not -0.9 <= z <= 0.9:
            result.append([0] * RESOLUTION)
            continue
        left, right = profile((z + 0.9) / 1.8)
        result.append([
            255 if left <= -1.0 + 2.0 * column / (RESOLUTION - 1) <= right else 0
            for column in range(RESOLUTION)
        ])
    return result


def write_pgm(path, pixels):
    rows = [" ".join(map(str, row)) for row in pixels]
    path.write_text(f"P2\n{RESOLUTION} {RESOLUTION}\n255\n" + "\n".join(rows) + "\n", encoding="ascii")


def write_png(path, pixels):
    def chunk(kind, payload):
        return struct.pack(">I", len(payload)) + kind + payload + struct.pack(">I", zlib.crc32(kind + payload) & 0xFFFFFFFF)

    raw = b"".join(b"\0" + bytes(value for pixel in row for value in (pixel, pixel, pixel, 255)) for row in pixels)
    header = struct.pack(">IIBBBBB", RESOLUTION, RESOLUTION, 8, 6, 0, 0, 0)
    path.write_bytes(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", header) + chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b""))


def make_mesh():
    vertices = []
    for level in range(LEVELS):
        height = level / (LEVELS - 1)
        z = -0.9 + 1.8 * height
        radius = 0.34 + 0.08 * math.sin(math.pi * height)
        for segment in range(SEGMENTS):
            angle = 2.0 * math.pi * segment / SEGMENTS
            vertices.append((radius * math.cos(angle), radius * math.sin(angle), z))

    faces = []
    for level in range(LEVELS - 1):
        for segment in range(SEGMENTS):
            next_segment = (segment + 1) % SEGMENTS
            lower = level * SEGMENTS
            upper = (level + 1) * SEGMENTS
            faces.append((lower + segment, lower + next_segment, upper + next_segment, upper + segment))
    faces.append(tuple(reversed(range(SEGMENTS))))
    faces.append(tuple((LEVELS - 1) * SEGMENTS + segment for segment in range(SEGMENTS)))
    return vertices, faces


def topology_hash(faces):
    return hashlib.sha256(json.dumps(faces, separators=(",", ":")).encode("ascii")).hexdigest()


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)


def make_material():
    material = bpy.data.materials.new("SilhouetteWhite")
    material.use_nodes = True
    principled = material.node_tree.nodes.get("Principled BSDF")
    principled.inputs["Base Color"].default_value = (1.0, 1.0, 1.0, 1.0)
    principled.inputs["Roughness"].default_value = 1.0
    principled.inputs["Emission Color"].default_value = (1.0, 1.0, 1.0, 1.0)
    principled.inputs["Emission Strength"].default_value = 1.0
    return material


def point_camera(camera, location):
    camera.location = location
    camera.rotation_euler = (-Vector(location)).to_track_quat("-Z", "Y").to_euler()


def render(scene, camera, path, location):
    point_camera(camera, location)
    scene.render.filepath = str(path)
    bpy.ops.render.render(write_still=True)


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    output = Path(arguments().output).resolve()
    if (output / "picture-to-mesh.blend").exists():
        raise FileExistsError(f"refusing to overwrite {output}")
    output.mkdir(parents=True, exist_ok=True)
    (output / "baseline").mkdir()
    (output / "fitted").mkdir()

    pixels = target_pixels()
    target_pgm = output / "target-front.pgm"
    target_png = output / "target-front.png"
    write_pgm(target_pgm, pixels)
    write_png(target_png, pixels)

    clear_scene()
    vertices, faces = make_mesh()
    fitted = fit_fixed_topology(vertices, read_pgm_spans(target_pgm))
    mesh = bpy.data.meshes.new("PictureToMeshTopology")
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    form = bpy.data.objects.new("PictureToMeshForm", mesh)
    bpy.context.collection.objects.link(form)
    form.data.materials.append(make_material())
    basis = form.shape_key_add(name="Basis")
    fitted_key = form.shape_key_add(name="Fitted")
    for index, coordinate in enumerate(fitted):
        fitted_key.data[index].co = coordinate

    camera_data = bpy.data.cameras.new("EvidenceCamera")
    camera_data.type = "ORTHO"
    camera_data.ortho_scale = 2.0
    camera = bpy.data.objects.new("EvidenceCamera", camera_data)
    bpy.context.collection.objects.link(camera)
    scene = bpy.context.scene
    scene.camera = camera
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = RESOLUTION
    scene.render.resolution_y = RESOLUTION
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.image_settings.color_mode = "RGBA"
    scene.render.film_transparent = False
    scene.view_settings.look = "None"
    scene.view_settings.view_transform = "Standard"
    world = scene.world or bpy.data.worlds.new("BlackWorld")
    scene.world = world
    world.use_nodes = True
    world.node_tree.nodes["Background"].inputs["Color"].default_value = (0.0, 0.0, 0.0, 1.0)
    world.node_tree.nodes["Background"].inputs["Strength"].default_value = 0.0

    fitted_key.value = 0.0
    render(scene, camera, output / "baseline" / "front.png", (0.0, -6.0, 0.0))
    render(scene, camera, output / "baseline" / "side.png", (6.0, 0.0, 0.0))
    fitted_key.value = 1.0
    render(scene, camera, output / "fitted" / "front.png", (0.0, -6.0, 0.0))
    render(scene, camera, output / "fitted" / "side.png", (6.0, 0.0, 0.0))
    render(scene, camera, output / "fitted" / "three-quarter.png", (4.2, -4.2, 1.2))

    form["proofRole"] = "fixed-topology-silhouette-fit"
    form["sourceViews"] = json.dumps(["front", "side"])
    blend = output / "picture-to-mesh.blend"
    bpy.ops.wm.save_as_mainfile(filepath=str(blend))
    artifact_paths = [
        blend,
        target_png,
        output / "baseline" / "front.png",
        output / "baseline" / "side.png",
        output / "fitted" / "front.png",
        output / "fitted" / "side.png",
        output / "fitted" / "three-quarter.png",
    ]
    manifest = {
        "schemaVersion": 1,
        "status": "built-unverified",
        "inputType": "single-front-orthographic-raster-silhouette",
        "topology": {"vertexCount": len(vertices), "faceCount": len(faces), "connectivitySha256": topology_hash(faces)},
        "recipe": {"levels": LEVELS, "segments": SEGMENTS, "shapeKey": "Fitted"},
        "tools": {
            "blender": bpy.app.version_string,
            "python": sys.version.split()[0],
            "builderSha256": sha256(Path(__file__).resolve()),
            "fitterSha256": sha256(ROOT / "silhouette_fit.py"),
        },
        "inputs": {"front": {"path": target_pgm.name, "sha256": sha256(target_pgm)}},
        "artifacts": {
            path.relative_to(output).as_posix(): {"sha256": sha256(path)}
            for path in artifact_paths
        },
    }
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"PICTURE_TO_MESH_BUILD_OK output={output}")


if __name__ == "__main__":
    main()
