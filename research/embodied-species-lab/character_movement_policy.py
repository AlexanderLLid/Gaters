import argparse
import json
import math
from pathlib import Path


SCHEMA_VERSION = 1
EVALUATOR_VERSION = 1
CHARACTER_LAB = "/Game/Gaters/Generated/CharacterLab"
EXPECTED_ASSETS = {
    "skeletalMesh": f"{CHARACTER_LAB}/SK_GeneratedHumanoid.SK_GeneratedHumanoid",
    "skeleton": f"{CHARACTER_LAB}/SK_GeneratedHumanoid_Skeleton.SK_GeneratedHumanoid_Skeleton",
    "physicsAsset": f"{CHARACTER_LAB}/SK_GeneratedHumanoid_PhysicsAsset.SK_GeneratedHumanoid_PhysicsAsset",
    "ikRig": f"{CHARACTER_LAB}/IK_GeneratedHumanoid.IK_GeneratedHumanoid",
}


def _issue(issues, rule_id, message):
    issues.append({"ruleId": rule_id, "message": message})


def _sample_by_phase(report):
    return {sample.get("phase"): sample for sample in report.get("samples", [])}


def _horizontal_speed(sample):
    velocity = sample.get("velocityCentimetersPerSecond", [0.0, 0.0])
    return math.hypot(float(velocity[0]), float(velocity[1])) if len(velocity) >= 2 else 0.0


def evaluate_character_movement(report, contract):
    issues = []
    required_phases = contract.get("requiredPhases", [])
    required_clips = contract.get("requiredClips", [])
    assets = report.get("assets", {})
    runtime = report.get("runtime", {})
    samples = report.get("samples", [])
    by_phase = _sample_by_phase(report)
    final = report.get("final", {})

    if (
        contract.get("schemaVersion") != 1
        or contract.get("routeVersion") != 1
        or report.get("schemaVersion") != 1
        or report.get("challengerVersion") != 1
        or report.get("routeVersion") != contract.get("routeVersion")
    ):
        _issue(issues, "movement.input.version", "Unsupported movement contract or report version.")

    if report.get("map") != f"{CHARACTER_LAB}/L_CharacterLab":
        _issue(issues, "movement.assets.map", "The route did not run in the generated CharacterLab map.")

    if any(assets.get(key) != value for key, value in EXPECTED_ASSETS.items()):
        _issue(issues, "movement.assets.generated", "Generated cooked asset references do not match CharacterLab.")

    expected_clip_paths = {
        phase: f"{CHARACTER_LAB}/{clip}.{clip}"
        for phase, clip in zip(required_phases, required_clips)
    }
    if assets.get("clips") != expected_clip_paths:
        _issue(issues, "movement.assets.clips", "Cooked clip references do not exactly match the route contract.")

    if report.get("observedPhases") != required_phases:
        _issue(issues, "movement.route.order", "Movement phases were not observed in the required order.")

    if (
        runtime.get("actorClass") != "Character"
        or runtime.get("movementComponentClass") != "CharacterMovementComponent"
        or runtime.get("updatedComponentClass") != "CapsuleComponent"
    ):
        _issue(issues, "movement.runtime.native_authority", "ACharacter, CharacterMovement, and the capsule must own movement.")

    if runtime.get("editorAdapterLoaded") is not False:
        _issue(issues, "movement.runtime.editor_boundary", "The editor-only intake adapter was loaded at runtime.")

    expected_sample_clips = dict(zip(required_phases, required_clips))
    if (
        set(by_phase) != set(required_phases)
        or any(by_phase[phase].get("clip") != clip for phase, clip in expected_sample_clips.items() if phase in by_phase)
    ):
        _issue(issues, "movement.route.samples", "Every route phase must have one matching clip sample.")

    grounded_phases = ("idle", "walk", "run", "turn", "stop", "land")
    if any(
        by_phase.get(phase, {}).get("movementMode") != "Walking"
        or by_phase.get(phase, {}).get("hasWalkableFloor") is not True
        for phase in grounded_phases
    ):
        _issue(issues, "movement.floor.grounded", "Every grounded route phase must use walking mode on a walkable floor.")

    falling_samples = [
        sample
        for sample in samples
        if sample.get("movementMode") == "Falling" and sample.get("hasWalkableFloor") is False
    ]
    if (
        len(falling_samples) < int(contract.get("minimumFallingSamples", 1))
        or by_phase.get("land", {}).get("movementMode") != "Walking"
    ):
        _issue(issues, "movement.floor.fall_land", "The route must enter falling and then return to walking on landing.")

    idle_x = float(by_phase.get("idle", {}).get("locationCentimeters", [0.0, 0.0])[0])
    run_x = float(by_phase.get("run", {}).get("locationCentimeters", [0.0, 0.0])[0])
    if run_x - idle_x < float(contract.get("minimumForwardDisplacementCentimeters", 0.0)):
        _issue(issues, "movement.route.forward", "Native walking and running did not produce enough forward displacement.")

    run_y = float(by_phase.get("run", {}).get("locationCentimeters", [0.0, 0.0])[1])
    turn_y = float(by_phase.get("turn", {}).get("locationCentimeters", [0.0, 0.0])[1])
    if abs(turn_y - run_y) < float(contract.get("minimumTurnDisplacementCentimeters", 0.0)):
        _issue(issues, "movement.route.turn", "Native turning did not produce enough lateral displacement.")

    walk_speed = _horizontal_speed(by_phase.get("walk", {}))
    run_speed = _horizontal_speed(by_phase.get("run", {}))
    speed_fraction = float(contract.get("minimumGroundedSpeedFraction", 1.0))
    if (
        walk_speed < float(contract.get("walkSpeedCentimetersPerSecond", 0.0)) * speed_fraction
        or run_speed < float(contract.get("runSpeedCentimetersPerSecond", 0.0)) * speed_fraction
    ):
        _issue(issues, "movement.route.speeds", "Measured walking or running speed did not reach the route contract.")

    if (
        final.get("movementMode") != "Walking"
        or final.get("hasWalkableFloor") is not True
        or float(final.get("speedCentimetersPerSecond", float("inf")))
        > float(contract.get("maximumFinalSpeedCentimetersPerSecond", 0.0))
    ):
        _issue(issues, "movement.route.final", "The route did not finish stopped on a walkable floor.")

    return {
        "schemaVersion": SCHEMA_VERSION,
        "evaluatorVersion": EVALUATOR_VERSION,
        "passed": not issues,
        "checks": {
            "phaseCount": len(report.get("observedPhases", [])),
            "sampleCount": len(samples),
            "fallingSampleCount": len(falling_samples),
            "forwardDisplacementCentimeters": round(run_x - idle_x, 6),
            "turnDisplacementCentimeters": round(abs(turn_y - run_y), 6),
            "walkSpeedCentimetersPerSecond": round(walk_speed, 6),
            "runSpeedCentimetersPerSecond": round(run_speed, 6),
        },
        "issues": issues,
    }


def _load(path):
    return json.loads(path.read_text(encoding="utf-8"))


def main():
    parser = argparse.ArgumentParser(description="Evaluate native CharacterMovement route evidence.")
    parser.add_argument("--report", required=True, type=Path)
    parser.add_argument("--contract", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    result = evaluate_character_movement(_load(args.report), _load(args.contract))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print("PASS character-movement-policy" if result["passed"] else "FAIL character-movement-policy")
    for issue in result["issues"]:
        print(f"{issue['ruleId']}: {issue['message']}")
    return 0 if result["passed"] else 2


if __name__ == "__main__":
    raise SystemExit(main())
