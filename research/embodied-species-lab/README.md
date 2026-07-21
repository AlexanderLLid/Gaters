# Embodied species lab

Isolated proof of the top-level wish: a species brief plus an event produces an embodied
humanoid and motion without manual Blender edits.

## Run

```powershell
./Test-HumanoidMachine.ps1
./Test-PhysicalHumanoid.ps1
./Test-RecoveryMachine.ps1
./Test-LocomotionMachine.ps1
./Test-UnrealHumanoidIntake.ps1
./Test-CharacterMovementMachine.ps1
./Test-FootPlacementMachine.ps1
```

Unreal build and automation commands must be sent to the shared `Unreal Runner` task;
the Runner invokes the Unreal-backed scripts serially.

## Authority and outputs

- Authoritative: `species/humanoid.json`, `species/ik-evaluation.json`, `reaction.py`,
  `physical_profile.py`, `locomotion.py`, `species/character-movement-route.json`, the
  `species/foot-placement-route.json` contract, independent policies, and the Blender
  generators.
- Derived under ignored `Derived/humanoid-v1/`: `humanoid.blend`, `humanoid.fbx`,
  `reaction.json`, and `manifest.json`.
- Derived under ignored `Derived/humanoid-physical-v1/`: a constrained Blend file,
  baked FBX, physical profile, semantic manifest, and rest/peak/settled PNG previews.
- Derived under ignored `Derived/humanoid-recovery-v1/<case>/`: an active-body Blend,
  baked FBX, generated recovery plan, manifest, and rest/impact/step/settled previews.
- Derived under ignored `Derived/humanoid-locomotion-v1/`: one source Blend, eight
  mechanical in-place FBXs, and a deterministic locomotion manifest.
- Derived under ignored `Derived/P/`: the packaged UE 5.8 editor-only Physics Asset
  profile adapter.
- Derived under ignored `Derived/M/`: the packaged UE 5.8 runtime-only native movement
  challenger.
- Derived under `Unreal/Prototype/Content/Gaters/Generated/CharacterLab/`: the imported
  Skeletal Mesh, Skeleton, Physics Asset, IK Rig, and minimal preview map.

## Current guarantee

- The same impact event and seed produce the same semantic reaction recipe.
- A different seed can produce a different bounded reaction.
- The brief generates one block humanoid, a twenty-bone hierarchy, complete rigid
  weights, one-second impact reaction, Blender source, and FBX transport. The hierarchy
  includes deforming left/right ball bones and non-deforming left/right IK foot targets.
- The build clears stale output, rebuilds twice, reopens the `.blend`, and verifies
  hierarchy, weights, modifier, material, scale, timing, action, and neutral recovery.
- The physical challenger generates a 75 kg body with fourteen collision parts,
  thirteen limited spring joints, anchored feet, and protected elbow/knee directions.
- Blender simulates a chest impact, then the generator bakes the result onto the
  twenty-bone skeleton. The verifier bounds pre-impact drift and peak displacement,
  checks settling, reopens the Blend file, validates the baked action, renders previews,
  rebuilds twice, and compares semantic manifests byte-for-byte.
- The recovery challenger makes every humanoid body part active. Impact direction and
  strength generate a stepping foot, bounded foot path, and final support center.
- Kinematic contact targets guide active feet; a generated pelvis target restores the
  center of mass over the new stance. Blender still resolves the articulated body.
- The challenge harness verifies left, right, forward, and backward impacts twice each,
  including step direction, target accuracy, floor clearance, support, settling, reopened
  animation, FBX, previews, and identical semantic manifests.
- The locomotion compiler produces `A_Idle`, `A_TurnLeft`, `A_Walk`, `A_Run`, `A_Stop`,
  `A_Jump`, `A_Fall`, and `A_Land`. The source Blend is reopened to verify action ranges,
  rigid weights, hierarchy, and zero root curves; two builds produce identical manifests.
