# Generated Humanoid Pipeline Design

Status: approved by Human.

## Outcome

- One versioned mechanical humanoid recipe becomes a controllable isolated placeholder
  in UE 5.8 without manual rigging, skinning, Physics Asset, IK, or animation setup.
- The placeholder can idle, turn, walk, run, stop, jump, fall, contact uneven terrain,
  react to an impact, and recover into locomotion.
- One command regenerates the source, imports it, prepares a test map, runs independent
  checks, and preserves causal diagnostics.
- One controlled proportion change passes through the unchanged pipeline.

This milestone is mechanically complete, not product-ready. It does not satisfy or seek
promotion against the final visual requirements in `ART-1` or `ART-2`.

If this existed, we would no longer need to manually construct the technical body, rig,
physics, IK, locomotion, and recovery setup for each generated humanoid variant.

## Scope

- Build the complete humanoid vertical slice first.
- Defer quadrupeds, other anatomy families, named species, lineage, evolution, runtime
  generation, finished materials, clothing, faces, hair, and animation style.
- Keep the current Blender humanoid and offline reaction/recovery artifacts as champions
  until an Unreal challenger passes its own gates.
- Do not edit shared Unreal modules or `research/machines.json`; verified isolated work
  returns to Primary Builder through an `INTEGRATE` exchange.

## Authoritative input and derived output

The versioned mechanical recipe is authoritative. It declares dimensions, stable bone
roles, joints, contacts, mass distribution, collision intent, movement requirements,
runtime budgets, and source/generator/adapter versions.

Blender source files, FBX, manifests, Unreal assets, Actors, maps, and reports are derived
outputs behind adapters. Visual style and species aesthetics remain separate requirement
inputs owned by Art Direction.

## Data flow

1. The humanoid recipe enters the existing Blender compiler.
2. Blender produces the mesh, skeleton, skin weights, physical profile, source motion,
   FBX, source file, and manifest.
3. The Unreal editor adapter imports the package and derives native Skeletal Mesh,
   Skeleton, Physics Asset, IK Rig, animation assets, Animation Blueprint, and test Actor.
4. A dedicated test map and scripted driver exercise runtime movement, terrain contact,
   reactions, falls, and recovery.
5. An independent policy reads native assets and runtime measurements, then accepts or
   rejects the run without trusting generator assertions.
6. Every failure preserves the recipe, versions, reports, logs, and causal rule IDs. The
   last passing champion remains unchanged.

## Blender and Unreal boundary

Blender owns offline generation and source evidence:

- body geometry, family skeleton, skinning, deformation evidence, source motion;
- physical intent: masses, collision proxies, joints, limits, springs, damping, contacts;
- deterministic source artifacts and transport manifests.

Unreal owns live game behavior through native systems:

- `CharacterMovement` floor state, walking, falling, capsule movement, authority, and
  replication behavior;
- Skeletal Mesh, Skeleton, Animation Blueprint, IK Rig solvers, native floor traces,
  Physics Asset, Chaos, and Physics Control;
- terrain contact, runtime collision, physical reactions, recovery transitions, and
  performance evidence.

Animation Warping Foot Placement is a later native challenger, not an assumed foundation.
It may replace the trace-and-IK baseline only after its skeleton requirements and actual
pose effect pass the same evaluation gates. Experimental Mover is not a foundation.

## Runtime movement contract

- The capsule and `CharacterMovement` own world translation, rotation, grounded state,
  and falling state.
- Baseline locomotion clips are in-place. The Animation Blueprint consumes velocity,
  acceleration, facing, grounded/falling state, and movement intent.
- IK must contain a real solver. Validation moves each goal and proves the evaluated
  native pose changes at the expected limb; goal and chain metadata alone cannot pass.
- Native floor queries provide contact targets. IK adjusts feet and pelvis without
  changing authoritative capsule movement.
- Root motion and Motion Warping are deferred until a targeted action requires them.

## Physics and recovery contract

- Physics Asset topology is the first gate: exact required body bones and parent/child
  constraint pairs.
- Impact and fall work cannot begin until native readback also proves collision-shape
  coverage, mass distribution, joint frames and limits, springs, and damping match the
  physical profile within named tolerances.
- Recovery follows one explicit authority sequence:
  `locomotion -> controlled reaction or full fall -> grounded capsule placement ->
  get-up blend -> CharacterMovement locomotion`.
- Unreal rejects unsafe capsule placement, unresolved overlaps, unsupported ground, or
  invalid recovery orientation. Recovery completion is server-authoritative in the
  eventual shared runtime; the isolated test must model and report the same handoff.

## Test harness and evidence

The command, test map, scripted driver, report schema, and independent policy begin with
the Physics Asset wave and grow with each capability. They are not an end-stage wrapper.

Held-out challenges cover:

- repeat import and malformed physical-profile counterexamples;
- required movement-state coverage, trajectory and facing;
- real IK pose response, foot slide, penetration, pelvis discontinuity, and contacts on
  flat ground, slopes, steps, uneven ground, and a moving floor;
- collision overlap, constraint violations, impact/fall/recovery completion, safe
  capsule placement, and return to locomotion;
- runtime cost and deterministic report fields;
- one controlled proportion variant through unchanged adapters and gates.

Tunables and tolerances live in versioned data, not prose. A challenger promotes only
after regression, held-out, determinism, and budget checks pass.

## Build waves

1. Build the isolated Physics Asset topology adapter and preserve invalid-profile tests.
2. Establish the one-command harness, test map, scripted driver, report, and policy.
3. Import or generate reusable in-place idle, turn, walk, run, stop, jump, fall, and land
   clips; prove a real IK solver changes the native pose.
4. Evaluate scripted `CharacterMovement` locomotion.
5. Prove trace-driven foot and pelvis placement across the terrain challenge set.
6. Map full physical-profile semantics, then prove impact, fall, and recovery authority
   transitions.
7. Run the complete held-out command and playable-map evaluation twice.
8. Change one controlled proportion input and rerun the unchanged pipeline.
9. Only then reopen quadrupeds, other species, lineage, evolution, or runtime generation.

The selected frontier remains Physics Asset topology because every runtime physics and
recovery claim depends on a correct native articulated body. The harness begins in the
same wave so the adapter is judged through the eventual user-facing test path.

## Rejected approaches

- Generalize humanoids and quadrupeds now: delays the first complete playable body.
- Generate live movement or terrain contact in Blender: duplicates Unreal runtime
  authority and native systems.
- Treat IK metadata as proof: does not demonstrate pose deformation.
- Begin reaction/recovery after topology alone: ignores the profile semantics that make
  the physical behavior meaningful.
- Build a custom movement stack: `CharacterMovement` already owns the required baseline.

## Ownership and requirements

- Character Generation & Animation owns the isolated lab, adapters, harness, reports,
  motion mechanics, and workstream status.
- Primary Builder owns shared Unreal integration and `research/machines.json`.
- Art Direction owns final proportions, appearance, materials, clothing, and animation
  presentation requirements.

Requirements checked: Global none recorded; `ART-1`, `ART-2`, `CHAR-1`, `CHAR-2`,
`CHAR-3`, generated-content boundary.
Exceptions: `ART-1`, `ART-2` are intentionally outside this isolated mechanical
placeholder; no body, motion, or character factory promotion is claimed.
