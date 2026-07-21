# Unanchored Recovery Design

## Outcome

A generated humanoid standing on generated ground receives an impact and produces a
baked recovery animation without any body part being permanently passive. It either
holds its stance or releases one foot, takes a corrective step, replants, and settles.

## Candidate mechanisms

| Mechanism | Source | Work deleted | Limitation |
|---|---|---|---|
| Active body + generated contact schedule | Adapt Blender rigid bodies and constraints | Hand-authored impact, stumble, and recovery clips | First proof uses a simple support rule, not human cognition |
| Procedural IK pose sequence | Build on current keyframe synthesizer | Hand-keying individual clips | Motion only imitates physics; contacts remain authored rules |
| Learned control policy | Build/train | Most explicit motion rules | Needs training data, compute, and a separate evaluator |

**Call:** active body plus generated contact schedule. It deletes the closest remaining
manual work while keeping every failure inspectable in Blender.

## Capability graph

| Node | Unlocks | Edge | Source | Contract | Verifier | Promotion gate |
|---|---|---|---|---|---|---|
| Blender rigid-body world | physical motion | SEQUENCE | Borrow | Simulates active collision bodies and limited spring joints | Blender reopen | Existing native solver runs |
| Recovery planner | foot schedule | AND | Build | Event + stance -> hold or one bounded recovery step | Pure Python tests | Direction, side, and bounds pass |
| Foot contact adapter | stable ground contact | AND | Adapt | Plant/release/replant active feet against generated ground | Scene inspection | No humanoid part is passive; schedule persists |
| Balance target adapter | upright recovery | AND | Adapt | Move pelvis over the generated support center without making it passive | COM measurement | Final COM returns inside support region |
| Active recovery compiler | baked animation | SEQUENCE | Build | Brief -> simulation -> skeleton action + FBX | Black-box harness | Challenge set passes twice identically |

`Active recovery compiler <- AND(Blender solver, recovery planner, foot contact adapter, balance target adapter)`

If this existed, we would no longer need to author a separate animation for every shove,
stumble direction, and corrective step.

## Data flow

1. Body proportions generate masses, collision proxies, and joint limits.
2. Impact direction and strength generate a hold-or-step recovery plan.
3. Both feet begin as active rigid bodies with temporary plant constraints.
4. For a step, one plant releases, its ground target moves, and it replants.
5. A generated pelvis target moves above the final support center to restore balance.
6. Blender resolves the body reaction while joint springs preserve anatomical limits.
7. Proxy transforms bake onto the generated skeleton and export as Blend and FBX.

## Verification

- Independent verifier: reopened Blender artifact plus numeric scene measurements.
- Challenge set: weak and strong impacts from front, back, left, and right.
- Required evidence: no passive humanoid parts, bounded pre-impact drift, no floor
  penetration, expected step side and direction, bounded step length, settled final
  motion, persistent baked action, and repeatable semantic manifests.
- Diagnostic artifact: rest, impact, step, and settled renders for every failed case.
- Champion gate: retain the anchored physical build until every challenge case passes.

## Deliberate limits

- One recovery step maximum.
- Flat ground only.
- No walking cycle, navigation, hands, obstacle stepping, or runtime game integration.
- The first controller is an inspectable rule, not a trained policy.
