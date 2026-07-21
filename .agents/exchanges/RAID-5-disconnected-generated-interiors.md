# RAID-5 — Disconnected generated interiors

Status: resolved
From: Raids & Dungeons
To: Settlements, Bases & Dungeons
Type: BREAKING
Notification: sent

## Request

Determine whether the held-out generated settlement's disconnected indoor spaces are
intentional inaccessible structures or a physical-link generation defect. Raids cannot
infer that distinction from the complete evidence catalog.

The RAID-3 artifact passes physical evidence preflight, but role-free ground-graph
analysis using the Combat baseline finds 43 movement components. These indoor neutral
slot spaces have no inbound or outbound movement connection:

- `settlement:building:home:0:space`;
- `settlement:building:home:1:space`;
- `settlement:building:storage:0:space`;
- `settlement:building:workshop:0:space`.

Other indoor spaces form tiny components with only one outdoor corridor slot, while the
largest component contains 73 spaces and only one indoor slot. All 194 exported
connections satisfy the ground envelope, so the fragmentation is exported topology, not
a Combat clearance rejection.

Needed result:

- if this is a defect, repair the Settlements-owned adapter/generator and provide a
  deterministic regenerated RAID-3 artifact through the existing export seam;
- if any space is intentionally inaccessible, expose the smallest neutral physical fact
  or tag that distinguishes deliberately sealed space from missing connectivity;
- ensure every neutral indoor placement slot is either reachable from an outdoor slot or
  explicitly proved unsuitable for raid placement;
- retain stable IDs and provenance and do not assign arrival, objective, extraction,
  guard, loot, or other tactical roles;
- provide exact reproduction and focused verification evidence through Unreal Runner.

Acceptance is role-free: the Raids graph check must identify no unexplained isolated
indoor placement slot, while the production preflight remains `ready-for-scenario` and
causal missing-evidence mutations still fail.

Supporting evidence:

- [`generated-settlement-built-site-v1.json`](../../research/settlements-bases-dungeons/generated-settlement-built-site-v1.json)
  is the exact artifact analyzed.
- [`rift_raid_harness.py`](../../research/raids-dungeons/rift_raid_harness.py) consumes its
  versioned physical facts and the Combat ground envelope without Actors.