- The UE 5.8 intake challenger imports the generated Skeletal Mesh with the exact
  twenty-bone hierarchy and meter-authored scale, imports all eight animations onto its
  Skeleton, validates the four Foot Placement roles, and samples every root track at zero
  centimeters.
- The native IK Rig has one pelvis-rooted FullBodyIK solver connected to both foot goals.
  An independent `FIKRigProcessor` test displaces each foot goal upward by 10 cm and
  measures the evaluated pose: left/right driven displacement is 9.828911/9.389122 cm,
  goal error is 0.171782/0.612087 cm, and opposite-foot drift is 0.617606/0.162854 cm.
- The isolated native adapter validates the profile graph and rebuilds the Physics Asset
  through UE 5.8 utilities. Readback matches all fourteen profile bodies and thirteen
  constraints; a copied profile with an unknown bone is rejected causally.
- One command packages the adapter, passes six native topology tests plus one native
  evaluated-IK test, imports twice, compares reports, policies, and IK evidence
  byte-for-byte, and creates
  `/Game/Gaters/Generated/CharacterLab/L_CharacterLab` with a floor and generated mesh.
- Evidence lives under `Unreal/Prototype/Saved/CharacterLab/`; the independent policy
  passes in `unreal-intake-policy-2.json`.
- The runtime-only challenger loads the cooked mesh, Skeleton, Physics Asset, IK Rig, and
  eight clips without loading the editor adapter. Native `ACharacter` and
  `CharacterMovement` own the capsule, flat-floor query, walking, running, turning,
  stopping, jumping, falling, and landing.
- Two fixed-step runs are byte-identical. The independent policy measures 150/450 cm/s
  walk/run speed, 576.168920 cm forward displacement, 106.154803 cm turn displacement,
  two falling samples, and a stopped grounded finish.
- The runtime-only uneven-terrain challenger composes native experimental
  `FAnimNode_FootPlacement` with native `FAnimNode_TwoBoneIK`; CharacterMovement remains
  responsible for capsule motion and live floor finding. It traverses split supports, a
  step, a slope, and a stop without loading the editor adapter.
- Two fixed-step uneven runs are byte-identical. The independent policy measures
  147.603482 cm route displacement, 12.213489 cm CharacterMovement floor-height change,
  2.721855 cm maximum foot-contact error, 7.333659 cm maximum pelvis offset, and
  10.721852 cm maximum terrain target response with no NaNs. Native/policy SHA-256 values
  are `A14C2E19DBE7B6248434F4FDC4A661A68636679567C54346968FA0C8CC496DCF` and
  `30A65E944E54B52557EE20E41B22371364D5161B30E2BC0B8DD94D277AA20A86`.

## Deliberate limits

- Mechanical placeholder only: no useful art, topology deformation, hands, face, hair,
  equipment, network runtime, or visual evaluator.
- Both reactions are generated offline motion artifacts. The physical version proves
  event/body-to-motion compilation, not continuous runtime simulation.
- The anchored champion still anchors feet. The recovery challenger replaces this with
  generated active-foot and pelvis targets, but supports only one step on flat ground.
- The eight clips and flat-route challenger prove generated mechanical source motion,
  Unreal import, capsule movement, and baseline state transitions. They do not yet prove
  repeated stepping, navigation, playable input, or production animation blending.
- The uneven-terrain challenger proves native floor response from a reference pose. It
  does not yet prove generated clip advancement, evaluated animated poses, repeated gait
  contacts, or production blending. Native Foot Placement is experimental and remains a
  replaceable challenger behind the generated skeleton-role contract.
- Foot orientation planning, physics-driven fall/recovery/locomotion transitions, and
  network authority remain unproven.
- Next isolated experiment: advance the generated walk clip through the same native
  uneven-terrain graph and verify evaluated pose motion plus bounded foot contact. Unreal
  stays responsible for floor queries, collision, IK evaluation, and runtime authority.
