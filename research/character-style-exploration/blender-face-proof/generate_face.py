"""Generate one isolated, editable MPFB face proof for Art Direction."""

import argparse
import hashlib
import json
import shutil
import sys
from pathlib import Path

import bpy
from mathutils import Vector

from bl_ext.blender_org.mpfb.services import HumanService, LocationService, TargetService


ROOT = Path(__file__).resolve().parent
MPFB_VERSION = "2.0.16"
EXPECTED_POLICY = {
    "brief": "authoritative",
    "generator": "authoritative",
    "blend": "derived-reproducible",
    "renders": "derived-evidence",
}


def arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--brief", required=True)
    parser.add_argument("--output", required=True)
    return parser.parse_args(sys.argv[sys.argv.index("--") + 1 :])


def load_brief(path):
    brief = json.loads(path.read_text(encoding="utf-8"))
    if brief.get("schemaVersion") != 2 or brief.get("generatorVersion") != 2:
        raise ValueError("unsupported face brief version")
    if brief.get("artifactPolicy") != EXPECTED_POLICY:
        raise ValueError("unexpected artifact policy")
    if brief.get("proofId") != "art.face.highland-a1.v2":
        raise ValueError("unexpected proof identity")
    if brief.get("styleId") != "gaters.clean-midpoly-painted-1":
        raise ValueError("unexpected style identity")
    if brief.get("render", {}).get("views") != ["front", "three-quarter", "profile"]:
        raise ValueError("face proof requires the fixed three-view set")
    return brief


def recreate_output(output):
    expected = (ROOT / "Derived" / "face-v2").resolve()
    if output.resolve() != expected or output.is_symlink():
        raise ValueError(f"output must be the non-symlink directory {expected}")
    if output.exists():
        shutil.rmtree(output)
    output.mkdir(parents=True)


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for blocks in (
        bpy.data.meshes,
        bpy.data.curves,
        bpy.data.materials,
        bpy.data.cameras,
        bpy.data.lights,
    ):
        for item in list(blocks):
            if item.users == 0:
                blocks.remove(item)


def group_indices(obj, name):
    group = obj.vertex_groups.get(name)
    if group is None:
        raise RuntimeError(f"MPFB vertex group missing: {name}")
    result = set()
    for vertex in obj.data.vertices:
        if any(link.group == group.index and link.weight > 0.5 for link in vertex.groups):
            result.add(vertex.index)
    if not result:
        raise RuntimeError(f"MPFB vertex group empty: {name}")
    return result


def mixed_coordinates(obj):
    keys = obj.data.shape_keys
    if keys is None or not keys.key_blocks:
        return [vertex.co.copy() for vertex in obj.data.vertices]
    basis = keys.key_blocks[0]
    coords = [point.co.copy() for point in basis.data]
    for key in keys.key_blocks[1:]:
        if abs(key.value) < 0.000001:
            continue
        for index, point in enumerate(key.data):
            coords[index] += (point.co - basis.data[index].co) * key.value
    return coords


def add_target_controls(basemesh, brief):
    targets_root = Path(LocationService.get_mpfb_data("targets"))
    applied = []
    for control in brief["target"]["controls"]:
        path = targets_root / control["section"] / f"{control['target']}.target.gz"
        if not path.is_file():
            raise FileNotFoundError(path)
        TargetService.load_target(
            basemesh,
            str(path),
            weight=float(control["weight"]),
            name=control["name"],
        )
        applied.append(control["name"])
    return applied


def contracted_assets(brief):
    derived = (ROOT / "Derived").resolve()
    asset_root = (ROOT / brief["assetPack"]["root"]).resolve()
    asset_root.relative_to(derived)
    paths = {}
    for name, contract in brief["assets"].items():
        path = (asset_root / contract["path"]).resolve()
        path.relative_to(asset_root)
        if not path.is_file():
            raise FileNotFoundError(path)
        paths[name] = path
    return paths


def tune_principled(material, roughness, specular, diffuse_roughness):
    if material is None or material.node_tree is None:
        return
    bsdf = material.node_tree.nodes.get("Principled BSDF")
    if bsdf is None:
        return
    for name, value in (
        ("Roughness", roughness),
        ("Specular IOR Level", specular),
        ("Diffuse Roughness", diffuse_roughness),
    ):
        socket = bsdf.inputs.get(name)
        if socket is not None:
            socket.default_value = value


def tint_texture_material(obj, color):
    for material in obj.data.materials:
        if material is None or material.node_tree is None:
            continue
        mix = material.node_tree.nodes.get("diffuseIntensity")
        if mix is None:
            continue
        mix.blend_type = "MULTIPLY"
        mix.inputs["Factor"].default_value = 1.0
        mix.inputs["Color1"].default_value = tuple(color)


