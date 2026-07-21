import argparse
import hashlib
import json
import math
from pathlib import Path


def _max_position_delta(expected, actual):
    if len(expected) != len(actual):
        return float("inf")
    maximum = 0.0
    for left, right in zip(expected, actual):
        if len(left) != 3 or len(right) != 3 or not all(isinstance(value, (int, float)) and math.isfinite(value) for value in left + right):
            return float("inf")
        maximum = max(maximum, math.dist(left, right))
    return maximum


def _sha256(value):
    try:
        canonical = json.dumps(value, sort_keys=True, separators=(",", ":"), allow_nan=False).encode("utf-8")
    except (TypeError, ValueError):
        return None
    return hashlib.sha256(canonical).hexdigest()


def verify_motion_sequence(source, motion, readback, motion_sha256, source_scene_sha256, tolerance=1e-6, maximum_frame_surface_delta_m=0.25):
    failures = []
    computed_motion_sha256 = _sha256(motion)
    if readback.get("faces") != source.get("faces") or readback.get("modules") != source.get("modules") or _max_position_delta(source.get("positions", []), readback.get("positions", [])) > tolerance:
        failures.append({"rule": "MOTION-SEQUENCE-TOPOLOGY-1", "subject": "captured rest surface"})

    frames = readback.get("frames", [])
    expected_frames = motion.get("frames", [])
    if len(frames) != len(expected_frames):
        failures.append({"rule": "MOTION-SEQUENCE-FRAMES-1", "subject": "frame count"})
    if readback.get("frame_range") != [1.0, float(len(expected_frames))]:
        failures.append({"rule": "MOTION-SEQUENCE-FRAMES-1", "subject": "native playback range"})
    maximum_surface_motion = 0.0
    maximum_frame_delta = 0.0
    maximum_skeleton_delta = 0.0
    previous_positions = None
    for expected, actual in zip(expected_frames, frames):
        if actual.get("index") != expected["index"] or abs(actual.get("time_seconds", float("inf")) - expected["time_seconds"]) > tolerance:
            failures.append({"rule": "MOTION-SEQUENCE-FRAMES-1", "subject": f"frame {expected['index']}"})
        positions = actual.get("positions", [])
        maximum_surface_motion = max(maximum_surface_motion, _max_position_delta(source.get("positions", []), positions))
        if previous_positions is not None:
            maximum_frame_delta = max(maximum_frame_delta, _max_position_delta(previous_positions, positions))
        previous_positions = positions
        expected_skeleton = {joint["name"]: joint["position"] for joint in expected["joints"]}
        actual_skeleton = actual.get("skeleton_positions", {})
        if set(expected_skeleton) != set(actual_skeleton):
            maximum_skeleton_delta = float("inf")
        else:
            maximum_skeleton_delta = max(maximum_skeleton_delta, max((math.dist(expected_skeleton[name], actual_skeleton[name]) for name in expected_skeleton), default=0.0))
    if maximum_skeleton_delta > tolerance:
        failures.append({"rule": "MOTION-SEQUENCE-SKELETON-1", "subject": "generated skeleton frames"})
    if maximum_surface_motion <= 0.1:
        failures.append({"rule": "MOTION-SEQUENCE-ACTIVE-1", "subject": "deformed surface"})
    if maximum_frame_delta > maximum_frame_surface_delta_m:
        failures.append({"rule": "MOTION-SEQUENCE-CONTINUITY-1", "subject": "deformed surface"})
    if not frames or _max_position_delta(source.get("positions", []), frames[0].get("positions", [])) > tolerance or _max_position_delta(source.get("positions", []), frames[-1].get("positions", [])) > tolerance:
        failures.append({"rule": "MOTION-SEQUENCE-LOOP-1", "subject": "rest endpoints"})
    if readback.get("schema") != "houdini-motion-readback/0" or computed_motion_sha256 is None or motion_sha256 != computed_motion_sha256 or readback.get("motion_sha256") != computed_motion_sha256 or readback.get("source_skin_scene_sha256") != source_scene_sha256:
        failures.append({"rule": "MOTION-SEQUENCE-PROVENANCE-1", "subject": "inputs"})
    return {
        "schema": "motion-sequence-verification/0",
        "passed": not failures,
        "failures": failures,
        "frame_count": len(frames),
        "maximum_surface_motion_m": maximum_surface_motion,
        "maximum_frame_surface_delta_m": maximum_frame_delta,
        "maximum_skeleton_delta_m": maximum_skeleton_delta,
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("source_readback", type=Path)
    parser.add_argument("source_scene", type=Path)
    parser.add_argument("motion_run", type=Path)
    parser.add_argument("readback", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    source = json.loads(args.source_readback.read_text(encoding="utf-8"))
    motion = json.loads((args.motion_run / "motion.json").read_text(encoding="utf-8"))
    motion_sha256 = json.loads((args.motion_run / "receipt.json").read_text(encoding="utf-8"))["motion_sha256"]
    readback = json.loads(args.readback.read_text(encoding="utf-8"))
    verification = verify_motion_sequence(source, motion, readback, motion_sha256, hashlib.sha256(args.source_scene.read_bytes()).hexdigest())
    args.output.write_text(json.dumps(verification, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"MOTION_SEQUENCE_VERIFY_{'PASS' if verification['passed'] else 'FAIL'} frames={verification['frame_count']} motion={verification['maximum_surface_motion_m']:.6f}")
    raise SystemExit(0 if verification["passed"] else 1)


if __name__ == "__main__":
    main()
