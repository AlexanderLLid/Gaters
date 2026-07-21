"""Build a deliberately rough, unconstrained head maquette in stock Blender."""

import argparse
import hashlib
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Vector


ROOT = Path(__file__).resolve().parent


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def material(name, color, roughness=0.7):
    value = bpy.data.materials.new(name)
    value.diffuse_color = (*color, 1.0)
    value.use_nodes = True
    bsdf = value.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = (*color, 1.0)
    bsdf.inputs["Roughness"].default_value = roughness
    return value


def ellipsoid(name, location, scale, material_value, segments=48, rings=32):
    bpy.ops.mesh.primitive_uv_sphere_add(
        segments=segments,
        ring_count=rings,
        location=location,
    )
    obj = bpy.context.object
    obj.name = name
    obj.scale = scale
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    obj.data.materials.append(material_value)
    return obj


def torus(name, location, scale, material_value):
    bpy.ops.mesh.primitive_torus_add(
        major_radius=0.23,
        minor_radius=0.085,
        major_segments=48,
        minor_segments=16,
        location=location,
        rotation=(math.radians(90), 0.0, 0.0),
    )
    obj = bpy.context.object
    obj.name = name
    obj.scale = scale
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    obj.data.materials.append(material_value)
    return obj


def curve(name, points, bevel, material_value):
    data = bpy.data.curves.new(name, "CURVE")
    data.dimensions = "3D"
    data.bevel_depth = bevel
    data.bevel_resolution = 3
    spline = data.splines.new("BEZIER")
    spline.bezier_points.add(len(points) - 1)
    for point, coordinate in zip(spline.bezier_points, points):
        point.co = coordinate
        point.handle_left_type = "AUTO"
        point.handle_right_type = "AUTO"
    obj = bpy.data.objects.new(name, data)
    bpy.context.collection.objects.link(obj)
    obj.data.materials.append(material_value)
    return obj


def join_and_remesh(objects, name):
    bpy.ops.object.select_all(action="DESELECT")
    for obj in objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    bpy.ops.object.join()
    obj = bpy.context.object
    obj.name = name
    obj.data.remesh_voxel_size = 0.035
    obj.data.remesh_voxel_adaptivity = 0.0
    bpy.ops.object.voxel_remesh()
    for polygon in obj.data.polygons:
        polygon.use_smooth = True
    return obj


def gaussian(x, z, center_x, center_z, radius_x, radius_z):
    return math.exp(-(((x - center_x) / radius_x) ** 2 + ((z - center_z) / radius_z) ** 2))


def continuous_head(material_value):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=96, ring_count=64)
    obj = bpy.context.object
    obj.name = "DesignSculpt"
    for vertex in obj.data.vertices:
        source = vertex.co.copy()
        x = source.x * 0.72
        y = source.y * 0.64
        z = source.z * 1.05 + 0.05
        lower = max(0.0, min(1.0, (-z - 0.18) / 0.82))
        upper = max(0.0, min(1.0, (z - 0.56) / 0.52))
        x *= 1.0 - 0.23 * lower - 0.07 * upper
        y *= 1.0 - 0.10 * lower

        front = max(0.0, -source.y) ** 2.4
        if front:
            displacement = 0.0
            for eye_x in (-0.29, 0.29):
                displacement += 0.075 * gaussian(x, z, eye_x, 0.14, 0.19, 0.12)
                displacement -= 0.035 * gaussian(x, z, eye_x, 0.34, 0.22, 0.10)
                displacement -= 0.055 * gaussian(x, z, eye_x * 1.22, -0.10, 0.24, 0.23)
            displacement += 0.035 * gaussian(x, z, 0.0, 0.62, 0.48, 0.32)
            displacement -= 0.115 * gaussian(x, z, 0.0, 0.06, 0.115, 0.38)
            displacement -= 0.235 * gaussian(x, z, 0.0, -0.20, 0.18, 0.15)
            displacement -= 0.055 * gaussian(x, z, 0.0, -0.47, 0.29, 0.18)
            displacement -= 0.060 * gaussian(x, z, 0.0, -0.82, 0.28, 0.18)
            displacement += 0.030 * gaussian(abs(x), z, 0.56, 0.22, 0.16, 0.28)
            y += displacement * front
        vertex.co = (x, y, z)
    obj.data.materials.append(material_value)
    for polygon in obj.data.polygons:
        polygon.use_smooth = True
    return obj


def look_at(camera, target):
    camera.rotation_euler = (Vector(target) - camera.location).to_track_quat("-Z", "Y").to_euler()


def add_camera(name, location, target=(0.0, 0.0, -0.05)):
    data = bpy.data.cameras.new(name)
    data.type = "ORTHO"
    data.ortho_scale = 2.75
    camera = bpy.data.objects.new(name, data)
    bpy.context.collection.objects.link(camera)
    camera.location = location
    look_at(camera, target)
    return camera


def add_area(name, location, energy, size, color):
    data = bpy.data.lights.new(name, "AREA")
    data.energy = energy
    data.shape = "DISK"
    data.size = size
    data.color = color
    obj = bpy.data.objects.new(name, data)
    bpy.context.collection.objects.link(obj)
    obj.location = location
    look_at(obj, (0.0, 0.0, 0.0))


