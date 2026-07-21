"""Compile a simple humanoid, skeleton, and event reaction into Blender artifacts."""

import argparse
import hashlib
import json
import math
import shutil
import sys
from pathlib import Path

import bpy
from mathutils import Euler, Vector


LAB_ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(LAB_ROOT))

from reaction import GENERATOR_VERSION, synthesize_reaction
from humanoid_skeleton import humanoid_bones


ARTIFACT_POLICY = {
    "brief": "authoritative",
    "generator": "authoritative",
    "blend": "derived-reproducible",
}


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def rounded_vector(value):
    return [round(float(component), 6) for component in value]


def load_brief(path):
    brief = json.loads(path.read_text(encoding="utf-8"))
    if brief.get("schemaVersion") != 1 or brief.get("generatorVersion") != GENERATOR_VERSION:
        raise ValueError("unsupported humanoid brief version")
    if brief.get("artifactPolicy") != ARTIFACT_POLICY:
        raise ValueError(f"humanoid artifactPolicy must be {ARTIFACT_POLICY}")
    if brief.get("speciesId") != "lab.humanoid" or brief.get("speciesVersion") != 1:
        raise ValueError("unexpected humanoid identity")
    body = brief.get("body", {})
    ranges = {
        "heightMeters": (1.2, 2.4),
        "shoulderWidthMeters": (0.25, 0.8),
        "hipWidthMeters": (0.2, 0.6),
        "depthMeters": (0.12, 0.5),
    }
    for name, (minimum, maximum) in ranges.items():
        value = body.get(name)
        if not isinstance(value, (int, float)) or not minimum <= value <= maximum:
            raise ValueError(f"body {name} must be between {minimum} and {maximum}")
    motion = brief.get("motion", {})
    if not motion.get("name") or not isinstance(motion.get("seed"), int):
        raise ValueError("motion must declare a name and integer seed")
    synthesize_reaction(motion.get("event", {}), motion["seed"], motion.get("fps"))
    return brief


def recreate_output(output, brief):
    expected = (LAB_ROOT / "Derived" / f"humanoid-v{brief['speciesVersion']}").resolve()
    if output.resolve() != expected or output.is_symlink():
        raise ValueError(f"output must be the non-symlink directory {expected}")
    if output.exists():
        shutil.rmtree(output)
    output.mkdir(parents=True)


def make_armature(bones):
    data = bpy.data.armatures.new("GeneratedHumanoidSkeleton")
    armature = bpy.data.objects.new("Armature", data)
    armature["lab_role"] = "humanoid-armature"
    bpy.context.collection.objects.link(armature)
    bpy.context.view_layer.objects.active = armature
    armature.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")
    created = {}
    for spec in bones:
        bone = data.edit_bones.new(spec["name"])
        bone.head = Vector(spec["head"])
        bone.tail = Vector(spec["tail"])
        bone.use_deform = spec["deform"]
        if spec["parent"] is not None:
            bone.parent = created[spec["parent"]]
        created[spec["name"]] = bone
    bpy.ops.object.mode_set(mode="OBJECT")
    return armature


def append_box(vertices, faces, assignments, center, size, bone):
    base = len(vertices)
    cx, cy, cz = center
    sx, sy, sz = (component / 2.0 for component in size)
    vertices.extend([
        (cx - sx, cy - sy, cz - sz), (cx + sx, cy - sy, cz - sz),
        (cx + sx, cy + sy, cz - sz), (cx - sx, cy + sy, cz - sz),
        (cx - sx, cy - sy, cz + sz), (cx + sx, cy - sy, cz + sz),
        (cx + sx, cy + sy, cz + sz), (cx - sx, cy + sy, cz + sz),
    ])
    faces.extend([
        (base + 0, base + 1, base + 2, base + 3),
        (base + 4, base + 7, base + 6, base + 5),
        (base + 0, base + 4, base + 5, base + 1),
        (base + 1, base + 5, base + 6, base + 2),
        (base + 2, base + 6, base + 7, base + 3),
        (base + 4, base + 0, base + 3, base + 7),
    ])
    assignments.setdefault(bone, []).extend(range(base, base + 8))


