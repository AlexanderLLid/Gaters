import argparse
import hashlib
import json
import platform
from datetime import datetime, timezone
from pathlib import Path

from head_compiler import compile_recipe
from head_verifier import verify


def _canonical(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"), allow_nan=False).encode("utf-8")


def _sha256(value):
    return hashlib.sha256(_canonical(value)).hexdigest()


def _write_json(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True, allow_nan=False) + "\n", encoding="utf-8")


def run(recipe_path, output_root, repeat=2):
    recipe_path = Path(recipe_path)
    output_root = Path(output_root)
    recipe = json.loads(recipe_path.read_text(encoding="utf-8"))
    stamp = datetime.now(timezone.utc).strftime("%Y%m%d-%H%M%S-%f")
    root = output_root / f"{recipe['id']}-{stamp}"
    root.mkdir(parents=True, exist_ok=False)
    run_paths = []
    mesh_hashes = []

    for run_index in range(1, repeat + 1):
        run_path = root / f"run-{run_index}"
        run_path.mkdir()
        mesh = compile_recipe(recipe)
        verification = verify(recipe, mesh)
        mesh_artifact = {key: mesh[key] for key in ("schema", "recipe_id", "vertices", "faces")}
        regions_artifact = {"schema": "head-regions/0", "weights": mesh["regions"]}
        receipt = {
            "schema": "head-receipt/0",
            "compiler": "head-compiler/0",
            "verifier": "head-verifier/0",
            "python": platform.python_version(),
            "recipe_sha256": _sha256(recipe),
            "mesh_sha256": _sha256(mesh),
            "topology_sha256": _sha256(mesh["faces"]),
        }
        _write_json(run_path / "recipe.json", recipe)
        _write_json(run_path / "mesh.json", mesh_artifact)
        _write_json(run_path / "regions.json", regions_artifact)
        _write_json(run_path / "verification.json", verification)
        _write_json(run_path / "receipt.json", receipt)
        obj_lines = [f"v {x} {y} {z}" for x, y, z in mesh["vertices"]]
        obj_lines.extend("f " + " ".join(str(index + 1) for index in face) for face in mesh["faces"])
        (run_path / "head.obj").write_text("\n".join(obj_lines) + "\n", encoding="utf-8")
        run_paths.append(str(run_path))
        mesh_hashes.append(receipt["mesh_sha256"])

    summary = {
        "schema": "head-run-summary/0",
        "recipe_id": recipe["id"],
        "passed": len(set(mesh_hashes)) == 1 and all(
            json.loads((Path(path) / "verification.json").read_text(encoding="utf-8"))["passed"]
            for path in run_paths
        ),
        "mesh_sha256": mesh_hashes[0],
        "runs": run_paths,
    }
    _write_json(root / "summary.json", summary)
    return summary


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("recipe", type=Path)
    parser.add_argument("--output-root", type=Path, required=True)
    parser.add_argument("--repeat", type=int, default=2)
    args = parser.parse_args()
    summary = run(args.recipe, args.output_root, args.repeat)
    status = "PASS" if summary["passed"] else "FAIL"
    print(
        f"PROCEDURAL_HEAD_{status} recipe={summary['recipe_id']} "
        f"mesh_sha256={summary['mesh_sha256']} runs={len(summary['runs'])}"
    )
    raise SystemExit(0 if summary["passed"] else 1)


if __name__ == "__main__":
    main()
