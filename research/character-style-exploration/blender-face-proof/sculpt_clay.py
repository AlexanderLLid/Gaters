"""Create a view-independent clay sculpt challenger from the MPFB face proof."""

import argparse
import hashlib
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Vector
from bl_ext.blender_org.mpfb.services import LocationService, TargetService


ROOT = Path(__file__).resolve().parent
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))


from face_search import RUNS_ROOT, candidate_to_controls, load_space, validate_candidate


DERIVED_RUNS_ROOT = ROOT / "Derived" / "search-runs"
TARGET_CONTROLS = {
    "sculpt.head.narrower": ("head", "head-scale-horiz-decr"),
    "sculpt.head.wider": ("head", "head-scale-horiz-incr"),
    "sculpt.head.shorter": ("head", "head-scale-vert-decr"),
    "sculpt.head.taller": ("head", "head-scale-vert-incr"),
    "sculpt.chin.recess": ("chin", "chin-prominent-decr"),
    "sculpt.chin.prominent": ("chin", "chin-prominent-incr"),
    "sculpt.eye.left.in": ("eyes", "l-eye-trans-in"),
    "sculpt.eye.left.out": ("eyes", "l-eye-trans-out"),
    "sculpt.eye.right.in": ("eyes", "r-eye-trans-in"),
    "sculpt.eye.right.out": ("eyes", "r-eye-trans-out"),
    "sculpt.upperlip.volume.decr": ("mouth", "mouth-upperlip-volume-decr"),
    "sculpt.upperlip.volume.incr": ("mouth", "mouth-upperlip-volume-incr"),
    "sculpt.lowerlip.volume.decr": ("mouth", "mouth-lowerlip-volume-decr"),
    "sculpt.lowerlip.volume.incr": ("mouth", "mouth-lowerlip-volume-incr"),
    "sculpt.nose.tip": ("nose", "nose-point-width-incr"),
    "sculpt.nose.nostrils": ("nose", "nose-nostrils-width-incr"),
}

DIRECT_CONTROLS = {
    "nose.width": "nose.width",
    "nose.length": "nose.length",
    "nose.bridgeDepth": "nose.bridgeDepth",
    "sculpt.cheek.left.bone": "cheek.leftBone",
    "sculpt.cheek.right.bone": "cheek.rightBone",
    "sculpt.cheek.left.volume.decr": "cheek.leftVolume",
    "sculpt.cheek.right.volume.decr": "cheek.rightVolume",
    "sculpt.eye_bag.left": "age.eyeBagsLeft",
    "sculpt.eye_bag.right": "age.eyeBagsRight",
}


def arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--candidate", required=True)
    parser.add_argument("--output", required=True)
    return parser.parse_args(sys.argv[sys.argv.index("--") + 1 :])


