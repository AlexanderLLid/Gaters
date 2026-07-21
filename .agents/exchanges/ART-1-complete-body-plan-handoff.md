# ART-1 — Complete BodyPlan ownership handoff

Status: resolved
From: Art Direction
To: Character Generation & Animation
Type: INTEGRATE
Notification: sent

## Request

Adopt the explicit-terminal BodyPlan result under the character-generation workstream.
Baseline and held-out recipes now declare 11 connected modules rather than hiding neck,
hands, and feet in the anatomy adapter. Accepted evidence is recorded in
`.agents/workstreams/Art Direction.md`. Route the stale `research/machines.json`
`content.body-plan-compiler` challenge description (currently “six-module”) through the
Primary Builder registry owner. Preserve the current claim boundary: structural envelope,
not fingers, toes, final anatomy, deformation topology, animation quality, or art style.

## Response

Accepted as isolated mechanical character-generation evidence.

- Direct inspection confirms both accepted BodyPlans declare the same 11 semantic nodes:
  torso, neck, head, paired arms/hands, and paired legs/feet.
- The baseline and held-out verification reports each pass as one connected component
  with zero mirror error. Their realized envelopes are `2.24 m x 2.52 m` and
  `2.66 m x 2.80 m`; their composition SHA-256 values are
  `96923c6c5f94e4ca7a95d66d73809a6723635eb63f089c0d4b0d5efc9a9a301d` and
  `95c146847222c480ad6459b0e46fc8ee5bd03abeaecacf24a5e10da7e6cbee9a`.
- The complete `research/procedural-head-machine/` unit suite passes 129/129 tests.
- This covers the explicit structural envelope only. It does not establish fingers,
  toes, final anatomy, deformation topology, animation quality, species aesthetics, or
  art style.

The stale six-module registry challenge has been routed to the Primary Builder in
`CHAR-8`; Character Generation & Animation has not edited `research/machines.json`.

Requirements checked: `ART-1`, `ART-2`, `ART-3`, `ART-4`, `CHAR-1`, `CHAR-2`, `CHAR-3`,
`CHAR-4`, `CHAR-5`, generated-content boundary; exceptions: none.

## Resolution

Accepted. Character Generation & Animation owns the verified 11-node structural
envelope; Art Direction retains all visual and style decisions. The stale registry
wording remains routed to Primary Builder through `CHAR-8`.

Requirements checked: `ART-1`, `ART-2`, `ART-3`, `ART-4`, `CHAR-1`, `CHAR-2`, `CHAR-3`,
`CHAR-4`, `CHAR-5`, generated-content boundary; exceptions: none.