def make_weighted_mesh(armature, body):
    height = float(body["heightMeters"])
    shoulder = float(body["shoulderWidthMeters"])
    hip = float(body["hipWidthMeters"])
    depth = float(body["depthMeters"])
    scale = height / 1.8
    vertices, faces, assignments = [], [], {}
    segments = [
        ((0.0, 0.0, 0.91 * scale), (hip, depth, 0.20 * scale), "pelvis"),
        ((0.0, 0.0, 1.14 * scale), (shoulder * 0.72, depth, 0.30 * scale), "spine"),
        ((0.0, 0.0, 1.39 * scale), (shoulder, depth * 1.08, 0.28 * scale), "chest"),
        ((0.0, 0.0, 1.56 * scale), (0.12 * scale, 0.12 * scale, 0.12 * scale), "neck"),
        ((0.0, 0.0, 1.71 * scale), (0.22 * scale, 0.20 * scale, 0.24 * scale), "head"),
        ((shoulder / 2.0 + 0.14 * scale, 0.0, 1.36 * scale), (0.29 * scale, 0.13 * scale, 0.14 * scale), "upper_arm_l"),
        ((shoulder / 2.0 + 0.40 * scale, 0.0, 1.22 * scale), (0.27 * scale, 0.11 * scale, 0.12 * scale), "lower_arm_l"),
        ((-shoulder / 2.0 - 0.14 * scale, 0.0, 1.36 * scale), (0.29 * scale, 0.13 * scale, 0.14 * scale), "upper_arm_r"),
        ((-shoulder / 2.0 - 0.40 * scale, 0.0, 1.22 * scale), (0.27 * scale, 0.11 * scale, 0.12 * scale), "lower_arm_r"),
        ((hip / 2.0, 0.0, 0.68 * scale), (0.16 * scale, 0.18 * scale, 0.42 * scale), "thigh_l"),
        ((hip / 2.0, 0.0, 0.29 * scale), (0.13 * scale, 0.15 * scale, 0.38 * scale), "shin_l"),
        ((hip / 2.0, -0.10 * scale, 0.07 * scale), (0.15 * scale, 0.30 * scale, 0.10 * scale), "foot_l"),
        ((-hip / 2.0, 0.0, 0.68 * scale), (0.16 * scale, 0.18 * scale, 0.42 * scale), "thigh_r"),
        ((-hip / 2.0, 0.0, 0.29 * scale), (0.13 * scale, 0.15 * scale, 0.38 * scale), "shin_r"),
        ((-hip / 2.0, -0.10 * scale, 0.07 * scale), (0.15 * scale, 0.30 * scale, 0.10 * scale), "foot_r"),
    ]
    for center, size, bone in segments:
        append_box(vertices, faces, assignments, center, size, bone)

    data = bpy.data.meshes.new("SK_LabHumanoid_Mesh")
    data.from_pydata(vertices, [], faces)
    data.update()
    mesh = bpy.data.objects.new("SK_LabHumanoid", data)
    mesh["lab_role"] = "humanoid-mesh"
    bpy.context.collection.objects.link(mesh)
    material = bpy.data.materials.new("M_LabHumanoidPlaceholder")
    material.diffuse_color = (0.28, 0.34, 0.40, 1.0)
    material.roughness = 0.85
    material["meaning"] = "mechanical-placeholder"
    mesh.data.materials.append(material)
    for bone, indices in assignments.items():
        mesh.vertex_groups.new(name=bone).add(indices, 1.0, "REPLACE")
    modifier = mesh.modifiers.new(name="Armature", type="ARMATURE")
    modifier.object = armature
    mesh.parent = armature
    return mesh


