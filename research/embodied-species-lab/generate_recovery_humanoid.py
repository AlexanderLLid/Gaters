"""Generate an active humanoid, corrective step, and baked recovery animation."""

import argparse
import json
import math
import shutil
import sys
from pathlib import Path

import bpy
from mathutils import Matrix, Vector


LAB_ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(LAB_ROOT))

from generate_humanoid import hierarchy_records, humanoid_bones, load_brief, make_armature, make_weighted_mesh
from generate_physical_humanoid import (
    PHYSICAL_PARTS,
    bake_skeleton,
    bone_lookup,
    export_fbx,
    make_joint,
    make_physics_proxy,
    make_preview_stage,
    make_projectile,
    render_previews,
    reopen_and_validate,
    sha256,
    simulate,
    validate_scene,
)
from physical_profile import PROFILE_VERSION, generate_physical_profile
from recovery import RECOVERY_VERSION, physical_structure_passed, plan_recovery


GENERATOR_VERSION = 1


def load_case(path, case_id):
    data = json.loads(path.read_text(encoding="utf-8"))
    if data.get("schemaVersion") != 1 or not isinstance(data.get("cases"), list):
        raise ValueError("unsupported recovery case file")
    matches = [case for case in data["cases"] if case.get("caseId") == case_id]
    if len(matches) != 1:
        raise ValueError(f"expected exactly one recovery case named {case_id}")
    case = matches[0]
    if case.get("expectedStepFoot") not in {"foot_l", "foot_r"}:
        raise ValueError("recovery case must declare expectedStepFoot")
    return case


def recreate_output(output, case_id):
    expected = (LAB_ROOT / "Derived" / "humanoid-recovery-v1" / case_id).resolve()
    if output.resolve() != expected or output.is_symlink():
        raise ValueError(f"output must be the non-symlink directory {expected}")
    if output.exists():
        shutil.rmtree(output)
    output.mkdir(parents=True)


def make_ground():
    floor = bpy.data.objects.get("PreviewFloor")
    if not floor:
        raise RuntimeError("preview stage did not create a floor")
    bpy.context.view_layer.objects.active = floor
    floor.select_set(True)
    bpy.ops.rigidbody.object_add(type="PASSIVE")
    floor.rigid_body.collision_shape = "MESH"
    floor.rigid_body.friction = 0.9
    floor.rigid_body.restitution = 0.0
    floor["lab_role"] = "recovery-ground"
    floor.select_set(False)
    return floor


def keyframe_target(target, waypoints, frame_end):
    for waypoint in waypoints:
        target.location = Vector(waypoint["positionMeters"])
        target.keyframe_insert(data_path="location", frame=waypoint["frame"])
    target.location = Vector(waypoints[-1]["positionMeters"])
    target.keyframe_insert(data_path="location", frame=frame_end)
    if target.animation_data and target.animation_data.action:
        from generate_physical_humanoid import action_fcurves

        for fcurve in action_fcurves(target.animation_data.action):
            for keyframe in fcurve.keyframe_points:
                keyframe.interpolation = "LINEAR"


