"""Compile a constrained humanoid, simulate an impact, and bake it to a skeleton."""

import argparse
import hashlib
import json
import math
import shutil
import sys
from pathlib import Path

import bpy
from mathutils import Matrix, Vector


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
from physical_profile import PROFILE_VERSION, generate_physical_profile


PHYSICAL_GENERATOR_VERSION = 1
PHYSICAL_PARTS = (
    "pelvis",
    "spine",
    "chest",
    "head",
    "upper_arm_l",
    "lower_arm_l",
    "upper_arm_r",
    "lower_arm_r",
    "thigh_l",
    "shin_l",
    "foot_l",
    "thigh_r",
    "shin_r",
    "foot_r",
)


def action_fcurves(action):
    if hasattr(action, "fcurves"):
        return list(action.fcurves)
    return [
        fcurve
        for layer in action.layers
        for strip in layer.strips
        for channelbag in strip.channelbags
        for fcurve in channelbag.fcurves
    ]


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def recreate_output(output, brief):
    expected = (LAB_ROOT / "Derived" / f"humanoid-physical-v{brief['speciesVersion']}").resolve()
    if output.resolve() != expected or output.is_symlink():
        raise ValueError(f"output must be the non-symlink directory {expected}")
    if output.exists():
        shutil.rmtree(output)
    output.mkdir(parents=True)


def bone_lookup(bones):
    return {bone["name"]: bone for bone in bones}


def part_width_depth(body, bone):
    scale = float(body["heightMeters"]) / 1.8
    shoulder = float(body["shoulderWidthMeters"])
    hip = float(body["hipWidthMeters"])
    depth = float(body["depthMeters"])
    values = {
        "pelvis": (hip, depth),
        "spine": (shoulder * 0.72, depth),
        "chest": (shoulder, depth * 1.08),
        "head": (0.22 * scale, 0.20 * scale),
        "upper_arm_l": (0.13 * scale, 0.14 * scale),
        "lower_arm_l": (0.11 * scale, 0.12 * scale),
        "upper_arm_r": (0.13 * scale, 0.14 * scale),
        "lower_arm_r": (0.11 * scale, 0.12 * scale),
        "thigh_l": (0.16 * scale, 0.18 * scale),
        "shin_l": (0.13 * scale, 0.15 * scale),
        "foot_l": (0.15 * scale, 0.10 * scale),
        "thigh_r": (0.16 * scale, 0.18 * scale),
        "shin_r": (0.13 * scale, 0.15 * scale),
        "foot_r": (0.15 * scale, 0.10 * scale),
    }
    return values[bone]


def aligned_matrix(head, tail):
    head = Vector(head)
    tail = Vector(tail)
    direction = tail - head
    rotation = direction.to_track_quat("Y", "Z").to_matrix().to_4x4()
    return Matrix.Translation((head + tail) * 0.5) @ rotation


def make_physics_proxy(spec, part, body):
    head = Vector(spec["head"])
    tail = Vector(spec["tail"])
    length = max((tail - head).length, 0.08)
    width, depth = part_width_depth(body, spec["name"])
    bpy.ops.mesh.primitive_cube_add(size=1.0)
    proxy = bpy.context.object
    proxy.name = f"RB_{spec['name']}"
    proxy["lab_role"] = "physics-part"
    proxy["bone"] = spec["name"]
    proxy.matrix_world = aligned_matrix(head, tail)
    proxy.dimensions = (width, length, depth)
    bpy.context.view_layer.objects.active = proxy
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    bpy.ops.rigidbody.object_add(type="PASSIVE" if part["anchored"] else "ACTIVE")
    rigid = proxy.rigid_body
    rigid.collision_shape = part["collisionShape"]
    rigid.mass = part["massKg"]
    rigid.linear_damping = part["linearDamping"]
    rigid.angular_damping = part["angularDamping"]
    rigid.friction = 0.65
    rigid.restitution = 0.05
    rigid.use_margin = True
    rigid.collision_margin = 0.008
    collision_layers = [False] * 20
    collision_layers[0] = True
    if spec["name"] == "chest":
        collision_layers[1] = True
    rigid.collision_collections = collision_layers
    proxy.display_type = "WIRE"
    proxy.hide_render = True
    return proxy


