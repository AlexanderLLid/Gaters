# Uneven-Terrain Foot Placement Design

Status: approved by the Human's instruction to verify the existing champion and continue
autonomously on the recommended next path.

## Outcome

- One generated humanoid stands and moves across deterministic uneven supports while
  both solved feet stay within contract distance of their walkable contact planes.
- Native `CharacterMovement` remains authoritative for capsule movement and floor state.
- Unreal owns live traces, pelvis adjustment, and leg solving. Blender only emits the
  mechanical bones required by the runtime contract.
- The editor-only intake adapter remains unloaded during runtime evidence.

If this existed, we would no longer need to manually add or inspect baseline
uneven-ground foot contact for every compatible generated humanoid.

## Native inventory and boundary

- Borrow: UE 5.8 `ACharacter` and `UCharacterMovementComponent` already pass the flat
  route and own capsule movement, walkable-floor tests, falling, and landing.
- Borrow as challenger: the installed `AnimationWarping` plugin is a stable Runtime
  plugin, but `FAnimNode_FootPlacement` itself is marked `Experimental` in UE 5.8.
- Borrow: `AnimGraphRuntime` `FAnimNode_TwoBoneIK` solves each deform leg from the
  Foot Placement targets. No custom trace or IK solver is permitted in this experiment.
- Adapt: the current generated skeleton lacks the distinct ball and IK-foot roles that
  Foot Placement validates. Add `ball_l`, `ball_r`, `ik_foot_l`, and `ik_foot_r` as
  mechanical roles without changing mesh shape, proportions, physics bodies, or art.
- The stable foundation is the contact contract and adapter seam, not the experimental
  node. A later native replacement may consume the same generated roles and evidence.

## Candidates

1. **Foot Placement plus Two Bone IK — selected.** Input: generated target roles,
   locomotion pose, CharacterMovement floor state. Output: pelvis and IK-foot targets,
   then solved deform legs. Guarantee: native nodes produce measured terrain contact.
   Deletes manual traces and a custom leg solver. Limitation: Foot Placement is
   experimental and must remain replaceable.
2. **FullBodyIK with custom floor-goal orchestration.** Reuses the proven IK Rig, but
   requires us to build the contact planner that Foot Placement already supplies. Keep
   only as the fallback if the challenger fails its contract.
3. **Control Rig terrain graph.** Can express the behavior, but adds editor graph
   generation and a second procedural rig surface before the simpler native graph is
   falsified. Rejected for this wave.

## Capability graph

| Node | Unlocks | AND/OR/SEQUENCE | Borrow/Adapt/Build | Contract | Work deleted | Verifier | Challenge set | Promotion gate |
|---|---|---|---|---|---|---|---|---|
| Generated mechanical foot roles | Native Foot Placement evaluation | SEQUENCE | Adapt | Source and Unreal skeleton contain distinct deform foot, ball, and IK-foot roles with stable parents | Per-character skeleton patching | Reopened Blender source plus native Unreal readback | Normal generated humanoid; missing-role counterexample | Same generator and intake command pass twice with stable reports |
| CharacterMovement floor authority | Grounded contact graph | AND | Borrow | Capsule and movement component report walkable floor while owning translation | Custom movement and capsule floor logic | Existing flat-route native report and policy | Idle, walk, run, turn, jump, fall, land | Existing flat champion remains passing |
| AnimationWarping Foot Placement | IK-foot targets and pelvis offset | AND | Borrow | Native node evaluates with two valid legs and emits terrain-sensitive target transforms | Custom foot traces, plant locks, slope alignment, pelvis planner | Independent native pose/contact report | Split-height stance, step, slope, unsupported edge | Every target changes causally and remains within contract bounds |
| AnimGraphRuntime Two Bone IK | Deformed legs reaching targets | SEQUENCE | Borrow | Each thigh-shin-foot chain reaches its native IK-foot target without stretch or NaNs | Custom leg solver | Native bone transforms versus independent world traces | Left-high, right-high, slope, moving capsule | Both deform feet meet contact bounds with bounded pelvis motion |
| Runtime graph adapter | Repeatable uneven route | AND | Adapt | Runtime-only graph composes source pose, Foot Placement, and bilateral Two Bone IK; editor adapter stays unloaded | Manual Anim Blueprint construction during feasibility testing | Automation test plus independent Python policy | Static split stance, short deterministic traversal, malformed contract | Two native and policy reports are byte-identical; flat route regresses cleanly |
| Generated grounded humanoid | End-to-end character machine | SEQUENCE | Build | One recipe regenerates, imports, moves, and contacts uneven terrain without manual setup | Manual per-character terrain-contact setup | Complete public harness | Fresh regeneration and held-out uneven layout | Handoff accepted; no registry promotion until later recovery and variation gates pass |

## Verification order

- Seed nodes: passing generated intake, evaluated FullBodyIK evidence, and passing native
  CharacterMovement flat route.
- Falsifier 1: regenerate and import the four missing mechanical roles. Failure keeps the
  sixteen-bone champion and diagnoses the exact export/import role loss.
- Falsifier 2: evaluate native Foot Placement on a split-height stance. Failure rejects
  the plugin challenger before route or animation-graph generation expands.
- Falsifier 3: traverse a short deterministic step and slope. Independent traces compare
  solved foot contact, penetration/gap, pelvis offset, floor state, and NaNs.
- First build wave: mechanical roles, static native graph, then traversal and repeatability.
- Selected frontier: generated mechanical foot roles, because Foot Placement cannot
  initialize without them and every later uneven-contact measurement depends on that
  mapping. No nearer blocker remains after the freshly passing flat-route verification.
- Unlocked dream machine: one generated mechanical recipe produces a grounded Unreal
  humanoid without manual rig, movement, trace, or baseline foot-IK setup.

Requirements checked: `CHAR-1`, `CHAR-2`, `CHAR-3`, `ART-1`, `ART-2`, generated-content
boundary. Exceptions: `ART-1` and `ART-2` are inputs for later visual assets only; this
experiment changes mechanical roles and runtime behavior, not art.
