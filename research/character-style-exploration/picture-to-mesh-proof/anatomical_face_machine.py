"""Replay one front-derived MPFB face candidate through build and evaluation."""

import argparse
import hashlib
import json
import shutil
import subprocess
from datetime import datetime, timezone
from pathlib import Path


ROOT = Path(__file__).resolve().parent
FACE_ROOT = ROOT.parent / "blender-face-proof"
BLENDER_DEFAULT = Path(r"C:\Program Files\Blender Foundation\Blender 5.2\blender.exe")
RATIO_NAMES = ("eyeAperture", "noseLength", "alarWidth")


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def _input_record(path):
    path = Path(path).resolve()
    if not path.is_file():
        raise FileNotFoundError(path)
    return {"path": str(path), "sha256": sha256(path)}


def load_contract(path):
    contract_path = Path(path).resolve()
    contract = json.loads(contract_path.read_text(encoding="utf-8"))
    required = {
        "schemaVersion", "machineId", "sourceBlend", "candidateRecipe",
        "targetGeometry", "targetImage", "maximumRatioError",
    }
    if set(contract) != required or contract["schemaVersion"] != 1:
        raise ValueError("unsupported anatomical face machine contract")
    if contract["machineId"] != "research.anatomical-front-fit":
        raise ValueError("unexpected machine identity")
    if not isinstance(contract["maximumRatioError"], (int, float)) or contract["maximumRatioError"] <= 0:
        raise ValueError("maximumRatioError must be positive")
    for name in ("sourceBlend", "candidateRecipe", "targetGeometry", "targetImage"):
        contract[name] = str((contract_path.parent / contract[name]).resolve())
        if not Path(contract[name]).is_file():
            raise FileNotFoundError(contract[name])
    target = json.loads(Path(contract["targetGeometry"]).read_text(encoding="utf-8"))
    if set(target.get("ratios", {})) != set(RATIO_NAMES):
        raise ValueError("target geometry ratios are incomplete")
    if target.get("image", {}).get("sha256") != sha256(contract["targetImage"]):
        raise ValueError("target image does not match target geometry")
    return contract


def initialize_run(contract, run_root):
    run_root = Path(run_root).resolve()
    if run_root.exists():
        raise FileExistsError(f"refusing to overwrite run: {run_root}")
    candidate_path = Path(contract["candidateRecipe"]).resolve()
    inputs = {
        name: _input_record(contract[name])
        for name in ("sourceBlend", "candidateRecipe", "targetGeometry", "targetImage")
    }
    candidate_root = run_root / "candidates"
    candidate_root.mkdir(parents=True)
    copied_candidate = candidate_root / candidate_path.name
    shutil.copyfile(candidate_path, copied_candidate)
    record = {
        "schemaVersion": 1,
        "machineId": contract["machineId"],
        "runId": run_root.name,
        "status": "initialized",
        "createdUtc": datetime.now(timezone.utc).isoformat(),
        "inputs": inputs,
        "maximumRatioError": contract["maximumRatioError"],
        "candidateRecipe": str(copied_candidate),
    }
    (run_root / "run.json").write_text(
        json.dumps(record, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    return record


def decide(target, measured, maximum_error):
    errors = {
        name: abs(float(measured["ratios"][name]) - float(target["ratios"][name]))
        for name in RATIO_NAMES
    }
    total = sum(errors.values())
    mechanical = "pass" if total <= maximum_error else "reject"
    return {
        "decision": "review" if mechanical == "pass" else "reject",
        "mechanicalDecision": mechanical,
        "visualDecision": "pending-human",
        "ratioErrors": errors,
        "totalRatioError": total,
        "maximumRatioError": maximum_error,
    }


def _run(command, log_path):
    completed = subprocess.run(command, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    Path(log_path).write_text(completed.stdout, encoding="utf-8")
    if completed.returncode:
        raise RuntimeError(f"command failed ({completed.returncode}); see {log_path}")


def run_machine(contract_path, run_id, blender=BLENDER_DEFAULT):
    contract = load_contract(contract_path)
    blender = Path(blender).resolve()
    if not blender.is_file():
        raise FileNotFoundError(blender)
    run_root = FACE_ROOT / "Runs" / run_id
    initialized = initialize_run(contract, run_root)
    candidate = json.loads(Path(initialized["candidateRecipe"]).read_text(encoding="utf-8"))
    candidate_id = candidate["candidateId"]
    output = FACE_ROOT / "Derived" / "search-runs" / run_id / "round-9" / candidate_id
    logs = run_root / "logs"
    logs.mkdir()
    commands = [
        [str(blender), "--background", "--python-exit-code", "1", contract["sourceBlend"],
         "--python", str(FACE_ROOT / "sculpt_calibrated.py"), "--",
         "--candidate", initialized["candidateRecipe"], "--output", str(output)],
        [str(blender), "--background", "--python-exit-code", "1", str(output / "face-sculpt.blend"),
         "--python", str(FACE_ROOT / "verify_sculpt.py"), "--", "--output", str(output)],
        [str(blender), "--background", "--python-exit-code", "1", str(output / "face-sculpt.blend"),
         "--python", str(FACE_ROOT / "measure_feature_geometry.py"), "--", "--output", str(output)],
    ]
    try:
        for index, command in enumerate(commands, start=1):
            _run(command, logs / f"stage-{index}.log")
        target = json.loads(Path(contract["targetGeometry"]).read_text(encoding="utf-8"))
        measured_path = output / "feature-geometry.json"
        measured = json.loads(measured_path.read_text(encoding="utf-8"))
        result = {
            "schemaVersion": 1,
            "machineId": contract["machineId"],
            "runId": run_id,
            "candidateId": candidate_id,
            "output": str(output),
            **decide(target, measured, contract["maximumRatioError"]),
            "artifacts": {
                name: sha256(output / name)
                for name in ("face-sculpt.blend", "front.png", "three-quarter.png", "profile.png",
                             "manifest.json", "feature-geometry.json")
            },
        }
    except Exception as error:
        result = {
            "schemaVersion": 1, "machineId": contract["machineId"], "runId": run_id,
            "decision": "reject", "mechanicalDecision": "error",
            "visualDecision": "not-reviewed", "failure": str(error),
        }
        (run_root / "result.json").write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        raise
    (run_root / "result.json").write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--contract", required=True)
    parser.add_argument("--run-id", required=True)
    parser.add_argument("--blender", default=str(BLENDER_DEFAULT))
    args = parser.parse_args()
    result = run_machine(args.contract, args.run_id, args.blender)
    print(json.dumps(result, sort_keys=True))


if __name__ == "__main__":
    main()
