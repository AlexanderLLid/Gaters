"""Pure ART-4 binding checks for candidate sculpt manifests."""

import hashlib
import json
from pathlib import Path
import re

from face_search import RUNS_ROOT, _canonical, candidate_to_controls, load_space, validate_candidate
from detail_grid import detail_effective_controls, validate_detail_recipe, validate_detail_target
from feature_calibration import calibration_effective_controls, load_task8_sources, validate_calibration_recipe
from feature_grid import feature_effective_controls, validate_feature_recipe


ROOT = Path(__file__).resolve().parent
DERIVED_RUNS_ROOT = ROOT / "Derived" / "search-runs"
SOURCE_BLEND = ROOT / "Derived" / "face-v2" / "face-proof.blend"
FEATURE_PARENT_RECIPE = ROOT / "Runs" / "macro-grid-20260720-071000" / "candidates" / "eaec583dac7c9adf295510e82a06dd721dceb2aea371ced31bfdcaa88b884d5b.json"
FEATURE_TARGET = ROOT / "target-feature-geometry.json"
DETAIL_TARGET = ROOT / "target-detail-geometry.json"
DETAIL_PARENT_RECIPE = ROOT / "Runs" / "feature-calibration-20260720-100000" / "candidates" / "9032fd4dc9db2de64564b3f19e32ca5e6199881df111c2fa2d37feb1c7cc12f8.json"


def file_hash(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def required_tool_scripts(is_feature_recipe, is_calibration_recipe, is_detail_recipe):
    """Return only scripts that actually participate in this build path."""
    scripts = [("sculptScript", ROOT / "sculpt_clay.py"), ("faceSearch", ROOT / "face_search.py")]
    if is_feature_recipe:
        scripts.append(("featureScript", ROOT / "sculpt_features.py"))
    if is_calibration_recipe and not is_detail_recipe:
        scripts.extend((
            ("calibrationScript", ROOT / "sculpt_calibrated.py"),
            ("calibrationContract", ROOT / "feature_calibration.py"),
            ("featureScript", ROOT / "sculpt_features.py"),
        ))
    if is_detail_recipe:
        scripts.extend((
            ("detailScript", ROOT / "sculpt_details.py"),
            ("detailContract", ROOT / "detail_grid.py"),
            ("featureScript", ROOT / "sculpt_features.py"),
            ("calibrationContract", ROOT / "feature_calibration.py"),
            ("integrityContract", ROOT / "sculpt_integrity.py"),
        ))
    return scripts


def detail_override_duplicates(active_shape_keys):
    """Find additive copies of Task 10 controls that must replace source keys."""
    if not isinstance(active_shape_keys, list):
        raise ValueError("active shape keys are missing")
    duplicate = re.compile(r"(?:mouth\.width|nose\.pointDown)\.\d{3}\Z")
    return sorted(
        item["name"] for item in active_shape_keys
        if isinstance(item, dict) and isinstance(item.get("name"), str)
        and duplicate.fullmatch(item["name"]) and abs(item.get("value", 0.0)) > 0.000001
    )


def invalidated_run_ids():
    invalidated = set()
    safe_run_id = re.compile(r"[A-Za-z0-9][A-Za-z0-9._-]*\Z")
    for path in sorted(RUNS_ROOT.rglob("*.json")):
        is_artifact_loss = "artifact-loss" in path.name
        try:
            record = _read_manifest(path)
        except (OSError, json.JSONDecodeError) as error:
            if is_artifact_loss:
                raise ValueError(f"artifact-loss incident is unreadable: {path.name}") from error
            continue
        if is_artifact_loss:
            required = {"schemaVersion", "incident", "invalidatedRunIds"}
            if (
                not isinstance(record, dict)
                or not required <= set(record)
                or record["schemaVersion"] != 1
                or record["incident"] != "task-2-derived-artifact-loss"
                or not isinstance(record["invalidatedRunIds"], list)
                or any(not isinstance(run_id, str) or not safe_run_id.fullmatch(run_id) for run_id in record["invalidatedRunIds"])
            ):
                raise ValueError(f"artifact-loss incident is malformed: {path.name}")
        if not isinstance(record, dict) or not isinstance(record.get("incident"), str):
            continue
        run_ids = record.get("invalidatedRunIds", [])
        if isinstance(run_ids, list):
            invalidated.update(run_id for run_id in run_ids if isinstance(run_id, str) and safe_run_id.fullmatch(run_id))
        run_id = record.get("runId")
        if record.get("status") == "invalid-for-promotion" and isinstance(run_id, str) and safe_run_id.fullmatch(run_id):
            invalidated.add(run_id)
    return invalidated


def temporary_run_root(run_id):
    if not isinstance(run_id, str) or not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9._-]*", run_id):
        raise ValueError("temporary run ID must be a safe single path component")
    root = (DERIVED_RUNS_ROOT / run_id).resolve()
    if root.parent != DERIVED_RUNS_ROOT.resolve():
        raise ValueError("temporary run root escapes Derived/search-runs")
    return root


