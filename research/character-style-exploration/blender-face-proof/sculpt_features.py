"""Build one Task 8 feature-grid candidate from the integrity-pinned source blend."""

import argparse
import hashlib
import json
import sys
from pathlib import Path

import bpy
from bl_ext.blender_org.mpfb.services import LocationService, TargetService

ROOT = Path(__file__).resolve().parent
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from face_search import RUNS_ROOT, load_space
from feature_grid import feature_effective_controls, validate_feature_recipe
from sculpt_integrity import SOURCE_BLEND
from sculpt_clay import (
    ROOT, active_shape_keys, configure_scene, file_hash, freeze_direct_sculpt,
    hard_checks, mask_shoulders, render_views, output_path, apply_sculpt_targets,
)


FEATURE_PARENT_RECIPE = ROOT / "Runs" / "macro-grid-20260720-071000" / "candidates" / "eaec583dac7c9adf295510e82a06dd721dceb2aea371ced31bfdcaa88b884d5b.json"
EYE_TARGETS = {"sculpt.eye.left.height1": ("eyes", "l-eye-height1-incr"), "sculpt.eye.right.height1": ("eyes", "r-eye-height1-incr")}


def arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--candidate", required=True)
    parser.add_argument("--output", required=True)
    return parser.parse_args(sys.argv[sys.argv.index("--") + 1:])


def load_feature_candidate(path):
    candidate_path = Path(path).resolve()
    relative = candidate_path.relative_to(RUNS_ROOT.resolve())
    if len(relative.parts) != 3 or relative.parts[1] != "candidates":
        raise ValueError("candidate must be stored under Runs/<run-id>/candidates")
    candidate = json.loads(candidate_path.read_text(encoding="utf-8"))
    parent = json.loads(FEATURE_PARENT_RECIPE.read_text(encoding="utf-8"))
    validate_feature_recipe(load_space(ROOT / "face-search-space.json"), parent, candidate)
    if candidate_path.name != f"{candidate['candidateId']}.json":
        raise ValueError("candidate recipe filename does not match candidateId")
    return candidate_path, candidate


def apply_feature_targets(head, controls):
    apply_sculpt_targets(head, controls)
    target_root = Path(LocationService.get_mpfb_data("targets"))
    for name, (section, target) in EYE_TARGETS.items():
        TargetService.load_target(head, str(target_root / section / f"{target}.target.gz"), weight=controls[name], name=name)


def scene_lock(head, camera):
    """Hash presentation state only; sculpt controls and shape-key coordinates stay out."""
    target = json.loads((ROOT / "target-feature-geometry.json").read_text(encoding="utf-8"))
    def transform(obj):
        return {"location": list(obj.location), "rotation": list(obj.rotation_euler), "scale": list(obj.scale)}

    def material_record(material):
        image_nodes = [] if not material or not material.use_nodes else [node.name for node in material.node_tree.nodes if node.type == "TEX_IMAGE"]
        return {"name": material.name if material else None, "useNodes": bool(material and material.use_nodes), "imageNodes": image_nodes}

    world = bpy.context.scene.world
    background = world.node_tree.nodes.get("Background") if world and world.use_nodes else None
    lights = [{"name": obj.name, "type": obj.data.type, "shape": obj.data.shape, "energy": obj.data.energy,
               "size": obj.data.size, **transform(obj)} for obj in bpy.context.scene.objects if obj.type == "LIGHT"]
    lights.sort(key=lambda item: item["name"])
    payload = {
        "camera": {"type": camera.data.type, "orthoScale": camera.data.ortho_scale, "lens": camera.data.lens,
                   "location": list(camera.location), "rotation": list(camera.rotation_euler),
                   "resolution": [bpy.context.scene.render.resolution_x, bpy.context.scene.render.resolution_y],
                   "engine": bpy.context.scene.render.engine},
        "objectTransforms": {name: transform(bpy.data.objects[name]) for name in ("FaceProofRoot", "FaceProofHead", "SculptEyeL", "SculptEyeR", "SculptPupilL", "SculptPupilR")},
        "visibleMaterials": {obj.name: [material_record(material) for material in obj.data.materials]
                             for obj in bpy.context.scene.objects if obj.type == "MESH" and not obj.hide_render},
        "lights": lights,
        "suppressedObjects": {name: bool(bpy.data.objects[name].hide_render) for name in ("FaceProofHead.braid01", "FaceProofHead.eyebrow003", "FaceProofHead.high-poly")},
        "world": {"useNodes": bool(world and world.use_nodes), "backgroundColor": list(background.inputs["Color"].default_value) if background else None,
                  "backgroundStrength": background.inputs["Strength"].default_value if background else None},
        "targetImage": target["image"],
    }
    canonical = json.dumps(payload, sort_keys=True, separators=(",", ":")).encode("utf-8")
    return {"sha256": hashlib.sha256(canonical).hexdigest(), "payload": payload}


