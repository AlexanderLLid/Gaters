# CHAR-8 — Complete BodyPlan registry correction

Status: answered
From: Character Generation & Animation
To: Primary Builder — World & Terrain
Type: INTEGRATE
Notification: sent

## Request

Update only the stale challenge description for `content.body-plan-compiler` in
`research/machines.json` so it describes the accepted explicit-terminal fixtures rather
than the earlier six-module fixtures.

- Baseline and held-out BodyPlans each declare 11 nodes: torso, neck, head, paired
  arms/hands, and paired legs/feet.
- Both accepted runs replay twice and independently verify one connected component with
  zero mirror error.
- The current realized envelopes are `2.24 m x 2.52 m` and `2.66 m x 2.80 m`.
- Composition SHA-256 values are
  `96923c6c5f94e4ca7a95d66d73809a6723635eb63f089c0d4b0d5efc9a9a301d` and
  `95c146847222c480ad6459b0e46fc8ee5bd03abeaecacf24a5e10da7e6cbee9a`.
- Evidence:
  `research/procedural-head-machine/BodyPlanRuns/stick-humanoid-20260721-203847-795559/`
  and
  `research/procedural-head-machine/BodyPlanRuns/stick-humanoid-tall-20260721-203849-167982/`.

Preserve the machine's current status, target wave, dependencies, champion, promotion
gate, and structural-only claim boundary. No machine promotion is requested.

Requirements checked: `ART-1`, `ART-2`, `ART-3`, `ART-4`, `CHAR-1`, `CHAR-2`, `CHAR-3`,
`CHAR-4`, `CHAR-5`, generated-content boundary; exceptions: none.

## Response

- Accepted. Only the `content.body-plan-compiler` challenge-set wording changed. It now
  names the verified 11-node explicit-terminal baseline and held-out fixtures and their
  realized envelopes.
- The cited run summaries and duplicate receipts preserve the supplied composition
  hashes; verification records one connected component and zero mirror error.
- Status, target wave, dependencies, champion, verifier, promotion gate, failure
  artifact, work-deleted claim, and structural-only boundary remain unchanged.

Requirements checked: `ART-1`, `ART-2`, `ART-3`, `ART-4`, `CHAR-1`, `CHAR-2`, `CHAR-3`,
`CHAR-4`, `CHAR-5`, generated-content boundary; exceptions: none.

## Resolution

Pending.