def joint_anchor(joint, bones):
    spec = bones[joint["anchorBone"]]
    if joint["anchorBone"] == joint["child"]:
        return Vector(spec["head"])
    return (Vector(spec["head"]) + Vector(spec["tail"])) * 0.5


def make_joint(joint, proxies, bones):
    parent = proxies[joint["parent"]]
    anchor = joint_anchor(joint, bones)
    constraint = bpy.data.objects.new(f"RB_{joint['name']}", None)
    constraint["lab_role"] = "physics-joint"
    constraint["joint"] = joint["name"]
    bpy.context.collection.objects.link(constraint)
    constraint.matrix_world = Matrix.Translation(anchor) @ parent.rotation_quaternion.to_matrix().to_4x4()
    bpy.context.view_layer.objects.active = constraint
    constraint.select_set(True)
    bpy.ops.rigidbody.constraint_add(type=joint["type"])
    settings = constraint.rigid_body_constraint
    settings.spring_type = "SPRING2"
    settings.object1 = parent
    settings.object2 = proxies[joint["child"]]
    settings.disable_collisions = True
    settings.use_override_solver_iterations = True
    settings.solver_iterations = 80
    for axis in "xyz":
        setattr(settings, f"use_limit_lin_{axis}", True)
        limits = joint["linearLimitsMeters"][axis]
        setattr(settings, f"limit_lin_{axis}_lower", limits[0])
        setattr(settings, f"limit_lin_{axis}_upper", limits[1])
        setattr(settings, f"use_limit_ang_{axis}", True)
        angular = joint["angularLimitsDegrees"][axis]
        setattr(settings, f"limit_ang_{axis}_lower", math.radians(angular[0]))
        setattr(settings, f"limit_ang_{axis}_upper", math.radians(angular[1]))
        setattr(settings, f"use_spring_ang_{axis}", True)
        setattr(settings, f"spring_stiffness_ang_{axis}", joint["angularSpringStiffness"])
        setattr(settings, f"spring_damping_ang_{axis}", joint["angularSpringDamping"])
    constraint.empty_display_type = "ARROWS"
    constraint.empty_display_size = 0.05
    constraint.hide_render = True
    constraint.select_set(False)
    return constraint


def make_projectile(event, chest_spec, frame_start, impact_frame, stop_frame):
    direction = Vector(event["direction"]).normalized()
    target = (Vector(chest_spec["head"]) + Vector(chest_spec["tail"])) * 0.5
    strength = float(event["strength"])
    contact = target - direction * 0.31
    start = contact - direction * (0.45 + 0.10 * strength)
    end = contact + direction * (0.035 + 0.025 * strength)
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=2, radius=0.11, location=start)
    projectile = bpy.context.object
    projectile.name = "ImpactDriver"
    projectile["lab_role"] = "impact-driver"
    bpy.ops.rigidbody.object_add(type="ACTIVE")
    projectile.rigid_body.kinematic = True
    projectile.rigid_body.mass = 4.0 + 8.0 * strength
    projectile.rigid_body.collision_shape = "SPHERE"
    projectile.rigid_body.restitution = 0.05
    projectile.rigid_body.friction = 0.2
    projectile_layers = [False] * 20
    projectile_layers[1] = True
    projectile.rigid_body.collision_collections = projectile_layers
    projectile.rigid_body.enabled = True
    projectile.keyframe_insert(data_path="rigid_body.enabled", frame=frame_start)
    projectile.location = start
    projectile.keyframe_insert(data_path="location", frame=frame_start)
    projectile.location = contact
    projectile.keyframe_insert(data_path="location", frame=impact_frame)
    projectile.location = end
    projectile.keyframe_insert(data_path="location", frame=stop_frame)
    projectile.rigid_body.enabled = False
    projectile.keyframe_insert(data_path="rigid_body.enabled", frame=stop_frame + 1)
    if projectile.animation_data and projectile.animation_data.action:
        for fcurve in action_fcurves(projectile.animation_data.action):
            for keyframe in fcurve.keyframe_points:
                keyframe.interpolation = "CONSTANT" if "enabled" in fcurve.data_path else "LINEAR"
    material = bpy.data.materials.new("M_ImpactDriver")
    material.diffuse_color = (0.8, 0.15, 0.05, 1.0)
    projectile.data.materials.append(material)
    projectile.hide_render = True
    return projectile


