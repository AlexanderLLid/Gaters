import argparse
import json
import math
from pathlib import Path


SCHEMA_VERSION = 1
EVALUATOR_VERSION = 1
EXPECTED_NODES = {
    "footPlacement": "FAnimNode_FootPlacement",
    "legSolver": "FAnimNode_TwoBoneIK",
    "movement": "CharacterMovementComponent",
}
EXPECTED_ROLES = {
    "ikRoot": "root",
    "pelvis": "pelvis",
    "left": {"fkFoot": "foot_l", "ball": "ball_l", "ikFoot": "ik_foot_l"},
    "right": {"fkFoot": "foot_r", "ball": "ball_r", "ikFoot": "ik_foot_r"},
}


def _issue(issues, rule_id, message):
    issues.append({"ruleId": rule_id, "message": message})


def _number(value, default=math.inf):
    try:
        result = float(value)
    except (TypeError, ValueError):
        return default
    return result if math.isfinite(result) else default


def evaluate(report, contract):
    issues = []
    cases = report.get("cases", [])
    maximum_contact = float(contract.get("maximumAbsoluteContactErrorCentimeters", -1.0))
    maximum_pelvis = float(contract.get("maximumAbsolutePelvisOffsetCentimeters", -1.0))
    minimum_target_delta = float(contract.get("minimumTerrainTargetDeltaCentimeters", -1.0))

    if (
        report.get("schemaVersion") != 1
        or report.get("challengerVersion") != 1
        or contract.get("schemaVersion") != 1
        or report.get("routeVersion") != contract.get("routeVersion")
    ):
        _issue(issues, "foot_placement.version", "Report and route versions must match version 1.")

    if report.get("nativeNodes") != EXPECTED_NODES or report.get("footPlacementExperimental") is not True:
        _issue(
            issues,
            "foot_placement.native_nodes",
            "Evidence must identify native Foot Placement and Two Bone IK and retain the experimental flag.",
        )

    if report.get("editorAdapterLoaded") is not False:
        _issue(issues, "foot_placement.runtime_boundary", "Editor intake adapter loaded during runtime evidence.")

    if report.get("roles") != EXPECTED_ROLES:
        _issue(issues, "foot_placement.roles", "Foot Placement role mapping is incomplete or changed.")

    required_cases = contract.get("requiredCases", [])
    if [item.get("name") for item in cases] != required_cases:
        _issue(issues, "foot_placement.route", "Observed uneven-terrain cases are missing or out of order.")

    floor_passes = bool(cases)
    contact_passes = maximum_contact >= 0.0
    pose_passes = maximum_pelvis >= 0.0
    causality_passes = minimum_target_delta >= 0.0
    measured_contacts = []
    measured_pelvis = []
    measured_deltas = []
    measured_route_x = []
    measured_floor_heights = []
    for item in cases:
        feet = item.get("feet", {})
        floor_passes = floor_passes and (
            item.get("movementMode") == "Walking"
            and item.get("hasWalkableFloor") is True
            and set(feet) == {"left", "right"}
            and all(foot.get("walkableHit") is True for foot in feet.values())
        )

        case_has_response = False
        for foot in feet.values():
            contact = _number(foot.get("contactErrorCentimeters"))
            delta = _number(foot.get("terrainTargetDeltaCentimeters"), -math.inf)
            measured_contacts.append(abs(contact))
            measured_deltas.append(delta)
            contact_passes = contact_passes and abs(contact) <= maximum_contact
            case_has_response = case_has_response or delta >= minimum_target_delta
        if item.get("name") != "stop":
            causality_passes = causality_passes and case_has_response

        pelvis = _number(item.get("pelvisOffsetCentimeters"))
        measured_pelvis.append(abs(pelvis))
        pose_passes = pose_passes and item.get("hasNaN") is False and abs(pelvis) <= maximum_pelvis

        location = item.get("actorLocationCentimeters", [])
        if len(location) == 3:
            measured_route_x.append(_number(location[0]))
        measured_floor_heights.append(_number(item.get("characterFloorHeightCentimeters")))

    if not floor_passes:
        _issue(issues, "foot_placement.floor", "Every case must retain native walking floor and bilateral walkable traces.")
    if not contact_passes:
        _issue(issues, "foot_placement.contact", "A solved foot exceeds the contact-error bound.")
    if not causality_passes:
        _issue(issues, "foot_placement.causality", "A terrain case did not cause a bounded native target response.")
    if not pose_passes:
        _issue(issues, "foot_placement.pose", "Pose contains NaN or exceeds the pelvis-offset bound.")

    forward_displacement = (
        max(measured_route_x) - min(measured_route_x)
        if len(measured_route_x) == len(cases) and measured_route_x else 0.0
    )
    floor_height_delta = (
        max(measured_floor_heights) - min(measured_floor_heights)
        if len(measured_floor_heights) == len(cases) and measured_floor_heights else 0.0
    )
    if (
        forward_displacement < float(contract.get("minimumRouteForwardDisplacementCentimeters", -1.0))
        or floor_height_delta < float(contract.get("minimumCharacterFloorHeightDeltaCentimeters", -1.0))
    ):
        _issue(
            issues,
            "foot_placement.character_movement",
            "CharacterMovement did not traverse the required distance and uneven floor-height range.",
        )

    final = cases[-1] if cases else {}
    if (
        final.get("name") != "stop"
        or _number(final.get("speedCentimetersPerSecond"))
        > float(contract.get("maximumFinalSpeedCentimetersPerSecond", -1.0))
    ):
        _issue(issues, "foot_placement.finish", "Final stop must be grounded and within the speed bound.")

    return {
        "schemaVersion": SCHEMA_VERSION,
        "evaluatorVersion": EVALUATOR_VERSION,
        "passed": not issues,
        "metrics": {
            "caseCount": len(cases),
            "maximumAbsoluteContactErrorCentimeters": round(max(measured_contacts, default=0.0), 6),
            "maximumAbsolutePelvisOffsetCentimeters": round(max(measured_pelvis, default=0.0), 6),
            "maximumTerrainTargetDeltaCentimeters": round(max(measured_deltas, default=0.0), 6),
            "routeForwardDisplacementCentimeters": round(forward_displacement, 6),
            "characterFloorHeightDeltaCentimeters": round(floor_height_delta, 6),
        },
        "issues": issues,
    }


def main():
    parser = argparse.ArgumentParser(description="Evaluate native uneven-terrain Foot Placement evidence.")
    parser.add_argument("--report", required=True, type=Path)
    parser.add_argument("--contract", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    result = evaluate(
        json.loads(args.report.read_text(encoding="utf-8")),
        json.loads(args.contract.read_text(encoding="utf-8")),
    )
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"Foot Placement policy: {'PASS' if result['passed'] else 'FAIL'}")
    for issue in result["issues"]:
        print(f"- {issue['ruleId']}: {issue['message']}")
    return 0 if result["passed"] else 2


if __name__ == "__main__":
    raise SystemExit(main())
