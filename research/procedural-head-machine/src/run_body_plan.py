import argparse
import hashlib
import json
from datetime import datetime, timezone
from pathlib import Path

from body_plan_compiler import compile_body_plan
from body_plan_verifier import verify_body_plan
from render_body_plan_preview import render


def _canonical(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"), allow_nan=False).encode("utf-8")


def _sha256(value):
    return hashlib.sha256(_canonical(value)).hexdigest()


def _write(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True, allow_nan=False) + "\n", encoding="utf-8")


def run(plan_path, output_root, repeat=2):
    plan_path, output_root = map(Path, (plan_path, output_root))
    plan = json.loads(plan_path.read_text(encoding="utf-8"))
    stamp = datetime.now(timezone.utc).strftime("%Y%m%d-%H%M%S-%f")
    root = output_root / f"{plan['id']}-{stamp}"
    root.mkdir(parents=True, exist_ok=False)
    paths, hashes = [], []
    accepted_body = None
    for index in range(1, repeat + 1):
        path = root / f"run-{index}"
        path.mkdir()
        body = compile_body_plan(plan)
        accepted_body = body
        verification = verify_body_plan(plan, body)
        receipt = {
            "schema": "body-plan-receipt/0",
            "compiler": "body-plan-compiler/0",
            "verifier": "body-plan-verifier/0",
            "plan_sha256": _sha256(plan),
            "composition_sha256": _sha256(body),
            "topology_sha256": _sha256(body["faces"]),
        }
        _write(path / "body-plan.json", plan)
        _write(path / "composed-mesh.json", body)
        _write(path / "verification.json", verification)
        _write(path / "receipt.json", receipt)
        lines = [f"v {x} {y} {z}" for x, y, z in body["vertices"]]
        lines.extend("f " + " ".join(str(vertex + 1) for vertex in face) for face in body["faces"])
        (path / "stick-humanoid.obj").write_text("\n".join(lines) + "\n", encoding="utf-8")
        paths.append(str(path))
        hashes.append(receipt["composition_sha256"])
    summary = {
        "schema": "body-plan-run-summary/0",
        "body_plan_id": plan["id"],
        "passed": len(set(hashes)) == 1 and all(
            json.loads((Path(path) / "verification.json").read_text(encoding="utf-8"))["passed"]
            for path in paths
        ),
        "composition_sha256": hashes[0],
        "runs": paths,
    }
    preview = root / "preview.png"
    if summary["passed"]:
        render(accepted_body, preview)
        summary["preview"] = str(preview)
    _write(root / "summary.json", summary)
    return summary


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("plan", type=Path)
    parser.add_argument("--output-root", type=Path, required=True)
    parser.add_argument("--repeat", type=int, default=2)
    args = parser.parse_args()
    summary = run(args.plan, args.output_root, args.repeat)
    print(
        f"BODY_PLAN_{'PASS' if summary['passed'] else 'FAIL'} "
        f"plan={summary['body_plan_id']} composition_sha256={summary['composition_sha256']}"
    )
    raise SystemExit(0 if summary["passed"] else 1)


if __name__ == "__main__":
    main()
