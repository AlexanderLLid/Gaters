import argparse
import copy
import json
import math
import sys
from collections import deque
from pathlib import Path


HERE = Path(__file__).resolve().parent
RESOLVER_DIR = HERE.parent / "base-raid-lab"
if str(RESOLVER_DIR) not in sys.path:
    sys.path.insert(0, str(RESOLVER_DIR))

from headless_raid_resolver import run_acceptance_case


EVALUATOR_VERSION = 1


def canonical_json(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"))


def format_summary(results):
    lines = []
    for result in results:
        findings = ",".join(item["code"] for item in result["findings"]) or "none"
        if "selection" in result:
            lines.append(
                f"{result['site']['id']}: evaluation={result['evaluationState']} "
                f"representatives={result['metrics']['representativeCount']} "
                f"simulated={result['metrics']['simulatedRepresentativeCount']} "
                f"findings={findings}"
            )
        elif "evaluationState" in result:
            lines.append(
                f"{result['site']['id']}: evaluation={result['evaluationState']} "
                f"spaces={result['metrics']['spaceCount']} "
                f"connections={result['metrics']['connectionCount']} findings={findings}"
            )
        else:
            lines.append(
                f"{result['caseId']}: viability={result['ratings']['tacticalViability']} "
                f"approaches={result['metrics']['alternateApproachCount']} "
                f"success={result['metrics']['successfulRuns']}/{result['metrics']['totalRuns']} "
                f"findings={findings}"
            )
    return "\n".join(lines)


def by_id(items):
    result = {item["id"]: item for item in items}
    if len(result) != len(items):
        raise ValueError("duplicate stable id")
    return result


def slots_with_tag(site, tag):
    return sorted(
        (slot for slot in site["placementSlots"] if tag in slot["tags"]),
        key=lambda slot: slot["id"],
    )


def distance(a, b):
    return math.sqrt(sum((a[axis] - b[axis]) ** 2 for axis in ("x", "y", "z")))


def connection_fits(connection, spaces, envelope, blockers):
    if any("non-damageable" in blockers[blocker_id]["tags"] for blocker_id in connection["blockerIds"]):
        return False
    return (
        distance(
            spaces[connection["fromSpaceId"]]["positionCm"],
            spaces[connection["toSpaceId"]]["positionCm"],
        )
        <= envelope["perTickMoveDistanceCm"]
        and connection["widthCm"] >= envelope["capsuleRadiusCm"] * 2
        and connection["headroomCm"] >= envelope["capsuleHeightCm"]
        and connection["maxStepCm"] <= envelope["maxStepCm"]
        and connection["maxJumpGapCm"] <= envelope["maxJumpGapCm"]
    )


def shortest_path(connections, start, goal):
    if start == goal:
        return []
    outgoing = {}
    for connection in connections:
        outgoing.setdefault(connection["fromSpaceId"], []).append(connection)
    queue = deque([(start, [])])
    seen = {start}
    while queue:
        space_id, path = queue.popleft()
        for connection in sorted(outgoing.get(space_id, []), key=lambda item: item["id"]):
            next_id = connection["toSpaceId"]
            if next_id in seen:
                continue
            next_path = path + [connection]
            if next_id == goal:
                return next_path
            seen.add(next_id)
            queue.append((next_id, next_path))
    return None


def finding(site, code, severity, subject_ids, message):
    elements = {}
    for key in ("spaces", "connections", "visibility", "blockers", "placementSlots"):
        elements.update(by_id(site[key]))
    source_ids = sorted(
        {
            source_id
            for subject_id in subject_ids
            for source_id in elements.get(subject_id, {}).get("sourceIds", [])
        }
    )
    return {
        "code": code,
        "severity": severity,
        "subjectIds": sorted(subject_ids),
        "sourceIds": source_ids,
        "message": message,
    }


def validate_adapted_site(site):
    elements = [
        item
        for key in ("spaces", "connections", "visibility", "blockers", "placementSlots")
        for item in site[key]
    ]
    ids = [item["id"] for item in elements]
    if len(ids) != len(set(ids)):
        raise ValueError(f"exported site {site['siteId']} has duplicate stable IDs")
    if any(not item["sourceIds"] for item in elements):
        raise ValueError(f"exported site {site['siteId']} has missing source IDs")

    spaces = by_id(site["spaces"])
    blockers = by_id(site["blockers"])
    for connection in site["connections"]:
        if not connection["movementModeIds"]:
            raise ValueError(f"exported connection {connection['id']} has missing movement mode IDs")
        if connection["fromSpaceId"] not in spaces or connection["toSpaceId"] not in spaces:
            raise ValueError(f"exported connection {connection['id']} has a dangling space reference")
        if any(blocker_id not in blockers for blocker_id in connection["blockerIds"]):
            raise ValueError(f"exported connection {connection['id']} has a dangling blocker reference")
    for sight in site["visibility"]:
        if sight["fromSpaceId"] not in spaces or sight["toSpaceId"] not in spaces:
            raise ValueError(f"exported visibility {sight['id']} has a dangling space reference")
        if any(blocker_id not in blockers for blocker_id in sight["blockerIds"]):
            raise ValueError(f"exported visibility {sight['id']} has a dangling blocker reference")
    for slot in site["placementSlots"]:
        if slot["spaceId"] not in spaces:
            raise ValueError(f"exported slot {slot['id']} has a dangling space reference")
    coverage = site["evidenceCoverage"]
    if any(coverage[key] for key in ("placement", "traversalClearance", "visibility", "blockers")):
        if not coverage["sourceIds"]:
            raise ValueError(f"exported site {site['siteId']} has missing evidence coverage source IDs")


def adapt_built_site_export(export_catalog):
    if export_catalog.get("exportVersion") != 1:
        raise ValueError(f"unsupported Built Site export {export_catalog.get('exportVersion')}")
    units = (
        export_catalog.get("coordinateUnit"),
        export_catalog.get("lengthUnit"),
        export_catalog.get("areaUnit"),
    )
    if units != ("centimetres", "centimetres", "square centimetres"):
        raise ValueError(f"unsupported Built Site export units: {units}")

    sites = []
    for recipe in export_catalog.get("siteRecipes", []):
        if recipe.get("contractVersion") != 1:
            raise ValueError(
                f"unsupported Built Site Recipe contract {recipe.get('contractVersion')}"
            )
        coverage = recipe.get("evidenceCoverage", {})
        site = {
            "contractVersion": recipe["contractVersion"],
            "siteVersion": recipe["siteVersion"],
            "generatorVersion": recipe["generatorVersion"],
            "seed": recipe["seed"],
            "siteId": recipe["siteId"],
            "siteKind": recipe["kind"],
            "siteAreaCm2": recipe["siteArea"],
            "checksum": recipe["checksum"],
            "evidenceCoverage": {
                "placement": coverage.get("placement", False),
                "traversalClearance": coverage.get("traversalClearance", False),
                "visibility": coverage.get("visibility", False),
                "blockers": coverage.get("blockers", False),
                "sourceIds": coverage.get("sourceIds", []),
            },
            "spaces": [
                {
                    "id": space["id"],
                    "positionCm": space["center"],
                    "extentCm": space["extent"],
                    "semanticRole": space["semanticRole"],
                    "tags": space["tags"],
                    "sourceIds": space["sourceIds"],
                }
                for space in recipe["spaces"]
            ],
            "connections": [
                {
                    "id": connection["id"],
                    "fromSpaceId": connection["fromSpaceId"],
                    "toSpaceId": connection["toSpaceId"],
                    "widthCm": connection["width"],
                    "headroomCm": connection["headroom"],
                    "maxStepCm": connection["maxStepHeight"],
                    "maxJumpGapCm": connection["maxJumpDistance"],
                    "movementModeIds": connection.get("movementModeIds", []),
                    "blockerIds": connection["blockerIds"],
                    "tags": connection["tags"],
                    "sourceIds": connection["sourceIds"],
                }
                for connection in recipe["connections"]
            ],
            "visibility": [
                {
                    "id": sight["id"],
                    "fromSpaceId": sight["fromSpaceId"],
                    "toSpaceId": sight["toSpaceId"],
                    "distanceCm": sight["distance"],
                    "fromHeightCm": sight["fromHeight"],
                    "toHeightCm": sight["toHeight"],
                    "blockerIds": sight["blockerIds"],
                    "tags": sight["tags"],
                    "sourceIds": sight["sourceIds"],
                }
                for sight in recipe["visibility"]
            ],
            "blockers": [
                {
                    "id": blocker["id"],
                    "centerCm": blocker["center"],
                    "extentCm": blocker["extent"],
                    "tags": blocker["tags"],
                    "sourceIds": blocker["sourceIds"],
                }
                for blocker in recipe["blockers"]
            ],
            "placementSlots": [
                {
                    "id": slot["id"],
                    "spaceId": slot["spaceId"],
                    "positionCm": slot["location"],
                    "clearanceRadiusCm": slot["clearanceRadius"],
                    "clearanceHeightCm": slot["clearanceHeight"],
                    "tags": slot["tags"],
                    "sourceIds": slot["sourceIds"],
                }
                for slot in recipe["placementSlots"]
            ],
        }
        validate_adapted_site(site)
        sites.append(site)
    return {
        "builtSiteExportVersion": export_catalog["exportVersion"],
        "coordinateUnit": export_catalog["coordinateUnit"],
        "lengthUnit": export_catalog["lengthUnit"],
        "areaUnit": export_catalog["areaUnit"],
        "sites": sites,
    }


def inspect_built_site_export(export_catalog):
    adapted = adapt_built_site_export(export_catalog)
    results = []
    for site in adapted["sites"]:
        findings = []
        coverage = site["evidenceCoverage"]
        if not coverage["placement"]:
            findings.append(finding(
                site,
                "placement-evidence-unknown",
                "blocking",
                [site["siteId"]],
                "The export does not prove placement evidence coverage.",
            ))
        elif not site["placementSlots"]:
            findings.append(finding(
                site,
                "placement-evidence-empty",
                "blocking",
                [site["siteId"]],
                "Complete placement evidence contains no neutral placement slots.",
            ))
        if not coverage["visibility"]:
            findings.append(finding(
                site,
                "visibility-evidence-unknown",
                "blocking",
                [site["siteId"]],
                "The export does not prove visibility evidence coverage.",
            ))
        elif not site["visibility"]:
            findings.append(finding(
                site,
                "visibility-evidence-empty",
                "blocking",
                [site["siteId"]],
                "Complete visibility evidence contains no directed visibility facts.",
            ))
        if not coverage["blockers"]:
            findings.append(finding(
                site,
                "blocker-evidence-unknown",
                "blocking",
                [site["siteId"]],
                "The export does not prove blocker evidence coverage.",
            ))
        unknown_clearance = [
            connection
            for connection in site["connections"]
            if connection["widthCm"] == 0 or connection["headroomCm"] == 0
        ]
        if not coverage["traversalClearance"]:
            findings.append(finding(
                site,
                "traversal-clearance-unknown",
                "blocking",
                [site["siteId"]],
                "The export does not prove traversal-clearance evidence coverage.",
            ))
        elif unknown_clearance:
            findings.append(finding(
                site,
                "traversal-clearance-unknown",
                "blocking",
                [connection["id"] for connection in unknown_clearance],
                "One or more directed connections lack asserted width or headroom.",
            ))
        findings.sort(key=lambda item: (item["code"], item["subjectIds"]))
        results.append({
            "adapterVersion": 1,
            "evaluationState": "insufficient-evidence" if findings else "ready-for-scenario",
            "export": {
                "version": adapted["builtSiteExportVersion"],
                "coordinateUnit": adapted["coordinateUnit"],
                "lengthUnit": adapted["lengthUnit"],
                "areaUnit": adapted["areaUnit"],
            },
            "site": {
                "id": site["siteId"],
                "version": site["siteVersion"],
                "generatorVersion": site["generatorVersion"],
                "kind": site["siteKind"],
                "checksum": site["checksum"],
            },
            "evidenceCoverage": coverage,
            "metrics": {
                "spaceCount": len(site["spaces"]),
                "connectionCount": len(site["connections"]),
                "visibilityCount": len(site["visibility"]),
                "blockerCount": len(site["blockers"]),
                "placementSlotCount": len(site["placementSlots"]),
                "unknownClearanceCount": len(unknown_clearance),
            },
            "findings": findings,
        })
    return results


def analysis_finding(site, code, severity, subject_ids, message):
    result = finding(site, code, severity, subject_ids, message)
    if not result["sourceIds"] and site["siteId"] in subject_ids:
        result["sourceIds"] = sorted(site["evidenceCoverage"]["sourceIds"])
    return result


def evaluate_built_site_ensemble(export_catalog, scenario_catalog, combat_catalog):
    if scenario_catalog.get("scenarioCatalogVersion") != 1:
        raise ValueError(
            f"unsupported scenario catalog {scenario_catalog.get('scenarioCatalogVersion')}"
        )
    if combat_catalog["contractVersion"] != scenario_catalog["requires"]["combatContractVersion"]:
        raise ValueError("Combat contract version does not match the scenario catalog")

    adapted = adapt_built_site_export(export_catalog)
    preflight = {
        result["site"]["id"]: result for result in inspect_built_site_export(export_catalog)
    }
    results = []
    for site in adapted["sites"]:
        site_preflight = preflight[site["siteId"]]
        if site_preflight["evaluationState"] != "ready-for-scenario":
            failed = copy.deepcopy(site_preflight)
            failed.update({
                "ensembleVersion": 1,
                "scenario": {"catalogVersion": scenario_catalog["scenarioCatalogVersion"]},
                "combat": {
                    "contractVersion": combat_catalog["contractVersion"],
                    "fixtureVersion": combat_catalog["fixtureVersion"],
                },
                "selection": {"policy": "representative-ensemble", "physicalTupleCount": 0},
                "representatives": [],
            })
            failed["metrics"].update({
                "representativeCount": 0,
                "simulatedRepresentativeCount": 0,
                "siteNetworkDisconnectedObjectiveCount": 0,
            })
            results.append(failed)
            continue

        spaces = by_id(site["spaces"])
        blockers = by_id(site["blockers"])
        attacker_profile = next(
            profile
            for profile in combat_catalog["capabilityProfiles"]
            if profile["side"] == "attacker"
        )
        envelope = by_id(combat_catalog["traversalEnvelopes"])[
            attacker_profile["traversalEnvelopeId"]
        ]
        connections = [
            connection
            for connection in site["connections"]
            if "ground" in connection["movementModeIds"]
            and connection_fits(connection, spaces, envelope, blockers)
        ]
        outdoor_slots = slots_with_tag(site, "outdoors")
        indoor_slots = slots_with_tag(site, "indoors")
        path_cache = {}

        def path(start, goal):
            key = (start, goal)
            if key not in path_cache:
                path_cache[key] = shortest_path(connections, start, goal)
            return path_cache[key]

        site_network_space_ids = [
            space["id"] for space in site["spaces"] if "path-centerline" in space["tags"]
        ]
        network_reachable_objective_ids = {
            objective["id"]
            for objective in indoor_slots
            if any(path(root_id, objective["spaceId"]) is not None for root_id in site_network_space_ids)
        }
        candidates = []
        for arrival in outdoor_slots:
            for objective in indoor_slots:
                if objective["id"] not in network_reachable_objective_ids:
                    continue
                entry_path = path(arrival["spaceId"], objective["spaceId"])
                if entry_path is None:
                    continue
                first_approaches = sum(
                    connection["fromSpaceId"] == arrival["spaceId"]
                    and path(connection["toSpaceId"], objective["spaceId"]) is not None
                    for connection in connections
                )
                exposed = any(
                    sight["fromSpaceId"] == objective["spaceId"]
                    and sight["toSpaceId"] == arrival["spaceId"]
                    and not sight["blockerIds"]
                    for sight in site["visibility"]
                )
                for extraction in outdoor_slots:
                    exit_path = path(objective["spaceId"], extraction["spaceId"])
                    if exit_path is None:
                        continue
                    candidates.append({
                        "arrival": arrival,
                        "objective": objective,
                        "extraction": extraction,
                        "entryPath": entry_path,
                        "exitPath": exit_path,
                        "totalLinks": len(entry_path) + len(exit_path),
                        "firstApproachCount": first_approaches,
                        "visibilityExposed": exposed,
                    })

        def role_ids(candidate):
            return (
                candidate["arrival"]["id"],
                candidate["objective"]["id"],
                candidate["extraction"]["id"],
            )

        mask = attacker_profile["mask"]
        drain = mask["raidDrainPerTick"]
        movement_tick_budget = (
            mask["maximum"] // drain
            if drain > 0
            else combat_catalog["runTunables"]["timeLimitTicks"]
        )
        selected = {}
        if candidates:
            medium_target = movement_tick_budget * 2 // 3
            choices = [
                (
                    "shallow",
                    min(
                        candidates,
                        key=lambda item: (
                            item["totalLinks"],
                            len(item["entryPath"]),
                            len(item["exitPath"]),
                            role_ids(item),
                        ),
                    ),
                ),
                (
                    "medium",
                    min(
                        candidates,
                        key=lambda item: (
                            abs(item["totalLinks"] - medium_target),
                            abs(len(item["entryPath"]) - len(item["exitPath"])),
                            role_ids(item),
                        ),
                    ),
                ),
                (
                    "deep",
                    min(candidates, key=lambda item: (-item["totalLinks"], role_ids(item))),
                ),
                (
                    "route-choice",
                    min(
                        candidates,
                        key=lambda item: (
                            -item["firstApproachCount"],
                            abs(item["totalLinks"] - medium_target),
                            role_ids(item),
                        ),
                    ),
                ),
            ]
            exposed_candidates = [item for item in candidates if item["visibilityExposed"]]
            if exposed_candidates:
                choices.append((
                    "visibility-exposed",
                    min(
                        exposed_candidates,
                        key=lambda item: (
                            abs(item["totalLinks"] - medium_target),
                            role_ids(item),
                        ),
                    ),
                ))
            for bucket_id, candidate in choices:
                key = role_ids(candidate)
                selected.setdefault(key, {"candidate": candidate, "bucketIds": []})[
                    "bucketIds"
                ].append(bucket_id)

        compatible_scenario_catalog = sorted(
            (
                scenario
                for scenario in scenario_catalog["scenarios"]
                if site["siteKind"] in scenario["siteKinds"]
            ),
            key=lambda scenario: scenario["id"],
        )
        evaluation_scenario = (
            compatible_scenario_catalog[0]
            if compatible_scenario_catalog
            else {
                "id": "provisional-settlement-lab-v1",
                "version": 1,
                "siteKinds": [site["siteKind"]],
                "accessStateId": "provisional-analysis",
                "arrival": {"requiredSlotKinds": ["arrival"]},
                "objective": {"requiredSlotKinds": ["loot", "extraction"]},
            }
        )
        representatives = []
        for role_key, selection in selected.items():
            candidate = selection["candidate"]
            bucket_ids = sorted(selection["bucketIds"])
            role_site = copy.deepcopy(site)
            role_site["connections"] = copy.deepcopy(connections)
            role_slots = by_id(role_site["placementSlots"])
            tactical_tags = {"arrival", "objective", "loot", "extraction", "guard"}
            for slot in role_site["placementSlots"]:
                slot["tags"] = [tag for tag in slot["tags"] if tag not in tactical_tags]
            for tag, slot_id in (
                ("arrival", role_key[0]),
                ("objective", role_key[1]),
                ("loot", role_key[1]),
                ("guard", role_key[1]),
                ("extraction", role_key[2]),
            ):
                role_slots[slot_id]["tags"].append(tag)

            representative_id = f"{site['siteId']}:representative:{bucket_ids[0]}"
            evaluation = evaluate_case(
                {"id": representative_id},
                role_site,
                evaluation_scenario,
                combat_catalog,
                site["contractVersion"],
            )
            representative_findings = list(evaluation["findings"])
            attacker_successes = {
                policy["id"]: sum(
                    run["outcome"] == "success"
                    for run in evaluation["policyMatrix"]
                    if run["attackerPolicyId"] == policy["id"]
                )
                for policy in combat_catalog["policies"]
                if policy["side"] == "attacker"
            }
            subject_ids = list(role_key)
            if len(set(attacker_successes.values())) > 1:
                representative_findings.append(analysis_finding(
                    role_site,
                    "policy-sensitive",
                    "info",
                    subject_ids,
                    "Attacker policies produce different extraction counts for the same physical tuple.",
                ))
            if (
                (
                    candidate["totalLinks"] * drain >= mask["maximum"]
                    or candidate["totalLinks"]
                    >= combat_catalog["runTunables"]["timeLimitTicks"]
                )
                and evaluation["metrics"]["successfulRuns"] == 0
            ):
                representative_findings.append(analysis_finding(
                    role_site,
                    "raid-clock-impossible",
                    "error",
                    subject_ids
                    + [link["id"] for link in candidate["entryPath"] + candidate["exitPath"]],
                    "The minimum round trip consumes the attacker's full mask before combat delay.",
                ))
            representative_findings.sort(key=lambda item: (item["code"], item["subjectIds"]))
            representatives.append({
                "id": representative_id,
                "bucketIds": bucket_ids,
                "simulated": True,
                "roles": {
                    "arrivalSlotId": role_key[0],
                    "objectiveSlotId": role_key[1],
                    "extractionSlotId": role_key[2],
                    "guardSlotId": role_key[1],
                },
                "physical": {
                    "entryLinkCount": len(candidate["entryPath"]),
                    "exitLinkCount": len(candidate["exitPath"]),
                    "totalLinkCount": candidate["totalLinks"],
                    "alternateApproachCount": candidate["firstApproachCount"],
                    "guardSeesArrival": candidate["visibilityExposed"],
                    "entryConnectionIds": [link["id"] for link in candidate["entryPath"]],
                    "exitConnectionIds": [link["id"] for link in candidate["exitPath"]],
                },
                "metrics": evaluation["metrics"],
                "ratings": evaluation["ratings"],
                "findings": representative_findings,
                "policyMatrix": evaluation["policyMatrix"],
            })

        disconnected_slots = [
            slot
            for slot in indoor_slots
            if slot["id"] not in network_reachable_objective_ids
        ]
        aggregate_findings = [
            analysis_finding(
                site,
                "no-compatible-scenario",
                "blocking",
                [site["siteId"]],
                "No production raid scenario supports this site kind.",
            ),
            analysis_finding(
                site,
                "world-approach-evidence-unknown",
                "blocking",
                [site["siteId"]],
                "No composed world-to-site approach proves a valid external arrival.",
            ),
        ]
        if disconnected_slots:
            disconnected_finding = analysis_finding(
                site,
                "site-network-disconnected-objective",
                "error",
                [slot["id"] for slot in disconnected_slots]
                + [slot["spaceId"] for slot in disconnected_slots],
                "One or more indoor objective candidates have no supported route from the site path network.",
            )
            aggregate_findings.append(disconnected_finding)
            first_disconnected = disconnected_slots[0]
            representatives.append({
                "id": f"{site['siteId']}:representative:site-network-disconnected-objective",
                "bucketIds": ["site-network-disconnected-objective"],
                "simulated": False,
                "roles": {
                    "arrivalSlotId": None,
                    "objectiveSlotId": first_disconnected["id"],
                    "extractionSlotId": None,
                    "guardSlotId": first_disconnected["id"],
                },
                "physical": {
                    "entryLinkCount": None,
                    "exitLinkCount": None,
                    "totalLinkCount": None,
                    "alternateApproachCount": 0,
                    "guardSeesArrival": False,
                    "entryConnectionIds": [],
                    "exitConnectionIds": [],
                },
                "metrics": {"successfulRuns": 0, "totalRuns": 0},
                "ratings": {"tacticalViability": "fail"},
                "findings": [disconnected_finding],
                "policyMatrix": [],
            })

        compatible_scenarios = [scenario["id"] for scenario in compatible_scenario_catalog]
        if compatible_scenarios:
            aggregate_findings = [
                item for item in aggregate_findings if item["code"] != "no-compatible-scenario"
            ]
        aggregate_findings.sort(key=lambda item: (item["code"], item["subjectIds"]))
        representatives.sort(key=lambda item: item["id"])
        results.append({
            "evaluatorVersion": EVALUATOR_VERSION,
            "ensembleVersion": 1,
            "evaluationState": "analysis-only",
            "site": {
                "id": site["siteId"],
                "version": site["siteVersion"],
                "generatorVersion": site["generatorVersion"],
                "kind": site["siteKind"],
                "checksum": site["checksum"],
            },
            "scenario": {
                "catalogVersion": scenario_catalog["scenarioCatalogVersion"],
                "compatibleScenarioIds": compatible_scenarios,
                "evaluatedScenarioId": evaluation_scenario["id"],
                "evaluatedScenarioVersion": evaluation_scenario["version"],
                "accessStateId": evaluation_scenario["accessStateId"],
            },
            "combat": {
                "contractVersion": combat_catalog["contractVersion"],
                "fixtureVersion": combat_catalog["fixtureVersion"],
                "attackerProfileId": attacker_profile["id"],
                "attackerProfileVersion": attacker_profile["version"],
                "traversalEnvelopeId": envelope["id"],
                "traversalEnvelopeVersion": envelope["version"],
            },
            "selection": {
                "policy": "representative-ensemble",
                "physicalTupleCount": len(candidates),
                "mediumTargetLinkCount": movement_tick_budget * 2 // 3,
            },
            "metrics": {
                "outdoorSlotCount": len(outdoor_slots),
                "indoorSlotCount": len(indoor_slots),
                "siteNetworkReachableObjectiveCount": len(network_reachable_objective_ids),
                "siteNetworkDisconnectedObjectiveCount": len(disconnected_slots),
                "representativeCount": len(representatives),
                "simulatedRepresentativeCount": sum(
                    representative["simulated"] for representative in representatives
                ),
            },
            "findings": aggregate_findings,
            "representatives": representatives,
        })
    return results


def validate_site(site, scenario, expected_contract_version):
    if site["contractVersion"] != expected_contract_version:
        raise ValueError(f"unsupported Built Site Recipe contract {site['contractVersion']}")
    if site["siteKind"] not in scenario["siteKinds"]:
        raise ValueError(f"scenario {scenario['id']} does not support {site['siteKind']}")

    element_ids = []
    for key in ("spaces", "connections", "visibility", "blockers", "placementSlots"):
        element_ids.extend(item["id"] for item in site[key])
    if len(element_ids) != len(set(element_ids)):
        raise ValueError(f"site {site['siteId']} has duplicate stable IDs")

    spaces = by_id(site["spaces"])
    blockers = by_id(site["blockers"])
    for connection in site["connections"]:
        if connection["fromSpaceId"] not in spaces or connection["toSpaceId"] not in spaces:
            raise ValueError(f"connection {connection['id']} has a dangling space reference")
        if any(blocker_id not in blockers for blocker_id in connection["blockerIds"]):
            raise ValueError(f"connection {connection['id']} has a dangling blocker reference")
    for sight in site["visibility"]:
        if sight["fromSpaceId"] not in spaces or sight["toSpaceId"] not in spaces:
            raise ValueError(f"visibility {sight['id']} has a dangling space reference")
        if any(blocker_id not in blockers for blocker_id in sight["blockerIds"]):
            raise ValueError(f"visibility {sight['id']} has a dangling blocker reference")
    for slot in site["placementSlots"]:
        if slot["spaceId"] not in spaces:
            raise ValueError(f"slot {slot['id']} has a dangling space reference")

    required_tags = set(scenario["objective"]["requiredSlotKinds"])
    required_tags.update(scenario["arrival"]["requiredSlotKinds"])
    available_tags = {tag for slot in site["placementSlots"] for tag in slot["tags"]}
    missing = sorted(required_tags - available_tags)
    if missing:
        raise ValueError(f"site {site['siteId']} lacks required slot tags: {', '.join(missing)}")


def topology_findings(site, scenario, combat):
    spaces = by_id(site["spaces"])
    blockers = by_id(site["blockers"])
    envelope = by_id(combat["traversalEnvelopes"])["ground-baseline"]
    connections = [
        connection
        for connection in site["connections"]
        if connection_fits(connection, spaces, envelope, blockers)
    ]
    arrivals = slots_with_tag(site, "arrival")
    objectives = slots_with_tag(site, "loot") or slots_with_tag(site, "objective")
    extractions = slots_with_tag(site, "extraction")
    guards = slots_with_tag(site, "guard")
    objective = objectives[0]
    result = []

    direct_arrivals = [arrival for arrival in arrivals if arrival["spaceId"] == objective["spaceId"]]
    if direct_arrivals:
        result.append(
            finding(
                site,
                "direct-prize-arrival",
                "warning",
                [arrival["id"] for arrival in direct_arrivals] + [objective["id"]],
                "An arrival slot occupies the objective space.",
            )
        )

    reachable = []
    first_steps = {}
    for arrival in arrivals:
        path = shortest_path(connections, arrival["spaceId"], objective["spaceId"])
        if path is not None:
            reachable.append(arrival)
        for connection in connections:
            if connection["fromSpaceId"] != arrival["spaceId"]:
                continue
            if shortest_path(connections, connection["toSpaceId"], objective["spaceId"]) is not None:
                first_steps[connection["id"]] = connection

    if not reachable:
        blocked_links = [
            connection
            for connection in site["connections"]
            if connection["fromSpaceId"] in {arrival["spaceId"] for arrival in arrivals}
            and connection not in connections
        ]
        result.append(
            finding(
                site,
                "objective-unreachable",
                "error",
                [arrival["id"] for arrival in arrivals]
                + [objective["id"]]
                + [connection["id"] for connection in blocked_links]
                + [blocker_id for connection in blocked_links for blocker_id in connection["blockerIds"]],
                "No legal attacker route reaches the objective.",
            )
        )

    extraction_paths = [
        shortest_path(connections, objective["spaceId"], extraction["spaceId"])
        for extraction in extractions
    ]
    reachable_extractions = [path for path in extraction_paths if path is not None]
    if not reachable_extractions:
        result.append(
            finding(
                site,
                "extraction-unreachable",
                "error",
                [objective["id"]] + [slot["id"] for slot in extractions],
                "No legal route carries the objective to extraction.",
            )
        )
    elif min(len(path) for path in reachable_extractions) <= 1:
        result.append(
            finding(
                site,
                "trivial-extraction",
                "warning",
                [objective["id"]] + [slot["id"] for slot in extractions],
                "The objective is at most one movement link from extraction.",
            )
        )

    if len(first_steps) == 1:
        first = next(iter(first_steps.values()))
        if first["blockerIds"] or "chokepoint" in first["tags"]:
            result.append(
                finding(
                    site,
                    "one-door-bunker",
                    "warning",
                    [first["id"]] + first["blockerIds"],
                    "Every legal approach begins through one defended physical link.",
                )
            )

    if guards and arrivals:
        exposed_arrivals = []
        implicated_sight = []
        for arrival in arrivals:
            sight = next(
                (
                    link
                    for link in site["visibility"]
                    if link["fromSpaceId"] in {guard["spaceId"] for guard in guards}
                    and link["toSpaceId"] == arrival["spaceId"]
                    and not link["blockerIds"]
                ),
                None,
            )
            if sight:
                exposed_arrivals.append(arrival)
                implicated_sight.append(sight)
        if len(exposed_arrivals) == len(arrivals):
            result.append(
                finding(
                    site,
                    "unavoidable-first-shot",
                    "warning",
                    [slot["id"] for slot in guards + arrivals]
                    + [sight["id"] for sight in implicated_sight],
                    "Every arrival is immediately visible from a guard slot.",
                )
            )

    return sorted(result, key=lambda item: (item["code"], item["subjectIds"]))


def compile_arena(site, scenario, arrival, combat):
    spaces = by_id(site["spaces"])
    attacker_profile = next(profile for profile in combat["capabilityProfiles"] if profile["side"] == "attacker")
    defender_profile = next(profile for profile in combat["capabilityProfiles"] if profile["side"] == "defender")
    objective = (slots_with_tag(site, "loot") or slots_with_tag(site, "objective"))[0]
    extraction = slots_with_tag(site, "extraction")[0]
    guards = slots_with_tag(site, "guard")
    defender_start = guards[0]["spaceId"] if guards else objective["spaceId"]

    referenced_blockers = sorted(
        {
            blocker_id
            for connection in site["connections"]
            for blocker_id in connection["blockerIds"]
        }
    )
    blocker_data = by_id(site["blockers"])
    structure_template = next(
        structure
        for arena in combat["arenas"]
        for structure in arena["structures"]
    )
    structures = []
    for blocker_id in referenced_blockers:
        connection = next(item for item in site["connections"] if blocker_id in item["blockerIds"])
        tags = blocker_data[blocker_id]["tags"]
        structures.append(
            {
                "id": blocker_id,
                "nodeId": connection["toSpaceId"],
                "integrity": structure_template["integrity"],
                "defence": structure_template["defence"],
                "targetable": "non-damageable" not in tags and structure_template["targetable"],
            }
        )

    arena = {
        "id": f"{site['siteId']}:{scenario['id']}:{arrival['id']}",
        "version": site["siteVersion"],
        "nodes": [
            {"id": space["id"], "positionCm": space["positionCm"], "tags": space["tags"]}
            for space in sorted(site["spaces"], key=lambda item: item["id"])
        ],
        "movementLinks": [
            {
                "id": connection["id"],
                "fromNodeId": connection["fromSpaceId"],
                "toNodeId": connection["toSpaceId"],
                "lengthCm": distance(
                    spaces[connection["fromSpaceId"]]["positionCm"],
                    spaces[connection["toSpaceId"]]["positionCm"],
                ),
                "widthCm": connection["widthCm"],
                "heightCm": connection["headroomCm"],
                "stepCm": connection["maxStepCm"],
                "jumpGapCm": connection["maxJumpGapCm"],
                "jumpRiseCm": 0,
                "blockerIds": connection["blockerIds"],
            }
            for connection in sorted(site["connections"], key=lambda item: item["id"])
        ],
        "sightLinks": [
            {
                "id": sight["id"],
                "fromNodeId": sight["fromSpaceId"],
                "toNodeId": sight["toSpaceId"],
                "distanceCm": sight["distanceCm"],
                "blockerIds": sight["blockerIds"],
            }
            for sight in sorted(site["visibility"], key=lambda item: item["id"])
        ],
        "agents": [
            {
                "id": "attacker-a",
                "side": "attacker",
                "profileId": attacker_profile["id"],
                "profileVersion": attacker_profile["version"],
                "policyId": "runner",
                "policyVersion": 1,
                "startNodeId": arrival["spaceId"],
            },
            {
                "id": "defender-a",
                "side": "defender",
                "profileId": defender_profile["id"],
                "profileVersion": defender_profile["version"],
                "policyId": "hold",
                "policyVersion": 1,
                "startNodeId": defender_start,
            },
        ],
        "structures": structures,
        "objective": {
            "lootId": f"loot:{scenario['id']}",
            "lootNodeId": objective["spaceId"],
            "extractionNodeId": extraction["spaceId"],
        },
    }
    return arena


def run_matrix(site, scenario, combat):
    attackers = sorted(
        (policy for policy in combat["policies"] if policy["side"] == "attacker"),
        key=lambda policy: policy["id"],
    )
    defenders = sorted(
        (policy for policy in combat["policies"] if policy["side"] == "defender"),
        key=lambda policy: policy["id"],
    )
    matrix = []
    for arrival in slots_with_tag(site, "arrival"):
        arena = compile_arena(site, scenario, arrival, combat)
        for attacker in attackers:
            for defender in defenders:
                case = {
                    "id": f"run:{arrival['id']}:{attacker['id']}:{defender['id']}",
                    "arenaId": arena["id"],
                    "attackerPolicyId": attacker["id"],
                    "defenderPolicyId": defender["id"],
                }
                run_catalog = {**combat, "arenas": [arena], "acceptanceCases": [case]}
                result = run_acceptance_case(run_catalog, case["id"])
                matrix.append(
                    {
                        "arrivalSlotId": arrival["id"],
                        "attackerPolicyId": attacker["id"],
                        "defenderPolicyId": defender["id"],
                        "outcome": result["outcome"],
                        "terminationReason": result["terminationReason"],
                        "terminalTick": result["terminalTick"],
                        "decisiveEventType": result["decisiveEvent"]["type"],
                    }
                )
    return matrix


def evaluate_case(case, site, scenario, combat, expected_contract_version):
    validate_site(site, scenario, expected_contract_version)
    findings = topology_findings(site, scenario, combat)
    matrix = run_matrix(site, scenario, combat)
    codes = {item["code"] for item in findings}
    fatal = bool(codes & {"objective-unreachable", "extraction-unreachable"})
    spaces = by_id(site["spaces"])
    blockers = by_id(site["blockers"])
    envelope = by_id(combat["traversalEnvelopes"])["ground-baseline"]
    usable = [
        connection
        for connection in site["connections"]
        if connection_fits(connection, spaces, envelope, blockers)
    ]
    objective = (slots_with_tag(site, "loot") or slots_with_tag(site, "objective"))[0]
    first_steps = {
        connection["id"]
        for arrival in slots_with_tag(site, "arrival")
        for connection in usable
        if connection["fromSpaceId"] == arrival["spaceId"]
        and shortest_path(usable, connection["toSpaceId"], objective["spaceId"]) is not None
    }
    return {
        "evaluatorVersion": EVALUATOR_VERSION,
        "caseId": case["id"],
        "site": {
            "id": site["siteId"],
            "version": site["siteVersion"],
            "generatorVersion": site["generatorVersion"],
            "kind": site["siteKind"],
        },
        "scenario": {
            "id": scenario["id"],
            "version": scenario["version"],
            "accessStateId": scenario["accessStateId"],
        },
        "combat": {
            "contractVersion": combat["contractVersion"],
            "fixtureVersion": combat["fixtureVersion"],
        },
        "metrics": {
            "arrivalCount": len(slots_with_tag(site, "arrival")),
            "alternateApproachCount": len(first_steps),
            "successfulRuns": sum(run["outcome"] == "success" for run in matrix),
            "totalRuns": len(matrix),
        },
        "ratings": {
            "tacticalViability": "fail" if fatal else "pass",
            "approachQuality": "fail" if fatal else ("good" if len(first_steps) >= 2 else "limited"),
            "fairness": "warn" if codes & {"direct-prize-arrival", "unavoidable-first-shot"} else "pass",
            "variety": "warn" if "one-door-bunker" in codes else "pass",
            "exploitRisk": "high" if codes & {"direct-prize-arrival", "one-door-bunker", "trivial-extraction"} else "low",
        },
        "findings": findings,
        "policyMatrix": matrix,
    }


def evaluate_catalogs(site_catalog, scenario_catalog, combat_catalog):
    if site_catalog["catalogVersion"] != 1:
        raise ValueError(f"unsupported site catalog {site_catalog['catalogVersion']}")
    if scenario_catalog["scenarioCatalogVersion"] != 1:
        raise ValueError(f"unsupported scenario catalog {scenario_catalog['scenarioCatalogVersion']}")
    if combat_catalog["contractVersion"] != scenario_catalog["requires"]["combatContractVersion"]:
        raise ValueError("Combat contract version does not match the scenario catalog")

    sites = {site["siteId"]: site for site in site_catalog["sites"]}
    scenarios = by_id(scenario_catalog["scenarios"])
    results = []
    for case in sorted(site_catalog["cases"], key=lambda item: item["id"]):
        if case["siteId"] not in sites or case["scenarioId"] not in scenarios:
            raise ValueError(f"case {case['id']} has a dangling stable reference")
        results.append(
            evaluate_case(
                case,
                sites[case["siteId"]],
                scenarios[case["scenarioId"]],
                combat_catalog,
                site_catalog["builtSiteContractVersion"],
            )
        )
    return results


def load(path):
    return json.loads(path.read_text(encoding="utf-8"))


def main():
    parser = argparse.ArgumentParser(description="Run deterministic headless Rift raid evaluations.")
    parser.add_argument("--sites", type=Path, default=HERE / "synthetic-built-sites-v1.json")
    parser.add_argument("--scenarios", type=Path, default=HERE / "rift-raid-scenarios-v1.json")
    parser.add_argument(
        "--combat",
        type=Path,
        default=HERE.parent / "combat" / "headless-combat-fixtures-v1.json",
    )
    parser.add_argument("--built-site-export", type=Path, help="Inspect a production Built Site JSON catalog.")
    parser.add_argument(
        "--representative-ensemble",
        action="store_true",
        help="Evaluate a deterministic physical-evidence ensemble from a Built Site export.",
    )
    parser.add_argument("--summary", action="store_true", help="Print one readable line per case.")
    args = parser.parse_args()
    if args.representative_ensemble and not args.built_site_export:
        parser.error("--representative-ensemble requires --built-site-export")
    if args.representative_ensemble:
        results = evaluate_built_site_ensemble(
            load(args.built_site_export),
            load(args.scenarios),
            load(args.combat),
        )
    elif args.built_site_export:
        results = inspect_built_site_export(load(args.built_site_export))
    else:
        results = evaluate_catalogs(load(args.sites), load(args.scenarios), load(args.combat))
    print(format_summary(results) if args.summary else canonical_json(results))


if __name__ == "__main__":
    main()