def make_foot_target(foot_name, foot, waypoints, frame_end):
    start = Vector(waypoints[0]["positionMeters"])
    bpy.ops.mesh.primitive_cube_add(size=0.025, location=start)
    target = bpy.context.object
    target.name = f"RecoveryTarget_{foot_name}"
    target["lab_role"] = "recovery-foot-target"
    target["foot"] = foot_name
    bpy.ops.rigidbody.object_add(type="PASSIVE")
    target.rigid_body.collision_shape = "BOX"
    target.rigid_body.kinematic = True
    target.rigid_body.collision_collections = [False] * 19 + [True]
    keyframe_target(target, waypoints, frame_end)

    constraint = bpy.data.objects.new(f"RecoveryContact_{foot_name}", None)
    constraint["lab_role"] = "recovery-foot-contact"
    constraint["foot"] = foot_name
    bpy.context.collection.objects.link(constraint)
    constraint.matrix_world = Matrix.Translation(start)
    bpy.context.view_layer.objects.active = constraint
    constraint.select_set(True)
    bpy.ops.rigidbody.constraint_add(type="GENERIC_SPRING")
    settings = constraint.rigid_body_constraint
    settings.spring_type = "SPRING2"
    settings.object1 = target
    settings.object2 = foot
    settings.disable_collisions = True
    settings.use_override_solver_iterations = True
    settings.solver_iterations = 100
    for axis in "xyz":
        setattr(settings, f"use_limit_lin_{axis}", True)
        setattr(settings, f"limit_lin_{axis}_lower", -0.04)
        setattr(settings, f"limit_lin_{axis}_upper", 0.04)
        setattr(settings, f"use_spring_{axis}", True)
        setattr(settings, f"spring_stiffness_{axis}", 2600.0)
        setattr(settings, f"spring_damping_{axis}", 90.0)
        setattr(settings, f"use_limit_ang_{axis}", True)
        setattr(settings, f"limit_ang_{axis}_lower", math.radians(-12.0))
        setattr(settings, f"limit_ang_{axis}_upper", math.radians(12.0))
        setattr(settings, f"use_spring_ang_{axis}", True)
        setattr(settings, f"spring_stiffness_ang_{axis}", 900.0)
        setattr(settings, f"spring_damping_ang_{axis}", 35.0)
    constraint.hide_render = True
    target.hide_render = True
    target.hide_set(True)
    constraint.hide_set(True)
    constraint.select_set(False)
    return target, constraint


def contact_waypoints(foot_name, start, plan, frame_start, frame_end):
    start = [float(value) for value in start]
    if plan["stepFoot"] == foot_name:
        return [
            {"frame": frame_start, "positionMeters": start},
            *plan["waypoints"],
            {"frame": frame_end, "positionMeters": plan["targetMeters"]},
        ]
    return [
        {"frame": frame_start, "positionMeters": start},
        {"frame": frame_end, "positionMeters": start},
    ]


def make_balance_target(pelvis, plan, frame_start, frame_end):
    start = pelvis.matrix_world.translation.copy()
    final = Vector((plan["supportCenterMeters"][0], plan["supportCenterMeters"][1], start.z))
    bpy.ops.mesh.primitive_cube_add(size=0.03, location=start)
    target = bpy.context.object
    target.name = "RecoveryBalanceTarget"
    target["lab_role"] = "recovery-balance-target"
    bpy.ops.rigidbody.object_add(type="PASSIVE")
    target.rigid_body.kinematic = True
    target.rigid_body.collision_shape = "BOX"
    target.rigid_body.collision_collections = [False] * 19 + [True]
    for frame, position in (
        (frame_start, start),
        (plan["frames"]["release"], start),
        (plan["frames"]["plant"], final),
        (frame_end, final),
    ):
        target.location = position
        target.keyframe_insert(data_path="location", frame=frame)
    if target.animation_data and target.animation_data.action:
        from generate_physical_humanoid import action_fcurves

        for fcurve in action_fcurves(target.animation_data.action):
            for keyframe in fcurve.keyframe_points:
                keyframe.interpolation = "LINEAR"

    constraint = bpy.data.objects.new("RecoveryBalanceController", None)
    constraint["lab_role"] = "recovery-balance-controller"
    bpy.context.collection.objects.link(constraint)
    constraint.matrix_world = Matrix.Translation(start)
    bpy.context.view_layer.objects.active = constraint
    constraint.select_set(True)
    bpy.ops.rigidbody.constraint_add(type="GENERIC_SPRING")
    settings = constraint.rigid_body_constraint
    settings.spring_type = "SPRING2"
    settings.object1 = target
    settings.object2 = pelvis
    settings.disable_collisions = True
    settings.use_override_solver_iterations = True
    settings.solver_iterations = 100
    for axis, stiffness, damping in (
        ("x", 500.0, 55.0),
        ("y", 500.0, 55.0),
        ("z", 1200.0, 85.0),
    ):
        setattr(settings, f"use_limit_lin_{axis}", True)
        setattr(settings, f"limit_lin_{axis}_lower", -0.35)
        setattr(settings, f"limit_lin_{axis}_upper", 0.35)
        setattr(settings, f"use_spring_{axis}", True)
        setattr(settings, f"spring_stiffness_{axis}", stiffness)
        setattr(settings, f"spring_damping_{axis}", damping)
        setattr(settings, f"use_limit_ang_{axis}", True)
        setattr(settings, f"limit_ang_{axis}_lower", math.radians(-25.0))
        setattr(settings, f"limit_ang_{axis}_upper", math.radians(25.0))
        setattr(settings, f"use_spring_ang_{axis}", True)
        setattr(settings, f"spring_stiffness_ang_{axis}", 1500.0)
        setattr(settings, f"spring_damping_ang_{axis}", 55.0)
    target.hide_render = True
    constraint.hide_render = True
    target.hide_set(True)
    constraint.hide_set(True)
    constraint.select_set(False)
    return target, constraint


