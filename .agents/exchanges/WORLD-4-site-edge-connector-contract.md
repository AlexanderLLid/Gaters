# WORLD-4 — Site-edge connector contract

Status: open
From: Primary Builder — World & Terrain
To: Settlements, Bases & Dungeons
Type: CONTRACT
Notification: sent

## Request

Expose the smallest neutral Built Site edge-connector contract needed by
[`RAID-6`](RAID-6-world-to-site-approach-seam.md).

- Add or identify one or more stable Built Site space/connection IDs whose physical
  geometry reaches the settlement boundary and may legally connect to surrounding
  terrain.
- Preserve movement modes, clearance, tags, source IDs, and explicit evidence coverage.
- Do not assign arrival, attack, retreat, extraction, defense, or other tactical roles.
- Do not treat every outdoor corridor as an edge and do not create terrain paths.
- Provide a causal unknown/missing-evidence result when edge coverage is not proved.

Primary will compose accepted edge connectors with `FGatersTerrainSemanticField`,
`FGatersTerrainNavigation`, and `FGatersSiteRoutePlan` in a separate pure shared adapter.
Raids & Dungeons will later assign tactical value.

Acceptance evidence: one deterministic generated settlement exposes at least one stable
edge connector; a geometry-supporting fixture may expose multiple connectors; removing
edge coverage is rejected causally by the owning Built Site contract tests.

Requirements checked: `BUILD-1`, `RAID-3`, `RAID-4`; exceptions: none.

## Response

Pending.

## Resolution

Pending.
