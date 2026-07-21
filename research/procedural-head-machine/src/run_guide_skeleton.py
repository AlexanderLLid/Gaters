import argparse
import hashlib
import json
from datetime import datetime, timezone
from pathlib import Path

from guide_skeleton_compiler import compile_guide_skeleton
from guide_skeleton_verifier import verify_guide_skeleton
from render_guide_skeleton_preview import render


def _canonical(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"), allow_nan=False).encode("utf-8")


def _sha256(value):
    return hashlib.sha256(_canonical(value)).hexdigest()


def _write(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True, allow_nan=False) + "\n", encoding="utf-8")


def run(guide_run, recipe_path, output_root, repeat=2):
    guide_run, recipe_path, output_root = map(Path, (guide_run, recipe_path, output_root))
    guide = json.loads((guide_run / "guide.json").read_text(encoding="utf-8"))
    guide_receipt = json.loads((guide_run / "receipt.json").read_text(encoding="utf-8"))
    recipe = json.loads(recipe_path.read_text(encoding="utf-8"))
    guide_sha256 = guide_receipt["guide_sha256"]
    stamp = datetime.now(timezone.utc).strftime("%Y%m%d-%H%M%S-%f")
    root = output_root / f"{recipe['id']}-{guide['body_plan_id']}-{stamp}"
    root.mkdir(parents=True, exist_ok=False)
    paths, hashes = [], []
    accepted = None
    for index in range(1, repeat + 1):
        path = root / f"run-{index}"
        path.mkdir()
        skeleton = compile_guide_skeleton(guide, recipe, guide_sha256)
        accepted = skeleton
        verification = verify_guide_skeleton(guide, recipe, skeleton, guide_sha256)
        receipt = {
            "schema": "guide-skeleton-receipt/0",
            "compiler": "guide-skeleton-compiler/0",
            "verifier": "guide-skeleton-verifier/0",
            "source_guide_sha256": guide_sha256,
            "skeleton_recipe_sha256": _sha256(recipe),
            "skeleton_sha256": _sha256(skeleton),
        }
        _write(path / "skeleton-recipe.json", recipe)
        _write(path / "skeleton.json", skeleton)
        _write(path / "verification.json", verification)
        _write(path / "receipt.json", receipt)
        paths.append(str(path))
        hashes.append(receipt["skeleton_sha256"])
    passed = len(set(hashes)) == 1 and all(json.loads((Path(path) / "verification.json").read_text())["passed"] for path in paths)
    preview = root / "preview.png"
    if passed:
        render(accepted, preview)
    summary = {
        "schema": "guide-skeleton-run-summary/0",
        "guide_id": guide["id"],
        "skeleton_id": recipe["id"],
        "passed": passed,
        "skeleton_sha256": hashes[0],
        "runs": paths,
        "preview": str(preview) if passed else None,
    }
    _write(root / "summary.json", summary)
    return summary


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("guide_run", type=Path)
    parser.add_argument("recipe", type=Path)
    parser.add_argument("--output-root", type=Path, required=True)
    args = parser.parse_args()
    summary = run(args.guide_run, args.recipe, args.output_root)
    print(f"GUIDE_SKELETON_RUN_{'PASS' if summary['passed'] else 'FAIL'} skeleton_sha256={summary['skeleton_sha256']}")
    raise SystemExit(0 if summary["passed"] else 1)


if __name__ == "__main__":
    main()