def lowest_body_point(proxies, samples):
    lowest = float("inf")
    for frame_samples in samples.values():
        for name, proxy in proxies.items():
            matrix = frame_samples[name]
            lowest = min(
                lowest,
                min((matrix @ Vector(corner)).z for corner in proxy.bound_box),
            )
    return lowest


def recovery_metrics(profile, proxies, samples, plan, event, contacts, balance_target):
    start_frame = min(samples)
    end_frame = max(samples)
    step_foot = plan["stepFoot"]
    start = samples[start_frame][step_foot].translation
    final = samples[end_frame][step_foot].translation
    displacement = final - start
    horizontal_travel = math.hypot(displacement.x, displacement.y)
    direction = Vector((event["direction"][0], event["direction"][1]))
    direction.normalize()
    direction_dot = displacement.x * direction.x + displacement.y * direction.y
    target = Vector(plan["targetMeters"])
    final_target_error = (final - target).length
    pelvis_target_error = (
        samples[end_frame]["pelvis"].translation - balance_target.matrix_world.translation
    ).length
    final_motion = max(
        (samples[end_frame][name].translation - samples[end_frame - 1][name].translation).length
        for name in proxies
    )
    total_mass = sum(part["massKg"] for part in profile["parts"])
    masses = {part["bone"]: part["massKg"] for part in profile["parts"]}
    final_com = sum(
        (samples[end_frame][name].translation * masses[name] for name in proxies),
        Vector((0.0, 0.0, 0.0)),
    ) / total_mass
    feet = [samples[end_frame][name].translation for name in ("foot_l", "foot_r")]
    support_margin = 0.28
    support_contains_com = (
        min(foot.x for foot in feet) - support_margin
        <= final_com.x
        <= max(foot.x for foot in feet) + support_margin
        and min(foot.y for foot in feet) - support_margin
        <= final_com.y
        <= max(foot.y for foot in feet) + support_margin
    )
    target_positions = {
        target["foot"]: [float(value) for value in target.matrix_world.translation]
        for target, _constraint in contacts
    }
    contact_states = {
        constraint["foot"]: bool(constraint.rigid_body_constraint.enabled)
        for _target, constraint in contacts
    }
    return {
        "passiveHumanoidPartCount": sum(
            1 for proxy in proxies.values() if proxy.rigid_body.type == "PASSIVE"
        ),
        "actualStepTravelMeters": horizontal_travel,
        "stepDirectionDotMeters": direction_dot,
        "finalTargetErrorMeters": final_target_error,
        "finalPelvisTargetErrorMeters": pelvis_target_error,
        "lowestBodyPointMeters": lowest_body_point(proxies, samples),
        "finalBodyMotionMetersPerFrame": final_motion,
        "finalCenterOfMassMeters": [final_com.x, final_com.y, final_com.z],
        "supportContainsCenterOfMass": support_contains_com,
        "targetFinalPositionsMeters": target_positions,
        "contactConstraintsEnabled": contact_states,
    }


def render_step_preview(output, frame):
    scene = bpy.context.scene
    scene.frame_set(frame)
    path = output / "preview-step.png"
    scene.render.filepath = str(path)
    result = bpy.ops.render.render(write_still=True)
    if result != {"FINISHED"} or not path.exists() or path.stat().st_size == 0:
        raise RuntimeError("could not render recovery step preview")
    scene.frame_set(scene.frame_start)
    return {"label": "step", "frame": frame, "file": path.name}


