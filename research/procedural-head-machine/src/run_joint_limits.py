import argparse
import hashlib
import json
from datetime import datetime, timezone
from pathlib import Path

from joint_limit_compiler import compile_joint_limits
from joint_limit_verifier import verify_joint_limits


def _canonical(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"), allow_nan=False).encode("utf-8")


def _sha256(value):
    return hashlib.sha256(_canonical(value)).hexdigest()


def _write(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True, allow_nan=False) + "\n", encoding="utf-8")


def run(skeleton_run, recipe_path, pose_suite_path, output_root, repeat=2):
    skeleton_run, recipe_path, pose_suite_path, output_root = map(Path, (skeleton_run, recipe_path, pose_suite_path, output_root))
    skeleton = json.loads((skeleton_run / "skeleton.json").read_text(encoding="utf-8"))
    skeleton_sha256 = json.loads((skeleton_run / "receipt.json").read_text(encoding="utf-8"))["skeleton_sha256"]
    recipe = json.loads(recipe_path.read_text(encoding="utf-8"))
    pose_suite = json.loads(pose_suite_path.read_text(encoding="utf-8"))
    stamp = datetime.now(timezone.utc).strftime("%Y%m%d-%H%M%S-%f")
    root = output_root / f"{recipe['id']}-{skeleton['id']}-{stamp}"
    root.mkdir(parents=True, exist_ok=False)
    runs, hashes = [], []
    for index in range(1, repeat + 1):
        path = root / f"run-{index}"
        path.mkdir()
        limits = compile_joint_limits(skeleton, recipe, skeleton_sha256)
        verification = verify_joint_limits(skeleton, recipe, limits, pose_suite, skeleton_sha256)
        receipt = {
            "schema": "joint-limit-receipt/0",
            "compiler": "joint-limit-compiler/0",
            "verifier": "joint-limit-verifier/0",
            "source_skeleton_sha256": skeleton_sha256,
            "joint_limit_recipe_sha256": _sha256(recipe),
            "pose_suite_recipe_sha256": _sha256(pose_suite),
            "joint_limits_sha256": _sha256(limits),
        }
        for name, value in (("joint-limit-recipe.json", recipe), ("joint-limits.json", limits), ("verification.json", verification), ("receipt.json", receipt)):
            _write(path / name, value)
        runs.append(str(path))
        hashes.append(receipt["joint_limits_sha256"])
    passed = len(set(hashes)) == 1 and all(json.loads((Path(path) / "verification.json").read_text(encoding="utf-8"))["passed"] for path in runs)
    summary = {"schema": "joint-limit-run-summary/0", "passed": passed, "joint_limits_sha256": hashes[0], "runs": runs}
    _write(root / "summary.json", summary)
    return summary


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("skeleton_run", type=Path)
    parser.add_argument("recipe", type=Path)
    parser.add_argument("pose_suite", type=Path)
    parser.add_argument("--output-root", type=Path, required=True)
    args = parser.parse_args()
    summary = run(args.skeleton_run, args.recipe, args.pose_suite, args.output_root)
    print(f"JOINT_LIMIT_RUN_{'PASS' if summary['passed'] else 'FAIL'} joint_limits_sha256={summary['joint_limits_sha256']}")
    raise SystemExit(0 if summary["passed"] else 1)


if __name__ == "__main__":
    main()
