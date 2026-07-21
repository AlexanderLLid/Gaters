import argparse
import copy
import json
import math
from pathlib import Path

from foot_placement_policy import evaluate as evaluate_grounding


EXPECTED_SOURCE_NODE = "FAnimNode_SequencePlayer_Standalone"


def _issue(issues, rule_id, message):
    issues.append({"ruleId": rule_id, "message": message})


def _number(value):
    try:
        result = float(value)
    except (TypeError, ValueError):
        return math.nan
    return result if math.isfinite(result) else math.nan


def _vector(value):
    if not isinstance(value, list) or len(value) != 3:
        return None
    result = tuple(_number(item) for item in value)
    return result if all(math.isfinite(item) for item in result) else None


def _distance(left, right):
    return math.sqrt(sum((a - b) ** 2 for a, b in zip(left, right)))


def evaluate(report, contract):
    grounding_report = copy.deepcopy(report)
    grounding_report["nativeNodes"] = {
        key: report.get("nativeNodes", {}).get(key)
        for key in ("footPlacement", "legSolver", "movement")
    }
    grounding = evaluate_grounding(grounding_report, contract)
    issues = list(grounding["issues"])

    if report.get("animationEvidenceVersion") != 1:
        _issue(issues, "animated_foot_placement.version", "Animation evidence version must be 1.")

    if report.get("nativeNodes", {}).get("source") != EXPECTED_SOURCE_NODE:
        _issue(
            issues,
            "animated_foot_placement.native_source",
            "The grounded graph must use Unreal's standalone native Sequence Player.",
        )

    source = report.get("sourceAnimation", {})
    if (
        source.get("asset") != contract.get("generatedWalkClip")
        or source.get("name") != "A_Walk"
        or source.get("looping") is not True
    ):
        _issue(
            issues,
            "animated_foot_placement.source_animation",
            "The source must be the generated looping A_Walk asset declared by the contract.",
        )

    samples = report.get("animationSamples", [])
    valid_samples = (
        len(samples) == 2
        and [sample.get("name") for sample in samples] == ["phase_a", "phase_b"]
    )
    times = [_number(sample.get("assetTimeSeconds")) for sample in samples] if valid_samples else []
    left = [_vector(sample.get("leftThighDirection")) for sample in samples] if valid_samples else []
    right = [_vector(sample.get("rightThighDirection")) for sample in samples] if valid_samples else []
    valid_samples = valid_samples and all(math.isfinite(value) for value in times) and all(left) and all(right)
    if not valid_samples:
        _issue(
            issues,
            "animated_foot_placement.samples",
            "Two finite opposed-phase animation samples are required.",
        )

    time_advance = times[1] - times[0] if valid_samples else 0.0
    left_delta = _distance(left[0], left[1]) if valid_samples else 0.0
    right_delta = _distance(right[0], right[1]) if valid_samples else 0.0
    if time_advance < float(contract.get("minimumAnimationTimeAdvanceSeconds", -1.0)):
        _issue(issues, "animated_foot_placement.time", "Generated walk asset time did not advance enough.")
    minimum_pose_delta = float(contract.get("minimumAnimatedThighDirectionDelta", -1.0))
    if left_delta < minimum_pose_delta or right_delta < minimum_pose_delta:
        _issue(
            issues,
            "animated_foot_placement.pose",
            "Both evaluated thigh directions must change across opposed gait phases.",
        )

    metrics = dict(grounding["metrics"])
    metrics.update(
        {
            "animationTimeAdvanceSeconds": round(time_advance, 6),
            "leftThighDirectionDelta": round(left_delta, 6),
            "rightThighDirectionDelta": round(right_delta, 6),
        }
    )
    return {
        "schemaVersion": 1,
        "evaluatorVersion": 1,
        "passed": not issues,
        "metrics": metrics,
        "issues": issues,
    }


def main():
    parser = argparse.ArgumentParser(description="Evaluate animated native Foot Placement evidence.")
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
    print(f"Animated Foot Placement policy: {'PASS' if result['passed'] else 'FAIL'}")
    for issue in result["issues"]:
        print(f"- {issue['ruleId']}: {issue['message']}")
    return 0 if result["passed"] else 2


if __name__ == "__main__":
    raise SystemExit(main())
