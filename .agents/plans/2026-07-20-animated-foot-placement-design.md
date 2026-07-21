# Animated Foot Placement Challenger

## Outcome

- The generated `A_Walk` clip advances through the existing native uneven-terrain graph.
- Evaluated leg transforms change across animation phases while CharacterMovement crosses
  the existing step and slope and bilateral contact remains bounded.
- Two fixed-step runs produce byte-identical native and independent-policy evidence.

## Capability graph

| Node | Unlocks | AND/OR/SEQUENCE | Borrow/Adapt/Build | Contract | Work deleted | Verifier | Challenge set | Promotion gate |
|---|---|---|---|---|---|---|---|---|
| Generated `A_Walk` | Animated source pose | SEQUENCE | Borrow | Imported clip uses generated Skeleton and has non-zero leg motion | Manual animation authoring | Native asset and pose assertions | Opposed gait phases | Clip identity, time advance, pose delta |
| Native Sequence Player | Evaluated source pose | SEQUENCE | Borrow | Fixed-delta looping playback exposes asset time | Custom playback clock and sampler | Native automation plus independent report policy | Looping time samples | Byte-identical causal evidence |
| Existing Foot Placement graph | Grounded animated pose | AND | Adapt | `SequencePlayer -> FootPlacement -> TwoBoneIK` | Second terrain query and leg solver | Native contact assertions | Split support, step, slope, stop | Animated pose and prior contact guarantees pass together |
| CharacterMovement route | Moving terrain evidence | AND | Borrow | Capsule traverses the existing deterministic route | Custom movement and floor ownership | Existing route assertions | Step and slope | Prior flat and reference-pose regressions remain green |

## Options

- **Recommended:** replace the graph's reference-pose source with
  `FAnimNode_SequencePlayer_Standalone`. This is the smallest runtime-native adapter and
  leaves Foot Placement, Two Bone IK, and CharacterMovement unchanged.
- Use an Animation Blueprint asset. Rejected for this experiment because it introduces a
  mutable editor-authored derived asset without adding evidence.
- Sample `UAnimSequence` manually into a pose. Rejected because Unreal's Sequence Player
  already owns playback, extraction, looping, and update timing.

## Evidence contract

- New challenger evidence records generated clip identity, native node identity, asset
  times at opposed phases, evaluated thigh rotation delta, existing contact bounds,
  CharacterMovement displacement/floor change, stopped finish, and NaN state.
- The independent Python policy rejects wrong source nodes/assets, non-advancing time,
  static evaluated poses, contact regression, missing terrain traversal, and a moving or
  airborne finish.
- The current reference-pose challenger remains separately runnable and unchanged until
  the animated challenger passes.

## Boundary

- Blender remains responsible for generating the body, skeleton, skinning, and `A_Walk`.
- Unreal remains responsible for playback, capsule movement, floor queries, Foot
  Placement, Two Bone IK, collision, and runtime authority.
- No proportions, materials, clothing, species appearance, or other art-direction data
  enters this experiment.

## Frontier

- **Why now:** CHAR-5 proved grounding only from a reference pose; generated pose
  evaluation is the nearest missing causal link in `CHAR-1`.
- **If this existed, we would no longer need to** infer that a loadable generated clip is
  actually evaluated by the grounded runtime graph.
- Next failure determines the following experiment: playback failure stays at the source
  node; pose-with-contact failure stays at the graph; a passing result unlocks runtime
  impact/fall/recovery/return-to-locomotion.

Requirements checked: `CHAR-1`, `CHAR-2`, `CHAR-3`, `ART-1`, `ART-2`, `ART-3`, `ART-4`,
generated-content boundary; exceptions: none.