def build(contract_path):
    contract_path = Path(contract_path).resolve()
    contract = json.loads(contract_path.read_text(encoding="utf-8"))
    output = (contract_path.parent / contract["outputDirectory"]).resolve()
    output.mkdir(parents=True, exist_ok=True)

    bpy.ops.wm.read_factory_settings(use_empty=True)
    clay = material("Design clay", (0.34, 0.12, 0.065), 0.82)
    lip = material("Lip clay", (0.20, 0.045, 0.038), 0.72)
    sclera = material("Eye neutral", (0.26, 0.25, 0.23), 0.5)
    iris = material("Iris", (0.055, 0.025, 0.014), 0.52)
    dark = material("Feature dark", (0.018, 0.012, 0.010), 0.76)

    # One continuous volume replaces the rejected assembled-primitives v1.
    head = continuous_head(clay)
    ellipsoid("Ear.L", (-0.705, -0.01, -0.06), (0.12, 0.075, 0.245), clay)
    ellipsoid("Ear.R", (0.705, -0.01, -0.06), (0.12, 0.075, 0.245), clay)

    # Readable feature proxies make the large forms judgeable without textures.
    for side in (-1.0, 1.0):
        x = side * 0.29
        eye = ellipsoid(f"Eye.{side:+.0f}", (x, -0.575, 0.13), (0.175, 0.060, 0.073), sclera)
        iris_obj = ellipsoid(f"Iris.{side:+.0f}", (x, -0.634, 0.13), (0.050, 0.014, 0.050), iris, 32, 20)
        ellipsoid(f"Pupil.{side:+.0f}", (x, -0.647, 0.13), (0.020, 0.008, 0.020), dark, 24, 16)
        eye["artifactRole"] = "feature-proxy"
        iris_obj["artifactRole"] = "feature-proxy"

        curve(
            f"UpperLid.{side:+.0f}",
            [(x - 0.17, -0.638, 0.13), (x, -0.657, 0.195), (x + 0.17, -0.638, 0.13)],
            0.014,
            clay,
        )
        curve(
            f"LowerLid.{side:+.0f}",
            [(x - 0.16, -0.635, 0.125), (x, -0.647, 0.085), (x + 0.16, -0.635, 0.125)],
            0.011,
            clay,
        )
        curve(
            f"Brow.{side:+.0f}",
            [(x - 0.19, -0.615, 0.32), (x, -0.638, 0.355), (x + 0.19, -0.615, 0.315)],
            0.018,
            dark,
        )

    ellipsoid("UpperLip", (0.0, -0.635, -0.47), (0.285, 0.036, 0.038), lip)
    ellipsoid("LowerLip", (0.0, -0.640, -0.525), (0.275, 0.036, 0.043), lip)
    curve("MouthSeam", [(-0.285, -0.678, -0.50), (0.0, -0.684, -0.505), (0.285, -0.678, -0.50)], 0.008, dark)
    ellipsoid("Nostril.L", (-0.105, -0.805, -0.23), (0.036, 0.013, 0.018), dark, 24, 16)
    ellipsoid("Nostril.R", (0.105, -0.805, -0.23), (0.036, 0.013, 0.018), dark, 24, 16)

    head["artifactRole"] = "unconstrained-design-volume"
    head["sourceTopology"] = "stock-blender-sphere-design-volume"
    head["targetImageSha256"] = sha256((contract_path.parent / contract["targetImage"]).resolve())

    world = bpy.data.worlds.new("Neutral world")
    bpy.context.scene.world = world
    world.use_nodes = True
    world.node_tree.nodes["Background"].inputs["Color"].default_value = (0.012, 0.014, 0.018, 1.0)
    world.node_tree.nodes["Background"].inputs["Strength"].default_value = 0.35
    add_area("Key", (-3.6, -4.8, 4.6), 850, 4.0, (1.0, 0.89, 0.78))
    add_area("Fill", (3.4, -3.0, 1.8), 500, 3.5, (0.70, 0.82, 1.0))
    add_area("Rim", (0.0, 2.6, 3.2), 650, 3.0, (1.0, 0.72, 0.55))

    cameras = {
        "front": add_camera("Camera.Front", (0.0, -7.0, -0.02)),
        "three-quarter": add_camera("Camera.ThreeQuarter", (4.8, -6.0, 0.10)),
        "profile": add_camera("Camera.Profile", (7.0, -0.05, 0.0)),
    }
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 768
    scene.render.resolution_y = 768
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.film_transparent = False
    scene.view_settings.look = "AgX - Medium High Contrast"

    blend_path = output / "raw-head.blend"
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
    for name, camera in cameras.items():
        scene.camera = camera
        scene.render.filepath = str(output / f"{name}.png")
        bpy.ops.render.render(write_still=True)

    manifest = {
        "schemaVersion": 1,
        "machineId": contract["machineId"],
        "status": "human-review",
        "input": {
            "targetImage": str((contract_path.parent / contract["targetImage"]).resolve()),
            "sha256": head["targetImageSha256"],
        },
        "output": {
            "blend": str(blend_path),
            "headVertices": len(head.data.vertices),
            "headFaces": len(head.data.polygons),
            "objects": len(bpy.data.objects),
        },
        "usesMpfb": False,
        "usesImageProjection": False,
        "claims": ["unconstrained major-form maquette", "editable Blender geometry"],
        "excludes": ["likeness approval", "production topology", "texture", "rigging", "animation"],
        "artifacts": {},
    }
    for name in ("raw-head.blend", "front.png", "three-quarter.png", "profile.png"):
        manifest["artifacts"][name] = sha256(output / name)
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--contract", default=str(ROOT / "raw-sculpt-v2.json"))
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    args = parser.parse_args(argv)
    build(args.contract)


if __name__ == "__main__":
    main()