def apply_look_dev(basemesh, assets, root, brief):
    look = brief["look"]
    for material in basemesh.data.materials:
        tune_principled(material, float(look["skinRoughness"]), 0.22, 0.24)
    for material in assets["eyes"].data.materials:
        tune_principled(material, 0.34, 0.38, 0.08)
    for material in assets["eyebrows"].data.materials:
        tune_principled(material, 0.88, 0.12, 0.30)
    for material in assets["hair"].data.materials:
        tune_principled(material, 0.90, 0.16, 0.34)
    tint_texture_material(assets["eyebrows"], look["browTint"])
    tint_texture_material(assets["hair"], look["hairTint"])
    root["look_dev"] = json.dumps(look)


def add_official_assets(basemesh, root, brief):
    paths = contracted_assets(brief)
    skin = brief["assets"]["skin"]
    HumanService.set_character_skin(
        str(paths["skin"]),
        basemesh,
        skin_type=skin["materialModel"],
        material_instances=True,
    )
    objects = {}
    for name in ("eyes", "eyebrows", "hair"):
        contract = brief["assets"][name]
        obj = HumanService.add_mhclo_asset(
            str(paths[name]),
            basemesh,
            asset_type=name,
            subdiv_levels=1,
            material_type=contract["materialModel"],
            set_up_rigging=False,
            interpolate_weights=False,
            import_subrig=False,
            import_weights=False,
        )
        obj["proof_role"] = f"official-{name}-asset"
        objects[name] = obj
    root["official_asset_objects"] = json.dumps([obj.name for obj in objects.values()])
    return objects


def build_face(brief):
    phenotype = brief["target"]["phenotype"]
    macro = TargetService.get_default_macro_info_dict()
    for name in ("gender", "age", "muscle", "weight", "height", "proportions"):
        macro[name] = float(phenotype[name])
    macro["race"] = {name: float(value) for name, value in phenotype["race"].items()}

    basemesh = HumanService.create_human(
        mask_helpers=True,
        detailed_helpers=True,
        extra_vertex_groups=True,
        feet_on_ground=True,
        scale=0.1,
        macro_detail_dict=macro,
    )
    basemesh.name = "FaceProofHead"
    basemesh.data.name = "FaceProofStableTopology"
    basemesh["proof_role"] = "parametric-head"
    basemesh["foundation"] = "MPFB"
    basemesh["stable_topology"] = True
    applied = add_target_controls(basemesh, brief)
    basemesh["named_controls"] = json.dumps(applied)
    basemesh["face_recipe"] = json.dumps(brief, separators=(",", ":"))

    root = bpy.data.objects.new("FaceProofRoot", None)
    root["proof_role"] = "art-direction-root"
    root["art3_isolation"] = True
    bpy.context.collection.objects.link(root)
    basemesh.parent = root

    body_indices = group_indices(basemesh, "body")
    visible_indices = body_indices
    render_group = basemesh.vertex_groups.new(name="FaceProofVisible")
    render_group.add(list(visible_indices), 1.0, "REPLACE")
    mask = next((modifier for modifier in basemesh.modifiers if modifier.type == "MASK"), None)
    if mask is None:
        raise RuntimeError("MPFB helper mask missing")
    mask.vertex_group = render_group.name

    coords = mixed_coordinates(basemesh)
    body_z = [coords[index].z for index in body_indices]
    top_z = max(body_z)
    bottom_z = min(body_z)
    human_height = top_z - bottom_z
    for polygon in basemesh.data.polygons:
        polygon.use_smooth = True

    assets = add_official_assets(basemesh, root, brief)
    apply_look_dev(basemesh, assets, root, brief)

    subdivision = basemesh.modifiers.new("FaceProofSubdivision", "SUBSURF")
    subdivision.levels = 1
    subdivision.render_levels = 1

    head_floor = top_z - human_height * 0.19
    head_vertices = sum(1 for index in body_indices if coords[index].z >= head_floor)
    root["head_top_z"] = top_z
    root["human_height"] = human_height
    root["head_vertex_count"] = head_vertices
    return root, basemesh, applied, top_z, human_height


def point_at(obj, target):
    obj.rotation_euler = (Vector(target) - obj.location).to_track_quat("-Z", "Y").to_euler()