def write_manifest(output, candidate_path, candidate, controls, source_blend, status, error=None, checks=None, lock=None):
    artifacts = {path.name: file_hash(path) for path in output.iterdir() if path.is_file() and path.name != "manifest.json"}
    manifest = {
        "schemaVersion": 1, "status": status,
        "purpose": "Task 8 front-supported feature geometry grid",
        "candidate": candidate,
        "candidateRecipe": {"path": candidate_path.relative_to(ROOT).as_posix(), "sha256": file_hash(candidate_path)},
        "featureControls": candidate["featureControls"],
        "sourceBlend": {"path": str(source_blend), "sha256": file_hash(source_blend)},
        "toolVersions": {
            "blender": bpy.app.version_string, "python": sys.version.split()[0],
            "sculptScript": file_hash(ROOT / "sculpt_clay.py"), "featureScript": file_hash(Path(__file__).resolve()),
            "faceSearch": file_hash(ROOT / "face_search.py"),
        },
        "controls": controls, "activeShapeKeys": active_shape_keys(bpy.data.objects["FaceProofHead"]) if status == "passed" else [],
        "hardChecks": checks or {"completed": False}, "artifacts": artifacts,
    }
    if lock is not None:
        manifest["sceneLock"] = lock
    if error:
        manifest["failure"] = str(error)
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def main():
    args = arguments()
    candidate_path, candidate = load_feature_candidate(args.candidate)
    output = output_path(candidate_path, candidate, args.output)
    if output.exists():
        raise FileExistsError(f"Refusing to overwrite sculpt evidence: {output}")
    output.mkdir(parents=True)
    source_blend = Path(bpy.data.filepath).resolve()
    if source_blend != SOURCE_BLEND.resolve():
        raise RuntimeError("feature sculpt must start from the integrity-pinned face proof")
    controls = feature_effective_controls(candidate["parameters"], candidate["featureControls"])
    try:
        head = bpy.data.objects.get("FaceProofHead")
        source_eyes = bpy.data.objects.get("FaceProofHead.high-poly")
        if head is None or source_eyes is None:
            raise RuntimeError("MPFB feature source objects are missing")
        apply_feature_targets(head, controls)
        freeze_direct_sculpt(head, source_eyes, controls)
        mask_shoulders(head)
        camera, target, center_x = configure_scene(output, head)
        root = bpy.data.objects.get("FaceProofRoot")
        root["proof_role"] = "view-independent-clay-sculpt"
        root["reference_projection"] = False
        head["sculpt_targets"] = json.dumps(controls, sort_keys=True)
        head["feature_controls"] = json.dumps(candidate["featureControls"], sort_keys=True)
        blend_path = output / "face-sculpt.blend"
        bpy.ops.file.pack_all()
        bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
        render_views(camera, target, center_x, output)
        bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
        checks = {"completed": True, **hard_checks(root, head)}
        write_manifest(output, candidate_path, candidate, controls, source_blend, "passed", checks=checks, lock=scene_lock(head, camera))
    except Exception as error:
        write_manifest(output, candidate_path, candidate, controls, source_blend, "failed", error=error)
        raise
    print(f"FEATURE_SCULPT_OK output={output}")


if __name__ == "__main__":
    main()
