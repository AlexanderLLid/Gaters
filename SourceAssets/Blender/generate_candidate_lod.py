"""Authoritative recipe for a derived style-neutral Blender candidate set."""

import argparse
import hashlib
import json
import math
import shutil
import sys
from pathlib import Path

import bpy
from mathutils import Vector


GENERATOR_VERSION = 1
ARTIFACT_POLICY = {
    "contract": "authoritative",
    "generator": "authoritative",
    "blend": "derived-reproducible",
}
EXPECTED_ROLES = ["near", "mid", "far"]


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load_contract(path):
    contract = json.loads(path.read_text(encoding="utf-8"))
    roles = [item.get("role") for item in contract.get("representations", [])]
    subdivisions = [item.get("subdivisions") for item in contract.get("representations", [])]
    if contract.get("schemaVersion") != 1:
        raise ValueError("contract schemaVersion must be 1")
    if contract.get("generatorVersion") != GENERATOR_VERSION:
        raise ValueError(f"contract generatorVersion must be {GENERATOR_VERSION}")
    if contract.get("artifactPolicy") != ARTIFACT_POLICY:
        raise ValueError(f"contract artifactPolicy must be {ARTIFACT_POLICY}")
    if contract.get("styleNeutral") is not True:
        raise ValueError("fixture contract must declare styleNeutral true")
    if "selectedStyle" not in contract or contract["selectedStyle"] is not None:
        raise ValueError("neutral fixture contract must declare selectedStyle null")
    if roles != EXPECTED_ROLES:
        raise ValueError(f"representation roles must be {EXPECTED_ROLES}")
    if not all(isinstance(value, int) and 1 <= value <= 6 for value in subdivisions):
        raise ValueError("representation subdivisions must be integers from 1 through 6")
    if not all(left > right for left, right in zip(subdivisions, subdivisions[1:])):
        raise ValueError("representation detail must strictly decrease from near to far")
    dimensions = contract.get("dimensionsMeters", [])
    if len(dimensions) != 3 or not all(isinstance(value, (int, float)) and value > 0 for value in dimensions):
        raise ValueError("dimensionsMeters must contain three positive numbers")
    return contract


def recreate_output(output, contract):
    expected_name = f"{contract['assetId'].rsplit('.', 1)[-1]}-v{contract['assetVersion']}"
    if output.name != expected_name or output.parent.name != "Derived":
        raise ValueError(f"output must be a Derived/{expected_name} candidate directory")
    if output.is_symlink():
        raise ValueError("output candidate directory must not be a symlink")
    if output.exists():
        shutil.rmtree(output)
    output.mkdir(parents=True)


def make_material(meaning):
    material = bpy.data.materials.new("M_NeutralStonePlaceholder")
    material.diffuse_color = (0.32, 0.34, 0.36, 1.0)
    material.roughness = 0.82
    material["meaning"] = meaning
    return material


def normalize_mesh(obj, dimensions):
    vertices = obj.data.vertices
    minimum = Vector((min(v.co.x for v in vertices), min(v.co.y for v in vertices), min(v.co.z for v in vertices)))
    maximum = Vector((max(v.co.x for v in vertices), max(v.co.y for v in vertices), max(v.co.z for v in vertices)))
    size = maximum - minimum
    center_xy = Vector(((minimum.x + maximum.x) / 2, (minimum.y + maximum.y) / 2, minimum.z))
    for vertex in vertices:
        local = vertex.co - center_xy
        vertex.co = Vector((
            local.x * dimensions[0] / size.x,
            local.y * dimensions[1] / size.y,
            local.z * dimensions[2] / size.z,
        ))
    obj.data.update()


def make_representation(role, subdivisions, dimensions, material):
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=subdivisions, radius=1.0, location=(0, 0, 0))
    obj = bpy.context.object
    obj.name = f"SM_NeutralRock_{role.capitalize()}"
    obj.data.name = f"{obj.name}_Mesh"
    normalize_mesh(obj, dimensions)
    obj.data.materials.append(material)
    obj["representation_role"] = role
    obj["candidate_only"] = True
    return obj


def object_bounds(obj):
    points = [obj.matrix_world @ Vector(corner) for corner in obj.bound_box]
    return {
        "min": [round(min(point[index] for point in points), 6) for index in range(3)],
        "max": [round(max(point[index] for point in points), 6) for index in range(3)],
    }


def export_obj(obj, path):
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    result = bpy.ops.wm.obj_export(
        filepath=str(path),
        export_selected_objects=True,
        export_materials=True,
        export_triangulated_mesh=True,
        forward_axis="NEGATIVE_Z",
        up_axis="Y",
    )
    if result != {"FINISHED"}:
        raise RuntimeError(f"OBJ export failed for {obj.name}: {result}")


def export_fbx(obj, path):
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    result = bpy.ops.export_scene.fbx(
        filepath=str(path),
        use_selection=True,
        object_types={"MESH"},
        apply_unit_scale=True,
        apply_scale_options="FBX_SCALE_UNITS",
        axis_forward="-Y",
        axis_up="Z",
        use_mesh_modifiers=True,
        use_triangles=True,
        add_leaf_bones=False,
        bake_anim=False,
        path_mode="STRIP",
        embed_textures=False,
    )
    if result != {"FINISHED"}:
        raise RuntimeError(f"FBX export failed for {obj.name}: {result}")


