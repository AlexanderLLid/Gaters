"""Evaluate native Unreal motion evidence against its authoritative source manifest."""

import argparse
import hashlib
import json
from pathlib import Path


EVALUATOR_VERSION = 2
TOLERANCE = 0.01
FOOT_SLIDE_LIMIT_CM = 2.0


def load_json(path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def issue(issues, rule_id, message):
    issues.append({"ruleId": rule_id, "message": message})


def hierarchy_matches(manifest, report):
    expected = manifest["skeleton"]["bones"]
    expected_names = [bone["name"] for bone in expected]
    if report.get("boneNames") != expected_names:
        return False
    actual = report.get("requiredReferenceHierarchy", [])
    actual_by_name = {bone.get("name"): bone for bone in actual}
    for bone in expected:
        record = actual_by_name.get(bone["name"])
        if not record or record.get("parent") != bone.get("parent"):
            return False
        children = sorted(item["name"] for item in expected if item.get("parent") == bone["name"])
        if sorted(record.get("children", [])) != children:
            return False
    return len(actual) == len(expected)


def root_path_matches(manifest, report):
    expected = manifest["clip"]["rootSamples"]
    actual = report.get("rootSamples", [])
    if len(actual) != len(expected):
        return False
    for source, measured in zip(expected, actual):
        expected_cm = [component * 100.0 for component in source["locationMeters"]]
        actual_cm = measured.get("translationCentimeters", [])
        if len(actual_cm) != 3 or any(
            abs(actual_cm[index] - expected_cm[index]) > TOLERANCE for index in range(3)
        ):
            return False
    return True


def events_match(manifest, report):
    events = manifest["clip"].get("events", [])
    if events != report.get("sourceContactEvents", []):
        return False
    start = manifest["clip"]["startFrame"]
    end = manifest["clip"]["endFrame"]
    names = {event.get("name") for event in events}
    return (
        events == sorted(events, key=lambda event: event.get("frame", -1))
        and all(event.get("name") and start <= event.get("frame", -1) <= end for event in events)
        and {"contact.left", "contact.right"}.issubset(names)
    )


def evaluate_foot_sliding(manifest, report):
    clip = manifest["clip"]
    start_frame = clip["startFrame"]
    events = clip.get("events", [])
    samples_by_foot = report.get("footWorldSamples", {})
    stances = []
    for event, next_event in zip(events, events[1:]):
        foot_name = {"contact.left": "foot_l", "contact.right": "foot_r"}.get(event["name"])
        samples = samples_by_foot.get(foot_name, []) if foot_name else []
        start_index = event["frame"] - start_frame
        end_index = next_event["frame"] - start_frame
        if start_index < 0 or end_index >= len(samples) or start_index >= end_index:
            return False, stances
        origin = samples[start_index]["translationCentimeters"]
        maximum = 0.0
        for sample in samples[start_index:end_index + 1]:
            position = sample.get("translationCentimeters", [])
            if len(position) != 3:
                return False, stances
            drift = ((position[0] - origin[0]) ** 2 + (position[1] - origin[1]) ** 2) ** 0.5
            maximum = max(maximum, drift)
        stances.append({
            "foot": foot_name,
            "startFrame": event["frame"],
            "endFrame": next_event["frame"],
            "maxDriftCentimeters": round(maximum, 4),
            "passed": maximum <= FOOT_SLIDE_LIMIT_CM,
        })
    return len(stances) == max(0, len(events) - 1) and all(
        stance["passed"] for stance in stances), stances


def evaluate(manifest_path, report_path):
    manifest = load_json(manifest_path)
    report = load_json(report_path)
    issues = []
    checks = {}

    manifest_hash = hashlib.sha256(manifest_path.read_bytes()).hexdigest()
    checks["inputVersions"] = (
        manifest.get("schemaVersion") == 1
        and report.get("schemaVersion") == 1
        and report.get("importerVersion") == 2
    )
    if not checks["inputVersions"]:
        issue(issues, "motion.input.version", "manifest or import report version is unsupported")

    checks["provenance"] = report.get("manifestSha256") == manifest_hash
    if not checks["provenance"]:
        issue(issues, "motion.input.provenance", "import report does not match the manifest bytes")

    checks["skeleton"] = hierarchy_matches(manifest, report)
    if not checks["skeleton"]:
        issue(issues, "motion.skeleton.bones", "native Unreal hierarchy differs from the manifest")

    clip = manifest["clip"]
    expected_duration = clip["durationSeconds"]
    expected_keys = clip["endFrame"] - clip["startFrame"] + 1
    checks["timing"] = (
        abs(report.get("durationSeconds", -1.0) - expected_duration) <= 0.001
        and report.get("sampledKeys", 0) >= expected_keys
    )
    if not checks["timing"]:
        issue(issues, "motion.timing.duration", "native duration or sampled-key coverage differs from the manifest")

    checks["rootPath"] = root_path_matches(manifest, report)
    if not checks["rootPath"]:
        issue(issues, "motion.root.path", "native root samples differ from the manifest")

    checks["gameplayEvents"] = events_match(manifest, report)
    if not checks["gameplayEvents"]:
        issue(issues, "motion.events.required", "required contact events were lost, changed, or invalid")

    foot_sliding_passed, foot_stances = evaluate_foot_sliding(manifest, report)
    checks["footSliding"] = checks["gameplayEvents"] and foot_sliding_passed
    if checks["gameplayEvents"] and not checks["footSliding"]:
        issue(issues, "motion.contact.foot_sliding", "a planted foot exceeds the component-space drift limit")

    fps = clip["fps"]
    start = clip["startFrame"]
    gameplay_events = [
        {
            "name": event["name"],
            "frame": event["frame"],
            "timeSeconds": round((event["frame"] - start) / fps, 6),
            "source": "motion-manifest",
        }
        for event in clip.get("events", [])
    ] if checks["gameplayEvents"] else []

    return {
        "schemaVersion": 1,
        "evaluatorVersion": EVALUATOR_VERSION,
        "candidateId": manifest.get("stableIdentity", manifest.get("assetId", "unknown")),
        "manifestSha256": manifest_hash,
        "importReportSha256": hashlib.sha256(report_path.read_bytes()).hexdigest(),
        "passed": not issues,
        "checks": checks,
        "issues": issues,
        "gameplayEvents": gameplay_events,
        "footSliding": {
            "limitCentimeters": FOOT_SLIDE_LIMIT_CM,
            "stances": foot_stances,
        },
        "unrealNotifyCount": report.get("unrealNotifyCount", 0),
        "eventTransport": "versioned-sidecar",
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--import-report", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    result = evaluate(args.manifest.resolve(), args.import_report.resolve())
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(
        f"GATERS_MOTION_MECHANICAL passed={str(result['passed']).lower()} "
        f"issues={len(result['issues'])} candidate={result['candidateId']}"
    )
    raise SystemExit(0 if result["passed"] else 2)


if __name__ == "__main__":
    main()