def rounded_metrics(metrics):
    return {
        name: [round(float(value), 6) for value in value]
        if isinstance(value, list)
        else round(float(value), 6)
        if isinstance(value, (int, float)) and not isinstance(value, bool)
        else value
        for name, value in metrics.items()
    }


def build(brief_path, cases_path, case_id, output):
    brief = load_brief(brief_path)
    case = load_case(cases_path, case_id)
    recreate_output(output, case_id)
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.unit_settings.system = "METRIC"
    scene.unit_settings.scale_length = 1.0
    scene.render.fps = int(brief["motion"]["fps"])
    scene.render.fps_base = 1.0
    scene.frame_start = 1
    scene.frame_end = 91
    bpy.ops.rigidbody.world_add()
    scene.rigidbody_world.substeps_per_frame = 12
    scene.rigidbody_world.solver_iterations = 60
    scene.rigidbody_world.point_cache.frame_start = scene.frame_start
    scene.rigidbody_world.point_cache.frame_end = scene.frame_end

    bones = humanoid_bones(brief["body"])
    bones_by_name = bone_lookup(bones)
    profile = generate_physical_profile(brief["body"], anchored_feet=False)
    profile_path = output / "physical-profile.json"
    profile_path.write_text(json.dumps(profile, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    armature = make_armature(bones)
    mesh = make_weighted_mesh(armature, brief["body"])
    make_preview_stage()
    make_ground()
    parts = {part["bone"]: part for part in profile["parts"]}
    proxies = {
        name: make_physics_proxy(bones_by_name[name], parts[name], brief["body"])
        for name in PHYSICAL_PARTS
    }
    constraints = [make_joint(joint, proxies, bones_by_name) for joint in profile["joints"]]
    stance = {
        foot: [float(value) for value in proxies[foot].matrix_world.translation]
        for foot in ("foot_l", "foot_r")
    }
    plan = plan_recovery(case["event"], stance, scene.render.fps)
    if plan["stepFoot"] != case["expectedStepFoot"]:
        raise ValueError(
            f"planner selected {plan['stepFoot']}, expected {case['expectedStepFoot']}"
        )
    plan_path = output / "recovery-plan.json"
    plan_path.write_text(json.dumps(plan, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    contacts = []
    for foot in ("foot_l", "foot_r"):
        waypoints = contact_waypoints(
            foot, stance[foot], plan, scene.frame_start, scene.frame_end
        )
        contacts.append(
            make_foot_target(foot, proxies[foot], waypoints, scene.frame_end)
        )
    balance_target, balance_constraint = make_balance_target(
        proxies["pelvis"], plan, scene.frame_start, scene.frame_end
    )
    make_projectile(case["event"], bones_by_name["chest"], 1, 6, 8)
    scene.timeline_markers.new("event.impact", frame=6)
    scene.timeline_markers.new("recovery.step", frame=plan["frames"]["apex"])
    scene.timeline_markers.new("recovery.plant", frame=plan["frames"]["plant"])

    rest, samples, simulation_metrics = simulate(
        scene, proxies, scene.frame_start, scene.frame_end, impact_frame=6
    )
    action_name = f"A_GeneratedRecovery_{case_id.replace('-', '_')}"
    action, baked_count = bake_skeleton(armature, rest, samples, action_name)
    base_validation = validate_scene(
        armature, mesh, profile, action, simulation_metrics, baked_count
    )
    metrics = recovery_metrics(
        profile, proxies, samples, plan, case["event"], contacts, balance_target
    )
    checks = {
        "baseSimulation": physical_structure_passed(base_validation["checks"]),
        "allHumanoidPartsActive": metrics["passiveHumanoidPartCount"] == 0,
        "stepDirectionMatches": metrics["stepDirectionDotMeters"] >= 0.10,
        "stepTravelBounded": 0.12 <= metrics["actualStepTravelMeters"] <= 0.55,
        "stepReachedTarget": metrics["finalTargetErrorMeters"] <= 0.18,
        "pelvisReachedBalanceTarget": metrics["finalPelvisTargetErrorMeters"] <= 0.35,
        "floorPenetrationBounded": metrics["lowestBodyPointMeters"] >= -0.06,
        "bodySettled": metrics["finalBodyMotionMetersPerFrame"] <= 0.02,
        "centerOfMassSupported": metrics["supportContainsCenterOfMass"],
    }
    simulation_validation = {"passed": all(checks.values()), "checks": checks}
    if not simulation_validation["passed"]:
        failed = [name for name, passed in checks.items() if not passed]
        raise ValueError(
            f"recovery simulation failed: {failed}; metrics={rounded_metrics(metrics)}; "
            f"base={rounded_metrics(simulation_metrics)}"
        )

    for proxy in proxies.values():
        proxy.hide_set(True)
    for constraint in constraints:
        constraint.hide_set(True)
    scene.frame_set(scene.frame_start)
    blend_path = output / "humanoid-recovery.blend"
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path), check_existing=False)
    blend_path.with_suffix(blend_path.suffix + "1").unlink(missing_ok=True)

    armature, mesh, artifact_validation = reopen_and_validate(
        blend_path, bones, profile, action_name, maximum_final_displacement=0.35
    )
    scene = bpy.context.scene
    previews = render_previews(output, simulation_metrics["peakFrame"])
    previews.insert(2, render_step_preview(output, plan["frames"]["apex"]))
    fbx_path = output / "humanoid-recovery.fbx"
    export_fbx(armature, mesh, fbx_path)
    overall_validation = {
        "passed": simulation_validation["passed"] and artifact_validation["passed"],
        "simulation": simulation_validation,
        "artifact": artifact_validation,
    }
    manifest = {
        "schemaVersion": 1,
        "caseId": case_id,
        "speciesId": brief["speciesId"],
        "speciesVersion": brief["speciesVersion"],
        "generatorVersion": GENERATOR_VERSION,
        "blenderVersion": bpy.app.version_string,
        "authority": {
            "brief": {"file": brief_path.name, "sha256": sha256(brief_path)},
            "cases": {"file": cases_path.name, "sha256": sha256(cases_path)},
            "planner": {"file": "recovery.py", "sha256": sha256(LAB_ROOT / "recovery.py")},
            "adapter": {"file": Path(__file__).name, "sha256": sha256(Path(__file__).resolve())},
        },
        "skeleton": {"bones": hierarchy_records(armature)},
        "physics": {
            "profileVersion": PROFILE_VERSION,
            "totalMassKg": profile["totalMassKg"],
            "partCount": len(profile["parts"]),
            "jointCount": len(profile["joints"]),
            "passiveHumanoidPartCount": metrics["passiveHumanoidPartCount"],
            "profileFile": profile_path.name,
        },
        "recovery": {
            "recoveryVersion": RECOVERY_VERSION,
            "expectedStepFoot": case["expectedStepFoot"],
            "planFile": plan_path.name,
            "plan": plan,
            "metrics": rounded_metrics(metrics),
        },
        "motion": {
            "name": action_name,
            "source": "blender-active-recovery-simulation",
            "event": case["event"],
            "startFrame": scene.frame_start,
            "endFrame": scene.frame_end,
            "fps": scene.render.fps,
            "bakedKeyframeCount": baked_count,
            "simulation": rounded_metrics(simulation_metrics),
        },
        "previews": previews,
        "blenderArtifact": {
            "file": blend_path.name,
            "derived": True,
            "validation": artifact_validation,
        },
        "fbxTransport": {"file": fbx_path.name, "derived": True, "deterministicBytes": False},
        "validation": overall_validation,
    }
    (output / "manifest.json").write_text(
        json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    print(f"RECOVERY_HUMANOID_OK case={case_id} manifest={output / 'manifest.json'}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--brief", type=Path, required=True)
    parser.add_argument("--cases", type=Path, required=True)
    parser.add_argument("--case-id", required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else [])
    try:
        build(
            args.brief.resolve(),
            args.cases.resolve(),
            args.case_id,
            args.output.resolve(),
        )
    except Exception as error:
        print(f"RECOVERY_HUMANOID_ERROR: {error}", file=sys.stderr)
        raise SystemExit(2) from error


if __name__ == "__main__":
    main()