def inspect_obj(path):
    vertices = []
    triangles = 0
    for line in path.read_text(encoding="utf-8").splitlines():
        if line.startswith("v "):
            vertices.append(tuple(float(value) for value in line.split()[1:4]))
        elif line.startswith("f "):
            corners = line.split()[1:]
            if len(corners) != 3:
                raise ValueError(f"non-triangle face in {path.name}")
            triangles += 1
    if not vertices or not triangles:
        raise ValueError(f"empty OBJ geometry in {path.name}")
    return {
        "triangles": triangles,
        "bounds": {
            "min": [round(min(vertex[index] for vertex in vertices), 6) for index in range(3)],
            "max": [round(max(vertex[index] for vertex in vertices), 6) for index in range(3)],
        },
    }


def blender_bounds_to_portable(bounds):
    """Apply OBJ export mapping (x, y, z) -> (x, z, -y) to an axis-aligned box."""
    return {
        "min": [bounds["min"][0], bounds["min"][2], -bounds["max"][1]],
        "max": [bounds["max"][0], bounds["max"][2], -bounds["min"][1]],
    }


def reopen_and_validate_source(blend_path, contract, expected_triangles):
    result = bpy.ops.wm.open_mainfile(filepath=str(blend_path))
    if result != {"FINISHED"}:
        raise RuntimeError(f"could not reopen derived blend: {result}")

    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    by_role = {obj.get("representation_role"): obj for obj in meshes}
    objects = [by_role.get(role) for role in EXPECTED_ROLES]
    records = []
    for role, obj in zip(EXPECTED_ROLES, objects):
        if obj is None:
            continue
        obj.data.calc_loop_triangles()
        records.append({"role": role, "object": obj.name, "triangles": len(obj.data.loop_triangles)})

    checks = {
        "rolesMatch": len(meshes) == len(EXPECTED_ROLES) and all(objects),
        "triangleCountsMatch": len(records) == len(EXPECTED_ROLES)
        and all(record["triangles"] == expected_triangles[record["role"]] for record in records),
        "sharedOrigin": all(obj is not None and tuple(obj.location) == (0.0, 0.0, 0.0) for obj in objects),
        "unitScale": all(obj is not None and tuple(obj.scale) == (1.0, 1.0, 1.0) for obj in objects),
        "sharedMaterialMeaning": all(
            obj is not None
            and obj.data.materials
            and obj.data.materials[0].get("meaning") == contract["materialMeaning"]
            for obj in objects
        ),
    }
    evidence = {"reopened": True, "passed": all(checks.values()), "checks": checks, "representations": records}
    if not evidence["passed"]:
        failed = [name for name, passed in checks.items() if not passed]
        raise ValueError(f"reopened source validation failed: {failed}")
    return objects, evidence


def setup_preview(obj, output_path):
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 640
    scene.render.resolution_y = 400
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.filepath = str(output_path)
    scene.world = scene.world or bpy.data.worlds.new("PreviewWorld")
    scene.world.color = (0.055, 0.065, 0.08)

    bpy.ops.object.camera_add(location=(4.2, -5.0, 3.1))
    camera = bpy.context.object
    camera.data.type = "ORTHO"
    camera.data.ortho_scale = 3.6
    camera.rotation_euler = (math.radians(67), 0, math.radians(39))
    direction = Vector((0, 0, 0.7)) - camera.location
    camera.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()
    scene.camera = camera

    bpy.ops.object.light_add(type="AREA", location=(-3.0, -4.0, 6.0))
    bpy.context.object.data.energy = 900
    bpy.context.object.data.shape = "DISK"
    bpy.context.object.data.size = 4.0
    bpy.ops.object.light_add(type="AREA", location=(4.0, 1.0, 2.5))
    bpy.context.object.data.energy = 450
    bpy.context.object.data.size = 3.0

    for candidate in bpy.context.scene.objects:
        candidate.hide_render = candidate.type == "MESH" and candidate != obj
    bpy.ops.render.render(write_still=True)


def validate(objects, records, material_meaning, source_validation):
    checks = {
        "rolesOrdered": [record["role"] for record in records] == EXPECTED_ROLES,
        "trianglesDecrease": all(
            left["triangles"] > right["triangles"] for left, right in zip(records, records[1:])
        ),
        "sharedOrigin": all(tuple(obj.location) == (0.0, 0.0, 0.0) for obj in objects),
        "unitScale": all(tuple(obj.scale) == (1.0, 1.0, 1.0) for obj in objects),
        "sharedMaterialMeaning": all(
            obj.data.materials and obj.data.materials[0].get("meaning") == material_meaning for obj in objects
        ),
        "blenderBoundsMatch": all(
            record["blenderBounds"] == records[0]["blenderBounds"] for record in records[1:]
        ),
        "portableBoundsMatch": all(
            record["portableBounds"] == records[0]["portableBounds"] for record in records[1:]
        ),
        "axisConvertedBoundsMatch": all(
            record["portableBounds"] == blender_bounds_to_portable(record["blenderBounds"])
            for record in records
        ),
        "portableArtifactsParsed": all(record["triangles"] > 0 for record in records),
        "blenderArtifactReopened": source_validation["passed"],
    }
    return {"passed": all(checks.values()), "checks": checks}


