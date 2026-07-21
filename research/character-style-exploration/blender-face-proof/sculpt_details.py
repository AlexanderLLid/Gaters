"""Build one Task 10 visible-detail candidate from the pinned source blend."""

import argparse
import json
import sys
from pathlib import Path

import bpy
from bl_ext.blender_org.mpfb.services import LocationService, TargetService

ROOT = Path(__file__).resolve().parent
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from detail_grid import detail_effective_controls, direct_detail_shape_key_values, validate_detail_recipe
from face_search import RUNS_ROOT, load_space
from sculpt_clay import active_shape_keys, configure_scene, file_hash, freeze_direct_sculpt, hard_checks, mask_shoulders, output_path, render_views
from sculpt_features import apply_feature_targets, scene_lock
from sculpt_integrity import SOURCE_BLEND


DETAIL_TARGETS = {
    "sculpt.eye.left.corner1.down": ("eyes", "l-eye-corner1-down"),
    "sculpt.eye.right.corner1.down": ("eyes", "r-eye-corner1-down"),
    "sculpt.upperlip.height.incr": ("mouth", "mouth-upperlip-height-incr"),
}


def arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--candidate", required=True)
    parser.add_argument("--output", required=True)
    return parser.parse_args(sys.argv[sys.argv.index("--") + 1:])


def load_detail_candidate(path):
    candidate_path = Path(path).resolve()
    relative = candidate_path.relative_to(RUNS_ROOT.resolve())
    if len(relative.parts) != 3 or relative.parts[1] != "candidates":
        raise ValueError("candidate must be stored under Runs/<run-id>/candidates")
    candidate = json.loads(candidate_path.read_text(encoding="utf-8"))
    parent_path = ROOT / "Runs" / "feature-calibration-20260720-100000" / "candidates" / "9032fd4dc9db2de64564b3f19e32ca5e6199881df111c2fa2d37feb1c7cc12f8.json"
    parent = json.loads(parent_path.read_text(encoding="utf-8"))
    validate_detail_recipe(load_space(ROOT / "face-search-space.json"), parent, candidate.get("targetDetail"), candidate.get("sourceReceipts"), candidate)
    if candidate_path.name != f"{candidate['candidateId']}.json":
        raise ValueError("candidate recipe filename does not match candidateId")
    return candidate_path, candidate, parent


def apply_detail_targets(head, controls):
    apply_feature_targets(head, controls)
    target_root = Path(LocationService.get_mpfb_data("targets"))
    for name, (section, target) in DETAIL_TARGETS.items():
        TargetService.load_target(head, str(target_root / section / f"{target}.target.gz"), weight=controls[name], name=name)
    for shape_key, value in direct_detail_shape_key_values(controls).items():
        key = head.data.shape_keys.key_blocks.get(shape_key)
        if key is None:
            raise RuntimeError(f"Missing absolute detail control: {shape_key}")
        key.value = value


def write_manifest(output, candidate_path, candidate, controls, source_blend, status, error=None, checks=None, lock=None):
    artifacts = {path.name: file_hash(path) for path in output.iterdir() if path.is_file() and path.name != "manifest.json"}
    manifest = {
        "schemaVersion": 1, "status": status, "purpose": "Task 10 visible landmark detail grid",
        "candidate": candidate, "candidateRecipe": {"path": candidate_path.relative_to(ROOT).as_posix(), "sha256": file_hash(candidate_path)},
        "calibrationControls": candidate["calibrationControls"], "detailControls": candidate["detailControls"],
        "targetDetail": candidate["targetDetail"], "sourceReceipts": candidate["sourceReceipts"],
        "sourceBlend": {"path": str(source_blend), "sha256": file_hash(source_blend)},
        "toolVersions": {"blender": bpy.app.version_string, "python": sys.version.split()[0],
                         "sculptScript": file_hash(ROOT / "sculpt_clay.py"), "featureScript": file_hash(ROOT / "sculpt_features.py"),
                         "calibrationContract": file_hash(ROOT / "feature_calibration.py"), "detailContract": file_hash(ROOT / "detail_grid.py"),
                         "detailScript": file_hash(Path(__file__).resolve()), "integrityContract": file_hash(ROOT / "sculpt_integrity.py"),
                         "faceSearch": file_hash(ROOT / "face_search.py")},
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
    candidate_path, candidate, parent = load_detail_candidate(args.candidate)
    output = output_path(candidate_path, candidate, args.output)
    if output.exists():
        raise FileExistsError(f"Refusing to overwrite sculpt evidence: {output}")
    output.mkdir(parents=True)
    source_blend = Path(bpy.data.filepath).resolve()
    if source_blend != SOURCE_BLEND.resolve():
        raise RuntimeError("detail sculpt must start from the integrity-pinned face proof")
    controls = detail_effective_controls(parent, candidate["detailControls"])
    try:
        head, source_eyes = bpy.data.objects.get("FaceProofHead"), bpy.data.objects.get("FaceProofHead.high-poly")
        if head is None or source_eyes is None:
            raise RuntimeError("MPFB feature source objects are missing")
        apply_detail_targets(head, controls)
        freeze_direct_sculpt(head, source_eyes, controls)
        mask_shoulders(head)
        camera, target, center_x = configure_scene(output, head)
        root = bpy.data.objects.get("FaceProofRoot")
        root["proof_role"], root["reference_projection"] = "view-independent-clay-sculpt", False
        head["sculpt_targets"] = json.dumps(controls, sort_keys=True)
        head["detail_controls"] = json.dumps(candidate["detailControls"], sort_keys=True)
        blend_path = output / "face-sculpt.blend"
        bpy.ops.file.pack_all(); bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
        render_views(camera, target, center_x, output); bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
        write_manifest(output, candidate_path, candidate, controls, source_blend, "passed", checks={"completed": True, **hard_checks(root, head)}, lock=scene_lock(head, camera))
    except Exception as error:
        write_manifest(output, candidate_path, candidate, controls, source_blend, "failed", error=error)
        raise
    print(f"DETAIL_SCULPT_OK output={output}")


if __name__ == "__main__":
    main()
