import argparse
import hashlib
import json
from datetime import datetime, timezone
from pathlib import Path

from anatomical_guide_compiler import compile_anatomical_guide
from anatomical_guide_verifier import verify_anatomical_guide
from render_anatomical_guide_preview import render


def _canonical(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"), allow_nan=False).encode("utf-8")


def _sha256(value):
    return hashlib.sha256(_canonical(value)).hexdigest()


def _write(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True, allow_nan=False) + "\n", encoding="utf-8")


def run(body_run, recipe_path, output_root, repeat=2):
    body_run, recipe_path, output_root = map(Path, (body_run, recipe_path, output_root))
    body = json.loads((body_run / "composed-mesh.json").read_text(encoding="utf-8"))
    body_receipt = json.loads((body_run / "receipt.json").read_text(encoding="utf-8"))
    recipe = json.loads(recipe_path.read_text(encoding="utf-8"))
    stamp = datetime.now(timezone.utc).strftime("%Y%m%d-%H%M%S-%f")
    root = output_root / f"{recipe['id']}-{body['body_plan_id']}-{stamp}"
    root.mkdir(parents=True, exist_ok=False)
    paths, hashes = [], []
    accepted = None
    for index in range(1, repeat + 1):
        path = root / f"run-{index}"
        path.mkdir()
        guide = compile_anatomical_guide(body, recipe)
        accepted = guide
        verification = verify_anatomical_guide(body, recipe, guide)
        receipt = {
            "schema": "anatomical-guide-receipt/0",
            "compiler": "anatomical-guide-compiler/0",
            "verifier": "anatomical-guide-verifier/0",
            "body_composition_sha256": body_receipt["composition_sha256"],
            "guide_recipe_sha256": _sha256(recipe),
            "guide_sha256": _sha256(guide),
        }
        _write(path / "guide-recipe.json", recipe)
        _write(path / "guide.json", guide)
        _write(path / "verification.json", verification)
        _write(path / "receipt.json", receipt)
        paths.append(str(path))
        hashes.append(receipt["guide_sha256"])
    passed = len(set(hashes)) == 1 and all(json.loads((Path(path) / "verification.json").read_text())["passed"] for path in paths)
    preview = root / "preview.png"
    if passed:
        render(accepted, preview)
    summary = {
        "schema": "anatomical-guide-run-summary/0",
        "body_plan_id": body["body_plan_id"],
        "guide_id": recipe["id"],
        "passed": passed,
        "guide_sha256": hashes[0],
        "runs": paths,
        "preview": str(preview) if passed else None,
    }
    _write(root / "summary.json", summary)
    return summary


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("body_run", type=Path)
    parser.add_argument("recipe", type=Path)
    parser.add_argument("--output-root", type=Path, required=True)
    args = parser.parse_args()
    summary = run(args.body_run, args.recipe, args.output_root)
    print(f"ANATOMICAL_GUIDE_RUN_{'PASS' if summary['passed'] else 'FAIL'} guide_sha256={summary['guide_sha256']}")
    raise SystemExit(0 if summary["passed"] else 1)


if __name__ == "__main__":
    main()