def build(contract_path, output):
    contract = load_contract(contract_path)
    recreate_output(output, contract)
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.unit_settings.system = "METRIC"
    scene.unit_settings.scale_length = 1.0

    material = make_material(contract["materialMeaning"])
    objects = [
        make_representation(item["role"], item["subdivisions"], contract["dimensionsMeters"], material)
        for item in contract["representations"]
    ]
    expected_triangles = {}
    for item, obj in zip(contract["representations"], objects):
        obj.data.calc_loop_triangles()
        expected_triangles[item["role"]] = len(obj.data.loop_triangles)
    blend_name = f"neutral-rock-v{contract['assetVersion']}.blend"
    blend_path = output / blend_name
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path), check_existing=False)
    blend_path.with_suffix(blend_path.suffix + "1").unlink(missing_ok=True)
    objects, source_validation = reopen_and_validate_source(blend_path, contract, expected_triangles)

    records = []
    for obj, item in zip(objects, contract["representations"]):
        obj_path = output / f"neutral-rock-{item['role']}-v{contract['assetVersion']}.obj"
        fbx_path = output / f"neutral-rock-{item['role']}-v{contract['assetVersion']}.fbx"
        export_obj(obj, obj_path)
        export_fbx(obj, fbx_path)
        inspected = inspect_obj(obj_path)
        blender_bounds = object_bounds(obj)
        records.append({
            "role": item["role"],
            "authorship": "candidate",
            "selectionRule": "use only when silhouette validation beats borrowed Unreal runtime/build reduction",
            "file": obj_path.name,
            "sha256": sha256(obj_path),
            "triangles": inspected["triangles"],
            "blenderBounds": blender_bounds,
            "portableBounds": inspected["bounds"],
            "pivotMeters": [0.0, 0.0, 0.0],
            "scale": [1.0, 1.0, 1.0],
            "materialMeaning": contract["materialMeaning"],
            "unrealFbx": {
                "file": fbx_path.name,
                "forwardAxis": "-Y",
                "upAxis": "Z",
                "deterministicBytes": False,
                "validation": "measure imported LOD triangles and bounds in Unreal",
            },
        })

    preview_path = output / "preview-near.png"
    setup_preview(objects[0], preview_path)
    validation = validate(objects, records, contract["materialMeaning"], source_validation)
    if not validation["passed"]:
        failed = [name for name, passed in validation["checks"].items() if not passed]
        raise ValueError(f"generated representation validation failed: {failed}")

    manifest = {
        "schemaVersion": 1,
        "assetId": contract["assetId"],
        "assetVersion": contract["assetVersion"],
        "stableIdentity": f"{contract['assetId']}@{contract['assetVersion']}",
        "generatorVersion": GENERATOR_VERSION,
        "blenderVersion": bpy.app.version_string,
        "authority": {
            "contract": {
                "file": contract_path.name,
                "sha256": sha256(contract_path),
                "authoritative": True,
            },
            "generator": {
                "file": Path(__file__).name,
                "sha256": sha256(Path(__file__).resolve()),
                "version": GENERATOR_VERSION,
                "authoritative": True,
            },
        },
        "sourceIdentity": {
            "selectedStyle": contract["selectedStyle"],
            "styleNeutral": contract["styleNeutral"],
        },
        "dimensionsMeters": contract["dimensionsMeters"],
        "units": {"system": "metric", "metersPerBlenderUnit": 1.0},
        "blenderArtifact": {
            "file": blend_name,
            "format": "blend",
            "derived": True,
            "reproducible": True,
            "validation": source_validation,
        },
        "portableFormat": "Wavefront OBJ (triangulated; current Unreal Interchange intake format)",
        "portableAxisConversion": {
            "from": "Blender right-handed Z-up",
            "to": "Wavefront OBJ right-handed Y-up",
            "mapping": ["x", "z", "-y"],
            "exporter": {"forwardAxis": "NEGATIVE_Z", "upAxis": "Y"},
        },
        "representations": records,
        "preview": {"file": preview_path.name, "representationRole": "near"},
        "validation": validation,
        "boundary": {
            "borrowedUnrealOptions": ["automatic LOD", "HLOD", "Nanite"],
            "blenderRepresentations": "authored candidates, not automatic runtime policy",
        },
    }
    (output / "manifest.json").write_text(
        json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    print(f"GATERS_BLENDER_CANDIDATE_OK manifest={output / 'manifest.json'}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--contract", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else [])
    try:
        build(args.contract.resolve(), args.output.resolve())
    except Exception as error:
        print(f"GATERS_BLENDER_CANDIDATE_ERROR: {error}", file=sys.stderr)
        raise SystemExit(2) from error


if __name__ == "__main__":
    main()
