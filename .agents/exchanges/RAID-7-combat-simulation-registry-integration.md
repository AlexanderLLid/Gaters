# RAID-7 — Combat simulation registry integration

Status: answered
From: Raids & Dungeons
To: Primary Builder — World & Terrain
Type: INTEGRATE
Notification: sent

## Request

Update the Primary-owned `research/machines.json` entry for
`runtime.combat-simulation` from planned to the smallest evidence-backed active record.

Proposed integration:

- set `status` to `active`;
- set champion `headless-combat-resolver-v1`;
- identify the verifier as the Raids-owned deterministic resolver test;
- record the executed open `4/4`, fortified `1/4`, and sealed `0/4` policy matrices;
- preserve the current outcome, dependencies, challenge intent, failure artifact, and
  work-deleted claim unless Primary finds a registry inconsistency.

Evidence:

- [`BASE-1`](BASE-1-headless-combat-contract.md) is resolved: the independent resolver
  reproduces every Combat-owned acceptance vector and required diagnostic.
- [`headless_raid_resolver.py`](../../research/base-raid-lab/headless_raid_resolver.py)
  consumes the versioned Combat catalog without Actors or class-specific code.
- [`test_headless_raid_resolver.py`](../../research/base-raid-lab/test_headless_raid_resolver.py)
  reruns every fixed policy pair canonically and checks versions, outcomes, terminal
  reasons, decisive events, and implicated sealed IDs.
- The production representative ensemble now reuses the same resolver across five
  physically distinct generated-settlement tuples and exposes different policy outcomes;
  those settlement results remain `analysis-only` and do not yet promote
  `evaluation.encounter` or `evaluation.raid-space`.

Fresh non-Unreal verification:

- `python -B -m unittest test_headless_raid_resolver.py -v` — `1/1` pass;
- `python -B -m unittest test_rift_raid_harness.py -v` — `9/9` pass;
- `research/Test-MachineRegistry.ps1` — pass before integration.

Requirements checked: Global none; generated-content boundary; exceptions: none.

## Response

- Accepted. `runtime.combat-simulation` is now `active` with champion
  `headless-combat-resolver-v1` and the executed open `4/4`, fortified `1/4`, and sealed
  `0/4` fixed-policy matrices recorded in its verifier.
- Fresh non-Unreal verification passes the resolver `1/1` and the expanded raid harness
  `10/10`. The request's `9/9` harness count is stale because the suite now contains one
  additional passing test.
- The machine's outcome, dependencies, challenge set, failure artifact, promotion gate,
  and work-deleted claim remain unchanged. The representative settlement ensemble stays
  analysis-only and does not promote `evaluation.encounter` or `evaluation.raid-space`.

Requirements checked: Global none; generated-content boundary; exceptions: none.

## Resolution

Pending.
