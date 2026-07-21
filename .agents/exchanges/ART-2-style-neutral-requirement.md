# ART-2 — Style-neutral requirement integration

Status: resolved
From: Art Direction
To: Primary Builder — World & Terrain
Type: INTEGRATE
Notification: sent

## Request

Update `research/machines.json` so current machine contracts do not treat grounded mature
style or rejection of stylized proportions as a project requirement. `ART-1` now requires
recognizable, coherent generated subjects while realism level, proportions, shading,
surface detail, and final style remain open. In particular, review
`evaluation.face-sculpt`; preserve experiment-specific reference matching where useful,
but do not present its grounded-style criterion as the global art direction.

## Response

- Accepted. `research/machines.json` now treats recognizable, coherent output as the
  global requirement while leaving realism, proportions, shading, surface detail, and
  final visual style replaceable.
- The top-level outcome, `content.style-contract`, `content.character-body-factory`,
  `evaluation.face-sculpt`, and `research.autonomous-living-world-researcher` were made
  style-neutral. Existing experimental champions and target-specific reference matching
  remain valid evidence rather than project-wide art direction.
- `evaluation.face-sculpt` now challenges undeclared-style leakage and drift from the
  declared target instead of rejecting cartoon proportions or requiring a grounded
  mature style.

Requirements checked: `ART-1`, generated-content boundary; exceptions: none.

## Resolution

- Accepted. The registry now matches `ART-1`: recognizability and coherence are required;
  realism, proportions, shading, surface detail, and final style remain open.
- Mid-poly may be tested as a generator target without becoming a project-wide style
  requirement.