def add_scene(top_z, human_height, resolution):
    face_center_z = top_z - human_height * 0.095

    distance = human_height * 0.48
    bpy.ops.object.camera_add(location=(0.0, -distance, face_center_z))
    camera = bpy.context.object
    camera.name = "FaceProofCamera"
    camera.data.lens = 86
    camera.data.sensor_width = 36
    point_at(camera, (0.0, -human_height * 0.012, face_center_z - human_height * 0.007))

    lights = [
        ("Key", (-0.65, -0.75, top_z + 0.34), 55.0, 0.62, (1.0, 0.86, 0.72)),
        ("Fill", (0.62, -0.52, face_center_z + 0.04), 15.0, 0.55, (0.58, 0.70, 1.0)),
        ("Rim", (0.42, 0.38, top_z + 0.22), 30.0, 0.48, (0.74, 0.85, 1.0)),
    ]
    for name, location, energy, size, color in lights:
        data = bpy.data.lights.new(name, "AREA")
        data.energy = energy
        data.shape = "DISK"
        data.size = size
        data.color = color
        light = bpy.data.objects.new(name, data)
        bpy.context.collection.objects.link(light)
        light.location = location
        point_at(light, (0.0, 0.0, face_center_z))

    scene = bpy.context.scene
    scene.camera = camera
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = resolution
    scene.render.resolution_y = resolution
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.image_settings.color_mode = "RGBA"
    scene.render.film_transparent = False
    scene.render.image_settings.color_depth = "8"
    scene.world.color = (0.012, 0.015, 0.022)
    return camera, face_center_z, distance


def render_views(camera, face_center_z, distance, human_height, output):
    target = (0.0, -human_height * 0.012, face_center_z - human_height * 0.007)
    views = {
        "front": (0.0, -distance, face_center_z),
        "three-quarter": (distance * 0.57, -distance * 0.80, face_center_z),
        "profile": (distance, -human_height * 0.01, face_center_z),
    }
    for name, location in views.items():
        camera.location = location
        point_at(camera, target)
        bpy.context.scene.render.filepath = str(output / f"{name}.png")
        bpy.ops.render.render(write_still=True)
    camera.location = views["front"]
    point_at(camera, target)


def reopened_checks():
    roots = [obj for obj in bpy.context.scene.objects if obj.get("proof_role") == "art-direction-root"]
    heads = [obj for obj in bpy.context.scene.objects if obj.get("proof_role") == "parametric-head"]
    if len(roots) != 1 or len(heads) != 1:
        return {"singleProofRoot": False, "parametricHead": False, "namedControls": False}, None, None
    head = heads[0]
    controls = json.loads(head.get("named_controls", "[]"))
    keys = head.data.shape_keys.key_blocks if head.data.shape_keys else []
    key_names = {key.name for key in keys}
    return {
        "singleProofRoot": True,
        "parametricHead": bool(head.get("stable_topology")),
        "namedControls": all(name in key_names for name in controls),
    }, roots[0], head


def write_manifest(output, brief, brief_path, root, head, applied, checks):
    materials = sorted(material.name for material in bpy.data.materials)
    shape_keys = [key.name for key in head.data.shape_keys.key_blocks] if head.data.shape_keys else []
    manifest = {
        "schemaVersion": 2,
        "generatorVersion": 2,
        "proofId": brief["proofId"],
        "styleId": brief["styleId"],
        "art3Isolation": True,
        "foundation": "MPFB",
        "foundationVersion": MPFB_VERSION,
        "briefSha256": hashlib.sha256(brief_path.read_bytes()).hexdigest(),
        "stableTopology": True,
        "baseVertexCount": len(head.data.vertices),
        "headVertexCount": int(root.get("head_vertex_count", 0)),
        "assetPack": brief["assetPack"],
        "sourceAssets": brief["assets"],
        "officialAssetObjects": json.loads(root.get("official_asset_objects", "[]")),
        "lookDev": json.loads(root.get("look_dev", "{}")),
        "lightEnergyTotal": sum(
            float(obj.data.energy) for obj in bpy.context.scene.objects if obj.type == "LIGHT"
        ),
        "namedControls": applied,
        "shapeKeys": shape_keys,
        "materials": materials,
        "views": brief["render"]["views"],
        "hasArmature": any(obj.type == "ARMATURE" for obj in bpy.context.scene.objects),
        "hasAnimation": bool(bpy.data.actions),
        "reopenedChecks": checks,
    }
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")


def main():
    args = arguments()
    brief_path = Path(args.brief).resolve()
    output = Path(args.output).resolve()
    brief = load_brief(brief_path)
    recreate_output(output)
    clear_scene()
    _root, _head, applied, top_z, human_height = build_face(brief)
    camera, face_center_z, distance = add_scene(top_z, human_height, int(brief["render"]["resolution"]))
    blend_path = output / "face-proof.blend"
    bpy.ops.file.pack_all()
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
    bpy.ops.wm.open_mainfile(filepath=str(blend_path))
    checks, root, head = reopened_checks()
    if not all(checks.values()):
        raise RuntimeError(f"reopened face proof failed: {checks}")
    camera = bpy.data.objects.get("FaceProofCamera")
    render_views(camera, face_center_z, distance, human_height, output)
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
    write_manifest(output, brief, brief_path, root, head, applied, checks)
    print(f"FACE_PROOF_OK output={output}")


if __name__ == "__main__":
    main()
