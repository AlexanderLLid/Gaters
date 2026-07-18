"""Generate one disposable style-neutral rig and motion fixture from a JSON brief."""

import argparse
import hashlib
import json
import shutil
import sys
from pathlib import Path

import bpy
from mathutils import Vector


GENERATOR_VERSION = 1
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
    if brief.get("schemaVersion") != 1:
        raise ValueError("motion brief schemaVersion must be 1")
    if brief.get("generatorVersion") != GENERATOR_VERSION:
        raise ValueError(f"motion brief generatorVersion must be {GENERATOR_VERSION}")
    if brief.get("artifactPolicy") != ARTIFACT_POLICY:
        raise ValueError(f"motion artifactPolicy must be {ARTIFACT_POLICY}")
    if brief.get("styleNeutral") is not True or brief.get("selectedStyle", "missing") is not None:
        raise ValueError("motion fixture must be style-neutral with selectedStyle null")

    bones = brief.get("skeleton", {}).get("bones", [])
    if not bones or bones[0].get("name") != "root" or bones[0].get("parent") is not None:
        raise ValueError("motion skeleton must begin with a parentless root")
    names = []
    for bone in bones:
        name = bone.get("name")
        if not name or name in names:
            raise ValueError(f"invalid or duplicate bone name: {name}")
        if bone.get("parent") is not None and bone["parent"] not in names:
            raise ValueError(f"bone {name} parent must precede it")
        if len(bone.get("headMeters", [])) != 3 or len(bone.get("tailMeters", [])) != 3:
            raise ValueError(f"bone {name} must declare headMeters and tailMeters")
        if bone["headMeters"] == bone["tailMeters"]:
            raise ValueError(f"bone {name} cannot have zero length")
        names.append(name)

    clip = brief.get("clip", {})
    fps = clip.get("fps")
    start = clip.get("startFrame")
    end = clip.get("endFrame")
    if not isinstance(fps, int) or fps <= 0 or not isinstance(start, int) or not isinstance(end, int) or end <= start:
        raise ValueError("clip must declare positive fps and an increasing integer frame range")
    samples = clip.get("rootSamplesMeters", [])
    if [sample.get("frame") for sample in samples] != sorted(sample.get("frame") for sample in samples):
        raise ValueError("root samples must be ordered")
    if not samples or samples[0].get("frame") != start or samples[-1].get("frame") != end:
        raise ValueError("root samples must cover the clip endpoints")
    for sample in samples:
        if len(sample.get("location", [])) != 3 or not start <= sample["frame"] <= end:
            raise ValueError("root sample is malformed or outside the clip")
    for event in clip.get("events", []):
        if not event.get("name") or not start <= event.get("frame", start - 1) <= end:
            raise ValueError("clip event is malformed or outside the clip")
    return brief


def recreate_output(output, brief):
    expected = f"neutral-motion-v{brief['assetVersion']}"
    if output.name != expected or output.parent.name != "Derived":
        raise ValueError(f"output must be a Derived/{expected} directory")
    if output.is_symlink():
        raise ValueError("motion output cannot be a symlink")
    if output.exists():
        shutil.rmtree(output)
    output.mkdir(parents=True)


def make_armature(brief):
    data = bpy.data.armatures.new("NeutralMotionSkeleton")
    # Unreal's FBX importer recognizes Blender's conventional Armature node and removes
    # it instead of minting an extra bone above the declared root.
    armature = bpy.data.objects.new("Armature", data)
    armature["fixture_role"] = "motion-armature"
    bpy.context.collection.objects.link(armature)
    bpy.context.view_layer.objects.active = armature
    armature.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")
    created = {}
    for spec in brief["skeleton"]["bones"]:
        bone = data.edit_bones.new(spec["name"])
        bone.head = Vector(spec["headMeters"])
        bone.tail = Vector(spec["tailMeters"])
        bone.use_deform = True
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


