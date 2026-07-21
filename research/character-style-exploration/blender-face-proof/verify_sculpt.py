"""Fresh-load verification for the view-independent clay sculpt proof."""

import argparse
import hashlib
import json
import math
import sys
from pathlib import Path
from tempfile import TemporaryDirectory

import bpy
from mathutils import Vector


ROOT = Path(__file__).resolve().parent
RUNS_ROOT = ROOT / "Runs"
DERIVED_RUNS_ROOT = ROOT / "Derived" / "search-runs"
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from sculpt_integrity import file_hash, validate_manifest_integrity


def arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True)
    return parser.parse_args(sys.argv[sys.argv.index("--") + 1 :])


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


def point_at(obj, target):
    obj.rotation_euler = (Vector(target) - obj.location).to_track_quat("-Z", "Y").to_euler()


def assert_visible_materials_have_no_images():
    for obj in bpy.context.scene.objects:
        if obj.type != "MESH" or obj.hide_render:
            continue
        for material in obj.data.materials:
            if material and material.use_nodes:
                image_nodes = [node.name for node in material.node_tree.nodes if node.type == "TEX_IMAGE"]
                if image_nodes:
                    raise RuntimeError(f"Visible material uses image textures: {obj.name}: {image_nodes}")


def verify_manifest(output):
    validate_manifest_integrity(output)
    path = output / "manifest.json"
    if not path.is_file():
        raise RuntimeError("Sculpt manifest is missing")
    manifest = json.loads(path.read_text(encoding="utf-8"))
    artifacts = manifest.get("artifacts", {})
    for name in ("face-sculpt.blend", "front.png", "three-quarter.png", "profile.png"):
        artifact = output / name
        if not artifact.is_file() or artifacts.get(name) != file_hash(artifact):
            raise RuntimeError(f"Sculpt manifest artifact hash does not match: {name}")
    checks = manifest.get("hardChecks", {})
    if not (
        manifest.get("status") == "passed"
        and checks.get("completed") is True
        and checks.get("referenceProjection") is True
        and checks.get("editableShapeKey") is True
        and checks.get("hasArmature") is False
        and checks.get("hasAnimation") is False
        and checks.get("renderSize") == [768, 768]
        and checks.get("visibleMaterialsUseNoImages") is True
    ):
        raise RuntimeError("Sculpt manifest hard checks are incomplete")
    if not manifest.get("activeShapeKeys") or not manifest.get("toolVersions"):
        raise RuntimeError("Sculpt manifest is missing active shape keys or tool versions")
    return manifest


def render_views(output, head, camera):
    points = [head.matrix_world @ point for point in mixed_coordinates(head)]
    center_x = (min(point.x for point in points) + max(point.x for point in points)) * 0.5
    eye_objects = [bpy.data.objects.get("SculptEyeL"), bpy.data.objects.get("SculptEyeR")]
    if not all(eye_objects):
        raise RuntimeError("Sculpt eyes are missing")
    eye_z = sum(obj.location.z for obj in eye_objects) / len(eye_objects)
    top_z = max(point.z for point in points)
    head_height = (top_z - eye_z) * 2.25
    target = (center_x, -0.01, top_z - head_height * 0.52)
    radius = 1.18
    views = {
        "front": ((center_x, -radius, target[2]), target),
        "three-quarter": (
            (center_x + radius * math.sin(math.radians(38)), -radius * math.cos(math.radians(38)), target[2]),
            (target[0], -0.035, target[2]),
        ),
        "profile": ((center_x + radius, 0.0, target[2]), (target[0], -0.075, target[2])),
    }
    scene = bpy.context.scene
    with TemporaryDirectory() as directory:
        for name, (location, view_target) in views.items():
            camera.location = location
            point_at(camera, view_target)
            path = Path(directory) / f"{name}.png"
            scene.render.filepath = str(path)
            bpy.ops.render.render(write_still=True)
            image = bpy.data.images.load(str(path), check_existing=False)
            if tuple(image.size) != (768, 768):
                raise RuntimeError(f"Unexpected render size for {name}: {tuple(image.size)}")
            bpy.data.images.remove(image)


def main():
    output = Path(arguments().output).resolve()
    output.relative_to(DERIVED_RUNS_ROOT.resolve())
    if bpy.data.filepath != str(output / "face-sculpt.blend"):
        raise RuntimeError(f"Wrong reopened blend: {bpy.data.filepath}")
    verify_manifest(output)

    root = bpy.data.objects.get("FaceProofRoot")
    head = bpy.data.objects.get("FaceProofHead")
    camera = bpy.data.objects.get("FaceProofCamera")
    if not all((root, head, camera)):
        raise RuntimeError("Required sculpt objects are missing")
    if root.get("proof_role") != "view-independent-clay-sculpt" or root.get("reference_projection") is not False:
        raise RuntimeError("Sculpt provenance contract failed")
    direct = head.data.shape_keys.key_blocks.get("SculptDirect") if head.data.shape_keys else None
    if direct is None or abs(direct.value - 1.0) > 0.000001:
        raise RuntimeError("Editable SculptDirect shape key is missing or disabled")
    if any(obj.type == "ARMATURE" for obj in bpy.context.scene.objects) or bpy.data.actions:
        raise RuntimeError("Isolated art proof unexpectedly contains rigging or animation")
    for name in ("FaceProofHead.braid01", "FaceProofHead.eyebrow003", "FaceProofHead.high-poly"):
        obj = bpy.data.objects.get(name)
        if obj is None or not obj.hide_render:
            raise RuntimeError(f"Original presentation asset is not hidden: {name}")
    assert_visible_materials_have_no_images()
    render_views(output, head, camera)
    print("SCULPT_VERIFY_OK views=3 size=768x768 projection=false editable=SculptDirect armature=false animation=false")


if __name__ == "__main__":
    main()
