import argparse
import hashlib
import json
from datetime import datetime, timezone
from pathlib import Path

from motion_curve_compiler import compile_motion
from motion_curve_verifier import verify_motion


def _canonical(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"), allow_nan=False).encode("utf-8")


def _sha256(value):
    return hashlib.sha256(_canonical(value)).hexdigest()


def _write(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True, allow_nan=False) + "\n", encoding="utf-8")


def run(skeleton_run, limits, limits_sha256, recipe_path, output_root, repeat=2):
    skeleton_run, recipe_path, output_root = map(Path, (skeleton_run, recipe_path, output_root))
    skeleton = json.loads((skeleton_run / "skeleton.json").read_text(encoding="utf-8"))
    skeleton_sha256 = json.loads((skeleton_run / "receipt.json").read_text(encoding="utf-8"))["skeleton_sha256"]
    recipe = json.loads(recipe_path.read_text(encoding="utf-8"))
    root = output_root / f"{recipe['id']}-{datetime.now(timezone.utc).strftime('%Y%m%d-%H%M%S-%f')}"
    root.mkdir(parents=True, exist_ok=False)
    runs, hashes = [], []
    for index in range(1, repeat + 1):
        path = root / f"run-{index}"
        path.mkdir()
        motion = compile_motion(skeleton, limits, recipe, skeleton_sha256, limits_sha256)
        verification = verify_motion(skeleton, limits, recipe, motion, skeleton_sha256, limits_sha256)
        receipt = {
            "schema": "motion-receipt/0",
            "compiler": "motion-curve-compiler/0",
            "verifier": "motion-curve-verifier/0",
            "source_skeleton_sha256": skeleton_sha256,
            "source_joint_limits_sha256": limits_sha256,
            "motion_recipe_sha256": _sha256(recipe),
            "motion_sha256": _sha256(motion),
        }
        for name, value in (("motion-recipe.json", recipe), ("motion.json", motion), ("verification.json", verification), ("receipt.json", receipt)):
            _write(path / name, value)
        runs.append(str(path))
        hashes.append(receipt["motion_sha256"])
    passed = len(set(hashes)) == 1 and all(json.loads((Path(path) / "verification.json").read_text(encoding="utf-8"))["passed"] for path in runs)
    summary = {"schema": "motion-run-summary/0", "passed": passed, "motion_sha256": hashes[0], "runs": runs}
    _write(root / "summary.json", summary)
    return summary


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("skeleton_run", type=Path)
    parser.add_argument("joint_limit_run", type=Path)
    parser.add_argument("recipe", type=Path)
    parser.add_argument("--output-root", type=Path, required=True)
    args = parser.parse_args()
    limits = json.loads((args.joint_limit_run / "joint-limits.json").read_text(encoding="utf-8"))
    limits_sha256 = json.loads((args.joint_limit_run / "receipt.json").read_text(encoding="utf-8"))["joint_limits_sha256"]
    summary = run(args.skeleton_run, limits, limits_sha256, args.recipe, args.output_root)
    print(f"MOTION_RUN_{'PASS' if summary['passed'] else 'FAIL'} motion_sha256={summary['motion_sha256']}")
    raise SystemExit(0 if summary["passed"] else 1)


if __name__ == "__main__":
    main()
