import hashlib
import json
import os
from pathlib import Path

import unreal


IMPORTER_VERSION = 2


def required(name):
    value = os.environ.get(name, "").strip()
    if not value:
        raise RuntimeError(f"Missing environment variable {name}")
    return value


def validate_manifest(path):
    manifest = json.loads(path.read_text(encoding="utf-8"))
    if manifest.get("schemaVersion") != 1 or manifest.get("validation", {}).get("passed") is not True:
        raise RuntimeError("Motion manifest schema or validation is invalid")
    source = manifest.get("sourceIdentity", {})
    if source.get("styleNeutral") is not True or source.get("selectedStyle", "missing") is not None:
        raise RuntimeError("Motion fixture must remain style-neutral")
    if manifest.get("stableIdentity") != "motion.neutral-step@2":
        raise RuntimeError("Unexpected motion fixture identity")
    fbx_name = manifest.get("fbxTransport", {}).get("file", "")
    if not fbx_name or Path(fbx_name).name != fbx_name:
        raise RuntimeError("Motion FBX must be a simple relative filename")
    if manifest["fbxTransport"].get("deterministicBytes") is not False:
        raise RuntimeError("Motion FBX byte nondeterminism is not recorded honestly")
    fbx = path.parent / fbx_name
    if not fbx.is_file() or fbx.stat().st_size == 0:
        raise RuntimeError(f"Motion FBX is missing or empty: {fbx}")
    return manifest, fbx


def vector_record(vector):
    return [round(float(vector.x), 4), round(float(vector.y), 4), round(float(vector.z), 4)]


