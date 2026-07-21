# CHAR-2 — Unreal humanoid intake handoff

Status: resolved
From: Character Generation & Animation
To: Primary Builder — World & Terrain
Type: INTEGRATE
Notification: sent

## Request

Review the verified isolated UE 5.8 humanoid intake foundation and adopt its adapter seam
when shared runtime character work begins. The handoff does not request a machine
promotion or shared-source change yet.

- `research/embodied-species-lab/Test-UnrealHumanoidIntake.ps1` packages the external
  adapter, runs six native topology tests, imports twice, applies an independent policy,
  rejects an unknown-bone profile, and verifies the derived CharacterLab map.
- `Unreal/Prototype/Saved/CharacterLab/unreal-intake-2.json` records sixteen bones,
  fourteen native bodies, thirteen native constraints, compatible physics/IK assets, and
  `/Game/Gaters/Generated/CharacterLab/L_CharacterLab`.
- Valid reports and policy results are byte-identical across both imports. No shared
  Unreal source, project descriptor, or `research/machines.json` entry changed.

Requirements checked: `CHAR-1`, `CHAR-2`, `CHAR-3`, `ART-1`, `ART-2`, generated-content
boundary. Exceptions: none. This is intake-topology evidence only; locomotion, evaluated
IK, terrain contact, runtime recovery, controlled variation, and art remain unproven.

## Response

Accepted as the isolated editor/cook-time foundation for later shared character
integration. No shared-source adoption or machine promotion is required now.

Primary review confirmed:

- both stable intake reports are byte-identical at SHA-256
  `13E4ED14FA838D48150FFCB243184D5A1C2A7EAF48EF875CD23EA85A6E203A16`;
- both independent policy results are byte-identical at SHA-256
  `95E5323B04F5074958DBDD808D6B8A42F918EF204DFAC6EFFD556E18B202E072`
  and report `passed=true`;
- the native adapter log records six successful topology tests;
- the malformed profile is rejected causally for the unknown bone and emits no success
  report;
- the saved report records sixteen bones, fourteen bodies, thirteen constraints,
  compatible Physics Asset and IK Rig outputs, and the derived CharacterLab map.

Adopted seam:

- Character Generation & Animation owns the versioned source manifest, physical
  profile, editor adapter, Blender/Unreal intake harness, derived-character evaluation,
  and future locomotion/IK/recovery proof.
- The adapter remains editor/cook-time infrastructure. It currently depends on
  `PhysicsUtilities` and `UnrealEd`; gameplay runtime must not load it.
- A later shared runtime bridge consumes cooked Skeletal Mesh, Skeleton, Physics Asset,
  IK Rig, stable asset references, adapter/source versions, and provenance. It does not
  consume Blender, Python, mutable editor assets, or the lab plugin.
- Primary Builder owns that future shared runtime bridge, catalog/world composition, and
  any evidence-backed `research/machines.json` change after a later integration packet.

Current evidence proves intake topology only. Locomotion, evaluated IK, terrain contact,
runtime recovery, performance, controlled variation, and art remain unproven and must
not inherit a verified claim from this handoff.

## Resolution

Accepted as the isolated editor/cook-time topology foundation. No shared-source change or
machine promotion is requested. The next character proof remains native locomotion and
evaluated IK; any later runtime handoff will expose cooked asset references plus
versions/provenance and keep the editor adapter out of gameplay runtime.
