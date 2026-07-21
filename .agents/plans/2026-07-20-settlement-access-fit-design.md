# Settlement access-fit design

## Outcome

Every emitted ground-floor indoor neutral slot in a generated settlement has an
explained physically clear route from the settlement movement network. A generator may
reject a building candidate; it may not publish a usable slot and silently remove its
required access.

Requirements checked: generated-content boundary and `BUILD-1`; exceptions: none.

## Evidence driving the change

- RAID-5 reproduces 43 movement components in the seed-73 stage-1 artifact.
- Ten ground-floor indoor slots are emitted; only two are reachable from a
  `path-centerline` space through `ground` connections.
- The current adapter deletes blocker-intersecting connections but leaves corridor
  spaces and indoor slots, then accepts the recipe when any one interior is reachable.
- Current settlement buildings all declare a usable doorway. None is intentionally
  sealed.

## Approaches

1. **Hide or mark isolated slots sealed.** Rejected because it changes the meaning of
   buildings that already declare usable interiors and doorways.
2. **Ignore collisions with the doorway's own blockers.** Rejected because side-wall,
   crossing-footprint, and terrain-height approach failures remain.
3. **Coarse access-aware planning plus an exact independent gate.** Selected. It removes
   known impossible candidates before compilation and keeps the adapter capable of
   falsifying the planner.

## Selected design

### Coarse settlement planning

- A building facade faces its selected `EntranceCell`, not the settlement center.
- A candidate is rejected when its path crosses an accepted building cell or its
  building cell conflicts with an accepted path.
- Accepted building cells remain reserved for every later building path.
- The planner remains deterministic and terrain-query-only. It does not consume Actors,
  final assets, tactical roles, or a settlement-wide movement type.

This is a conservative land challenger, not the general future support solver. It uses
the current grid contract to prevent known full-footprint crossings without adding a
second pathfinder.

### Doorway approach compilation

- Each doorway produces stable exterior and interior threshold spaces at doorway height.
- The exterior threshold lies beyond the wall and clearance radius along the doorway's
  outward direction. The interior threshold lies beyond them inward.
- The entrance path connects to the exterior threshold; the thresholds connect through
  the opening; the interior threshold connects to the ground-floor usable space.
- Every resulting connection declares its movement modes under `BUILD-1` and retains
  assembly/building provenance.

Thresholds prevent terrain-height interpolation from sending a clearance capsule through
the lintel while preserving the actual doorway width and headroom.

### Independent adapter gate

- Blocker-intersecting connections remain diagnostic evidence; they are not silently
  removed from an otherwise accepted recipe.
- Compilation fails with the involved stable connection, blocker, building, and source
  IDs when a required connection is blocked.
- Every emitted indoor placement slot must be reachable from a `path-centerline` space
  through connections supporting an applicable movement mode.
- Current settlement grammar has no sealed-building exception. A future generator may
  add an explicit neutral sealed physical fact, but this repair does not invent it.

## Data flow

```text
terrain queries + seed + site
→ deterministic candidate and path
→ entrance-facing building transform
→ conservative path/building conflict rejection
→ modular building assembly
→ threshold and connection compilation
→ independent blocker and slot-reachability gate
→ Built Site Recipe or causal failure
```

## Verification

- TDD red: the held-out seed-73 recipe exposes all eight disconnected indoor slots.
- Planner fixtures prove facade-to-entrance alignment and reject path/building conflicts.
- Adapter fixtures reject one disconnected indoor slot and one blocked required access
  connection with stable causal IDs.
- Focused generated settlement evidence proves all emitted ground-floor indoor slots are
  reachable and no required connection was silently omitted.
- Repeat export is byte-identical and produces a recorded SHA-256.
- Raids production preflight remains `ready-for-scenario`; its causal missing-evidence
  mutations still fail.
- Unreal build, focused automation, and complete `Gaters` automation run only through
  Unreal Runner.

## Deferred

- Water, submerged, climbing, flying, and mixed-support generators. The recipe already
  permits them through per-connection movement modes.
- Runtime settlement growth. This repair generates only the initial site recipe.
- General earthworks, stairs, ramps, and terrain mutation.
- Tactical suitability, objectives, attackers, defenders, loot, and raid roles.