def make_animation(armature, recipe, name):
    scene = bpy.context.scene
    scene.render.fps = recipe["fps"]
    scene.render.fps_base = 1.0
    scene.frame_start = recipe["keyframes"][0]["frame"]
    scene.frame_end = recipe["keyframes"][-1]["frame"]
    armature.animation_data_create()
    action = bpy.data.actions.new(name)
    action.use_fake_user = True
    armature.animation_data.action = action
    for keyframe in recipe["keyframes"]:
        frame = keyframe["frame"]
        root = armature.pose.bones["root"]
        root.location = Vector(keyframe["rootLocationMeters"])
        root.keyframe_insert(data_path="location", frame=frame, group="root")
        for bone_name, degrees in keyframe["boneEulerDegrees"].items():
            bone = armature.pose.bones[bone_name]
            bone.rotation_mode = "XYZ"
            bone.rotation_euler = Euler(tuple(math.radians(value) for value in degrees), "XYZ")
            bone.keyframe_insert(data_path="rotation_euler", frame=frame, group=bone_name)
    scene.timeline_markers.new("event.impact", frame=4)
    scene.timeline_markers.new("reaction.peak", frame=12)
    scene.timeline_markers.new("reaction.recovered", frame=31)
    scene.frame_set(scene.frame_start)
    return action


def hierarchy_records(armature):
    return [
        {
            "name": bone.name,
            "parent": bone.parent.name if bone.parent else None,
            "headMeters": rounded_vector(bone.head_local),
            "tailMeters": rounded_vector(bone.tail_local),
        }
        for bone in armature.data.bones
    ]


def reopen_and_validate(blend_path, expected_bones, recipe, action_name):
    result = bpy.ops.wm.open_mainfile(filepath=str(blend_path))
    if result != {"FINISHED"}:
        raise RuntimeError(f"could not reopen humanoid blend: {result}")
    armatures = [obj for obj in bpy.context.scene.objects if obj.get("lab_role") == "humanoid-armature"]
    meshes = [obj for obj in bpy.context.scene.objects if obj.get("lab_role") == "humanoid-mesh"]
    armature = armatures[0] if len(armatures) == 1 else None
    mesh = meshes[0] if len(meshes) == 1 else None
    actual_bones = hierarchy_records(armature) if armature else []
    expected_hierarchy = [(bone["name"], bone["parent"]) for bone in expected_bones]
    actual_hierarchy = [(bone["name"], bone["parent"]) for bone in actual_bones]
    weighted = bool(mesh) and all(vertex.groups for vertex in mesh.data.vertices)
    modifiers = [modifier for modifier in mesh.modifiers if modifier.type == "ARMATURE"] if mesh else []
    scene = bpy.context.scene
    scene.frame_set(recipe["keyframes"][-1]["frame"])
    root_recovered = bool(armature) and armature.pose.bones["root"].location.length <= 0.000001
    rotations_recovered = bool(armature) and all(
        max(abs(value) for value in armature.pose.bones[name].rotation_euler) <= 0.000001
        for name in recipe["keyframes"][-1]["boneEulerDegrees"]
    )
    mesh_height = 0.0
    if mesh:
        zs = [vertex.co.z for vertex in mesh.data.vertices]
        mesh_height = max(zs) - min(zs)
    action = armature.animation_data.action if armature and armature.animation_data else None
    checks = {
        "singleArmature": armature is not None,
        "singleMesh": mesh is not None,
        "hierarchyMatches": actual_hierarchy == expected_hierarchy,
        "allVerticesWeighted": weighted,
        "armatureModifier": len(modifiers) == 1 and modifiers[0].object == armature,
        "singleMaterial": bool(mesh) and len(mesh.data.materials) == 1,
        "humanoidHeight": mesh_height >= 1.65,
        "timingMatches": scene.render.fps == recipe["fps"]
        and scene.frame_start == recipe["keyframes"][0]["frame"]
        and scene.frame_end == recipe["keyframes"][-1]["frame"],
        "actionAssigned": bool(action and action.name == action_name),
        "rootRecovered": root_recovered,
        "rotationsRecovered": rotations_recovered,
    }
    evidence = {"reopened": True, "passed": all(checks.values()), "checks": checks}
    if not evidence["passed"]:
        failed = [name for name, passed in checks.items() if not passed]
        raise ValueError(f"reopened humanoid validation failed: {failed}")
    scene.frame_set(scene.frame_start)
    return armature, mesh, actual_bones, evidence