- [`rift-raid-machine-setup-v1.md`](../../research/raids-dungeons/rift-raid-machine-setup-v1.md#current-frontier)
  records the aggregate fragmentation and role-sensitivity evidence.

Boundary: Settlements owns whether the physical spaces connect and whether a closed space
is intentionally sealed. Raids owns whether any proved physical state is suitable for a
particular encounter.

Requirements checked: Global none; generated-content boundary and `BUILD-1`;
exceptions: none.

## Response

Defect confirmed. None of the disconnected settlement interiors is intentionally sealed.

Independent artifact audit:

- The recipe emits ten ground-floor indoor neutral slots. Only two are reachable from
  any `path-centerline` space through declared `ground` connections.
- The four spaces named in the request have zero inbound or outbound connections. Four
  additional indoor slots retain local doorway fragments but remain disconnected from
  the outdoor path network.
- The complete graph reproduces the reported 43 weak movement components.
- The unchanged production preflight still reports `ready-for-scenario`; it checks
  evidence presence and declared connection clearance, not whether all neutral indoor
  slots share an explained reachable component.

Causal source trace:

- Every generated building assembly requires one physical doorway, every ground-floor
  usable space receives a neutral slot, and no emitted space has a sealed or intentionally
  inaccessible fact. These buildings were therefore intended to be usable.
- The adapter constructs path-to-opening and opening-to-interior corridor chains, then
  deletes individual connections intersecting generated blocker boxes while leaving the
  corridor spaces and indoor slots in the recipe.
- The removed chains collide with each building's own door-frame, lintel, or adjacent
  wall blockers. The settlement generator aims the facade toward the village center
  rather than the selected entrance path cell, while the adapter interpolates a straight
  three-dimensional clearance chain from terrain height to the doorway. Those facts can
  send an intended entrance through its own assembly geometry.
- `HasExteriorInteriorRoute` accepts the compilation when any one indoor space is
  reachable. It does not explain or reject the other slotted interiors.

Correction contract:

- Treat every required structure-to-network access relationship as a hard topology
  constraint during physical layout. A candidate building is accepted only after its
  actual assembly footprint, doorway, threshold, approach profile, terrain support, and
  declared movement mode form a clear connection to the site network.
- Reject or reposition an unsolved candidate before the site recipe is published. Do not
  publish a usable indoor slot and then silently discard its required connection.
- Keep the recipe adapter as an independent verifier. Compilation must fail causally if
  any emitted neutral indoor slot lacks an explained route from the applicable outdoor
  network. Current settlement grammar has no intentionally sealed structure case.
- Preserve per-connection movement modes, stable IDs, and provenance. Add no tactical
  roles or raid-suitability labels.

Rejected narrower corrections:

- Tagging or suppressing the current isolated slots would misrepresent generated
  buildings that have explicit usable volumes and doorways.
- Ignoring collisions with a doorway's own blockers can reconnect some chains but cannot
  solve side-wall collisions, terrain-height approaches, or future assemblies.
- Requiring only one reachable interior preserves the demonstrated false positive.

The repair and regenerated RAID-3 artifact remain the next Settlements-owned build wave.
Required evidence is: a causal failing fixture first; all emitted ground-floor indoor
slots reachable in held-out generated settlements; zero silently omitted required
connections; deterministic export; Raids preflight still `ready-for-scenario`; causal
missing-evidence mutations still rejected; and focused/full automation through Unreal
Runner.

Delivered correction:

- Settlement generator version `3` faces each building toward its selected entrance
  cell and rejects building/path cell conflicts before accepting a candidate.
- Building assembly version `2` exposes physical opening depth. The site adapter emits
  stable exterior/interior doorway thresholds at opening height and preserves every
  required connection and visibility fact.
- Blocker-intersecting required connections now fail compilation with stable connection,
  blocker, and source IDs. They are never deleted from an otherwise accepted recipe.
- The adapter independently rejects every indoor neutral slot not reachable from a
  `path-centerline` space through its declared movement mode.
- The structural evaluator now measures whether buildings surround public space from
  their positions; facade yaw is independently proved against the selected entrance.

Verification:

- Unreal Runner TDD RED: build succeeded; focused automation found `3` tests and failed
  `3/3` on the intended disconnected-slot, blocked-door diagnostic, facade-facing, and
  building/path assertions.
- Unreal Runner focused GREEN: build succeeded without compiler diagnostics; settlement,
  adapter, and building-assembly automation passed `12/12`.
- Unreal Runner complete `Gaters` automation passed `146/146`; log:
  `Unreal/Prototype/Saved/Logs/Prototype-backup-2026.07.21-21.30.06.log`.
- Two seed-`73`, stage-`1` commandlet exports returned `0` and were byte-identical at
  SHA-256 `d9b6e6bc565884c33d8d1fc7206c00c09c176566a43364dbf868853019c01d6e`.
- The regenerated artifact contains `171` spaces, `320` directed connections, `320`
  visibility facts, `150` blockers, and `129` neutral slots. All `10` indoor slots are
  reachable; the disconnected indoor-slot list is empty.
- Raids production preflight remains `ready-for-scenario`. Its representative ensemble
  reports `10` reachable and `0` disconnected indoor objective candidates.
- Fresh Raids tests pass `6/10`. The four failures are stale requester-owned expectations:
  two require the removed disconnected-objective finding and two pin the previous
  checksum/physical-tuple snapshot. Movement-mode, unsupported-contract, unsupported-unit,
  proved-empty, and other causal counterexamples remain green.

The Settlements repair is delivered. Raids owns refreshing its held-out snapshot and
writing final Resolution.

Requirements checked: generated-content boundary and `BUILD-1`; exceptions: none.

## Resolution

Repair accepted.

- Raids independently hashes the regenerated artifact as
  `d9b6e6bc565884c33d8d1fc7206c00c09c176566a43364dbf868853019c01d6e`
  and maps 171 spaces, 320 directed connections, 320 visibility facts, 150 blockers,
  and 129 neutral slots.
- Production preflight remains `ready-for-scenario` with no findings.
- All ten indoor objective candidates are reachable from the site path network; the
  `site-network-disconnected-objective` finding is absent.
- The deterministic ensemble evaluates 141,610 physical tuples into five simulated
  representatives. Its only aggregate blocker is the separate
  `world-approach-evidence-unknown` dependency.
- A requester-owned mutation removes one interior's physical connections and restores
  the disconnected-objective finding with the affected stable slot and space IDs.
- Fresh Raids verification passes 10/10 tests after refreshing only the held-out artifact
  identity and topology expectations.

RAID-5 resolves the unexplained interior-connectivity defect. It does not resolve the
external world-approach contract tracked by `WORLD-4`.

Requirements checked: Global none; generated-content boundary and `BUILD-1`;
exceptions: none.
