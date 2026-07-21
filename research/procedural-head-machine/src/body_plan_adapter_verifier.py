import argparse
import json
import math
from pathlib import Path


def verify_body_plan_adapter(body, readback, tolerance=1e-7):
    failures = []
    source_vertices = body.get("vertices", [])
    result_vertices = readback.get("vertices", [])
    max_position_delta = float("inf")
    if len(source_vertices) == len(result_vertices):
        max_position_delta = max(
            (math.dist(source, result) for source, result in zip(source_vertices, result_vertices)),
            default=0.0,
        )
    if not math.isfinite(max_position_delta) or max_position_delta > tolerance:
        failures.append({"rule": "BODY-ADAPTER-POSITION-1", "subject": "vertices"})
    if body.get("faces") != readback.get("faces"):
        failures.append({"rule": "BODY-ADAPTER-TOPOLOGY-1", "subject": "faces"})

    source_regions = body.get("regions", [])
    result_regions = readback.get("regions", [])
    max_region_delta = float("inf")
    if len(source_regions) == len(result_regions):
        try:
            if all(source.keys() == result.keys() for source, result in zip(source_regions, result_regions)):
                max_region_delta = max(
                    abs(source[name] - result[name])
                    for source, result in zip(source_regions, result_regions)
                    for name in source
                )
        except (AttributeError, KeyError, TypeError):
            max_region_delta = float("inf")
    labels_match = all(body.get(key) == readback.get(key) for key in ("vertex_modules", "face_modules"))
    if not labels_match or not math.isfinite(max_region_delta) or max_region_delta > tolerance:
        failures.append({"rule": "BODY-ADAPTER-SEMANTIC-1", "subject": "labels"})

    metadata_keys = ("placements", "connections", "cells", "mirrors")
    if any(body.get(key) != readback.get(key) for key in metadata_keys):
        failures.append({"rule": "BODY-ADAPTER-METADATA-1", "subject": "body plan"})
    return {
        "schema": "body-plan-adapter-verification/0",
        "passed": not failures,
        "failures": failures,
        "metrics": {
            "vertex_count": len(result_vertices),
            "face_count": len(readback.get("faces", [])),
            "max_position_delta_m": max_position_delta,
            "max_region_delta": max_region_delta,
        },
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("body_run", type=Path)
    parser.add_argument("readback", type=Path)
    parser.add_argument("report", type=Path)
    args = parser.parse_args()
    body = json.loads((args.body_run / "composed-mesh.json").read_text(encoding="utf-8"))
    readback = json.loads(args.readback.read_text(encoding="utf-8"))
    report = verify_body_plan_adapter(body, readback)
    args.report.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"BODY_PLAN_ADAPTER_{'PASS' if report['passed'] else 'FAIL'} report={args.report}")
    raise SystemExit(0 if report["passed"] else 1)


if __name__ == "__main__":
    main()
