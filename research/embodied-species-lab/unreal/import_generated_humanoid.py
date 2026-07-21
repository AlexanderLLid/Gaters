import hashlib
import json
import os
from pathlib import Path

import unreal


IMPORTER_VERSION = 3
ASSET_NAME = "SK_GeneratedHumanoid"
IK_RIG_NAME = "IK_GeneratedHumanoid"
FOOT_PLACEMENT_ROLES = {
    "ikRoot": "root",
    "pelvis": "pelvis",
    "left": {"fkFoot": "foot_l", "ball": "ball_l", "ikFoot": "ik_foot_l"},
    "right": {"fkFoot": "foot_r", "ball": "ball_r", "ikFoot": "ik_foot_r"},
}


class ExpectedProfileRejection(RuntimeError):
    pass


def required(name):
    value = os.environ.get(name, "").strip()
    if not value:
        raise RuntimeError(f"Missing environment variable {name}")
    return value


def load_inputs(manifest_path, profile_path, locomotion_manifest_path):
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    profile = json.loads(profile_path.read_text(encoding="utf-8"))
    locomotion_manifest = json.loads(locomotion_manifest_path.read_text(encoding="utf-8"))
    if manifest.get("schemaVersion") != 1 or manifest.get("validation", {}).get("passed") is not True:
        raise RuntimeError("Generated humanoid manifest schema or validation is invalid")
    if profile.get("schemaVersion") != 1 or profile.get("profileVersion") != 1:
        raise RuntimeError("Generated humanoid physical profile version is invalid")
    if (
        locomotion_manifest.get("schemaVersion") != 1
        or locomotion_manifest.get("locomotionVersion") != 1
        or locomotion_manifest.get("validation", {}).get("passed") is not True
    ):
        raise RuntimeError("Generated locomotion manifest schema or validation is invalid")
    fbx_name = manifest.get("fbxTransport", {}).get("file", "")
    if not fbx_name or Path(fbx_name).name != fbx_name:
        raise RuntimeError("Generated humanoid FBX must be a simple relative filename")
    fbx_path = manifest_path.parent / fbx_name
    if not fbx_path.is_file() or fbx_path.stat().st_size == 0:
        raise RuntimeError(f"Generated humanoid FBX is missing or empty: {fbx_path}")
    clips = []
    seen_names = set()
    for clip in locomotion_manifest.get("clips", []):
        name = clip.get("name", "")
        fbx_name = clip.get("fbxFile", "")
        if not name or name in seen_names:
            raise RuntimeError(f"Locomotion clip name is missing or duplicated: {name}")
        if not fbx_name or Path(fbx_name).name != fbx_name:
            raise RuntimeError(f"Locomotion FBX must be a simple relative filename: {fbx_name}")
        clip_fbx = locomotion_manifest_path.parent / fbx_name
        if not clip_fbx.is_file() or clip_fbx.stat().st_size == 0:
            raise RuntimeError(f"Generated locomotion FBX is missing or empty: {clip_fbx}")
        seen_names.add(name)
        clips.append((clip, clip_fbx))
    if not clips:
        raise RuntimeError("Generated locomotion manifest contains no clips")
    return manifest, profile, locomotion_manifest, fbx_path, clips


def vector_record(vector):
    return [round(float(vector.x), 4), round(float(vector.y), 4), round(float(vector.z), 4)]