def make_preview_stage():
    bpy.ops.mesh.primitive_plane_add(size=5.0, location=(0.0, 0.0, 0.0))
    floor = bpy.context.object
    floor.name = "PreviewFloor"
    floor["lab_role"] = "preview-stage"
    floor_material = bpy.data.materials.new("M_PreviewFloor")
    floor_material.diffuse_color = (0.055, 0.065, 0.075, 1.0)
    floor_material.roughness = 0.95
    floor.data.materials.append(floor_material)

    camera_data = bpy.data.cameras.new("PreviewCamera")
    camera = bpy.data.objects.new("PreviewCamera", camera_data)
    camera["lab_role"] = "preview-camera"
    bpy.context.collection.objects.link(camera)
    camera.location = (3.0, -5.5, 2.35)
    target = Vector((0.0, 0.0, 0.95))
    camera.rotation_euler = (target - camera.location).to_track_quat("-Z", "Y").to_euler()
    camera.data.lens = 58.0
    bpy.context.scene.camera = camera

    light_data = bpy.data.lights.new("PreviewKey", type="AREA")
    light_data.energy = 900.0
    light_data.shape = "DISK"
    light_data.size = 4.0
    light = bpy.data.objects.new("PreviewKey", light_data)
    light["lab_role"] = "preview-light"
    bpy.context.collection.objects.link(light)
    light.location = (-3.5, -4.0, 5.5)
    light.rotation_euler = (target - light.location).to_track_quat("-Z", "Y").to_euler()

    world = bpy.context.scene.world or bpy.data.worlds.new("PreviewWorld")
    bpy.context.scene.world = world
    world.color = (0.012, 0.016, 0.022)


def simulate(scene, proxies, frame_start, frame_end, impact_frame):
    scene.frame_set(frame_start)
    bpy.context.view_layer.update()
    rest = {name: proxy.matrix_world.copy() for name, proxy in proxies.items()}
    samples = {}
    chest_origin = rest["chest"].translation.copy()
    peak_chest_displacement = 0.0
    peak_frame = frame_start
    preimpact_chest_displacement = 0.0
    for frame in range(frame_start, frame_end + 1):
        scene.frame_set(frame)
        bpy.context.view_layer.update()
        depsgraph = bpy.context.evaluated_depsgraph_get()
        samples[frame] = {
            name: proxy.evaluated_get(depsgraph).matrix_world.copy() for name, proxy in proxies.items()
        }
        displacement = (samples[frame]["chest"].translation - chest_origin).length
        if displacement > peak_chest_displacement:
            peak_chest_displacement = displacement
            peak_frame = frame
        if frame < impact_frame:
            preimpact_chest_displacement = max(preimpact_chest_displacement, displacement)
    final_chest_displacement = (samples[frame_end]["chest"].translation - chest_origin).length
    final_frame_chest_motion = (
        samples[frame_end]["chest"].translation - samples[frame_end - 1]["chest"].translation
    ).length
    metrics = {
        "preimpactChestDisplacementMeters": preimpact_chest_displacement,
        "peakChestDisplacementMeters": peak_chest_displacement,
        "peakFrame": peak_frame,
        "finalChestDisplacementMeters": final_chest_displacement,
        "finalFrameChestMotionMeters": final_frame_chest_motion,
    }
    return rest, samples, metrics