def _read_manifest(path):
    with path.open(encoding="utf-8") as stream:
        return json.load(stream)


def _candidate_recipe_path(manifest):
    candidate = manifest.get("candidate")
    recipe = manifest.get("candidateRecipe")
    if not isinstance(candidate, dict) or not isinstance(recipe, dict):
        raise ValueError("candidate recipe is incomplete")
    candidate_id = candidate.get("candidateId")
    raw_path = recipe.get("path")
    if not isinstance(candidate_id, str) or not isinstance(raw_path, str):
        raise ValueError("candidate recipe path is invalid")
    relative = Path(raw_path)
    if relative.is_absolute() or ".." in relative.parts:
        raise ValueError("candidate recipe path escapes Runs")
    resolved = (ROOT / relative).resolve()
    try:
        run_relative = resolved.relative_to(RUNS_ROOT.resolve())
    except ValueError as error:
        raise ValueError("candidate recipe path escapes Runs") from error
    if len(run_relative.parts) != 3 or run_relative.parts[1] != "candidates":
        raise ValueError("candidate recipe path must be Runs/<run-id>/candidates/<candidate-id>.json")
    run_id = run_relative.parts[0]
    expected = RUNS_ROOT / run_id / "candidates" / f"{candidate_id}.json"
    if resolved != expected.resolve() or not resolved.is_file():
        raise ValueError("candidate recipe path does not match candidateId")
    return resolved, run_id


def detail_source_receipts():
    root = DERIVED_RUNS_ROOT / "feature-calibration-20260720-100000" / "round-9" / "9032fd4dc9db2de64564b3f19e32ca5e6199881df111c2fa2d37feb1c7cc12f8"
    paths = {"featureGeometry": root / "feature-geometry.json", "macroGeometry": root / "macro-geometry.json", "parentManifest": root / "manifest.json"}
    return {key: file_hash(path) for key, path in paths.items()}


def validate_detail_landmark_receipt(target):
    stable = target["stableVertexIndices"]
    receipt = stable["receipt"]
    relative = Path(receipt["path"])
    if relative.is_absolute() or ".." in relative.parts:
        raise ValueError("detail landmark receipt path escapes Runs")
    path = (ROOT / relative).resolve()
    try:
        path.relative_to(RUNS_ROOT.resolve())
    except ValueError as error:
        raise ValueError("detail landmark receipt path escapes Runs") from error
    if not path.is_file() or file_hash(path) != receipt["sha256"]:
        raise ValueError("detail landmark receipt hash does not match")
    record = _read_manifest(path)
    if (
        not isinstance(record, dict)
        or record.get("candidateId") != DETAIL_PARENT_RECIPE.stem
        or record.get("indices") != stable["indices"]
        or "frontmost" not in record.get("method", "")
    ):
        raise ValueError("detail landmark receipt does not bind the frozen map")