def import_skeletal_mesh(fbx_path, destination):
    options = unreal.FbxImportUI()
    options.set_editor_property("automated_import_should_detect_type", False)
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_animations", False)
    options.set_editor_property("import_as_skeletal", True)
    options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)
    options.set_editor_property("original_import_type", unreal.FBXImportType.FBXIT_SKELETAL_MESH)
    options.set_editor_property("create_physics_asset", False)
    options.set_editor_property("skeleton", None)
    options.set_editor_property("import_materials", False)
    options.set_editor_property("import_textures", False)
    options.get_editor_property("skeletal_mesh_import_data").set_editor_property("convert_scene_unit", True)

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(fbx_path))
    task.set_editor_property("destination_path", destination)
    task.set_editor_property("destination_name", ASSET_NAME)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("replace_existing_settings", True)
    task.set_editor_property("save", False)
    task.set_editor_property("options", options)
    task.set_editor_property("factory", unreal.FbxFactory())
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    objects = [
        unreal.load_asset(str(path))
        for path in unreal.EditorAssetLibrary.list_assets(destination, recursive=True, include_folder=False)
    ]
    objects = [obj for obj in objects if obj is not None]
    by_class = {}
    for obj in objects:
        by_class.setdefault(obj.get_class().get_name(), []).append(obj)
    for required_class in ("SkeletalMesh", "Skeleton"):
        if len(by_class.get(required_class, [])) != 1:
            found = {name: len(items) for name, items in by_class.items()}
            raise RuntimeError(f"Expected one {required_class} after humanoid import; found={found}")
    return by_class["SkeletalMesh"][0], by_class["Skeleton"][0], task


def read_hierarchy(skeletal_mesh, bones):
    hierarchy = []
    names = []
    for spec in bones:
        name = spec["name"]
        parent_name = str(skeletal_mesh.get_bone_parent(name))
        parent = None if parent_name in ("", "None") else parent_name
        children = sorted(str(child) for child in skeletal_mesh.get_bone_children(name))
        hierarchy.append({"name": name, "parent": parent, "children": children})
        names.append(name)
    return names, hierarchy


