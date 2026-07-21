"""Fresh-reopen verifier for the minimal raw design sculpt."""

import argparse
import json
import sys
from pathlib import Path

import bpy


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True)
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    args = parser.parse_args(argv)
    output = Path(args.output).resolve()
    head = bpy.data.objects.get("DesignSculpt")
    failures = []
    if head is None or head.type != "MESH":
        failures.append("DesignSculpt mesh missing")
    if any(obj.type == "ARMATURE" for obj in bpy.data.objects):
        failures.append("armature must not exist in isolated art proof")
    if any("mpfb" in obj.name.lower() for obj in bpy.data.objects):
        failures.append("MPFB object detected")
    for name in ("front.png", "three-quarter.png", "profile.png"):
        if not (output / name).is_file():
            failures.append(f"missing render: {name}")
    report = {
        "schemaVersion": 1,
        "decision": "mechanical-pass" if not failures else "reject",
        "humanVisualDecision": "pending",
        "headVertices": len(head.data.vertices) if head else 0,
        "headFaces": len(head.data.polygons) if head else 0,
        "armatures": sum(obj.type == "ARMATURE" for obj in bpy.data.objects),
        "failures": failures,
    }
    (output / "verification.json").write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(json.dumps(report, sort_keys=True))
    if failures:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
