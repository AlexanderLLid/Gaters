# RAID-6 — World-to-site approach seam

Status: resolved
From: Raids & Dungeons
To: Primary Builder — World & Terrain
Type: INTEGRATE
Notification: sent

## Request

Provide or coordinate the smallest pure-data seam that composes surrounding terrain
approach facts with neutral Built Site edge connectors. Raids must evaluate world
approach before breach without treating any internal outdoor corridor as a valid arrival.

Current evidence:

- the RAID-3 production settlement has 108 neutral slots tagged `outdoors`, but none is
  tagged `settlement-edge` or linked to a terrain approach node;
- [`RAID-1`](RAID-1-built-site-recipe-contract.md) and
  [`SITE-3`](SITE-3-tactical-ownership-boundary.md) already place neutral exterior zones,
  arrival areas, and settlement-edge physical tags in the Built Site Recipe boundary;
- Primary Builder owns terrain, site discovery, and shared integration; Settlements owns
  the site-local physical edge; Raids owns which proved candidate has tactical arrival,
  reinforcement, retreat, or extraction value;
- `RAID-3` in `docs/systems.md` requires world approach before breach, while terrain must
  not require a permanent Anchor-to-base corridor.

Minimum integrated contract:

- one or more stable site-edge connector IDs derived from site-local physical geometry;
- stable terrain approach node/link IDs reaching those connectors with movement
  clearance, visibility/occlusion evidence where known, tags, and provenance;
- explicit evidence coverage so no connector is inferred from a missing array;
- version/checksum identity suitable for the existing headless adapter;
- no tactical labels, arrival ranking, permanent Anchor corridor, Unreal Actor
  dependency, or duplicate terrain/site generator.

Acceptance evidence:

- one generated settlement exposes at least one legal terrain-to-site route and can
  expose multiple approach candidates when geometry supports them;
- removing edge or terrain-link coverage produces a causal unknown-evidence result;
- Raids can cite stable terrain and site IDs when rejecting an internal arrival shortcut
  or a sealed approach;
- the integrated recipe remains deterministic and passes owning automation through
  Unreal Runner.

Needed coordination result: identify the owning production structures/adapters for both
sides of this seam and either integrate them or return the smallest follow-up exchanges
to the owning workstreams. Do not assign tactical arrival roles in the shared layer.

Requirements checked: Global none; generated-content boundary, `BUILD-1`, `RAID-3`, and
`RAID-4`; exceptions: none.

## Response

Accepted as a shared pure-data composition seam, with one missing Built Sites input.

- World-owned sources already exist: `FGatersTerrainSemanticField` supplies neutral
  terrain facts, `FGatersTerrainNavigation` supplies deterministic path cells, and
  `FGatersSiteRoutePlan` supplies stable site and route IDs.
- Built Sites-owned sources already exist: `FGatersBuiltSiteRecipe` supplies stable
  spaces, connections, movement modes, physical clearances, blockers, visibility, source
  IDs, checksum identity, and explicit evidence coverage.
- The generated settlement recipe currently exposes outdoor path-centerline spaces but
  does not identify which spaces are legal site-edge connectors. Primary will not infer
  an edge from `outdoors`, because that reproduces the internal-arrival shortcut this
  packet is intended to prevent.
- Primary should own one future pure composer over those accepted inputs. Its output is a
  versioned world-site approach recipe containing terrain path/node IDs, one explicit
  Built Site edge-space/connection ID, movement and clearance facts, visibility coverage,
  source IDs, and checksum. It assigns no tactical role or ranking.
- `WORLD-4` requests the smallest missing neutral edge-connector contract from
  Settlements, Bases & Dungeons. Shared composer implementation should wait for that
  answer; terrain and land-access work remains independent.

Requirements checked: `BUILD-1`, `RAID-3`, `RAID-4`; exceptions: none.

## Resolution

Accepted as the completed coordination handoff.

- Source inspection confirms the named terrain, navigation, site-route, and Built Site
  recipe seams exist as pure production inputs.
- Primary correctly rejects deriving a site edge from generic `outdoors`; that would
  recreate the internal-arrival shortcut RAID-6 must prevent.
- The future composer remains Primary-owned and unimplemented. No integration or machine
  promotion is claimed by this Resolution.
- [`WORLD-4`](WORLD-4-site-edge-connector-contract.md) is the correct next prerequisite:
  Settlements must prove neutral site-edge connectors before Primary can compose terrain
  approaches and before Raids can assign tactical arrival value.
- Raids will consume the composed recipe by stable physical IDs and will not add a local
  terrain path, site-edge inference, or second site generator.

Requirements checked: generated-content boundary, `BUILD-1`, `RAID-3`, and `RAID-4`;
exceptions: none.
