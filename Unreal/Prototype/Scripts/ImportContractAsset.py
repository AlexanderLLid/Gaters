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


def import_asset():
    source = Path(required("GATERS_IMPORT_SOURCE")).resolve()
    destination = required("GATERS_IMPORT_DESTINATION").rstrip("/")
    asset_name = required("GATERS_IMPORT_NAME")
    report_path = Path(required("GATERS_IMPORT_REPORT")).resolve()
    if not source.is_file():
        raise RuntimeError(f"Source artifact does not exist: {source}")

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(source))
    task.set_editor_property("destination_path", destination)
    task.set_editor_property("destination_name", asset_name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("replace_existing_settings", True)
    task.set_editor_property("save", True)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    object_path = f"{destination}/{asset_name}.{asset_name}"
    package_path = f"{destination}/{asset_name}"
    imported_paths = [str(path) for path in task.get_editor_property("imported_object_paths")]
    if package_path not in imported_paths and object_path not in imported_paths:
        raise RuntimeError(
            f"Import task did not report expected output {package_path}; imported={imported_paths}"
        )
    asset = unreal.load_asset(object_path)
    if asset is None:
        raise RuntimeError(f"Import produced no loadable {object_path}; imported={imported_paths}")
    asset_class = asset.get_class().get_name()
    if asset_class != "StaticMesh":
        raise RuntimeError(f"Expected StaticMesh at {object_path}, got {asset_class}")
    if not unreal.EditorAssetLibrary.save_asset(object_path, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save imported asset: {object_path}")

    report_path.parent.mkdir(parents=True, exist_ok=True)
    report = {
        "schemaVersion": 1,
        "importerVersion": IMPORTER_VERSION,
        "engineVersion": unreal.SystemLibrary.get_engine_version(),
        "sourceFile": source.name,
        "sourceSha256": hashlib.sha256(source.read_bytes()).hexdigest(),
        "objectPath": object_path,
        "importedObjectPaths": imported_paths,
        "assetClass": asset_class,
        "importSettings": {
            "pipeline": "UnrealInterchangeOBJ",
            "automated": True,
            "replaceExisting": True,
            "replaceExistingSettings": True,
            "save": True,
        },
    }
    report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    unreal.log(f"GATERS_IMPORT_OK asset={object_path} source={source.name}")


import_asset()