def bake_skeleton(armature, rest_proxies, samples, action_name):
    scene = bpy.context.scene
    armature.animation_data_create()
    action = bpy.data.actions.new(action_name)
    action.use_fake_user = True
    armature.animation_data.action = action
    armature_inv = armature.matrix_world.inverted()
    for frame, frame_samples in samples.items():
        scene.frame_set(frame)
        bpy.context.view_layer.update()
        root = armature.pose.bones["root"]
        root.rotation_mode = "QUATERNION"
        root.location = (0.0, 0.0, 0.0)
        root.rotation_quaternion = (1.0, 0.0, 0.0, 0.0)
        root.keyframe_insert(data_path="location", frame=frame, group="root")
        root.keyframe_insert(data_path="rotation_quaternion", frame=frame, group="root")
        for bone_name in PHYSICAL_PARTS:
            pose_bone = armature.pose.bones[bone_name]
            rest_world = armature.matrix_world @ pose_bone.bone.matrix_local
            delta_world = frame_samples[bone_name] @ rest_proxies[bone_name].inverted()
            pose_bone.rotation_mode = "QUATERNION"
            pose_bone.matrix = armature_inv @ delta_world @ rest_world
            pose_bone.keyframe_insert(data_path="location", frame=frame, group=bone_name)
            pose_bone.keyframe_insert(data_path="rotation_quaternion", frame=frame, group=bone_name)
    return action, sum(len(fcurve.keyframe_points) for fcurve in action_fcurves(action))


def validate_scene(armature, mesh, profile, action, metrics, baked_count):
    physics_parts = [obj for obj in bpy.context.scene.objects if obj.get("lab_role") == "physics-part"]
    joints = [obj for obj in bpy.context.scene.objects if obj.get("lab_role") == "physics-joint"]
    masses = sorted(round(obj.rigid_body.mass, 6) for obj in physics_parts)
    expected_masses = sorted(round(part["massKg"], 6) for part in profile["parts"])
    checks = {
        "singleArmature": armature is not None,
        "singleMesh": mesh is not None,
        "allPartsCreated": len(physics_parts) == len(profile["parts"]),
        "allJointsCreated": len(joints) == len(profile["joints"]),
        "massesMatch": masses == expected_masses,
        "jointObjectsAssigned": all(
            obj.rigid_body_constraint
            and obj.rigid_body_constraint.object1
            and obj.rigid_body_constraint.object2
            for obj in joints
        ),
        "jointLimitsEnabled": all(
            all(
                getattr(obj.rigid_body_constraint, f"use_limit_ang_{axis}")
                and getattr(obj.rigid_body_constraint, f"use_limit_lin_{axis}")
                for axis in "xyz"
            )
            for obj in joints
        ),
        "stableBeforeImpact": metrics["preimpactChestDisplacementMeters"] <= 0.02,
        "simulationMovedChest": 0.01 <= metrics["peakChestDisplacementMeters"] <= 0.5,
        "reactionSettled": metrics["finalChestDisplacementMeters"] <= 0.2
        and metrics["finalFrameChestMotionMeters"] <= 0.01,
        "actionAssigned": bool(armature.animation_data and armature.animation_data.action == action),
        "skeletonBaked": baked_count >= 100,
    }
    return {"passed": all(checks.values()), "checks": checks}