def export_fbx(armature, mesh, path):
    bpy.ops.object.select_all(action="DESELECT")
    armature.select_set(True)
    mesh.select_set(True)
    bpy.context.view_layer.objects.active = armature
    result = bpy.ops.export_scene.fbx(
        filepath=str(path),
        use_selection=True,
        object_types={"ARMATURE", "MESH"},
        apply_unit_scale=True,
        apply_scale_options="FBX_SCALE_UNITS",
        axis_forward="-Y",
        axis_up="Z",
        add_leaf_bones=False,
        use_armature_deform_only=False,
        bake_anim=True,
        bake_anim_use_all_bones=True,
        bake_anim_use_nla_strips=False,
        bake_anim_use_all_actions=False,
        bake_anim_force_startend_keying=True,
        path_mode="STRIP",
        embed_textures=False,
    )
    if result != {"FINISHED"}:
        raise RuntimeError(f"humanoid FBX export failed: {result}")


def build(brief_path, output):
    brief = load_brief(brief_path)
    recreate_output(output, brief)
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.unit_settings.system = "METRIC"
    scene.unit_settings.scale_length = 1.0

    motion = brief["motion"]
    recipe = synthesize_reaction(motion["event"], motion["seed"], motion["fps"])
    recipe_path = output / "reaction.json"
    recipe_path.write_text(json.dumps(recipe, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    bones = humanoid_bones(brief["body"])
    armature = make_armature(bones)
    make_weighted_mesh(armature, brief["body"])
    make_animation(armature, recipe, motion["name"])

    blend_path = output / "humanoid.blend"
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path), check_existing=False)
    blend_path.with_suffix(blend_path.suffix + "1").unlink(missing_ok=True)

    armature, mesh, actual_bones, source_validation = reopen_and_validate(
        blend_path, bones, recipe, motion["name"]
    )
    fbx_path = output / "humanoid.fbx"
    export_fbx(armature, mesh, fbx_path)
    manifest = {
        "schemaVersion": 1,
        "speciesId": brief["speciesId"],
        "speciesVersion": brief["speciesVersion"],
        "stableIdentity": f"{brief['speciesId']}@{brief['speciesVersion']}",
        "generatorVersion": GENERATOR_VERSION,
        "blenderVersion": bpy.app.version_string,
        "authority": {
            "brief": {"file": brief_path.name, "sha256": sha256(brief_path), "authoritative": True},
            "generator": {"file": Path(__file__).name, "sha256": sha256(Path(__file__).resolve()), "authoritative": True},
            "reactionGenerator": {"file": "reaction.py", "sha256": sha256(LAB_ROOT / "reaction.py"), "authoritative": True},
        },
        "units": {"system": "metric", "metersPerBlenderUnit": 1.0},
        "body": brief["body"],
        "skeleton": {"bones": actual_bones},
        "motion": {
            "name": motion["name"],
            "seed": motion["seed"],
            "fps": recipe["fps"],
            "startFrame": recipe["keyframes"][0]["frame"],
            "endFrame": recipe["keyframes"][-1]["frame"],
            "durationSeconds": recipe["durationSeconds"],
            "recipeFile": recipe_path.name,
            "keyframes": recipe["keyframes"],
            "variation": recipe["variation"],
        },
        "blenderArtifact": {
            "file": blend_path.name,
            "derived": True,
            "reproducible": True,
            "validation": source_validation,
        },
        "fbxTransport": {
            "file": fbx_path.name,
            "derived": True,
            "forwardAxis": "-Y",
            "upAxis": "Z",
            "deterministicBytes": False,
            "validation": "import and measure in an engine before promotion",
        },
        "validation": {"passed": source_validation["passed"], "checks": source_validation["checks"]},
    }
    (output / "manifest.json").write_text(
        json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    print(f"EMBODIED_SPECIES_HUMANOID_OK manifest={output / 'manifest.json'}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--brief", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else [])
    try:
        build(args.brief.resolve(), args.output.resolve())
    except Exception as error:
        print(f"EMBODIED_SPECIES_HUMANOID_ERROR: {error}", file=sys.stderr)
        raise SystemExit(2) from error


if __name__ == "__main__":
    main()
