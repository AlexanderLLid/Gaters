"""Compile deterministic in-place mechanical locomotion clips for the lab humanoid."""

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

from generate_humanoid import (
    export_fbx,
    hierarchy_records,
    humanoid_bones,
    load_brief,
    make_armature,
    make_weighted_mesh,
)
from generate_physical_humanoid import action_fcurves
from locomotion import LOCOMOTION_VERSION, REQUIRED_CLIP_NAMES, synthesize_locomotion_clips


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def recreate_output(output):
    expected = (LAB_ROOT / "Derived" / "humanoid-locomotion-v1").resolve()
    if output.resolve() != expected or output.is_symlink():
        raise ValueError(f"output must be the non-symlink directory {expected}")
    if output.exists():
        shutil.rmtree(output)
    output.mkdir(parents=True)


def make_action(armature, clip):
    action = bpy.data.actions.new(clip["name"])
    action.use_fake_user = True
    armature.animation_data_create()
    armature.animation_data.action = action
    for key in clip["keyframes"]:
        root = armature.pose.bones["root"]
        root.location = Vector(key["rootLocationMeters"])
        root.rotation_mode = "XYZ"
        root.rotation_euler = Euler(
            tuple(math.radians(value) for value in key["rootEulerDegrees"]),
            "XYZ",
        )
        root.keyframe_insert("location", frame=key["frame"], group="root")
        root.keyframe_insert("rotation_euler", frame=key["frame"], group="root")
        for name, degrees in key["boneEulerDegrees"].items():
            bone = armature.pose.bones[name]
            bone.rotation_mode = "XYZ"
            bone.rotation_euler = Euler(
                tuple(math.radians(value) for value in degrees),
                "XYZ",
            )
            bone.keyframe_insert("rotation_euler", frame=key["frame"], group=name)
    return action


def rigid_weights_are_complete(mesh):
    for vertex in mesh.data.vertices:
        memberships = [group for group in vertex.groups if group.weight > 0.0]
        if len(memberships) != 1 or abs(memberships[0].weight - 1.0) > 0.000001:
            return False
    return True


def root_curves_are_zero(action, clip):
    root_paths = {
        'pose.bones["root"].location',
        'pose.bones["root"].rotation_euler',
    }
    curves = [curve for curve in action_fcurves(action) if curve.data_path in root_paths]
    if len(curves) != 6:
        return False
    return all(
        abs(float(curve.evaluate(key["frame"]))) <= 0.000001
        for curve in curves
        for key in clip["keyframes"]
    )


def reopen_and_validate(blend_path, expected_bones, clips):
    result = bpy.ops.wm.open_mainfile(filepath=str(blend_path))
    if result != {"FINISHED"}:
        raise RuntimeError(f"could not reopen locomotion Blend: {result}")

    armatures = [
        obj
        for obj in bpy.context.scene.objects
        if obj.get("lab_role") == "humanoid-armature"
    ]
    meshes = [
        obj
        for obj in bpy.context.scene.objects
        if obj.get("lab_role") == "humanoid-mesh"
    ]
    armature = armatures[0] if len(armatures) == 1 else None
    mesh = meshes[0] if len(meshes) == 1 else None
    actions = {action.name: action for action in bpy.data.actions}
    expected_hierarchy = [(bone["name"], bone["parent"]) for bone in expected_bones]
    actual_bones = hierarchy_records(armature) if armature else []
    actual_hierarchy = [(bone["name"], bone["parent"]) for bone in actual_bones]
    action_names_match = tuple(sorted(actions)) == tuple(sorted(REQUIRED_CLIP_NAMES))
    action_ranges_match = action_names_match and all(
        tuple(round(float(value)) for value in actions[clip["name"]].frame_range)
        == (clip["startFrame"], clip["endFrame"])
        for clip in clips
    )
    root_motion_zero = action_names_match and all(
        root_curves_are_zero(actions[clip["name"]], clip) for clip in clips
    )
    modifiers = (
        [modifier for modifier in mesh.modifiers if modifier.type == "ARMATURE"]
        if mesh
        else []
    )
    checks = {
        "singleArmature": armature is not None,
        "singleMesh": mesh is not None,
        "hierarchyMatches": actual_hierarchy == expected_hierarchy,
        "rigidWeightsComplete": bool(mesh) and rigid_weights_are_complete(mesh),
        "armatureModifier": len(modifiers) == 1 and modifiers[0].object == armature,
        "actionNamesMatch": action_names_match,
        "actionRangesMatch": action_ranges_match,
        "rootMotionZero": root_motion_zero,
    }
    evidence = {"reopened": True, "passed": all(checks.values()), "checks": checks}
    if not evidence["passed"]:
        failed = [name for name, passed in checks.items() if not passed]
        raise ValueError(f"reopened locomotion validation failed: {failed}")
    return armature, mesh, actual_bones, actions, evidence