def import_motion():
    manifest_path = Path(required("GATERS_MOTION_MANIFEST")).resolve()
    destination = required("GATERS_IMPORT_DESTINATION").rstrip("/")
    asset_name = required("GATERS_IMPORT_NAME")
    report_path = Path(required("GATERS_IMPORT_REPORT")).resolve()
    if not manifest_path.is_file():
        raise RuntimeError(f"Motion manifest does not exist: {manifest_path}")
    if not destination.startswith("/Game/Gaters/Generated/") or "/" in asset_name or not asset_name:
        raise RuntimeError("Motion destination or asset name is invalid")
    manifest, fbx = validate_manifest(manifest_path)

    unreal.EditorAssetLibrary.make_directory(destination)

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
    task.set_editor_property("filename", str(fbx))
    task.set_editor_property("destination_path", destination)
    task.set_editor_property("destination_name", asset_name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("replace_existing_settings", True)
    task.set_editor_property("save", True)
    task.set_editor_property("options", options)
    task.set_editor_property("factory", unreal.FbxFactory())
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    imported_paths = [str(path) for path in task.get_editor_property("imported_object_paths")]
    object_paths = sorted(str(path) for path in unreal.EditorAssetLibrary.list_assets(
        destination, recursive=True, include_folder=False))
    objects = [unreal.load_asset(path) for path in object_paths]
    objects = [obj for obj in objects if obj is not None]
    by_class = {}
    for obj in objects:
        by_class.setdefault(obj.get_class().get_name(), []).append(obj)
    for required_class in ("SkeletalMesh", "Skeleton"):
        if len(by_class.get(required_class, [])) != 1:
            found = {name: len(items) for name, items in by_class.items()}
            raise RuntimeError(f"Expected one {required_class} after motion import; found={found}")

    skeletal_mesh = by_class["SkeletalMesh"][0]
    skeleton = by_class["Skeleton"][0]
    animation_object_path = f"{destination}/{manifest['clip']['name']}.{manifest['clip']['name']}"
    if unreal.EditorAssetLibrary.does_asset_exist(animation_object_path):
        if not unreal.EditorAssetLibrary.delete_asset(animation_object_path):
            raise RuntimeError(f"Could not replace derived animation asset: {animation_object_path}")
    animation_options = unreal.FbxImportUI()
    animation_options.set_editor_property("automated_import_should_detect_type", False)
    animation_options.set_editor_property("import_mesh", False)
    animation_options.set_editor_property("import_animations", True)
    animation_options.set_editor_property("import_as_skeletal", False)
    animation_options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_ANIMATION)
    animation_options.set_editor_property("original_import_type", unreal.FBXImportType.FBXIT_ANIMATION)
    animation_options.set_editor_property("skeleton", skeleton)
    animation_options.get_editor_property("anim_sequence_import_data").set_editor_property("convert_scene_unit", True)
    animation_options.get_editor_property("anim_sequence_import_data").set_editor_property("import_uniform_scale", 100.0)

    animation_task = unreal.AssetImportTask()
    animation_task.set_editor_property("filename", str(fbx))
    animation_task.set_editor_property("destination_path", destination)
    animation_task.set_editor_property("destination_name", manifest["clip"]["name"])
    animation_task.set_editor_property("automated", True)
    animation_task.set_editor_property("replace_existing", False)
    animation_task.set_editor_property("replace_existing_settings", False)
    animation_task.set_editor_property("save", True)
    animation_task.set_editor_property("options", animation_options)
    animation_task.set_editor_property("factory", unreal.FbxFactory())
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([animation_task])
    imported_paths.extend(str(path) for path in animation_task.get_editor_property("imported_object_paths"))

    object_paths = sorted(str(path) for path in unreal.EditorAssetLibrary.list_assets(
        destination, recursive=True, include_folder=False))
    objects = [unreal.load_asset(path) for path in object_paths]
    objects = [obj for obj in objects if obj is not None]
    by_class = {}
    for obj in objects:
        by_class.setdefault(obj.get_class().get_name(), []).append(obj)
    for required_class in ("SkeletalMesh", "Skeleton", "AnimSequence"):
        if len(by_class.get(required_class, [])) != 1:
            found = {name: len(items) for name, items in by_class.items()}
            raise RuntimeError(f"Expected one {required_class} after motion import; found={found}")
    imported_paths = sorted(set(imported_paths))
    skeletal_mesh = by_class["SkeletalMesh"][0]
    skeleton = by_class["Skeleton"][0]
    animation = by_class["AnimSequence"][0]
    expected_children = {spec["name"]: [] for spec in manifest["skeleton"]["bones"]}
    for spec in manifest["skeleton"]["bones"]:
        if spec["parent"] is not None:
            expected_children[spec["parent"]].append(spec["name"])
    reference_hierarchy = []
    for spec in manifest["skeleton"]["bones"]:
        parent_name = str(skeletal_mesh.get_bone_parent(spec["name"]))
        actual_parent = None if parent_name in ("", "None") else parent_name
        actual_children = sorted(str(name) for name in skeletal_mesh.get_bone_children(spec["name"]))
        required_children = sorted(expected_children[spec["name"]])
        if actual_parent != spec["parent"] or actual_children != required_children:
            raise RuntimeError(
                f"Imported hierarchy mismatch for {spec['name']}: "
                f"parent={actual_parent} children={actual_children} "
                f"expected_parent={spec['parent']} expected_children={required_children}")
        reference_hierarchy.append({
            "name": spec["name"],
            "parent": actual_parent,
            "children": actual_children,
        })
    animation_tracks = [item["name"] for item in reference_hierarchy]
    sampled_keys = int(animation.get_editor_property("number_of_sampled_keys"))
    duration = float(animation.get_editor_property("sequence_length"))
    root_samples = []
    for source_sample in manifest["clip"]["rootSamples"]:
        frame_index = min(sampled_keys - 1, max(0, int(source_sample["frame"]) - manifest["clip"]["startFrame"]))
        transform = unreal.AnimationLibrary.get_bone_pose_for_frame(
            animation, "root", frame_index, False)
        root_samples.append({
            "frameIndex": frame_index,
            "translationCentimeters": vector_record(transform.translation),
        })
    foot_world_samples = {"foot_l": [], "foot_r": []}
    pose_options = unreal.AnimPoseEvaluationOptions()
    for frame_index in range(sampled_keys):
        pose = unreal.AnimPoseExtensions.get_anim_pose_at_frame(
            animation, frame_index, pose_options)
        for foot_name in foot_world_samples:
            foot_pose = unreal.AnimPoseExtensions.get_bone_pose(
                pose, foot_name, unreal.AnimPoseSpaces.WORLD)
            foot_world_samples[foot_name].append({
                "frameIndex": frame_index,
                "translationCentimeters": vector_record(foot_pose.translation),
            })
    notifies = list(unreal.AnimationLibrary.get_animation_notify_events(animation))
    bounds_size = skeletal_mesh.get_bounds().box_extent * 2.0

    for obj in objects:
        object_path = obj.get_path_name()
        if not unreal.EditorAssetLibrary.save_asset(object_path, only_if_is_dirty=False):
            raise RuntimeError(f"Could not save imported motion asset: {object_path}")

    report = {
        "schemaVersion": 1,
        "importerVersion": IMPORTER_VERSION,
        "engineVersion": unreal.SystemLibrary.get_engine_version(),
        "manifestSha256": hashlib.sha256(manifest_path.read_bytes()).hexdigest(),
        "fbxFile": fbx.name,
        "importedObjectPaths": imported_paths,
        "assetClasses": sorted(by_class.keys()),
        "skeletalMeshPath": skeletal_mesh.get_path_name(),
        "skeletonPath": skeleton.get_path_name(),
        "animationPath": animation.get_path_name(),
        "boneNames": animation_tracks,
        "requiredReferenceHierarchy": reference_hierarchy,
        "meshBoundsSizeCentimeters": vector_record(bounds_size),
        "durationSeconds": round(duration, 6),
        "sampledKeys": sampled_keys,
        "rootSamples": root_samples,
        "footWorldSamples": foot_world_samples,
        "sourceContactEvents": manifest["clip"]["events"],
        "contactEventsTransported": len(notifies) > 0,
        "unrealNotifyCount": len(notifies),
        "importSettings": {
            "pipeline": "UnrealFBXSkeletal",
            "automated": True,
            "replaceExisting": True,
            "importMesh": True,
            "importAnimations": True,
            "createPhysicsAsset": False,
        },
    }
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    unreal.log(
        f"GATERS_MOTION_IMPORT_OK mesh={report['skeletalMeshPath']} "
        f"animation={report['animationPath']} keys={sampled_keys} duration={duration:.3f}")


import_motion()
