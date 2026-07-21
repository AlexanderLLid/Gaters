import argparse
import json
import math
from pathlib import Path


def verify_deformation(source_mesh, source_regions, command, readback, tolerance=1e-7):
    failures = []
    valid_command = (
        command.get("schema") == "head-deformation/0"
        and command.get("operation") == "widen_region"
        and command.get("axis") == "x"
        and isinstance(command.get("amount"), (int, float))
        and math.isfinite(command["amount"])
        and command["amount"] >= 0
    )
    if not valid_command:
        return {
            "schema": "head-deformation-verification/0",
            "passed": False,
            "failures": [{"rule": "DEFORM-COMMAND-1", "subject": "command"}],
            "metrics": {},
        }

    vertices = source_mesh.get("vertices", [])
    weights = source_regions.get("weights", [])
    result_vertices = readback.get("vertices", [])
    result_weights = readback.get("regions", [])
    region = command["region"]
    preserve = command["preserve"]
    amount = command["amount"]

    graph = readback.get("graph", {})
    graph_valid = (
        readback.get("deformation_id") == command.get("id")
        and graph.get("operation") == command["operation"]
        and isinstance(graph.get("amount"), (int, float))
        and math.isfinite(graph["amount"])
        and abs(graph["amount"] - amount) <= tolerance
    )
    if not graph_valid:
        failures.append({"rule": "DEFORM-GRAPH-1", "subject": "native_graph"})

    expected = []
    for vertex, vertex_weights in zip(vertices, weights):
        mask = max(vertex_weights[region] - vertex_weights[preserve], 0.0)
        expected.append([vertex[0] * (1.0 + amount * mask), vertex[1], vertex[2]])

    if source_mesh.get("faces") != readback.get("faces"):
        failures.append({"rule": "DEFORM-TOPOLOGY-1", "subject": "faces"})

    max_position_error = float("inf")
    if len(expected) == len(result_vertices):
        max_position_error = max(
            (math.dist(target, actual) for target, actual in zip(expected, result_vertices)),
            default=0.0,
        )
    if not math.isfinite(max_position_error) or max_position_error > tolerance:
        failures.append({"rule": "DEFORM-POSITION-1", "subject": "vertices"})

    protected_ids = [
        index for index, vertex_weights in enumerate(weights)
        if vertex_weights[preserve] == max(vertex_weights.values())
    ]
    max_protected_delta = float("inf")
    if len(vertices) == len(result_vertices):
        max_protected_delta = max(
            (math.dist(vertices[index], result_vertices[index]) for index in protected_ids),
            default=0.0,
        )
    if not math.isfinite(max_protected_delta) or max_protected_delta > tolerance:
        failures.append({"rule": "DEFORM-PRESERVE-1", "subject": preserve})

    active_ids = [
        index for index, vertex_weights in enumerate(weights)
        if vertex_weights[region] > vertex_weights[preserve] and abs(vertices[index][0]) > tolerance
    ]
    max_active_delta = max(
        (abs(expected[index][0] - vertices[index][0]) for index in active_ids),
        default=0.0,
    )
    if amount > 0 and max_active_delta <= tolerance:
        failures.append({"rule": "DEFORM-EFFECT-1", "subject": region})

    max_region_delta = float("inf")
    if len(weights) == len(result_weights):
        try:
            max_region_delta = max(
                abs(source[name] - result[name])
                for source, result in zip(weights, result_weights)
                for name in source
            )
        except (KeyError, TypeError):
            max_region_delta = float("inf")
    if not math.isfinite(max_region_delta) or max_region_delta > tolerance:
        failures.append({"rule": "DEFORM-REGION-1", "subject": "regions"})

    return {
        "schema": "head-deformation-verification/0",
        "passed": not failures,
        "failures": failures,
        "metrics": {
            "max_position_error_m": max_position_error,
            "max_protected_delta_m": max_protected_delta,
            "max_active_delta_m": max_active_delta,
            "max_region_delta": max_region_delta,
        },
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("source_run", type=Path)
    parser.add_argument("command", type=Path)
    parser.add_argument("readback", type=Path)
    parser.add_argument("report", type=Path)
    args = parser.parse_args()
    mesh = json.loads((args.source_run / "mesh.json").read_text(encoding="utf-8"))
    regions = json.loads((args.source_run / "regions.json").read_text(encoding="utf-8"))
    command = json.loads(args.command.read_text(encoding="utf-8"))
    readback = json.loads(args.readback.read_text(encoding="utf-8"))
    report = verify_deformation(mesh, regions, command, readback)
    args.report.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"HEAD_DEFORMATION_{'PASS' if report['passed'] else 'FAIL'} report={args.report}")
    raise SystemExit(0 if report["passed"] else 1)


if __name__ == "__main__":
    main()