def import_animations(clips, skeleton, destination):
    animations = []
    imported_paths = []
    for clip, fbx_path in clips:
        name = clip["name"]
        animation_object_path = f"{destination}/{name}.{name}"
        if unreal.EditorAssetLibrary.does_asset_exist(animation_object_path):
            if not unreal.EditorAssetLibrary.delete_asset(animation_object_path):
                raise RuntimeError(f"Could not replace derived animation: {animation_object_path}")

        options = unreal.FbxImportUI()
        options.set_editor_property("automated_import_should_detect_type", False)
        options.set_editor_property("import_mesh", False)
        options.set_editor_property("import_animations", True)
        options.set_editor_property("import_as_skeletal", False)
        options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_ANIMATION)
        options.set_editor_property("original_import_type", unreal.FBXImportType.FBXIT_ANIMATION)
        options.set_editor_property("skeleton", skeleton)
        animation_data = options.get_editor_property("anim_sequence_import_data")
        animation_data.set_editor_property("convert_scene_unit", True)
        animation_data.set_editor_property("import_uniform_scale", 100.0)

        task = unreal.AssetImportTask()
        task.set_editor_property("filename", str(fbx_path))
        task.set_editor_property("destination_path", destination)
        task.set_editor_property("destination_name", name)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", False)
        task.set_editor_property("replace_existing_settings", False)
        task.set_editor_property("save", False)
        task.set_editor_property("options", options)
        task.set_editor_property("factory", unreal.FbxFactory())
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

        task_paths = [
            str(path) for path in task.get_editor_property("imported_object_paths")
        ]
        imported_paths.extend(task_paths)
        imported_animations = [
            unreal.load_asset(path)
            for path in task_paths
            if unreal.load_asset(path) is not None
            and unreal.load_asset(path).get_class().get_name() == "AnimSequence"
        ]
        if len(imported_animations) != 1:
            raise RuntimeError(
                f"Expected one AnimSequence for {name}; imported paths={task_paths}"
            )
        animation = imported_animations[0]
        if animation.get_path_name() != animation_object_path:
            raise RuntimeError(
                f"Unexpected animation path for {name}: {animation.get_path_name()}"
            )
        sampled_keys = int(animation.get_editor_property("number_of_sampled_keys"))
        if sampled_keys < 1:
            raise RuntimeError(f"Imported animation has no sampled keys: {name}")
        frames = (0, sampled_keys // 2, sampled_keys - 1)
        root_translations = [
            vector_record(
                unreal.AnimationLibrary.get_bone_pose_for_frame(
                    animation,
                    "root",
                    frame,
                    False,
                ).translation
            )
            for frame in frames
        ]
        animations.append(
            {
                "name": name,
                "asset": animation.get_path_name(),
                "durationSeconds": round(
                    float(animation.get_editor_property("sequence_length")),
                    6,
                ),
                "sampledKeys": sampled_keys,
                "rootTranslationCentimeters": root_translations,
            }
        )
        if not unreal.EditorAssetLibrary.save_asset(
            animation.get_path_name(), only_if_is_dirty=False
        ):
            raise RuntimeError(f"Could not save imported animation: {animation.get_path_name()}")
    return animations, imported_paths


def create_physics_asset(skeletal_mesh, profile, expected_adapter_error=""):
    subsystem = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
    physics_asset = subsystem.create_physics_asset(skeletal_mesh, True, 0)
    if physics_asset is None:
        raise RuntimeError("Unreal native Physics Asset creation returned None")

    adapter = unreal.CharacterPhysicsProfileAdapterLibrary
    body_bones = [unreal.Name(part["bone"]) for part in profile["parts"]]
    parent_bones = [unreal.Name(joint["parent"]) for joint in profile["joints"]]
    child_bones = [unreal.Name(joint["child"]) for joint in profile["joints"]]
    adapter_error = adapter.rebuild_physics_asset_topology(
        skeletal_mesh,
        physics_asset,
        body_bones,
        parent_bones,
        child_bones,
    )
    if adapter_error:
        if adapter_error == expected_adapter_error:
            raise ExpectedProfileRejection(adapter_error)
        raise RuntimeError(f"Physics profile adapter failed: {adapter_error}")
    if expected_adapter_error:
        raise RuntimeError(
            f"Physics profile adapter accepted profile expected to fail: {expected_adapter_error}"
        )
    adapter_version = int(adapter.get_adapter_version())

    constraints = []
    body_bones = set()
    for accessor in physics_asset.get_constraints(False):
        _, parent, child = unreal.ConstraintInstanceBlueprintLibrary.get_attached_body_names(accessor)
        parent = str(parent)
        child = str(child)
        constraints.append({
            "parent": parent,
            "child": child,
        })
        body_bones.update((parent, child))
    constraints.sort(key=lambda item: (item["parent"], item["child"]))
    return physics_asset, {
        "compatible": bool(subsystem.is_physics_asset_compatible(skeletal_mesh, physics_asset)),
        "derivation": "profile-topology-v1",
        "bodyBones": sorted(body_bones),
        "bodyEvidence": "native-constraint-endpoints",
        "constraints": constraints,
    }, adapter_version


def create_ik_rig(skeletal_mesh, destination):
    ik_rig = unreal.IKRigDefinitionFactory.create_new_ik_rig_asset(destination, IK_RIG_NAME)
    if ik_rig is None:
        raise RuntimeError("Unreal IK Rig factory returned None")
    controller = unreal.IKRigController.get_controller(ik_rig)
    if controller is None or not controller.set_skeletal_mesh(skeletal_mesh):
        raise RuntimeError("Unreal IK Rig rejected the imported Skeletal Mesh")
    if not controller.set_retarget_root("pelvis"):
        raise RuntimeError("Could not set humanoid IK retarget root")

    for goal_name, bone_name in (("foot_l_goal", "foot_l"), ("foot_r_goal", "foot_r")):
        if str(controller.add_new_goal(goal_name, bone_name)) != goal_name:
            raise RuntimeError(f"Could not create IK goal {goal_name}")

    solver_index = int(controller.add_solver("/Script/IKRig.IKRigFullBodyIKSolver"))
    if solver_index != 0:
        raise RuntimeError(f"Expected first FullBodyIK solver at index 0, got {solver_index}")
    if not controller.set_start_bone("pelvis", solver_index):
        raise RuntimeError("Could not set FullBodyIK pelvis root")
    for goal_name in ("foot_l_goal", "foot_r_goal"):
        if not controller.connect_goal_to_solver(goal_name, solver_index):
            raise RuntimeError(f"Could not connect {goal_name} to FullBodyIK")
    solver_controller = controller.get_solver_controller(solver_index)
    if solver_controller is None:
        raise RuntimeError("FullBodyIK solver controller is unavailable")
    for chain_name, start, end, goal in (
        ("leg_l", "thigh_l", "foot_l", "foot_l_goal"),
        ("leg_r", "thigh_r", "foot_r", "foot_r_goal"),
    ):
        if str(controller.add_retarget_chain(chain_name, start, end, goal)) != chain_name:
            raise RuntimeError(f"Could not create IK retarget chain {chain_name}")

    goals = {
        str(goal.get_editor_property("goal_name")): str(
            controller.get_bone_for_goal(goal.get_editor_property("goal_name"))
        )
        for goal in controller.get_all_goals()
    }
    chains = {
        name: {
            "start": str(controller.get_retarget_chain_start_bone(name)),
            "end": str(controller.get_retarget_chain_end_bone(name)),
            "goal": str(controller.get_retarget_chain_goal(name)),
        }
        for name in ("leg_l", "leg_r")
    }
    solvers = [
        {
            "enabled": bool(controller.get_solver_enabled(index)),
            "controllerClass": controller.get_solver_controller(index)
            .get_class()
            .get_name(),
            "startBone": str(controller.get_start_bone(index)),
            "connectedGoals": [
                goal_name
                for goal_name in ("foot_l_goal", "foot_r_goal")
                if controller.is_goal_connected_to_solver(goal_name, index)
            ],
        }
        for index in range(int(controller.get_num_solvers()))
    ]
    return ik_rig, {
        "compatible": bool(controller.is_skeletal_mesh_compatible(skeletal_mesh)),
        "retargetRoot": str(controller.get_retarget_root()),
        "goals": goals,
        "chains": chains,
        "solvers": solvers,
    }


def create_test_map(skeletal_mesh, physics_asset, destination):
    map_path = f"{destination}/L_CharacterLab"
    if unreal.EditorAssetLibrary.does_asset_exist(map_path):
        if not unreal.EditorAssetLibrary.delete_asset(map_path):
            raise RuntimeError(f"Could not replace CharacterLab map: {map_path}")
    if not unreal.EditorLevelLibrary.new_level(map_path):
        raise RuntimeError(f"Could not create CharacterLab map: {map_path}")

    floor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(0.0, 0.0, -10.0)
    )
    floor_mesh = unreal.load_asset("/Engine/BasicShapes/Cube.Cube")
    if floor is None or floor_mesh is None:
        raise RuntimeError("Could not create CharacterLab floor")
    floor.static_mesh_component.set_static_mesh(floor_mesh)
    floor.set_actor_scale3d(unreal.Vector(10.0, 10.0, 0.1))

    preview = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkeletalMeshActor, unreal.Vector(0.0, 0.0, 100.0)
    )
    if preview is None:
        raise RuntimeError("Could not create CharacterLab humanoid preview")
    preview.skeletal_mesh_component.set_skeletal_mesh_asset(skeletal_mesh)
    preview.skeletal_mesh_component.set_physics_asset(physics_asset, True)
    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Could not save CharacterLab map")
    return map_path


