"""Build an outcome-first Blender still from the approved front reference.

This is deliberately a one-view look-development proof, not the generator.
The approved painting is camera-projected onto real MPFB geometry so the target
can be judged in Blender before its reusable construction is reverse-engineered.
"""

import argparse
import json
import sys
from pathlib import Path

import bpy
from bpy_extras.object_utils import world_to_camera_view


def arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--reference", required=True)
    parser.add_argument("--output", required=True)
    return parser.parse_args(sys.argv[sys.argv.index("--") + 1 :])


def projection_material(reference):
    image = bpy.data.images.load(str(reference), check_existing=True)
    material = bpy.data.materials.new("ApprovedFrontReference")
    material.use_nodes = True
    nodes = material.node_tree.nodes
    nodes.clear()

    output = nodes.new("ShaderNodeOutputMaterial")
    emission = nodes.new("ShaderNodeEmission")
    texture = nodes.new("ShaderNodeTexImage")
    uv_map = nodes.new("ShaderNodeUVMap")
    uv_map.uv_map = "ApprovedFrontProjection"
    texture.image = image
    texture.interpolation = "Cubic"

    material.node_tree.links.new(uv_map.outputs["UV"], texture.inputs["Vector"])
    material.node_tree.links.new(texture.outputs["Color"], emission.inputs["Color"])
    material.node_tree.links.new(emission.outputs["Emission"], output.inputs["Surface"])
    return material


def mixed_coordinates(obj):
    keys = obj.data.shape_keys
    if keys is None or not keys.key_blocks:
        return [vertex.co.copy() for vertex in obj.data.vertices]
    basis = keys.key_blocks[0]
    coordinates = [point.co.copy() for point in basis.data]
    for key in keys.key_blocks[1:]:
        if abs(key.value) < 0.000001:
            continue
        for index, point in enumerate(key.data):
            coordinates[index] += (point.co - basis.data[index].co) * key.value
    return coordinates


def project_object(obj, camera, material):
    uv_layer = obj.data.uv_layers.get("ApprovedFrontProjection")
    if uv_layer is None:
        uv_layer = obj.data.uv_layers.new(name="ApprovedFrontProjection")
    coordinates = mixed_coordinates(obj)
    for polygon in obj.data.polygons:
        for loop_index in polygon.loop_indices:
            vertex_index = obj.data.loops[loop_index].vertex_index
            world = obj.matrix_world @ coordinates[vertex_index]
            ndc = world_to_camera_view(bpy.context.scene, camera, world)
            uv_layer.data[loop_index].uv = (ndc.x, ndc.y)

    obj.data.materials.clear()
    obj.data.materials.append(material)
    for polygon in obj.data.polygons:
        polygon.material_index = 0


def configure_camera(head):
    points = [head.matrix_world @ point for point in mixed_coordinates(head)]
    center_x = (min(point.x for point in points) + max(point.x for point in points)) * 0.5

    camera = bpy.data.objects.get("FaceProofCamera")
    camera.location = (center_x, -1.25, 1.575)
    camera.rotation_euler = (1.5707963268, 0.0, 0.0)
    camera.data.type = "ORTHO"
    camera.data.ortho_scale = 0.55
    camera.data.lens = 85
    bpy.context.scene.camera = camera
    return camera


def configure_scene(output):
    for obj in list(bpy.context.scene.objects):
        if obj.type == "LIGHT":
            bpy.data.objects.remove(obj, do_unlink=True)

    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 1024
    scene.render.resolution_y = 1024
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.image_settings.color_mode = "RGBA"
    scene.render.image_settings.color_depth = "8"
    scene.render.film_transparent = False
    scene.render.filepath = str(output / "front.png")
    scene.render.image_settings.color_mode = "RGB"
    scene.display_settings.display_device = "sRGB"
    scene.view_settings.view_transform = "Standard"
    scene.view_settings.look = "None"
    scene.view_settings.exposure = 0.0
    scene.view_settings.gamma = 1.0

    scene.world.use_nodes = True
    background = scene.world.node_tree.nodes.get("Background")
    background.inputs["Color"].default_value = (0.300, 0.300, 0.296, 1.0)
    background.inputs["Strength"].default_value = 1.0


def main():
    args = arguments()
    reference = Path(args.reference).resolve()
    output = Path(args.output).resolve()
    root = (Path(__file__).resolve().parent / "Derived").resolve()
    output.relative_to(root)
    if output.exists():
        raise FileExistsError(f"Refusing to overwrite still: {output}")
    if not reference.is_file():
        raise FileNotFoundError(reference)
    output.mkdir(parents=True)

    head = bpy.data.objects.get("FaceProofHead")
    eyes = bpy.data.objects.get("FaceProofHead.high-poly")
    brows = bpy.data.objects.get("FaceProofHead.eyebrow003")
    hair = bpy.data.objects.get("FaceProofHead.braid01")
    if not all((head, eyes, brows, hair)):
        raise RuntimeError("Expected MPFB portrait objects are missing")

    hair.hide_render = True
    hair.hide_set(True)
    brows.hide_render = True
    brows.hide_set(True)

    camera = configure_camera(head)
    configure_scene(output)
    material = projection_material(reference)
    project_object(head, camera, material)
    project_object(eyes, camera, material)

    root_object = bpy.data.objects.get("FaceProofRoot")
    root_object["proof_role"] = "outcome-first-reference-still"
    root_object["reference_path"] = str(reference)
    root_object["limitation"] = "front-camera projection; reverse-engineering pending"

    blend_path = output / "reference-still.blend"
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
    bpy.ops.wm.open_mainfile(filepath=str(blend_path))
    bpy.context.scene.render.filepath = str(output / "front.png")
    bpy.context.view_layer.update()
    bpy.ops.render.render(write_still=True)
    (output / "manifest.json").write_text(
        json.dumps(
            {
                "schemaVersion": 1,
                "purpose": "outcome-first static art target",
                "sourceBlend": bpy.data.filepath,
                "reference": str(reference),
                "render": "front.png",
                "projection": "camera-space front projection",
                "hasArmature": any(obj.type == "ARMATURE" for obj in bpy.context.scene.objects),
                "hasAnimation": bool(bpy.data.actions),
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )
    print(f"REFERENCE_STILL_OK output={output}")


if __name__ == "__main__":
    main()
