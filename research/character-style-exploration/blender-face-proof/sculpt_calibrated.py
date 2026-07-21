"""Build one Task 9 measured-calibration candidate from the pinned source blend."""

import argparse
import json
import sys
from pathlib import Path

import bpy

ROOT = Path(__file__).resolve().parent
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from face_search import RUNS_ROOT, load_space
from feature_calibration import calibration_effective_controls, validate_calibration_recipe
from sculpt_clay import (
    active_shape_keys, configure_scene, file_hash, freeze_direct_sculpt, hard_checks,
    mask_shoulders, output_path, render_views,
)
from sculpt_features import FEATURE_PARENT_RECIPE, apply_feature_targets, scene_lock
from sculpt_integrity import SOURCE_BLEND


def arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--candidate", required=True)
    parser.add_argument("--output", required=True)
    return parser.parse_args(sys.argv[sys.argv.index("--") + 1:])


def load_calibrated_candidate(path):
    candidate_path = Path(path).resolve()
    relative = candidate_path.relative_to(RUNS_ROOT.resolve())
    if len(relative.parts) != 3 or relative.parts[1] != "candidates":
        raise ValueError("candidate must be stored under Runs/<run-id>/candidates")
    candidate = json.loads(candidate_path.read_text(encoding="utf-8"))
    parent = json.loads(FEATURE_PARENT_RECIPE.read_text(encoding="utf-8"))
    validate_calibration_recipe(load_space(ROOT / "face-search-space.json"), parent, candidate.get("calibration"), candidate)
    if candidate_path.name != f"{candidate['candidateId']}.json":
        raise ValueError("candidate recipe filename does not match candidateId")
    return candidate_path, candidate


def write_manifest(output, candidate_path, candidate, controls, source_blend, status, error=None, checks=None, lock=None):
    artifacts = {path.name: file_hash(path) for path in output.iterdir() if path.is_file() and path.name != "manifest.json"}
    manifest = {
        "schemaVersion": 1, "status": status,
        "purpose": "Task 9 bounded measured feature calibration",
        "candidate": candidate,
        "candidateRecipe": {"path": candidate_path.relative_to(ROOT).as_posix(), "sha256": file_hash(candidate_path)},
        "calibrationControls": candidate["calibrationControls"],
        "calibration": candidate["calibration"],
        "sourceBlend": {"path": str(source_blend), "sha256": file_hash(source_blend)},
        "toolVersions": {
            "blender": bpy.app.version_string, "python": sys.version.split()[0],
            "sculptScript": file_hash(ROOT / "sculpt_clay.py"),
            "calibrationScript": file_hash(Path(__file__).resolve()),
            "calibrationContract": file_hash(ROOT / "feature_calibration.py"),
            "featureScript": file_hash(ROOT / "sculpt_features.py"),
            "faceSearch": file_hash(ROOT / "face_search.py"),
        },
        "controls": controls,
        "activeShapeKeys": active_shape_keys(bpy.data.objects["FaceProofHead"]) if status == "passed" else [],
        "hardChecks": checks or {"completed": False}, "artifacts": artifacts,
    }
    if lock is not None:
        manifest["sceneLock"] = lock
    if error:
        manifest["failure"] = str(error)
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def main():
    args = arguments()
    candidate_path, candidate = load_calibrated_candidate(args.candidate)
    output = output_path(candidate_path, candidate, args.output)
    if output.exists():
        raise FileExistsError(f"Refusing to overwrite sculpt evidence: {output}")
    output.mkdir(parents=True)
    source_blend = Path(bpy.data.filepath).resolve()
    if source_blend != SOURCE_BLEND.resolve():
        raise RuntimeError("calibration sculpt must start from the integrity-pinned face proof")
    controls = calibration_effective_controls(candidate["parameters"], candidate["calibrationControls"])
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
        head["calibration_controls"] = json.dumps(candidate["calibrationControls"], sort_keys=True)
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
    print(f"CALIBRATION_SCULPT_OK output={output}")


if __name__ == "__main__":
    main()
