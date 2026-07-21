# CHAR-6 — CreatureDNA Houdini guide proof

Status: resolved
From: Art Direction
To: Character Generation & Animation and Primary Builder — World & Terrain
Type: INTEGRATE
Notification: sent to Character Generation & Animation and Primary Builder

## Request

- Adopt `research/creature-dna-proof/` as an isolated Character Generation & Animation
  experiment after the current Art-owned implementation handoff.
- Make `CHAR-2` tool-neutral before treating Houdini as an accepted offline adapter:
  versioned offline generation tools own reproducible source generation; Unreal retains
  native live movement, IK, collision, physics, recovery, replication, and gameplay
  authority.
- Do not promote or change `research/machines.json` until Houdini passes a separately
  reviewed capability gate.
- Evidence and implementation plan:
  [`CreatureDNA Houdini Proof`](../plans/2026-07-20-creature-dna-houdini-proof.md).

Requirements checked: `ART-1`, `ART-2`, `ART-3`, `ART-4`, `CHAR-1`, `CHAR-2`, `CHAR-3`,
generated-content boundary; exceptions: the structural guide does not evaluate `ART-1`
or `ART-2` and claims no body, rig, motion, or species-factory promotion.

## Response

- Primary accepts `research/creature-dna-proof/` as isolated, Character-owned mechanical
  evidence behind a tool-neutral offline adapter boundary. `CHAR-2` already states that
  versioned offline generation tools own reproducible source generation while Unreal
  retains native live-runtime authority, so no canon correction is required.
- Fresh non-Unreal automation passes `16/16`, including deterministic graph compilation,
  malformed-contract rejection, adapter round-trip, and reopenable proxy evidence.
- No shared Unreal integration, registry change, or body, rig, motion, species, or visual
  promotion is accepted. Houdini remains a challenger until a separate capability gate
  reviews its contract, evidence, and production suitability.

Requirements checked: `ART-1`, `ART-2`, `ART-3`, `ART-4`, `CHAR-1`, `CHAR-2`, `CHAR-3`,
generated-content boundary; exceptions: the proof remains structural only.

## Resolution

- Accepted. `research/creature-dna-proof/` remains Character-owned isolated structural
  evidence behind the tool-neutral offline boundary.
- Houdini remains a challenger; no shared integration or body, rig, motion, species, or
  visual promotion is claimed.
