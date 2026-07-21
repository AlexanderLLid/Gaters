import argparse
import copy
import json
from collections import deque
from pathlib import Path


def canonical_json(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"))


def by_id(items):
    return {item["id"]: item for item in items}


def run_acceptance_case(catalog, case_id):
    case = by_id(catalog["acceptanceCases"])[case_id]
    arena = by_id(catalog["arenas"])[case["arenaId"]]
    return Resolver(catalog, arena, case).run()


class Resolver:
    def __init__(self, catalog, arena, case):
        self.catalog = catalog
        self.arena = arena
        self.case = case
        self.nodes = by_id(arena["nodes"])
        self.links = by_id(arena["movementLinks"])
        self.sight = arena["sightLinks"]
        self.structures = {
            s["id"]: {**copy.deepcopy(s), "integrityLeft": s["integrity"], "destroyed": False}
            for s in arena["structures"]
        }
        self.profiles = by_id(catalog["capabilityProfiles"])
        self.envelopes = by_id(catalog["traversalEnvelopes"])
        self.policies = by_id(catalog["policies"])
        self.attacker_policy = self.policies[case["attackerPolicyId"]]
        self.defender_policy = self.policies[case["defenderPolicyId"]]
        self.events = []
        self.tick = 0
        self.no_progress_ticks = 0
        self.loot_node = arena["objective"]["lootNodeId"]
        self.loot_carrier = None
        self.extraction_node = arena["objective"]["extractionNodeId"]
        self.agents = {}
        for agent in arena["agents"]:
            profile = self.profiles[agent["profileId"]]
            self.agents[agent["id"]] = {
                **copy.deepcopy(agent),
                "nodeId": agent["startNodeId"],
                "mask": profile["mask"]["maximum"],
                "alive": True,
                "cooldowns": {mode["id"]: 0 for mode in profile["attackModes"]},
            }

    def run(self):
        for tick in range(1, self.catalog["runTunables"]["timeLimitTicks"] + 1):
            self.tick = tick
            progressed = False
            upkeep_deaths = self.upkeep()
            if upkeep_deaths and not self.living("attacker"):
                return self.finish("failure", "mask-expired", upkeep_deaths[-1])

            snapshot = self.snapshot()
            intents = [self.choose_intent(aid, snapshot) for aid in sorted(self.agents)]
            intents = [intent for intent in intents if intent]
            if not any(intent.get("progressCapable") for intent in intents):
                return self.finish("failure", "no-effective-action", self.no_action_event(snapshot))

            progressed = self.resolve_attacks(intents) or progressed
            if not self.living("attacker"):
                death_events = [e for e in self.events if e["tick"] == tick and e["type"] == "agent-died"]
                return self.finish("failure", "attackers-dead", death_events[-1])

            progressed = self.resolve_movement(intents) or progressed
            objective_event = self.resolve_objective()
            progressed = bool(objective_event) or progressed
            if objective_event and objective_event["type"] == "loot-extracted":
                return self.finish("success", "extracted", objective_event)

            self.no_progress_ticks = 0 if progressed else self.no_progress_ticks + 1
            if self.no_progress_ticks >= self.catalog["runTunables"]["stallTicks"]:
                return self.finish("failure", "stalled", self.event("stalled"))

        return self.finish("failure", "time-limit", self.event("time-limit"))

    def upkeep(self):
        deaths = []
        for agent in self.agents.values():
            if not agent["alive"]:
                continue
            profile = self.profiles[agent["profileId"]]
            for mode_id in agent["cooldowns"]:
                agent["cooldowns"][mode_id] = max(0, agent["cooldowns"][mode_id] - 1)
            mask = profile["mask"]
            if agent["side"] == "attacker":
                agent["mask"] -= mask["raidDrainPerTick"]
                self.event("upkeep", agentId=agent["id"], mask=agent["mask"])
            else:
                before = agent["mask"]
                agent["mask"] = min(mask["maximum"], agent["mask"] + mask["homeRechargePerTick"])
                if agent["mask"] != before:
                    self.event("upkeep", agentId=agent["id"], mask=agent["mask"])
            if agent["mask"] <= 0:
                deaths.append(self.kill(agent["id"], "mask-expired"))
        return deaths

    def snapshot(self):
        return {
            "agents": copy.deepcopy(self.agents),
            "structures": copy.deepcopy(self.structures),
            "lootNode": self.loot_node,
            "lootCarrier": self.loot_carrier,
        }

    def choose_intent(self, agent_id, snapshot):
        agent = snapshot["agents"][agent_id]
        if not agent["alive"]:
            return None
        policy = self.attacker_policy if agent["side"] == "attacker" else self.defender_policy

        if policy["id"] == "runner":
            return self.objective_intent(agent_id, snapshot, allow_defender_attack=True)
        if policy["id"] == "clear":
            attack = self.attack_nearest(agent_id, "defender", snapshot)
            return attack or self.objective_intent(agent_id, snapshot, allow_defender_attack=False)
        if policy["id"] == "hold":
            return self.attack_nearest(agent_id, "attacker", snapshot) or self.event_intent(agent_id, "hold")
        if policy["id"] == "intercept":
            return (
                self.attack_nearest(agent_id, "attacker", snapshot, carrier_first=True)
                or self.move_toward_nearest_attacker(agent_id, snapshot)
            )
        raise ValueError(f"unsupported policy {policy['id']}")

    def objective_intent(self, agent_id, snapshot, allow_defender_attack):
        agent = snapshot["agents"][agent_id]
        if snapshot["lootCarrier"] == agent_id and agent["nodeId"] == self.extraction_node:
            return {"kind": "extract", "agentId": agent_id, "progressCapable": True}
        if snapshot["lootCarrier"] is None and agent["nodeId"] == snapshot["lootNode"]:
            return {"kind": "pickup", "agentId": agent_id, "progressCapable": True}
        target = self.extraction_node if snapshot["lootCarrier"] == agent_id else snapshot["lootNode"]
        route = self.shortest_route(agent["nodeId"], target, snapshot, ignore_blockers=True)
        if route:
            link = route[0]
            blockers = self.live_blockers(link, snapshot)
            if not blockers:
                return {"kind": "move", "agentId": agent_id, "linkId": link["id"], "progressCapable": True}
            for blocker_id in sorted(blockers):
                attack = self.attack_target(agent_id, "structure", blocker_id, snapshot)
                if attack:
                    attack["progressCapable"] = True
                    return attack
        if allow_defender_attack:
            return self.attack_nearest(agent_id, "defender", snapshot)
        return None

    def move_toward_nearest_attacker(self, agent_id, snapshot):
        agent = snapshot["agents"][agent_id]
        routes = []
        for target in snapshot["agents"].values():
            if target["side"] == "attacker" and target["alive"]:
                route = self.shortest_route(agent["nodeId"], target["nodeId"], snapshot, ignore_blockers=True)
                if route:
                    routes.append((len(route), target["id"], route))
        for _, _, route in sorted(routes):
            link = route[0]
            if not self.live_blockers(link, snapshot):
                return {"kind": "move", "agentId": agent_id, "linkId": link["id"], "progressCapable": True}
        return None

    def attack_nearest(self, agent_id, target_side, snapshot, carrier_first=False):
        agent = snapshot["agents"][agent_id]
        candidates = []
        for target in snapshot["agents"].values():
            if target["side"] != target_side or not target["alive"]:
                continue
            attack = self.attack_target(agent_id, "agent", target["id"], snapshot)
            if attack:
                carrier_rank = 0 if carrier_first and snapshot["lootCarrier"] == target["id"] else 1
                candidates.append((carrier_rank, self.distance(agent["nodeId"], target["nodeId"]), target["id"], attack))
        return sorted(candidates)[0][3] if candidates else None

    def attack_target(self, agent_id, target_kind, target_id, snapshot):
        agent = snapshot["agents"][agent_id]
        profile = self.profiles[agent["profileId"]]
        target_node = (
            snapshot["agents"][target_id]["nodeId"]
            if target_kind == "agent"
            else snapshot["structures"][target_id]["nodeId"]
        )
        target_defence = (
            self.profiles[snapshot["agents"][target_id]["profileId"]]["mask"]["defence"]
            if target_kind == "agent"
            else snapshot["structures"][target_id]["defence"]
        )
        if target_kind == "structure":
            structure = snapshot["structures"][target_id]
            if structure["destroyed"] or not structure["targetable"]:
                return None
        for mode in sorted(profile["attackModes"], key=lambda m: m["id"]):
            if agent["cooldowns"][mode["id"]] > 0:
                continue
            damage = mode["damage"][target_kind]
            applied = max(0, damage - target_defence)
            if applied <= 0:
                continue
            if mode["requiresSight"] and not self.has_sight(agent["nodeId"], target_node, mode["rangeCm"], snapshot):
                continue
            return {
                "kind": "attack",
                "agentId": agent_id,
                "targetKind": target_kind,
                "targetId": target_id,
                "modeId": mode["id"],
                "damage": applied,
                "progressCapable": True,
            }
        return None

    def resolve_attacks(self, intents):
        progressed = False
        for intent in sorted([i for i in intents if i["kind"] == "attack"], key=lambda i: i["agentId"]):
            attacker = self.agents[intent["agentId"]]
            if not attacker["alive"]:
                continue
            self.event("target-selected", agentId=intent["agentId"], targetId=intent["targetId"], targetKind=intent["targetKind"])
            self.event("attack", agentId=intent["agentId"], targetId=intent["targetId"], modeId=intent["modeId"])
            attacker["cooldowns"][intent["modeId"]] = self.attack_mode(attacker, intent["modeId"])["cooldownTicks"]
            if intent["targetKind"] == "agent":
                target = self.agents[intent["targetId"]]
                target["mask"] -= intent["damage"]
                self.event("damage", sourceId=intent["agentId"], targetId=target["id"], amount=intent["damage"], remaining=target["mask"])
                progressed = True
            else:
                target = self.structures[intent["targetId"]]
                target["integrityLeft"] -= intent["damage"]
                self.event("damage", sourceId=intent["agentId"], targetId=target["id"], amount=intent["damage"], remaining=target["integrityLeft"])
                progressed = True
        for agent_id in sorted(self.agents):
            agent = self.agents[agent_id]
            if agent["alive"] and agent["mask"] <= 0:
                self.kill(agent_id, "combat")
                progressed = True
        for structure_id in sorted(self.structures):
            structure = self.structures[structure_id]
            if not structure["destroyed"] and structure["integrityLeft"] <= 0:
                structure["destroyed"] = True
                self.event("structure-destroyed", structureId=structure_id)
                progressed = True
        return progressed

    def resolve_movement(self, intents):
        progressed = False
        for intent in sorted([i for i in intents if i["kind"] == "move"], key=lambda i: i["agentId"]):
            agent = self.agents[intent["agentId"]]
            if not agent["alive"]:
                continue
            link = self.links[intent["linkId"]]
            if agent["nodeId"] != link["fromNodeId"] or self.live_blockers(link, self.snapshot()):
                continue
            self.event("path-selected", agentId=agent["id"], linkId=link["id"], fromNodeId=link["fromNodeId"], toNodeId=link["toNodeId"])
            agent["nodeId"] = link["toNodeId"]
            self.event("movement", agentId=agent["id"], linkId=link["id"], nodeId=agent["nodeId"])
            progressed = True
        return progressed

    def resolve_objective(self):
        if self.loot_carrier:
            carrier = self.agents[self.loot_carrier]
            if carrier["alive"] and carrier["nodeId"] == self.extraction_node:
                return self.event("loot-extracted", agentId=carrier["id"], lootId=self.arena["objective"]["lootId"])
            return None
        for agent in sorted(self.agents.values(), key=lambda a: a["id"]):
            if agent["alive"] and agent["side"] == "attacker" and agent["nodeId"] == self.loot_node:
                self.loot_carrier = agent["id"]
                event = self.event("loot-picked-up", agentId=agent["id"], lootId=self.arena["objective"]["lootId"])
                return event
        return None

    def kill(self, agent_id, reason):
        agent = self.agents[agent_id]
        agent["alive"] = False
        event = self.event("agent-died", agentId=agent_id, reason=reason)
        if self.loot_carrier == agent_id:
            self.loot_node = agent["nodeId"]
            self.loot_carrier = None
            self.event("loot-dropped", agentId=agent_id, lootId=self.arena["objective"]["lootId"], nodeId=self.loot_node)
        return event

    def finish(self, outcome, reason, decisive_event):
        termination = self.event("termination", outcome=outcome, reason=reason, decisiveEventId=decisive_event["id"])
        result = {
            "contractVersion": self.catalog["contractVersion"],
            "fixtureVersion": self.catalog["fixtureVersion"],
            "arena": {"id": self.arena["id"], "version": self.arena["version"]},
            "attackerPolicy": {"id": self.attacker_policy["id"], "version": self.attacker_policy["version"]},
            "defenderPolicy": {"id": self.defender_policy["id"], "version": self.defender_policy["version"]},
            "capabilityProfiles": [
                {"agentId": a["id"], "id": a["profileId"], "version": a["profileVersion"]}
                for a in sorted(self.arena["agents"], key=lambda item: item["id"])
            ],
            "outcome": outcome,
            "terminationReason": reason,
            "terminalTick": self.tick,
            "decisiveEvent": decisive_event,
            "events": self.events,
        }
        if reason == "no-effective-action":
            result["implicatedBlockerIds"] = decisive_event.get("implicatedBlockerIds", [])
            result["implicatedLinkIds"] = decisive_event.get("implicatedLinkIds", [])
        return result

    def no_action_event(self, snapshot):
        blockers = sorted(
            blocker
            for link in self.arena["movementLinks"]
            for blocker in self.live_blockers(link, snapshot)
        )
        links = sorted(link["id"] for link in self.arena["movementLinks"] if self.live_blockers(link, snapshot) or not self.link_requirements_fit(link))
        return self.event("no-effective-action", implicatedBlockerIds=blockers, implicatedLinkIds=links)

    def shortest_route(self, start, goal, snapshot, ignore_blockers):
        queue = deque([(start, [])])
        seen = {start}
        for node_id, path in iter(queue.popleft, None):
            if node_id == goal:
                return path
            for link in sorted(self.arena["movementLinks"], key=lambda item: item["id"]):
                if link["fromNodeId"] != node_id or link["toNodeId"] in seen:
                    continue
                if not self.link_requirements_fit(link):
                    continue
                if not ignore_blockers and self.live_blockers(link, snapshot):
                    continue
                seen.add(link["toNodeId"])
                queue.append((link["toNodeId"], path + [link]))
            if not queue:
                break
        return None

    def link_requirements_fit(self, link):
        envelope = self.envelopes["ground-baseline"]
        return (
            link["lengthCm"] <= envelope["perTickMoveDistanceCm"]
            and link["widthCm"] >= envelope["capsuleRadiusCm"] * 2
            and link["heightCm"] >= envelope["capsuleHeightCm"]
            and link["stepCm"] <= envelope["maxStepCm"]
            and link["jumpGapCm"] <= envelope["maxJumpGapCm"]
            and link["jumpRiseCm"] <= envelope["maxJumpRiseCm"]
        )

    def live_blockers(self, link, snapshot):
        return [
            blocker_id
            for blocker_id in link["blockerIds"]
            if blocker_id in snapshot["structures"] and not snapshot["structures"][blocker_id]["destroyed"]
        ]

    def has_sight(self, from_node, to_node, range_cm, snapshot):
        return any(
            sight["fromNodeId"] == from_node
            and sight["toNodeId"] == to_node
            and sight["distanceCm"] <= range_cm
            and not self.live_blockers(sight, snapshot)
            for sight in self.sight
        )

    def distance(self, a, b):
        pa = self.nodes[a]["positionCm"]
        pb = self.nodes[b]["positionCm"]
        return ((pa["x"] - pb["x"]) ** 2 + (pa["y"] - pb["y"]) ** 2 + (pa["z"] - pb["z"]) ** 2) ** 0.5

    def attack_mode(self, agent, mode_id):
        return by_id(self.profiles[agent["profileId"]]["attackModes"])[mode_id]

    def living(self, side):
        return [agent for agent in self.agents.values() if agent["side"] == side and agent["alive"]]

    def event_intent(self, agent_id, kind):
        return {"kind": kind, "agentId": agent_id, "progressCapable": False}

    def event(self, event_type, **fields):
        event = {"id": f"e{len(self.events) + 1:04d}", "tick": self.tick, "type": event_type, **fields}
        self.events.append(event)
        return event


def run_catalog(catalog):
    return [run_acceptance_case(catalog, case["id"]) for case in catalog["acceptanceCases"]]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("catalog", type=Path)
    args = parser.parse_args()
    catalog = json.loads(args.catalog.read_text(encoding="utf-8"))
    results = run_catalog(catalog)
    print(canonical_json(results))


if __name__ == "__main__":
    main()