def make_weighted_mesh(armature):
    vertices = []
    faces = []
    assignments = {}
    append_box(vertices, faces, assignments, (0.0, 0.0, 1.0), (0.44, 0.30, 0.80), "spine")
    append_box(vertices, faces, assignments, (0.0, 0.18, 0.48), (0.17, 0.17, 0.62), "foot_l")
    append_box(vertices, faces, assignments, (0.0, -0.18, 0.48), (0.17, 0.17, 0.62), "foot_r")

    data = bpy.data.meshes.new("SK_NeutralMotionFixture_Mesh")
    data.from_pydata(vertices, [], faces)
    data.update()
    mesh = bpy.data.objects.new("SK_NeutralMotionFixture", data)
    mesh["fixture_role"] = "motion-mesh"
    bpy.context.collection.objects.link(mesh)
    material = bpy.data.materials.new("M_NeutralMotionPlaceholder")
    material.diffuse_color = (0.34, 0.40, 0.46, 1.0)
    material.roughness = 0.8
    material["meaning"] = "style-neutral-motion-fixture"
    mesh.data.materials.append(material)
    for bone, indices in assignments.items():
        mesh.vertex_groups.new(name=bone).add(indices, 1.0, "REPLACE")
    modifier = mesh.modifiers.new(name="Armature", type="ARMATURE")
    modifier.object = armature
    mesh.parent = armature
    return mesh


def make_animation(armature, brief):
    clip = brief["clip"]
    scene = bpy.context.scene
    scene.render.fps = clip["fps"]
    scene.render.fps_base = 1.0
    scene.frame_start = clip["startFrame"]
    scene.frame_end = clip["endFrame"]
    armature.animation_data_create()
    action = bpy.data.actions.new(clip["name"])
    action.use_fake_user = True
    armature.animation_data.action = action

    root = armature.pose.bones["root"]
    for sample in clip["rootSamplesMeters"]:
        root.location = Vector(sample["location"])
        root.keyframe_insert(data_path="location", frame=sample["frame"], group="root")

    lift = float(clip["footLiftMeters"])
    left = armature.pose.bones["foot_l"]
    right = armature.pose.bones["foot_r"]
    foot_samples = {
        left: [(clip["startFrame"], 0.0), (23, lift), (clip["endFrame"], 0.0)],
        right: [(clip["startFrame"], 0.0), (8, lift), (16, 0.0), (clip["endFrame"], 0.0)],
    }
    for pose_bone, samples in foot_samples.items():
        for frame, z in samples:
            pose_bone.location = Vector((0.0, 0.0, z))
            pose_bone.keyframe_insert(data_path="location", frame=frame, group=pose_bone.name)

    for event in clip["events"]:
        scene.timeline_markers.new(f"{event['name']}|{event['frame']}", frame=event["frame"])
    scene.frame_set(clip["startFrame"])
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


def reopen_and_validate(blend_path, brief):
    result = bpy.ops.wm.open_mainfile(filepath=str(blend_path))
    if result != {"FINISHED"}:
        raise RuntimeError(f"could not reopen derived motion blend: {result}")
    armatures = [obj for obj in bpy.context.scene.objects if obj.get("fixture_role") == "motion-armature"]
    meshes = [obj for obj in bpy.context.scene.objects if obj.get("fixture_role") == "motion-mesh"]
    armature = armatures[0] if len(armatures) == 1 else None
    mesh = meshes[0] if len(meshes) == 1 else None
    expected_bones = brief["skeleton"]["bones"]
    actual_bones = hierarchy_records(armature) if armature else []
    scene = bpy.context.scene
    clip = brief["clip"]
    root_samples = []
    if armature:
        for sample in clip["rootSamplesMeters"]:
            scene.frame_set(sample["frame"])
            root_samples.append({
                "frame": sample["frame"],
                "locationMeters": rounded_vector(armature.pose.bones["root"].location),
            })
    event_pairs = [marker.name for marker in sorted(scene.timeline_markers, key=lambda item: item.frame)]
    expected_event_pairs = [f"{event['name']}|{event['frame']}" for event in clip["events"]]
    weighted = bool(mesh) and all(vertex.groups for vertex in mesh.data.vertices)
    armature_modifiers = [modifier for modifier in mesh.modifiers if modifier.type == "ARMATURE"] if mesh else []
    checks = {
        "singleArmature": armature is not None,
        "singleMesh": mesh is not None,
        "hierarchyMatches": actual_bones == expected_bones,
        "timingMatches": scene.render.fps == clip["fps"]
        and scene.frame_start == clip["startFrame"]
        and scene.frame_end == clip["endFrame"],
        "rootSamplesMatch": [item["locationMeters"] for item in root_samples]
        == [rounded_vector(item["location"]) for item in clip["rootSamplesMeters"]],
        "eventsMatch": event_pairs == expected_event_pairs,
        "allVerticesWeighted": weighted,
        "armatureModifier": len(armature_modifiers) == 1 and armature_modifiers[0].object == armature,
        "actionAssigned": bool(armature and armature.animation_data and armature.animation_data.action
                               and armature.animation_data.action.name == clip["name"]),
    }
    evidence = {
        "reopened": True,
        "passed": all(checks.values()),
        "checks": checks,
        "rootSamples": root_samples,
    }
    if not evidence["passed"]:
        failed = [name for name, passed in checks.items() if not passed]
        raise ValueError(f"reopened motion validation failed: {failed}")
    scene.frame_set(clip["startFrame"])
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
        use_armature_deform_only=True,
        bake_anim=True,
        bake_anim_use_all_bones=True,
        bake_anim_use_nla_strips=False,
        bake_anim_use_all_actions=False,
        bake_anim_force_startend_keying=True,
        path_mode="STRIP",
        embed_textures=False,
    )
    if result != {"FINISHED"}:
        raise RuntimeError(f"motion FBX export failed: {result}")


