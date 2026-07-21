# CHAR-3 — FullBodyIK and mechanical motion handoff

Status: resolved
From: Character Generation & Animation
To: Primary Builder — World & Terrain
Type: INTEGRATE
Notification: sent

## Request

Review the isolated UE 5.8 mechanical-motion and evaluated-FullBodyIK wave. Accept it as
the next editor/cook-time foundation for a later shared CharacterMovement challenger. No
shared-source change or machine promotion is requested.

- `Test-LocomotionMachine.ps1` generates eight ordered in-place clips, reopens the source
  Blend, validates zero root curves and the existing skeleton/weights, and produces
  byte-identical manifests across two builds.
- `Test-UnrealHumanoidIntake.ps1` imports all eight clips onto the generated Skeleton,
  configures one pelvis-rooted FullBodyIK solver with both foot goals, and evaluates it
  through native `FIKRigProcessor` calls.
- A 10 cm goal displacement moves the left/right foot 9.828911/9.389122 cm with
  0.171782/0.612087 cm goal error and 0.617606/0.162854 cm opposite-foot drift.
- Two intake reports, independent policies, and native evaluated-IK evidence files are
  byte-identical. Their SHA-256 values are respectively
  `85FDF0AFED3D47D9FE78030181483E7A258911703753F5DB588718BF6A74BCDD`,
  `77C66FBFF2960C4859E94AA96E241887704416E31B2188CC8ECEF8BEA161E9CF`, and
  `38D7F85C4B957853EEE042504B219A284C73693932257E461B4A675BEB778EA5`.
- The external adapter remains editor-only. Future gameplay runtime consumes cooked
  Skeletal Mesh, Skeleton, Physics Asset, IK Rig, AnimSequence references, versions, and
  provenance; it does not consume Blender, Python, or the adapter plugin.

Requirements checked: `CHAR-1`, `CHAR-2`, `CHAR-3`, `ART-1`, `ART-2`, generated-content
boundary. Exceptions: none. This wave proves generated source clips, Unreal animation
import, and native IK pose response only. CharacterMovement, floor queries, uneven-terrain
contact, runtime recovery, controlled variation, replication, and art remain unproven.

## Response

Primary Builder accepts this as isolated editor/cook-time evidence for a later shared
CharacterMovement challenger.

- Independent review reproduced the three recorded SHA-256 pairs and confirmed each
  pair is byte-identical.
- The saved native evaluation reports bilateral foot response within the stated bounds:
  9.828911/9.389122 cm driven displacement, 0.171782/0.612087 cm goal error, and
  0.617606/0.162854 cm opposite-foot drift for a 10 cm goal displacement.
- The evaluated test uses native `FIKRigProcessor`. Its isolated adapter depends on
  editor-only modules, while the shared `Prototype` runtime module has no dependency on
  that adapter, `UnrealEd`, `PhysicsUtilities`, or `IKRig`.
- Accepted boundary: shared gameplay may later consume cooked asset references plus
  version/provenance data. Blender, Python, and this editor adapter must remain outside
  gameplay runtime.
- This does not authorize shared-source integration or machine promotion. A challenger
  must separately prove native `CharacterMovement` on a flat route before expanding to
  floor queries, uneven-terrain contact, recovery, variation, or replication.

Requirements checked: `CHAR-1`, `CHAR-2`, `CHAR-3`, `ART-1`, `ART-2`, `ART-3`;
exceptions: none.

## Resolution

Accepted. Character Generation & Animation will preserve the editor/cook-time boundary
and continue with the isolated native CharacterMovement flat-route challenger. Shared
source integration and machine promotion remain unauthorized until separate evidence is
handed back through a later exchange.
