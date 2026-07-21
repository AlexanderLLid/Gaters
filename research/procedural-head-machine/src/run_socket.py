import argparse
import hashlib
import json
from datetime import datetime, timezone
from pathlib import Path

from socket_compiler import compose_head_neck
from socket_verifier import verify_socket


def _canonical(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"), allow_nan=False).encode("utf-8")


def _sha256(value):
    return hashlib.sha256(_canonical(value)).hexdigest()


def _write(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True, allow_nan=False) + "\n", encoding="utf-8")


def run(source_run, socket_path, output_root, repeat=2):
    source_run, socket_path, output_root = map(Path, (source_run, socket_path, output_root))
    mesh = json.loads((source_run / "mesh.json").read_text(encoding="utf-8"))
    regions = json.loads((source_run / "regions.json").read_text(encoding="utf-8"))
    source_receipt = json.loads((source_run / "receipt.json").read_text(encoding="utf-8"))
    socket = json.loads(socket_path.read_text(encoding="utf-8"))
    stamp = datetime.now(timezone.utc).strftime("%Y%m%d-%H%M%S-%f")
    root = output_root / f"{socket['id']}-{stamp}"
    root.mkdir(parents=True, exist_ok=False)
    paths, hashes = [], []

    for index in range(1, repeat + 1):
        path = root / f"run-{index}"
        path.mkdir()
        composition = compose_head_neck(mesh, regions, socket)
        verification = verify_socket(mesh, socket, composition)
        receipt = {
            "schema": "module-socket-receipt/0",
            "compiler": "socket-compiler/0",
            "verifier": "socket-verifier/0",
            "source_mesh_sha256": source_receipt["mesh_sha256"],
            "socket_sha256": _sha256(socket),
            "composition_sha256": _sha256(composition),
            "topology_sha256": _sha256(composition["faces"]),
        }
        _write(path / "socket.json", socket)
        _write(path / "composed-mesh.json", composition)
        _write(path / "verification.json", verification)
        _write(path / "receipt.json", receipt)
        lines = [f"v {x} {y} {z}" for x, y, z in composition["vertices"]]
        lines.extend("f " + " ".join(str(vertex + 1) for vertex in face) for face in composition["faces"])
        (path / "head-neck.obj").write_text("\n".join(lines) + "\n", encoding="utf-8")
        paths.append(str(path))
        hashes.append(receipt["composition_sha256"])

    summary = {
        "schema": "module-socket-run-summary/0",
        "socket_id": socket["id"],
        "passed": len(set(hashes)) == 1 and all(
            json.loads((Path(path) / "verification.json").read_text(encoding="utf-8"))["passed"]
            for path in paths
        ),
        "composition_sha256": hashes[0],
        "runs": paths,
    }
    _write(root / "summary.json", summary)
    return summary


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("source_run", type=Path)
    parser.add_argument("socket", type=Path)
    parser.add_argument("--output-root", type=Path, required=True)
    parser.add_argument("--repeat", type=int, default=2)
    args = parser.parse_args()
    summary = run(args.source_run, args.socket, args.output_root, args.repeat)
    print(
        f"HEAD_NECK_SOCKET_{'PASS' if summary['passed'] else 'FAIL'} "
        f"socket={summary['socket_id']} composition_sha256={summary['composition_sha256']}"
    )
    raise SystemExit(0 if summary["passed"] else 1)


if __name__ == "__main__":
    main()
