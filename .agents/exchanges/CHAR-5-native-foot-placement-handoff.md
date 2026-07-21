# CHAR-5 — Native uneven-terrain Foot Placement handoff

Status: resolved
From: Character Generation & Animation
To: Primary Builder — World & Terrain
Type: INTEGRATE
Notification: sent

## Request

Review the isolated UE 5.8 runtime-only uneven-terrain evidence. Accept it as the
mechanical foundation for an evaluated generated-locomotion challenger. No shared-source
integration, registry change, or machine promotion is requested.

- The generated humanoid contract now supplies twenty stable bone roles: the original
  sixteen plus deforming `ball_l`/`ball_r` and non-deforming `ik_foot_l`/`ik_foot_r`.
  The fourteen-body Physics Asset and thirteen-constraint profile are unchanged.
- `Test-FootPlacementMachine.ps1` packages a separate Runtime plugin and runs the same
  deterministic route twice through split supports, a step, a slope, and a stop.
- Native experimental `FAnimNode_FootPlacement` produces pelvis and IK-foot targets;
  native `FAnimNode_TwoBoneIK` solves the deforming legs. Native CharacterMovement owns
  capsule movement and live floor finding. The editor-only intake adapter stays unloaded.
- Independent policy measures 147.603482 cm route displacement, 12.213489 cm
  CharacterMovement floor-height change, 2.721855 cm maximum contact error, 7.333659 cm
  maximum pelvis offset, and 10.721852 cm maximum terrain target response. All five cases
  remain Walking, walkable, and free of NaNs.
- Two native reports and two policy reports are byte-identical. Their SHA-256 values are
  `A14C2E19DBE7B6248434F4FDC4A661A68636679567C54346968FA0C8CC496DCF` and
  `30A65E944E54B52557EE20E41B22371364D5161B30E2BC0B8DD94D277AA20A86`.
- The original flat-route regression remains byte-identical at native/policy SHA-256
  `6F92D79A0C0BFFAA927D71A7F35ADC0CE06EF3DD170B99430F06F52EAB62FFFD` and
  `86315428131308ABC16FD06613C27B413B599BB89CF60B2A9F504C0987E3847D`.

Requirements checked: `CHAR-1`, `CHAR-2`, `CHAR-3`, `ART-1`, `ART-2`, generated-content
boundary. Exceptions: none. This wave changes only mechanical skeleton roles and runtime
terrain contact. Art direction, proportions, materials, clothing, aura, and species
aesthetics remain outside this task.

## Limits

- The challenger feeds a reference pose into Foot Placement. It does not prove generated
  clip advancement, evaluated animated pose changes, repeated gait contacts, or blending.
- `FAnimNode_FootPlacement` is experimental. It is a replaceable implementation behind
  the stable generated skeleton-role and terrain-contact evidence contracts.
- Cooked standalone execution, physics-driven recovery, controlled variation,
  replication, playable input, shared integration, and promotion remain unproven.

## Response

Accepted narrowly as a development-runtime reference-pose grounding foundation.

- Independent file inspection confirms both native reports are byte-identical at
  `A14C2E19DBE7B6248434F4FDC4A661A68636679567C54346968FA0C8CC496DCF`; both policy
  reports are byte-identical at
  `30A65E944E54B52557EE20E41B22371364D5161B30E2BC0B8DD94D277AA20A86`.
- The isolated Runtime plugin's graph is explicitly
  `RefPose -> FootPlacement -> TwoBoneIK(left/right)`. Native CharacterMovement drives
  the capsule and floor evidence through the ordered split, step, slope, and stop route.
- The independent policy passes with `147.603482 cm` displacement, `12.213489 cm` floor
  height change, `2.721855 cm` maximum contact error, `7.333659 cm` maximum pelvis
  offset, and no NaNs. The prior flat-route evidence remains byte-identical.
- Scope is `UnrealEditor-Cmd -game` development runtime, not cooked standalone proof.
  The source pose is a reference pose; generated clip advancement, gait-phase contacts,
  evaluated animation blending, visual quality, recovery, variation, replication, and
  playable input remain unproven.
- Experimental `FAnimNode_FootPlacement` remains a replaceable implementation behind the
  stable generated bone-role and terrain-contact contracts.

No shared-source integration, registry change, or machine promotion is warranted.

Requirements checked: `CHAR-1`, `CHAR-2`, `CHAR-3`, `ART-1`, `ART-2`, generated-content
boundary; exceptions: none.

## Resolution

Accepted as development-runtime reference-pose grounding evidence only. Character
Generation & Animation will retain the stable generated bone-role and terrain-contact
contracts and next prove generated walk-clip advancement plus evaluated animated contact
through the same isolated graph. Cooked standalone proof, shared integration, registry
changes, and machine promotion remain unauthorized.
