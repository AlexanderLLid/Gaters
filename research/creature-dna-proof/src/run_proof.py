"""Compile, verify, and preserve a CreatureDNA proof run."""

from __future__ import annotations

import argparse
import json
import platform
from pathlib import Path

from compile_graph import compile_recipe, graph_sha256, load_recipe
from verify_graph import verify


def run_recipe(recipe_path: Path, output_dir: Path) -> dict:
    recipe = load_recipe(recipe_path)
    graph = compile_recipe(recipe)
    report = verify(recipe, graph)
    checksum = graph_sha256(graph)
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    _write_json(output_dir / "recipe.json", recipe)
    _write_json(output_dir / "anatomy-graph.json", graph)
    _write_json(output_dir / "verification.json", report)
    _write_obj(output_dir / "guide.obj", graph)
    result = {
        "passed": report["passed"],
        "recipe_id": recipe["id"],
        "graph_sha256": checksum,
        "python": platform.python_version(),
        "houdini": None,
    }
    _write_json(output_dir / "run.json", result)
    return result


def _write_json(path: Path, value: dict) -> None:
    path.write_text(json.dumps(value, indent=2, sort_keys=True, ensure_ascii=False) + "\n", encoding="utf-8")


def _write_obj(path: Path, graph: dict) -> None:
    joints = graph["joints"]
    indices = {joint["id"]: index for index, joint in enumerate(joints, start=1)}
    lines = [f"# CreatureDNA guide: {graph['recipe']['id']}"]
    lines.extend(f"v {x:.6f} {y:.6f} {z:.6f}" for x, y, z in (joint["position_m"] for joint in joints))
    for bone in graph["bones"]:
        lines.append(f"l {indices[bone['parent_joint']]} {indices[bone['child_joint']]}")
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("recipe", type=Path)
    parser.add_argument("output", type=Path)
    arguments = parser.parse_args()
    result = run_recipe(arguments.recipe, arguments.output)
    state = "PASS" if result["passed"] else "FAIL"
    print(f"CREATURE_DNA_{state} recipe={result['recipe_id']} graph_sha256={result['graph_sha256']}")
    return 0 if result["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
