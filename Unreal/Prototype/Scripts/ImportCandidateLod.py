import hashlib
import json
import os
from pathlib import Path

import unreal


IMPORTER_VERSION = 1
EXPECTED_ROLES = ["near", "mid", "far"]


def required(name):
    value = os.environ.get(name, "").strip()
    if not value:
        raise RuntimeError(f"Missing environment variable {name}")
    return value


def load_manifest(path):
    raw = path.read_bytes()
    manifest = json.loads(raw)
    if manifest.get("schemaVersion") != 1:
        raise RuntimeError("Candidate manifest schemaVersion must be 1")
    if not manifest.get("validation", {}).get("passed"):
        raise RuntimeError("Candidate manifest validation did not pass")

    identity = manifest.get("sourceIdentity", {})
    if identity.get("styleNeutral") is not True or identity.get("selectedStyle") is not None:
        raise RuntimeError("Candidate must remain style-neutral")

    dimensions = manifest.get("dimensionsMeters")
    if not isinstance(dimensions, list) or len(dimensions) != 3 or any(value <= 0 for value in dimensions):
        raise RuntimeError("Candidate dimensionsMeters must contain three positive values")

    representations = manifest.get("representations", [])
    if [item.get("role") for item in representations] != EXPECTED_ROLES:
        raise RuntimeError(f"Candidate representation roles must be {EXPECTED_ROLES}")
    triangles = [item.get("triangles") for item in representations]
    if not all(isinstance(value, int) and value > 0 for value in triangles):
        raise RuntimeError("Candidate triangle counts must be positive integers")
    if not all(left > right for left, right in zip(triangles, triangles[1:])):
        raise RuntimeError("Candidate triangle counts must strictly decrease")

    source_files = []
    for item in representations:
        transport = item.get("unrealFbx", {})
        filename = transport.get("file", "")
        if not filename or Path(filename).name != filename:
            raise RuntimeError(f"Candidate {item['role']} FBX must be a simple relative filename")
        if transport.get("deterministicBytes") is not False or "sha256" in transport:
            raise RuntimeError(f"Candidate {item['role']} FBX transport truth is invalid")
        source = (path.parent / filename).resolve()
        if source.parent != path.parent.resolve() or not source.is_file() or source.stat().st_size == 0:
            raise RuntimeError(f"Candidate {item['role']} FBX is missing or empty: {source}")
        source_files.append(source)

    return raw, manifest, representations, source_files


def import_base(source, destination, asset_name, object_path):
    if unreal.EditorAssetLibrary.does_asset_exist(object_path):
        if not unreal.EditorAssetLibrary.delete_asset(object_path):
            raise RuntimeError(f"Could not replace derived candidate asset: {object_path}")

    options = unreal.FbxImportUI()
    options.automated_import_should_detect_type = False
    options.import_mesh = True
    options.import_as_skeletal = False
    options.mesh_type_to_import = unreal.FBXImportType.FBXIT_STATIC_MESH
    options.original_import_type = unreal.FBXImportType.FBXIT_STATIC_MESH
    options.import_materials = False
    options.import_textures = False
    options.import_animations = False
    options.static_mesh_import_data.combine_meshes = True
    options.static_mesh_import_data.convert_scene_unit = True

    task = unreal.AssetImportTask()
    task.filename = str(source)
    task.destination_path = destination
    task.destination_name = asset_name
    task.automated = True
    task.replace_existing = False
    task.save = False
    task.factory = unreal.FbxFactory()
    task.options = options
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    imported_paths = [str(path) for path in task.imported_object_paths]
    asset = unreal.load_asset(object_path)
    if asset is None:
        raise RuntimeError(f"Base FBX import produced no loadable {object_path}; imported={imported_paths}")
    if asset.get_class().get_name() != "StaticMesh":
        raise RuntimeError(f"Expected StaticMesh at {object_path}, got {asset.get_class().get_name()}")
    return asset


def import_candidate():
    manifest_path = Path(required("GATERS_CANDIDATE_MANIFEST")).resolve()
    destination = required("GATERS_IMPORT_DESTINATION").rstrip("/")
    asset_name = required("GATERS_IMPORT_NAME")
    report_path = Path(required("GATERS_IMPORT_REPORT")).resolve()
    if not manifest_path.is_file():
        raise RuntimeError(f"Candidate manifest does not exist: {manifest_path}")
    if not destination.startswith("/Game/") or "/" in asset_name or "." in asset_name:
        raise RuntimeError("Candidate destination or asset name is invalid")

    manifest_raw, manifest, representations, sources = load_manifest(manifest_path)
    object_path = f"{destination}/{asset_name}.{asset_name}"
    asset = import_base(sources[0], destination, asset_name, object_path)

    subsystem = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
    if subsystem is None:
        raise RuntimeError("StaticMeshEditorSubsystem is unavailable")
    if subsystem.get_lod_count(asset) != 1:
        raise RuntimeError("Fresh base candidate import must contain exactly one LOD")
    for lod_index, source in enumerate(sources[1:], start=1):
        imported_index = subsystem.import_lod(asset, lod_index, str(source))
        if imported_index != lod_index:
            raise RuntimeError(f"Could not import {source.name} as LOD{lod_index}; result={imported_index}")

    lod_count = subsystem.get_lod_count(asset)
    lod_triangles = [asset.get_num_triangles(index) for index in range(lod_count)]
    expected_triangles = [item["triangles"] for item in representations]
    if lod_triangles != expected_triangles:
        raise RuntimeError(f"Imported LOD triangles {lod_triangles} do not match manifest {expected_triangles}")

    bounds = asset.get_bounding_box()
    size = bounds.max.subtract(bounds.min)
    bounds_size = [size.x, size.y, size.z]
    expected_bounds = [value * 100.0 for value in manifest["dimensionsMeters"]]
    if any(abs(actual - expected) > 0.1 for actual, expected in zip(bounds_size, expected_bounds)):
        raise RuntimeError(f"Imported bounds {bounds_size} do not match expected centimeters {expected_bounds}")

    if not unreal.EditorAssetLibrary.save_asset(object_path, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save imported candidate asset: {object_path}")

    report_path.parent.mkdir(parents=True, exist_ok=True)
    report = {
        "schemaVersion": 1,
        "importerVersion": IMPORTER_VERSION,
        "engineVersion": unreal.SystemLibrary.get_engine_version(),
        "manifestSha256": hashlib.sha256(manifest_raw).hexdigest(),
        "objectPath": object_path,
        "assetClass": asset.get_class().get_name(),
        "lodCount": lod_count,
        "lodTriangles": lod_triangles,
        "boundsSizeCentimeters": bounds_size,
        "lodScreenSizes": list(subsystem.get_lod_screen_sizes(asset)),
        "sourceFiles": [source.name for source in sources],
        "fbxTransportDeterministicBytes": False,
        "selectedStyle": manifest["sourceIdentity"]["selectedStyle"],
    }
    report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    unreal.log(
        f"GATERS_CANDIDATE_LOD_IMPORT_OK asset={object_path} "
        f"triangles={'/'.join(str(value) for value in lod_triangles)}"
    )


import_candidate()
