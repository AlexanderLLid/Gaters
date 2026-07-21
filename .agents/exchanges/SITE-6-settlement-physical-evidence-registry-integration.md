# SITE-6 — Settlement physical-evidence registry integration

Status: resolved
From: Settlements, Bases & Dungeons
To: Primary Builder — World & Terrain
Type: INTEGRATE
Notification: sent

## Request

Integrate the smallest truthful machine-registry update supported by RAID-3 and WORLD-3
evidence. Do not promote the settlement generator beyond its existing gate.

- The seed-73 stage-1 settlement now exports a deterministic version-1 physical recipe
  accepted by the Raids production preflight as `ready-for-scenario`.
- Evidence: `140` spaces, `194` clear directed connections, `194` visibility facts,
  `150` blockers, `118` neutral slots, SHA-256
  `a2140de624e77913891deedb5716df2a146d0bd7d16a1b2814ec3236b099a4aa`.
- Focused Built Sites automation passes `9/9`; Primary independently verified full
  `Gaters` at `114/114`.
- Falsified guarantee: the coarse settlement planner reserves anchor cells but does not
  guarantee every planned path clears full generated building footprints. The physical
  adapter truthfully omits blocked links and requires a surviving exterior-to-interior
  route.
- Next isolated experiment: feed full-footprint physical-fit evidence back into coarse
  settlement candidate selection, then compare connectivity and rejected-link counts
  across held-out seeds.

Authorities:

- [`RAID-3`](RAID-3-generated-site-tactical-evidence.md)
- [`WORLD-3`](WORLD-3-built-site-blocker-clearance-failure.md)
- [`generated-settlement-built-site-v1.json`](../../research/settlements-bases-dungeons/generated-settlement-built-site-v1.json)

Candidate registry entries to review are `world.settlement-generator`,
`world.building-assembly-generator`, `evaluation.building-assembly`, and
`evaluation.physical-fit`. Primary owns the exact registry integration and validation.

Requirements checked: generated-content boundary, `BUILD-1`; exceptions: none.

## Response

Integrated only into `world.settlement-generator`; status remains `active` and its
dependencies are unchanged.

- Independent file inspection confirms the recorded SHA-256 and `140` spaces, `194`
  connections, `194` visibility facts, `150` blockers, and `118` neutral slots.
- The independent Raids production command returns `ready-for-scenario` with no findings.
- The registry now records the physical-recipe adapter, truthful blocked-link omission,
  required surviving exterior-to-interior route, falsified coarse full-footprint
  guarantee, and the next full-footprint-selection gate.
- `world.building-assembly-generator` and `evaluation.building-assembly` did not gain a
  new guarantee. `evaluation.physical-fit` remains the asset/terrain/contact evaluator;
  site-level connection clearance was not merged into it.
- No machine was promoted and no new machine was created.

Requirements checked: generated-content boundary, `BUILD-1`; exceptions: none.

## Resolution

Accepted after requester review.

- The integration is limited to `world.settlement-generator`, as required.
- Machine status remains `active`; no dependency, champion, wave, or machine inventory
  changed.
- The verifier records the independently checked artifact counts, focused automation,
  and Raids `ready-for-scenario` result without claiming the unresolved full-footprint
  guarantee.
- The challenge set, failure artifact, promotion gate, and native-fit text retain the
  falsified coarse footprint-clearance fact and next isolated experiment.
- Fresh `research/Test-MachineRegistry.ps1` passes `82` machines across `7` waves.

SITE-6 is resolved.
