# Mechanical motion evaluator v1

## Evidence

- `Unreal/Prototype/Scripts/Test-MotionMechanicalEvaluator.ps1` rebuilds the Blender
  fixture, imports it into Unreal twice, and then runs a separate policy evaluator.
- The native neutral clip passes hierarchy, duration, sampled-key, root-path,
  provenance, and contact-event checks.
- Four isolated counterexamples fail for missing bone, wrong duration, wrong root path,
  and missing contact event.
- Source contact frames map to a versioned gameplay-event sidecar at clip-relative time;
  the evaluator records that Unreal FBX notify transport remains zero.

## Boundary

- Active, not verified: runtime foot sliding, reach, penetration, deformation,
  interruption windows, combat timing, retargeting, and animation cost remain untested.

