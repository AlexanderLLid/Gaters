# Mechanical motion evaluator v2

## Evidence

- Unreal importer v2 records deterministic component-space `foot_l` and `foot_r` poses
  for every native sampled frame.
- Evaluator v2 treats each contact event as a stance start ending at the next contact.
- Neutral-step v1 was falsified at `50 cm` horizontal drift in both stance windows.
- Blender generator v2 produced `motion.neutral-step@2`; repeated Unreal imports measure
  `0 cm` drift in both stance windows.
- A held-out report with one planted sample shifted `10 cm` fails
  `motion.contact.foot_sliding`.

## Boundary

- This proves one unretargeted alternating two-foot fixture. Runtime retargeting, terrain
  response, reach, penetration, deformation, combat timing, and animation cost remain
  outside the verified evidence.