def export_clip(armature, mesh, clip, action, output):
    armature.animation_data.action = action
    bpy.context.scene.render.fps = clip["fps"]
    bpy.context.scene.render.fps_base = 1.0
    bpy.context.scene.frame_start = clip["startFrame"]
    bpy.context.scene.frame_end = clip["endFrame"]
    bpy.context.scene.frame_set(clip["startFrame"])
    export_fbx(armature, mesh, output / f"{clip['name']}.fbx")


def build(brief_path, output):
    brief = load_brief(brief_path)
    recreate_output(output)
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.unit_settings.system = "METRIC"
    scene.unit_settings.scale_length = 1.0

    clips = synthesize_locomotion_clips(int(brief["motion"]["fps"]))
    if tuple(clip["name"] for clip in clips) != REQUIRED_CLIP_NAMES:
        raise ValueError("locomotion recipe order differs from the required clip contract")

    bones = humanoid_bones(brief["body"])
    armature = make_armature(bones)
    make_weighted_mesh(armature, brief["body"])
    for clip in clips:
        make_action(armature, clip)

    blend_path = output / "humanoid-locomotion.blend"
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path), check_existing=False)
    blend_path.with_suffix(blend_path.suffix + "1").unlink(missing_ok=True)

    armature, mesh, actual_bones, actions, validation = reopen_and_validate(
        blend_path,
        bones,
        clips,
    )
    for clip in clips:
        export_clip(armature, mesh, clip, actions[clip["name"]], output)

    manifest = {
        "schemaVersion": 1,
        "locomotionVersion": LOCOMOTION_VERSION,
        "speciesId": brief["speciesId"],
        "speciesVersion": brief["speciesVersion"],
        "stableIdentity": f"{brief['speciesId']}@{brief['speciesVersion']}",
        "blenderVersion": bpy.app.version_string,
        "authority": {
            "brief": {"file": brief_path.name, "sha256": sha256(brief_path)},
            "generator": {
                "file": Path(__file__).name,
                "sha256": sha256(Path(__file__).resolve()),
            },
            "locomotionRecipe": {
                "file": "locomotion.py",
                "sha256": sha256(LAB_ROOT / "locomotion.py"),
            },
        },
        "units": {"system": "metric", "metersPerBlenderUnit": 1.0},
        "body": brief["body"],
        "skeleton": {"bones": actual_bones},
        "clips": [
            {
                "name": clip["name"],
                "fps": clip["fps"],
                "startFrame": clip["startFrame"],
                "endFrame": clip["endFrame"],
                "durationSeconds": round(
                    (clip["endFrame"] - clip["startFrame"]) / clip["fps"],
                    6,
                ),
                "looping": clip["looping"],
                "fbxFile": f"{clip['name']}.fbx",
                "rootTranslationMeters": [0.0, 0.0, 0.0],
            }
            for clip in clips
        ],
        "blenderArtifact": {
            "file": blend_path.name,
            "derived": True,
            "reproducible": True,
            "validation": validation,
        },
        "fbxTransport": {
            "derived": True,
            "deterministicBytes": False,
            "forwardAxis": "-Y",
            "upAxis": "Z",
            "validation": "import and measure in Unreal before promotion",
        },
        "validation": validation,
    }
    manifest_path = output / "locomotion-manifest.json"
    manifest_path.write_text(
        json.dumps(manifest, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    print(f"LOCOMOTION_HUMANOID_OK clips={len(clips)} manifest={manifest_path}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--brief", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else [])
    try:
        build(args.brief.resolve(), args.output.resolve())
    except Exception as error:
        print(f"LOCOMOTION_HUMANOID_ERROR: {error}", file=sys.stderr)
        raise SystemExit(2) from error


if __name__ == "__main__":
    main()
