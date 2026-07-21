# Character Generation & Animation

## Current objective

- Prove that the generated walk clip advances and produces evaluated pose motion through
  the passing native uneven-terrain Foot Placement graph. Unreal continues to own live
  floor queries and terrain contact while the editor adapter remains unloaded.

## Owned outputs

- Generated bodies, skeletons, skinning, physical constraints, motion, reactions,
  recovery, locomotion, terrain contact, animation evaluation, and Blender-to-Unreal
  character adapters.
- Isolated evidence under `research/embodied-species-lab/` and character/motion source
  artifacts under `SourceAssets/Blender/`.

## Evidence

- [`Generated Character Pipeline Design`](../plans/2026-07-19-generated-character-pipeline-design.md)
  records the approved shared humanoid/quadruped pipeline, Blender/Unreal boundary,
  verification gates, build waves, and future UE Mutable runtime-generation lane.
- `research/embodied-species-lab/Test-HumanoidMachine.ps1` verifies generated hierarchy,
  weights, actions, recovery, Blender artifacts, and FBX transport.
- `Test-PhysicalHumanoid.ps1` and `Test-RecoveryMachine.ps1` verify physical constraints,
  impacts, bounded recovery, reopened artifacts, previews, and deterministic manifests.
- `research/embodied-species-lab/Test-UnrealHumanoidIntake.ps1` imports twice and
  independently evaluates the stable UE 5.8 result. Six native adapter tests and one
  native `FIKRigProcessor` test pass; readback verifies twenty bones, fourteen Physics
  Asset bodies, thirteen constraints, eight in-place clips, one pelvis-rooted FullBodyIK
  solver, bilateral 10 cm foot-goal response, and the generated CharacterLab map. A copied
  unknown-bone profile is rejected without a success report. Evidence:
  `Unreal/Prototype/Saved/CharacterLab/`.
- `research/embodied-species-lab/Test-LocomotionMachine.ps1` rebuilds the eight mechanical
  source clips twice and proves identical semantic manifests.
- `research/embodied-species-lab/Test-CharacterMovementMachine.ps1` packages a
  runtime-only plugin and drives the cooked humanoid through the ordered flat route twice.
  Native `ACharacter` and CharacterMovement own the capsule, floor, walking, running,
  turning, stopping, jumping, falling, and landing. The independent reports are
  byte-identical and verify 150/450 cm/s walk/run speed, 576.168920 cm forward movement,
  106.154803 cm lateral movement, two falling samples, and a stopped grounded finish.
- `research/embodied-species-lab/Test-FootPlacementMachine.ps1` packages the runtime-only
  challenger and traverses split supports, a step, a slope, and a stop twice. Native
  experimental Foot Placement plus native Two Bone IK retain bilateral walkable contact
  while CharacterMovement moves 147.603482 cm across 12.213489 cm of floor-height change.
  Reports are byte-identical; maximum contact error is 2.721855 cm, maximum pelvis offset
  is 7.333659 cm, and no case contains NaNs. This is reference-pose evidence; generated
  clip advancement and evaluated animated poses remain unproven.
- `research/creature-dna-proof/` is accepted as isolated tool-neutral structural-guide
  evidence. Its independent suite passes 16/16 tests; Houdini remains a challenger and
  no body, rig, motion, species, visual, or runtime capability is promoted.
- `research/procedural-head-machine/` is accepted as isolated mechanical evidence for its
  named tool-neutral contracts. Its independent suite passes 129/129 tests; all declared
  visual, production-body, topology, contact-motion, Unreal-runtime, and species gaps
  remain open.

## Exchanges

- [`CHAR-1 — Generated character pipeline contract`](../exchanges/CHAR-1-character-pipeline-contract.md)
  — resolved; establishes isolated character ownership and the cooked-asset runtime
  boundary.
- [`CHAR-2 — Unreal humanoid intake handoff`](../exchanges/CHAR-2-unreal-humanoid-intake.md)
  — resolved; accepts the topology adapter as editor/cook-time infrastructure without a
  shared-source change or machine promotion.
- [`CHAR-3 — FullBodyIK and mechanical motion handoff`](../exchanges/CHAR-3-fullbodyik-motion-handoff.md)
  — resolved; Primary Builder accepts the verified motion/IK foundation and requires
  separate native CharacterMovement evidence before shared integration or promotion.
- [`CHAR-4 — Native CharacterMovement flat-route handoff`](../exchanges/CHAR-4-native-character-movement-handoff.md)
  — resolved; Primary Builder accepts development-runtime flat-route evidence only and
  leaves cooked standalone proof, shared integration, and promotion unproven.
- [`CHAR-5 — Native uneven-terrain Foot Placement handoff`](../exchanges/CHAR-5-native-foot-placement-handoff.md)
  — resolved; Primary Builder accepts development-runtime reference-pose grounding
  evidence only, without shared integration, registry changes, or promotion.
- [`ART-1 — Complete BodyPlan ownership handoff`](../exchanges/ART-1-complete-body-plan-handoff.md)
  — answered; adopts the verified 11-node structural envelope without taking ownership
  of visual decisions or production-quality claims.
- [`CHAR-6 — CreatureDNA Houdini guide proof`](../exchanges/CHAR-6-creature-dna-houdini-proof.md)
  — resolved; accepts the isolated structural-guide evidence behind the tool-neutral
  offline boundary.
- [`CHAR-7 — Procedural head and deformation proof`](../exchanges/CHAR-7-procedural-head-deformation-handoff.md)
  — resolved; accepts the named mechanical contracts within their explicit limits.
- [`CHAR-8 — Complete BodyPlan registry correction`](../exchanges/CHAR-8-body-plan-registry-correction.md)
  — open; asks Primary Builder to replace only the stale six-module challenge wording.