def validate_manifest_integrity(output):
    """Return the validated candidate recipe bound to this exact output directory."""
    output = Path(output).resolve()
    invalidated = invalidated_run_ids()
    try:
        output_relative = output.relative_to(DERIVED_RUNS_ROOT.resolve())
    except ValueError:
        output_relative = None
    if output_relative and output_relative.parts and output_relative.parts[0] in invalidated:
        raise ValueError(f"output run is invalidated: {output_relative.parts[0]}")
    manifest_path = output / "manifest.json"
    if not manifest_path.is_file():
        raise ValueError("manifest is missing")
    manifest = _read_manifest(manifest_path)
    recipe_path, run_id = _candidate_recipe_path(manifest)
    recipe = _read_manifest(recipe_path)
    candidate = manifest["candidate"]
    recipe_hash = manifest["candidateRecipe"].get("sha256")
    if recipe_hash != file_hash(recipe_path):
        raise ValueError("candidate recipe hash does not match")
    if _canonical(recipe) != _canonical(candidate):
        raise ValueError("candidate recipe does not match manifest candidate")
    space = load_space(ROOT / "face-search-space.json")
    is_feature_recipe = "featureControls" in recipe or "featureControls" in manifest
    is_calibration_recipe = "calibrationControls" in recipe or "calibrationControls" in manifest
    is_detail_recipe = "detailControls" in recipe or "detailControls" in manifest
    if sum((is_feature_recipe, is_calibration_recipe, is_detail_recipe)) > 1 and not is_detail_recipe:
        raise ValueError("candidate cannot mix feature and calibration controls")
    if is_detail_recipe:
        if (
            manifest.get("detailControls") != recipe.get("detailControls")
            or manifest.get("targetDetail") != recipe.get("targetDetail")
            or manifest.get("sourceReceipts") != recipe.get("sourceReceipts")
        ):
            raise ValueError("detail provenance does not match the candidate recipe")
        parent = _read_manifest(DETAIL_PARENT_RECIPE)
        detail_target = _read_manifest(DETAIL_TARGET)
        validate_detail_target(detail_target)
        validate_detail_landmark_receipt(detail_target)
        validate_detail_recipe(space, parent, recipe.get("targetDetail"), recipe.get("sourceReceipts"), recipe)
        if recipe["targetDetail"] != {"path": DETAIL_TARGET.name, "sha256": file_hash(DETAIL_TARGET)}:
            raise ValueError("detail target hash does not match")
        if recipe["sourceReceipts"] != detail_source_receipts():
            raise ValueError("detail source receipt hashes do not match Task 9")
        duplicates = detail_override_duplicates(manifest.get("activeShapeKeys"))
        if duplicates:
            raise ValueError(f"detail controls created duplicate override keys: {duplicates}")
    elif is_calibration_recipe:
        if (
            "calibrationControls" not in recipe
            or manifest.get("calibrationControls") != recipe["calibrationControls"]
            or manifest.get("calibration") != recipe.get("calibration")
        ):
            raise ValueError("calibration controls do not match the candidate recipe")
        parent = _read_manifest(FEATURE_PARENT_RECIPE)
        validate_calibration_recipe(space, parent, recipe["calibration"], recipe)
        source_hashes = {item["candidateId"]: item["sha256"] for item in load_task8_sources(ROOT, invalidated=invalidated)}
        if recipe["calibration"]["sourceMetricHashes"] != source_hashes:
            raise ValueError("calibration source metric hashes do not match Task 8 receipts")
    elif is_feature_recipe:
        if "featureControls" not in recipe or manifest.get("featureControls") != recipe["featureControls"]:
            raise ValueError("featureControls do not match the candidate recipe")
        parent = _read_manifest(FEATURE_PARENT_RECIPE)
        validate_feature_recipe(space, parent, recipe)
    else:
        validate_candidate(space, recipe)

    expected_output = DERIVED_RUNS_ROOT / run_id / f"round-{recipe['round']}" / recipe["candidateId"]
    if output != expected_output.resolve():
        raise ValueError("output path does not match candidate lineage")
    if is_detail_recipe:
        expected_controls = detail_effective_controls(_read_manifest(DETAIL_PARENT_RECIPE), recipe["detailControls"])
    elif is_calibration_recipe:
        expected_controls = calibration_effective_controls(recipe["parameters"], recipe["calibrationControls"])
    elif is_feature_recipe:
        expected_controls = feature_effective_controls(recipe["parameters"], recipe["featureControls"])
    else:
        expected_controls = candidate_to_controls(recipe["parameters"])
    if _canonical(manifest.get("controls")) != _canonical(expected_controls):
        raise ValueError("manifest controls do not match candidate parameters")

    if is_feature_recipe or is_calibration_recipe or is_detail_recipe:
        lock = manifest.get("sceneLock")
        required = {"camera", "objectTransforms", "visibleMaterials", "lights", "suppressedObjects", "world", "targetImage"}
        if not isinstance(lock, dict) or set(lock) != {"sha256", "payload"} or not isinstance(lock["payload"], dict) or required - set(lock["payload"]):
            raise ValueError("scene lock is incomplete")
        if lock["sha256"] != hashlib.sha256(_canonical(lock["payload"]).encode("utf-8")).hexdigest():
            raise ValueError("scene lock hash does not match payload")
        target = _read_manifest(FEATURE_TARGET)
        if lock["payload"]["targetImage"] != target["image"]:
            raise ValueError("scene lock target image does not match annotation")

    source = manifest.get("sourceBlend")
    if not isinstance(source, dict) or source.get("path") != str(SOURCE_BLEND.resolve()):
        raise ValueError("source blend path is not the fixed face proof")
    if source.get("sha256") != file_hash(SOURCE_BLEND):
        raise ValueError("source blend hash does not match")

    tools = manifest.get("toolVersions")
    if not isinstance(tools, dict):
        raise ValueError("tool versions are missing")
    scripts = required_tool_scripts(is_feature_recipe, is_calibration_recipe, is_detail_recipe)
    for key, path in scripts:
        if tools.get(key) != file_hash(path):
            raise ValueError(f"{key} hash does not match")
    return recipe