def build(brief_path, output):
    brief = load_brief(brief_path)
    recreate_output(output, brief)
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.unit_settings.system = "METRIC"
    scene.unit_settings.scale_length = 1.0

    armature = make_armature(brief)
    make_weighted_mesh(armature)
    make_animation(armature, brief)
    blend_name = f"neutral-motion-v{brief['assetVersion']}.blend"
    blend_path = output / blend_name
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path), check_existing=False)
    blend_path.with_suffix(blend_path.suffix + "1").unlink(missing_ok=True)

    armature, mesh, bones, source_validation = reopen_and_validate(blend_path, brief)
    fbx_path = output / f"neutral-motion-v{brief['assetVersion']}.fbx"
    export_fbx(armature, mesh, fbx_path)
    clip = brief["clip"]
    duration = (clip["endFrame"] - clip["startFrame"]) / clip["fps"]
    manifest = {
        "schemaVersion": 1,
        "assetId": brief["assetId"],
        "assetVersion": brief["assetVersion"],
        "stableIdentity": f"{brief['assetId']}@{brief['assetVersion']}",
        "generatorVersion": GENERATOR_VERSION,
        "blenderVersion": bpy.app.version_string,
        "authority": {
            "brief": {"file": brief_path.name, "sha256": sha256(brief_path), "authoritative": True},
            "generator": {
                "file": Path(__file__).name,
                "sha256": sha256(Path(__file__).resolve()),
                "version": GENERATOR_VERSION,
                "authoritative": True,
            },
        },
        "sourceIdentity": {"selectedStyle": brief["selectedStyle"], "styleNeutral": True},
        "units": {"system": "metric", "metersPerBlenderUnit": 1.0},
        "skeleton": {"bones": bones},
        "clip": {
            "name": clip["name"],
            "fps": clip["fps"],
            "startFrame": clip["startFrame"],
            "endFrame": clip["endFrame"],
            "durationSeconds": round(duration, 6),
            "rootSamples": source_validation["rootSamples"],
            "events": clip["events"],
        },
        "blenderArtifact": {
            "file": blend_name,
            "format": "blend",
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
            "validation": "measure native skeleton, timing, root track, and events in Unreal",
        },
        "validation": {"passed": source_validation["passed"], "checks": source_validation["checks"]},
    }
    (output / "manifest.json").write_text(
        json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    print(f"GATERS_BLENDER_MOTION_OK manifest={output / 'manifest.json'}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--brief", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else [])
    try:
        build(args.brief.resolve(), args.output.resolve())
    except Exception as error:
        print(f"GATERS_BLENDER_MOTION_ERROR: {error}", file=sys.stderr)
        raise SystemExit(2) from error


if __name__ == "__main__":
    main()