def reopen_and_validate(
    blend_path,
    expected_bones,
    profile,
    expected_action,
    maximum_final_displacement=0.2,
):
    result = bpy.ops.wm.open_mainfile(filepath=str(blend_path))
    if result != {"FINISHED"}:
        raise RuntimeError(f"could not reopen physical humanoid blend: {result}")
    scene = bpy.context.scene
    armatures = [obj for obj in scene.objects if obj.get("lab_role") == "humanoid-armature"]
    meshes = [obj for obj in scene.objects if obj.get("lab_role") == "humanoid-mesh"]
    parts = [obj for obj in scene.objects if obj.get("lab_role") == "physics-part"]
    joints = [obj for obj in scene.objects if obj.get("lab_role") == "physics-joint"]
    armature = armatures[0] if len(armatures) == 1 else None
    mesh = meshes[0] if len(meshes) == 1 else None
    action = armature.animation_data.action if armature and armature.animation_data else None
    expected_hierarchy = [(bone["name"], bone["parent"]) for bone in expected_bones]
    actual_hierarchy = (
        [(bone["name"], bone["parent"]) for bone in hierarchy_records(armature)]
        if armature
        else []
    )
    baked_count = (
        sum(len(fcurve.keyframe_points) for fcurve in action_fcurves(action)) if action else 0
    )
    baked_metrics = {
        "peakChestDisplacementMeters": 0.0,
        "finalChestDisplacementMeters": 0.0,
        "finalFrameChestMotionMeters": 0.0,
    }
    if armature:
        positions = []
        for frame in range(scene.frame_start, scene.frame_end + 1):
            scene.frame_set(frame)
            positions.append(
                (armature.matrix_world @ armature.pose.bones["chest"].matrix).translation.copy()
            )
        origin = positions[0]
        displacements = [(position - origin).length for position in positions]
        baked_metrics = {
            "peakChestDisplacementMeters": max(displacements),
            "finalChestDisplacementMeters": displacements[-1],
            "finalFrameChestMotionMeters": (positions[-1] - positions[-2]).length,
        }
    checks = {
        "reopened": True,
        "singleArmature": armature is not None,
        "singleMesh": mesh is not None,
        "hierarchyMatches": actual_hierarchy == expected_hierarchy,
        "allPartsPersisted": len(parts) == len(profile["parts"]),
        "allJointsPersisted": len(joints) == len(profile["joints"]),
        "actionPersisted": bool(action and action.name == expected_action),
        "keyframesPersisted": baked_count >= 100,
        "bakedReactionBounded": 0.01 <= baked_metrics["peakChestDisplacementMeters"] <= 0.5,
        "bakedReactionSettled": baked_metrics["finalChestDisplacementMeters"]
        <= maximum_final_displacement
        and baked_metrics["finalFrameChestMotionMeters"] <= 0.01,
    }
    evidence = {
        "passed": all(checks.values()),
        "checks": checks,
        "bakedMotion": {name: round(value, 6) for name, value in baked_metrics.items()},
        "bakedKeyframeCount": baked_count,
    }
    if not evidence["passed"]:
        failed = [name for name, passed in checks.items() if not passed]
        raise ValueError(f"reopened physical humanoid validation failed: {failed}")
    scene.frame_set(scene.frame_start)
    return armature, mesh, evidence


def render_previews(output, peak_frame):
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 480
    scene.render.resolution_y = 480
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.film_transparent = False
    previews = []
    for label, frame in (
        ("rest", scene.frame_start),
        ("peak", int(peak_frame)),
        ("settled", scene.frame_end),
    ):
        scene.frame_set(frame)
        path = output / f"preview-{label}.png"
        scene.render.filepath = str(path)
        result = bpy.ops.render.render(write_still=True)
        if result != {"FINISHED"} or not path.exists() or path.stat().st_size == 0:
            raise RuntimeError(f"could not render physical humanoid preview: {label}")
        previews.append({"label": label, "frame": frame, "file": path.name})
    scene.frame_set(scene.frame_start)
    return previews


