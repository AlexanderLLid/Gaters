import argparse
import hashlib
import json
from pathlib import Path

from render_body_plan_preview import render


def _canonical(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"), allow_nan=False).encode("utf-8")


def _sha256(value):
    return hashlib.sha256(_canonical(value)).hexdigest()


def _write(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True, allow_nan=False) + "\n", encoding="utf-8")


def summarize(body_run, recipe_path, root, guide_run=None):
    body_run, recipe_path, root = map(Path, (body_run, recipe_path, root))
    body = json.loads((body_run / "composed-mesh.json").read_text(encoding="utf-8"))
    body_receipt = json.loads((body_run / "receipt.json").read_text(encoding="utf-8"))
    recipe = json.loads(recipe_path.read_text(encoding="utf-8"))
    guide = json.loads((Path(guide_run) / "guide.json").read_text(encoding="utf-8")) if guide_run else None
    readbacks = [json.loads((root / f"run-{index}" / "readback.json").read_text(encoding="utf-8")) for index in (1, 2)]
    verifications = [json.loads((root / f"run-{index}" / "verification.json").read_text(encoding="utf-8")) for index in (1, 2)]
    hashes = [_sha256(readback) for readback in readbacks]
    passed = len(set(hashes)) == 1 and all(report["passed"] for report in verifications)
    preview = root / "preview.png"
    clay_preview = root / "clay-preview.png"
    if passed:
        render(
            readbacks[0],
            preview,
            "ANATOMICAL MANNEQUIN v0 — ACTUAL GENERATED MESH" if guide else "SMOOTH BODY SURFACE v0 — ACTUAL GENERATED MESH",
            "Guide-driven tapered surface • accepted mechanical checks • no final anatomy or art-style claim" if guide else "Houdini VDB surface • accepted structural checks • no anatomy or art-style claim",
            show_edges=False,
        )
        if guide:
            render(
                readbacks[0],
                clay_preview,
                "PROFILED MID-POLY HUMANOID v0 — ACTUAL GENERATED MESH",
                "Clay-shaded geometry view • visual acceptance pending • final style open",
                show_edges=False,
                solid_color=(160, 166, 174),
            )
    lines = [f"v {x} {y} {z}" for x, y, z in readbacks[0]["vertices"]]
    lines.extend("f " + " ".join(str(index + 1) for index in face) for face in readbacks[0]["faces"])
    (root / "smooth-body.obj").write_text("\n".join(lines) + "\n", encoding="utf-8")
    receipt = {
        "schema": "smooth-body-receipt/0",
        "body_composition_sha256": body_receipt["composition_sha256"],
        "body_plan_sha256": _sha256(body),
        "surface_recipe_sha256": _sha256(recipe),
        "surface_sha256": hashes[0],
        "backend": "houdini",
        "backend_version": readbacks[0]["version"],
    }
    if guide is not None:
        receipt["anatomical_guide_sha256"] = _sha256(guide)
    summary = {
        "schema": "smooth-body-run-summary/0",
        "surface_id": recipe["id"],
        "body_plan_id": body["body_plan_id"],
        "passed": passed,
        "surface_sha256": hashes[0],
        "runs": [str(root / "run-1"), str(root / "run-2")],
        "preview": str(preview) if passed else None,
        "clay_preview": str(clay_preview) if passed and guide else None,
    }
    _write(root / "receipt.json", receipt)
    _write(root / "summary.json", summary)
    print(f"SMOOTH_BODY_RUN_{'PASS' if passed else 'FAIL'} root={root} surface_sha256={hashes[0]}")
    return summary


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("body_run", type=Path)
    parser.add_argument("recipe", type=Path)
    parser.add_argument("root", type=Path)
    parser.add_argument("--guide-run", type=Path)
    args = parser.parse_args()
    summary = summarize(args.body_run, args.recipe, args.root, args.guide_run)
    raise SystemExit(0 if summary["passed"] else 1)


if __name__ == "__main__":
    main()
