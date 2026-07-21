import argparse
import json
import math
from pathlib import Path


def verify_adapter(source_mesh, source_regions, readback, tolerance=1e-7):
    failures = []
    source_vertices = source_mesh.get("vertices", [])
    result_vertices = readback.get("vertices", [])
    source_faces = source_mesh.get("faces", [])
    result_faces = readback.get("faces", [])
    source_weights = source_regions.get("weights", [])
    result_weights = readback.get("regions", [])

    if source_faces != result_faces:
        failures.append({"rule": "ADAPTER-TOPOLOGY-1", "subject": "faces"})

    max_position_delta = float("inf")
    if len(source_vertices) == len(result_vertices):
        max_position_delta = max(
            (math.dist(source, result) for source, result in zip(source_vertices, result_vertices)),
            default=0.0,
        )
    if not math.isfinite(max_position_delta) or max_position_delta > tolerance:
        failures.append({"rule": "ADAPTER-POSITION-1", "subject": "vertices"})

    max_region_delta = float("inf")
    if len(source_weights) == len(result_weights):
        try:
            max_region_delta = max(
                abs(source[name] - result[name])
                for source, result in zip(source_weights, result_weights)
                for name in source
            )
        except (KeyError, TypeError):
            max_region_delta = float("inf")
    if not math.isfinite(max_region_delta) or max_region_delta > tolerance:
        failures.append({"rule": "ADAPTER-REGION-1", "subject": "regions"})

    return {
        "schema": "head-adapter-verification/0",
        "passed": not failures,
        "failures": failures,
        "metrics": {
            "vertex_count": len(result_vertices),
            "face_count": len(result_faces),
            "max_position_delta_m": max_position_delta,
            "max_region_delta": max_region_delta,
        },
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("source_run", type=Path)
    parser.add_argument("readback", type=Path)
    parser.add_argument("report", type=Path)
    args = parser.parse_args()
    source_mesh = json.loads((args.source_run / "mesh.json").read_text(encoding="utf-8"))
    source_regions = json.loads((args.source_run / "regions.json").read_text(encoding="utf-8"))
    readback = json.loads(args.readback.read_text(encoding="utf-8"))
    report = verify_adapter(source_mesh, source_regions, readback)
    args.report.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"HEAD_ADAPTER_{'PASS' if report['passed'] else 'FAIL'} report={args.report}")
    raise SystemExit(0 if report["passed"] else 1)


if __name__ == "__main__":
    main()