def build(brief_path, output):
    brief = load_brief(brief_path)
    recreate_output(output, brief)
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.unit_settings.system = "METRIC"
    scene.unit_settings.scale_length = 1.0
    scene.render.fps = int(brief["motion"]["fps"])
    scene.render.fps_base = 1.0
    scene.frame_start = 1
    scene.frame_end = 91
    bpy.ops.rigidbody.world_add()
    scene.rigidbody_world.substeps_per_frame = 10
    scene.rigidbody_world.solver_iterations = 50
    scene.rigidbody_world.point_cache.frame_start = scene.frame_start
    scene.rigidbody_world.point_cache.frame_end = scene.frame_end

    bones = humanoid_bones(brief["body"])
    bones_by_name = bone_lookup(bones)
    profile = generate_physical_profile(brief["body"])
    profile_path = output / "physical-profile.json"
    profile_path.write_text(json.dumps(profile, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    armature = make_armature(bones)
    mesh = make_weighted_mesh(armature, brief["body"])
    make_preview_stage()
    parts = {part["bone"]: part for part in profile["parts"]}
    proxies = {
        name: make_physics_proxy(bones_by_name[name], parts[name], brief["body"])
        for name in PHYSICAL_PARTS
    }
    constraints = [make_joint(joint, proxies, bones_by_name) for joint in profile["joints"]]
    make_projectile(brief["motion"]["event"], bones_by_name["chest"], 1, 6, 8)
    scene.timeline_markers.new("event.impact", frame=6)
    scene.timeline_markers.new("simulation.end", frame=91)

    rest_proxies, samples, metrics = simulate(
        scene, proxies, scene.frame_start, scene.frame_end, impact_frame=6
    )
    action_name = "A_PhysicalImpactReaction"
    action, baked_count = bake_skeleton(armature, rest_proxies, samples, action_name)
    validation = validate_scene(armature, mesh, profile, action, metrics, baked_count)
    if not validation["passed"]:
        failed = [name for name, passed in validation["checks"].items() if not passed]
        rounded_metrics = {name: round(value, 6) for name, value in metrics.items()}
        raise ValueError(
            f"physical humanoid validation failed: {failed}; metrics={rounded_metrics}"
        )

    for proxy in proxies.values():
        proxy.hide_set(True)
    for constraint in constraints:
        constraint.hide_set(True)
    scene.frame_set(scene.frame_start)
    blend_path = output / "humanoid-physical.blend"
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path), check_existing=False)
    blend_path.with_suffix(blend_path.suffix + "1").unlink(missing_ok=True)

    armature, mesh, artifact_validation = reopen_and_validate(
        blend_path, bones, profile, action_name
    )
    scene = bpy.context.scene
    previews = render_previews(output, metrics["peakFrame"])
    fbx_path = output / "humanoid-physical.fbx"
    export_fbx(armature, mesh, fbx_path)
    overall_validation = {
        "passed": validation["passed"] and artifact_validation["passed"],
        "simulation": validation,
        "artifact": artifact_validation,
    }
    manifest = {
        "schemaVersion": 1,
        "speciesId": brief["speciesId"],
        "speciesVersion": brief["speciesVersion"],
        "stableIdentity": f"{brief['speciesId']}@{brief['speciesVersion']}:physical-v{PROFILE_VERSION}",
        "generatorVersion": PHYSICAL_GENERATOR_VERSION,
        "blenderVersion": bpy.app.version_string,
        "authority": {
            "brief": {"file": brief_path.name, "sha256": sha256(brief_path), "authoritative": True},
            "physicalProfileGenerator": {
                "file": "physical_profile.py",
                "sha256": sha256(LAB_ROOT / "physical_profile.py"),
                "authoritative": True,
            },
            "blenderAdapter": {
                "file": Path(__file__).name,
                "sha256": sha256(Path(__file__).resolve()),
                "authoritative": True,
            },
        },
        "skeleton": {"bones": hierarchy_records(armature)},
        "physics": {
            "solver": "Blender rigid body world",
            "partCount": len(profile["parts"]),
            "jointCount": len(profile["joints"]),
            "totalMassKg": profile["totalMassKg"],
            "profileFile": profile_path.name,
            "anchoredParts": [part["bone"] for part in profile["parts"] if part["anchored"]],
        },
        "motion": {
            "name": action_name,
            "source": "blender-rigid-body-simulation",
            "event": brief["motion"]["event"],
            "startFrame": scene.frame_start,
            "endFrame": scene.frame_end,
            "fps": scene.render.fps,
            "preimpactChestDisplacementMeters": round(
                metrics["preimpactChestDisplacementMeters"], 6
            ),
            "peakChestDisplacementMeters": round(metrics["peakChestDisplacementMeters"], 6),
            "finalChestDisplacementMeters": round(metrics["finalChestDisplacementMeters"], 6),
            "finalFrameChestMotionMeters": round(metrics["finalFrameChestMotionMeters"], 6),
            "bakedKeyframeCount": baked_count,
        },
        "previews": previews,
        "blenderArtifact": {
            "file": blend_path.name,
            "derived": True,
            "reproducible": True,
            "validation": artifact_validation,
        },
        "fbxTransport": {
            "file": fbx_path.name,
            "derived": True,
            "forwardAxis": "-Y",
            "upAxis": "Z",
            "deterministicBytes": False,
            "validation": "import and measure in an engine before promotion",
        },
        "validation": overall_validation,
    }
    (output / "manifest.json").write_text(
        json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    print(f"PHYSICAL_HUMANOID_OK manifest={output / 'manifest.json'}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--brief", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else [])
    try:
        build(args.brief.resolve(), args.output.resolve())
    except Exception as error:
        print(f"PHYSICAL_HUMANOID_ERROR: {error}", file=sys.stderr)
        raise SystemExit(2) from error


if __name__ == "__main__":
    main()