def file_hash(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load_candidate(path):
    candidate_path = Path(path).resolve()
    try:
        relative = candidate_path.relative_to(RUNS_ROOT.resolve())
    except ValueError as error:
        raise ValueError("candidate must be an immutable recipe beneath Runs") from error
    if len(relative.parts) != 3 or relative.parts[1] != "candidates":
        raise ValueError("candidate must be stored under Runs/<run-id>/candidates")
    with candidate_path.open(encoding="utf-8") as stream:
        candidate = json.load(stream)
    validate_candidate(load_space(ROOT / "face-search-space.json"), candidate)
    if candidate_path.name != f"{candidate['candidateId']}.json":
        raise ValueError("candidate recipe filename does not match candidateId")
    return candidate_path, candidate


def output_path(candidate_path, candidate, output):
    relative = candidate_path.relative_to(RUNS_ROOT.resolve())
    expected = DERIVED_RUNS_ROOT / relative.parts[0] / f"round-{candidate['round']}" / candidate["candidateId"]
    resolved = Path(output).resolve()
    if resolved != expected.resolve():
        raise ValueError(f"output must be {expected}")
    return resolved


def material(name, color, roughness=0.82):
    result = bpy.data.materials.new(name)
    result.diffuse_color = color
    result.use_nodes = True
    bsdf = result.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = color
    bsdf.inputs["Roughness"].default_value = roughness
    specular = bsdf.inputs.get("Specular IOR Level")
    if specular:
        specular.default_value = 0.18
    return result


def replace_materials(obj, new_material):
    obj.data.materials.clear()
    obj.data.materials.append(new_material)
    for polygon in obj.data.polygons:
        polygon.material_index = 0
        polygon.use_smooth = True


def apply_sculpt_targets(head, controls):
    target_root = Path(LocationService.get_mpfb_data("targets"))
    for name, (section, target) in TARGET_CONTROLS.items():
        if name not in controls:
            continue
        path = target_root / section / f"{target}.target.gz"
        if not path.is_file():
            raise FileNotFoundError(path)
        TargetService.load_target(head, str(path), weight=controls[name], name=name)
    for control, name in DIRECT_CONTROLS.items():
        key = head.data.shape_keys.key_blocks.get(name)
        if key is None:
            raise RuntimeError(f"Missing sculpt control: {name}")
        key.value = controls[control]


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


def freeze_direct_sculpt(head, source_eyes, controls):
    original = mixed_coordinates(head)
    coordinates = [point.copy() for point in original]
    center_x = (min(point.x for point in coordinates) + max(point.x for point in coordinates)) * 0.5
    eye_points = [source_eyes.matrix_world @ point for point in mixed_coordinates(source_eyes)]
    left_eye = [point for point in eye_points if point.x < center_x]
    right_eye = [point for point in eye_points if point.x >= center_x]
    eye_centers = [
        Vector(
            (
                (min(point.x for point in side) + max(point.x for point in side)) * 0.5,
                (min(point.y for point in side) + max(point.y for point in side)) * 0.5,
                (min(point.z for point in side) + max(point.z for point in side)) * 0.5,
            )
        )
        for side in (left_eye, right_eye)
    ]
    eye_z = sum(point.z for point in eye_centers) * 0.5
    face_scale = sum(abs(point.x - center_x) for point in eye_centers) / 0.10

    def sx(value):
        return center_x + value * face_scale

    def sz(value):
        return eye_z + (value - 1.666) * face_scale

    def field(point, side_x, center_z, radius_x, radius_z):
        return math.exp(-(((point.x - side_x) / radius_x) ** 2 + ((point.z - center_z) / radius_z) ** 2))

    for point in coordinates:
        if point.y > 0.03 or point.z < sz(1.40):
            continue
        cheekbone = (
            field(point, sx(-0.073), sz(1.625), 0.048 * face_scale, 0.042 * face_scale)
            * 1.08 * controls["sculpt.cheek.left.bone"] / 0.36
            + field(point, sx(0.073), sz(1.625), 0.048 * face_scale, 0.042 * face_scale)
            * 0.92 * controls["sculpt.cheek.right.bone"] / 0.30
        )
        cheek_hollow = (
            field(point, sx(-0.071), sz(1.555), 0.052 * face_scale, 0.050 * face_scale)
            * 0.92 * controls["sculpt.cheek.left.volume.decr"] / 0.42
            + field(point, sx(0.071), sz(1.555), 0.052 * face_scale, 0.050 * face_scale)
            * 1.08 * controls["sculpt.cheek.right.volume.decr"] / 0.48
        )
        eye_bag = (
            field(point, sx(-0.052), sz(1.638), 0.038 * face_scale, 0.018 * face_scale)
            * 1.08 * controls["sculpt.eye_bag.left"] / 0.44
            + field(point, sx(0.052), sz(1.638), 0.038 * face_scale, 0.018 * face_scale)
            * 0.92 * controls["sculpt.eye_bag.right"] / 0.38
        )

        point.y += -0.0045 * cheekbone + 0.0060 * cheek_hollow
        point.y += -0.0040 * eye_bag
        if point.z < sz(1.585):
            jaw_weight = min(1.0, (sz(1.585) - point.z) / (0.12 * face_scale))
            point.x = center_x + (point.x - center_x) * (1.0 - controls["jaw.taper"] * jaw_weight)

    direct = head.shape_key_add(name="SculptDirect", from_mix=False)
    basis = head.data.shape_keys.key_blocks[0]
    for index, point in enumerate(coordinates):
        direct.data[index].co = basis.data[index].co + (point - original[index])
    direct.value = 1.0


def mask_shoulders(head):
    body_group = head.vertex_groups.get("body")
    if body_group is None:
        raise RuntimeError("MPFB body group is missing")
    body_indices = {
        vertex.index
        for vertex in head.data.vertices
        if any(link.group == body_group.index and link.weight > 0.5 for link in vertex.groups)
    }
    coordinates = mixed_coordinates(head)
    center_x = (min(coordinates[index].x for index in body_indices) + max(coordinates[index].x for index in body_indices)) * 0.5
    visible = [
        index
        for index in body_indices
        if coordinates[index].z >= 1.20
        and (coordinates[index].z >= 1.60 or abs(coordinates[index].x - center_x) <= 0.21)
    ]
    group = head.vertex_groups.new(name="SculptVisible")
    group.add(visible, 1.0, "REPLACE")
    mask = next((modifier for modifier in head.modifiers if modifier.type == "MASK"), None)
    if mask is None:
        raise RuntimeError("MPFB mask modifier is missing")
    mask.vertex_group = group.name


def point_at(obj, target):
    obj.rotation_euler = (Vector(target) - obj.location).to_track_quat("-Z", "Y").to_euler()


def add_clay_eyes(source_eyes, head_center_x):
    source_eyes.hide_render = True
    source_eyes.hide_set(True)
    coordinates = [source_eyes.matrix_world @ point for point in mixed_coordinates(source_eyes)]
    sclera = material("SculptSclera", (0.34, 0.31, 0.27, 1.0), 0.62)
    iris = material("SculptIris", (0.012, 0.009, 0.007, 1.0), 0.78)
    centers = []
    for name, side_points in (
        ("L", [point for point in coordinates if point.x < head_center_x]),
        ("R", [point for point in coordinates if point.x >= head_center_x]),
    ):
        center = Vector(
            (
                (min(point.x for point in side_points) + max(point.x for point in side_points)) * 0.5,
                (min(point.y for point in side_points) + max(point.y for point in side_points)) * 0.5,
                (min(point.z for point in side_points) + max(point.z for point in side_points)) * 0.5,
            )
        )
        centers.append(center)
        bpy.ops.mesh.primitive_uv_sphere_add(segments=24, ring_count=12, radius=1.0, location=center)
        eyeball = bpy.context.object
        eyeball.name = f"SculptEye{name}"
        eyeball.location.y += 0.012
        eyeball.scale = (0.0120, 0.0080, 0.0066)
        eyeball.data.materials.append(sclera)
        for polygon in eyeball.data.polygons:
            polygon.use_smooth = True

        pupil_center = center + Vector((0.0, 0.0035, 0.0))
        bpy.ops.mesh.primitive_uv_sphere_add(segments=20, ring_count=10, radius=1.0, location=pupil_center)
        pupil = bpy.context.object
        pupil.name = f"SculptPupil{name}"
        pupil.scale = (0.0023, 0.0009, 0.0023)
        pupil.data.materials.append(iris)
        for polygon in pupil.data.polygons:
            polygon.use_smooth = True
    return centers


def add_lights(target):
    for obj in list(bpy.context.scene.objects):
        if obj.type == "LIGHT":
            bpy.data.objects.remove(obj, do_unlink=True)
    lights = [
        ((-0.65, -0.85, 2.10), 52.0, 0.75),
        ((0.70, -0.45, 1.70), 16.0, 0.90),
        ((0.35, 0.65, 1.95), 28.0, 0.65),
    ]
    for index, (location, energy, size) in enumerate(lights):
        data = bpy.data.lights.new(f"ClayLight{index}", "AREA")
        data.energy = energy
        data.shape = "DISK"
        data.size = size
        light = bpy.data.objects.new(data.name, data)
        bpy.context.collection.objects.link(light)
        light.location = location
        point_at(light, target)


def configure_scene(output, head):
    hair = bpy.data.objects.get("FaceProofHead.braid01")
    brows = bpy.data.objects.get("FaceProofHead.eyebrow003")
    eyes = bpy.data.objects.get("FaceProofHead.high-poly")
    for obj in (hair, brows):
        obj.hide_render = True
        obj.hide_set(True)

    clay = material("SculptClay", (0.16, 0.075, 0.040, 1.0))
    replace_materials(head, clay)

    points = [head.matrix_world @ point for point in mixed_coordinates(head)]
    center_x = (min(point.x for point in points) + max(point.x for point in points)) * 0.5
    eye_centers = add_clay_eyes(eyes, center_x)
    eye_z = sum(point.z for point in eye_centers) / len(eye_centers)
    top_z = max(point.z for point in points)
    head_height = (top_z - eye_z) * 2.25
    target = (center_x, -0.01, top_z - head_height * 0.52)
    add_lights(target)

    camera = bpy.data.objects.get("FaceProofCamera")
    camera.data.type = "ORTHO"
    camera.data.ortho_scale = head_height * 1.22
    camera.data.lens = 85
    bpy.context.scene.camera = camera

    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 768
    scene.render.resolution_y = 768
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.image_settings.color_mode = "RGB"
    scene.render.image_settings.color_depth = "8"
    scene.render.film_transparent = False
    scene.world.use_nodes = True
    background = scene.world.node_tree.nodes.get("Background")
    background.inputs["Color"].default_value = (0.055, 0.055, 0.055, 1.0)
    background.inputs["Strength"].default_value = 0.45
    scene.view_settings.view_transform = "Standard"
    scene.view_settings.look = "Medium High Contrast"
    return camera, target, center_x


def render_views(camera, target, center_x, output):
    radius = 1.18
    views = {
        "front": ((center_x, -radius, target[2]), target),
        "three-quarter": (
            (center_x + radius * math.sin(math.radians(38)), -radius * math.cos(math.radians(38)), target[2]),
            (target[0], -0.035, target[2]),
        ),
        "profile": ((center_x + radius, 0.0, target[2]), (target[0], -0.075, target[2])),
    }
    for name, (location, view_target) in views.items():
        camera.location = location
        point_at(camera, view_target)
        bpy.context.scene.render.filepath = str(output / f"{name}.png")
        bpy.ops.render.render(write_still=True)
    camera.location = views["front"][0]
    point_at(camera, target)


def assert_visible_materials_have_no_images():
    for obj in bpy.context.scene.objects:
        if obj.type != "MESH" or obj.hide_render:
            continue
        for material in obj.data.materials:
            if material and material.use_nodes and any(
                node.type == "TEX_IMAGE" for node in material.node_tree.nodes
            ):
                raise RuntimeError(f"Visible material uses image textures: {obj.name}")


def hard_checks(root, head):
    direct = head.data.shape_keys.key_blocks.get("SculptDirect") if head.data.shape_keys else None
    checks = {
        "referenceProjection": root.get("reference_projection") is False,
        "editableShapeKey": direct is not None and abs(direct.value - 1.0) < 0.000001,
        "hasArmature": any(obj.type == "ARMATURE" for obj in bpy.context.scene.objects),
        "hasAnimation": bool(bpy.data.actions),
        "renderSize": [bpy.context.scene.render.resolution_x, bpy.context.scene.render.resolution_y],
    }
    assert_visible_materials_have_no_images()
    if not (
        checks["referenceProjection"]
        and checks["editableShapeKey"]
        and not checks["hasArmature"]
        and not checks["hasAnimation"]
        and checks["renderSize"] == [768, 768]
    ):
        raise RuntimeError(f"Sculpt hard checks failed: {checks}")
    checks["visibleMaterialsUseNoImages"] = True
    return checks


def active_shape_keys(head):
    return [
        {"name": key.name, "value": key.value}
        for key in head.data.shape_keys.key_blocks
        if abs(key.value) > 0.000001
    ]


def write_failure_manifest(output, candidate_path, candidate, controls, source_blend, error):
    artifacts = {
        path.name: file_hash(path)
        for path in output.iterdir()
        if path.is_file() and path.name != "manifest.json"
    }
    (output / "manifest.json").write_text(
        json.dumps(
            {
                "schemaVersion": 1,
                "status": "failed",
                "failure": str(error),
                "candidate": candidate,
                "candidateRecipe": {
                    "path": candidate_path.relative_to(ROOT).as_posix(),
                    "sha256": file_hash(candidate_path),
                },
                "sourceBlend": {"path": str(source_blend), "sha256": file_hash(source_blend)},
                "toolVersions": {
                    "blender": bpy.app.version_string,
                    "python": sys.version.split()[0],
                    "sculptScript": file_hash(Path(__file__).resolve()),
                    "faceSearch": file_hash(ROOT / "face_search.py"),
                },
                "controls": controls,
                "activeShapeKeys": [],
                "hardChecks": {"completed": False},
                "artifacts": artifacts,
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )


def main():
    args = arguments()
    candidate_path, candidate = load_candidate(args.candidate)
    controls = candidate_to_controls(candidate["parameters"])
    output = output_path(candidate_path, candidate, args.output)
    if output.exists():
        raise FileExistsError(f"Refusing to overwrite sculpt evidence: {output}")
    output.mkdir(parents=True)
    source_blend = Path(bpy.data.filepath).resolve()

    try:
        head = bpy.data.objects.get("FaceProofHead")
        if head is None:
            raise RuntimeError("MPFB head is missing")
        apply_sculpt_targets(head, controls)
        source_eyes = bpy.data.objects.get("FaceProofHead.high-poly")
        if source_eyes is None:
            raise RuntimeError("MPFB eye source is missing")
        freeze_direct_sculpt(head, source_eyes, controls)
        mask_shoulders(head)
        camera, target, center_x = configure_scene(output, head)

        root = bpy.data.objects.get("FaceProofRoot")
        root["proof_role"] = "view-independent-clay-sculpt"
        root["reference_projection"] = False
        head["sculpt_targets"] = json.dumps(controls, sort_keys=True)

        blend_path = output / "face-sculpt.blend"
        bpy.ops.file.pack_all()
        bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
        render_views(camera, target, center_x, output)
        bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
        checks = hard_checks(root, head)
        artifacts = {
            name: file_hash(output / name)
            for name in ("face-sculpt.blend", "front.png", "three-quarter.png", "profile.png")
        }
        (output / "manifest.json").write_text(
            json.dumps(
                {
                    "schemaVersion": 1,
                    "status": "passed",
                    "purpose": "view-independent clay face feasibility",
                    "candidate": candidate,
                    "candidateRecipe": {
                        "path": candidate_path.relative_to(ROOT).as_posix(),
                        "sha256": file_hash(candidate_path),
                    },
                    "sourceBlend": {"path": str(source_blend), "sha256": file_hash(source_blend)},
                    "toolVersions": {
                        "blender": bpy.app.version_string,
                        "python": sys.version.split()[0],
                        "sculptScript": file_hash(Path(__file__).resolve()),
                        "faceSearch": file_hash(ROOT / "face_search.py"),
                    },
                    "views": ["front", "three-quarter", "profile"],
                    "controls": controls,
                    "activeShapeKeys": active_shape_keys(head),
                    "hardChecks": {"completed": True, **checks},
                    "artifacts": artifacts,
                },
                indent=2,
            )
            + "\n",
            encoding="utf-8",
        )
    except Exception as error:
        write_failure_manifest(output, candidate_path, candidate, controls, source_blend, error)
        raise
    print(f"CLAY_SCULPT_OK output={output}")


if __name__ == "__main__":
    main()