def run():
    manifest_path = Path(required("GATERS_CHARACTER_MANIFEST")).resolve()
    profile_path = Path(required("GATERS_CHARACTER_PROFILE")).resolve()
    locomotion_manifest_path = Path(required("GATERS_LOCOMOTION_MANIFEST")).resolve()
    destination = required("GATERS_CHARACTER_DESTINATION").rstrip("/")
    report_path = Path(required("GATERS_CHARACTER_REPORT")).resolve()
    if (
        not manifest_path.is_file()
        or not profile_path.is_file()
        or not locomotion_manifest_path.is_file()
    ):
        raise RuntimeError("Generated humanoid, physical, or locomotion input does not exist")
    if not destination.startswith("/Game/Gaters/Generated/CharacterLab"):
        raise RuntimeError("Humanoid intake destination must remain inside CharacterLab")
    manifest, profile, locomotion_manifest, fbx_path, clips = load_inputs(
        manifest_path,
        profile_path,
        locomotion_manifest_path,
    )
    expected_adapter_error = os.environ.get("GATERS_EXPECT_ADAPTER_ERROR", "").strip()

    unreal.EditorAssetLibrary.make_directory(destination)
    skeletal_mesh, skeleton, import_task = import_skeletal_mesh(fbx_path, destination)
    animations, animation_import_paths = import_animations(clips, skeleton, destination)
    bone_names, hierarchy = read_hierarchy(skeletal_mesh, manifest["skeleton"]["bones"])
    try:
        physics_asset, physics_report, adapter_version = create_physics_asset(
            skeletal_mesh, profile, expected_adapter_error
        )
    except ExpectedProfileRejection as error:
        unreal.log(f"GATERS_CHARACTER_IMPORT_REJECTED error={error}")
        return
    ik_rig, ik_report = create_ik_rig(skeletal_mesh, destination)
    bounds_size = skeletal_mesh.get_bounds().box_extent * 2.0

    assets = [skeletal_mesh, skeleton, physics_asset, ik_rig]
    for asset in assets:
        if not unreal.EditorAssetLibrary.save_asset(asset.get_path_name(), only_if_is_dirty=False):
            raise RuntimeError(f"Could not save generated Unreal asset: {asset.get_path_name()}")
    test_map = create_test_map(skeletal_mesh, physics_asset, destination)

    report = {
        "schemaVersion": 3,
        "importerVersion": IMPORTER_VERSION,
        "adapterVersion": adapter_version,
        "engineVersion": unreal.SystemLibrary.get_engine_version(),
        "manifestSha256": hashlib.sha256(manifest_path.read_bytes()).hexdigest(),
        "profileSha256": hashlib.sha256(profile_path.read_bytes()).hexdigest(),
        "locomotionManifestSha256": hashlib.sha256(
            locomotion_manifest_path.read_bytes()
        ).hexdigest(),
        "sourceFbx": fbx_path.name,
        "testMap": test_map,
        "importedObjectPaths": sorted(
            set(
                str(path)
                for path in import_task.get_editor_property("imported_object_paths")
            ).union(animation_import_paths)
        ),
        "assets": {
            asset.get_class().get_name(): asset.get_path_name()
            for asset in assets
        },
        "boneNames": bone_names,
        "referenceHierarchy": hierarchy,
        "footPlacementRoles": {
            key: (
                value if isinstance(value, str) and value in bone_names else
                {role: bone if bone in bone_names else None for role, bone in value.items()}
            )
            for key, value in FOOT_PLACEMENT_ROLES.items()
        },
        "meshBoundsSizeCentimeters": vector_record(bounds_size),
        "physicsAsset": physics_report,
        "ikRig": ik_report,
        "animations": animations,
        "importSettings": {
            "pipeline": "UnrealFBXSkeletal",
            "convertSceneUnit": True,
            "createPhysicsAssetDuringImport": False,
            "nativePhysicsAssetDerivation": True,
            "importAnimations": True,
            "importMaterials": False,
            "importTextures": False,
        },
        "sourceContract": {
            "physicalPartCount": len(profile["parts"]),
            "physicalJointCount": len(profile["joints"]),
            "locomotionClipCount": len(locomotion_manifest["clips"]),
        },
    }
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    unreal.log(
        f"GATERS_CHARACTER_IMPORT_OK mesh={skeletal_mesh.get_path_name()} "
        f"bodies={len(physics_report['bodyBones'])} constraints={len(physics_report['constraints'])}"
    )


run()
