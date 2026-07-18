# Neutral motion round trip

## Outcome

- One style-neutral JSON brief and Blender Python generator produce a five-bone rig,
  fully weighted primitive mesh, one-second clip, root path, and contact-event timeline.
- Blender `.blend` and FBX files are disposable derived artifacts. Unreal imports native
  Skeletal Mesh, Skeleton, and Anim Sequence assets under ignored generated content.
- The same import runs twice with contract-equivalent hierarchy, scale, timing, and root
  samples. The derived animation is recreated before import because Unreal reimport
  compounded the animation unit scale during the falsifying test.

## Evidence

- Blender 5.2 harness rebuilds twice with an identical semantic manifest, removes stale
  output, reopens the `.blend`, and verifies five declared bones, complete vertex weights,
  a 30 fps frame range `1..31`, root samples `0 / 0.5 / 1.0 m`, and three contact markers.
- Unreal 5.8 imports exactly one `SkeletalMesh`, one `Skeleton`, and one `AnimSequence`.
- Required hierarchy: `root -> pelvis -> {spine, foot_l, foot_r}` with no Blender
  armature-object bone above `root`.
- Unreal-measured mesh size: `44 x 53 x 123 cm`.
- Unreal-measured clip: `1.0 s`, `31` sampled keys, root X samples
  `0 / 50 / 100 cm`.
- Both final import logs contain one success marker and zero Python, Interchange, or
  ensure failures.

## Honest boundary

- Blender timeline markers survive in the source manifest but FBX imports zero Unreal
  animation notifies. A later adapter must write versioned gameplay events or reject a
  clip that requires them; this fixture does not claim they transported automatically.
- This positive fixture does not evaluate foot sliding, penetration, reach, deformation,
  IK retargeting, terrain response, interruption windows, visual power, or runtime cost.
- No rig, motion factory, or mechanical evaluator is promoted until deliberately broken
  clips fail for their intended reasons.
