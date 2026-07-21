# Procedural head-envelope design

## Outcome

- One versioned head recipe produces a closed, smooth, deliberately simple head envelope
  with stable topology and named semantic regions.
- Parameter changes rebuild the head without manual point editing.
- An independent verifier checks topology, dimensions, regions, finite geometry, and
  reproducibility before any Blender or Houdini adapter is compared.

Requirements checked: `ART-1`, `ART-2`, `ART-3`, `ART-4`, `CHAR-1`, `CHAR-2`, `CHAR-3`,
`CHAR-4`, `CHAR-5`, generated-content boundary; exceptions: `ART-1` and `ART-2` are not visually
evaluated by this structural leaf, and no face, body, rig, motion, or style promotion is
claimed.

## Capability graph

| Node | Edge | Source | Contract | Work deleted |
|---|---|---|---|---|
| Head recipe | seed | Build | Tool-neutral JSON declares dimensions, resolution, and controls | Hidden DCC state |
| Head compiler | SEQUENCE | Build | Recipe becomes canonical vertices, faces, and named region weights | Manual base-mesh creation |
| Head verifier | AND | Build | Raw output independently passes geometry, region, and replay checks | Visual guessing about structural validity |
| Blender adapter | OR | Adapt later | Materializes the accepted output without changing it | Blender-specific source authority |
| Houdini adapter | OR | Adapt later | Materializes the accepted output without changing it | Houdini-specific source authority |
| Character module composer | SEQUENCE | Build | Accepted local modules compose through versioned sockets and measured transitions | Manual welding and seam repair |
| Procedural body builder | SEQUENCE later | Build | A verified AnatomyGraph places and composes accepted modules | Manual creature modelling |

## Complete character-generation dependency graph

This graph applies to humanoids and non-humanoid creatures. Body-plan modules and
validated adapters vary; the stage contracts do not.

```text
CharacterBrief
  |
  +--> StructureSpec ----> AnatomyGraph ----+----> ContactPlan
  |                                         +----> GuideSkeleton
  |                                         +----> VolumeLayout
  |
  +--> StyleSpec ----------------------------+----> SurfaceMesh
  |                                                   |
  +--> PerformanceSpec -------------------------------+--> Topology + LODs
                                                      |
AnatomyGraph + Topology ------------------------------+--> SemanticRegionMap
AnatomyGraph + GuideSkeleton + Topology ------------------> SkinWeights
GuideSkeleton + body capabilities -----------------------> Rig + joint limits
Topology + StyleSpec ------------------------------------> UVs + materials
Rig + SkinWeights + ContactPlan --------------------------> Deformation fixtures
Accepted deformation package + MotionBrief --------------> Motion candidates
Mesh + rig + motion + contacts + performance limits ------> CharacterPackage
CharacterPackage -----------------------------------------> Unreal adapter
```

- Every arrow is a versioned input/output contract, never an implicit `.blend` or `.hip`
  dependency.
- Every stage has an independent evaluator and preserves rejected candidates.
- A downstream stage cannot promote while an upstream contract is unverified.
- Humanoids may borrow standard joint-role adapters; a novel creature must validate its
  AnatomyGraph and guide-skeleton adapter before rigging or motion.

Established production systems support these boundaries: Houdini KineFX packages shape,
skeleton, and rig as separate character elements and treats capture weights separately
from deformation; Unreal consumes skeletal mesh, skeleton, physics, and animation through
distinct runtime assets and systems.

## Selected first leaf

- Generate a UV-sampled ellipsoidal envelope with explicit width, height, depth, jaw
  width, and chin taper controls. Width is the maximum envelope width; jaw width and chin
  taper are bounded fractions of that envelope.
- Emit `mesh.json`, `regions.json`, `receipt.json`, and `verification.json` from one recipe.
- Name only regions required by the first controls: `skull`, `face`, `jaw`, and `chin`.
- Preserve vertex order and face connectivity when dimensions change.
- Use Python standard library only. Blender and Houdini do not run in this wave.

## Evaluation

- Two fresh builds from the same recipe have identical canonical hashes.
- Every coordinate is finite; every face has distinct valid indices; triangular pole
  caps and quad body bands form a closed manifold without degenerate quads.
- Bounds match declared width, height, and depth within the sampling tolerance.
- Every vertex has region weights that are finite, bounded, and sum to one.
- A held-out broad-jaw recipe changes the jaw bounds while retaining vertex and face IDs.

## Promotion gate

- Promote the tool-neutral core only if every mechanical check passes twice.
- Do not infer visual quality from the pass.
- After promotion, implement the same accepted artifact through Blender and Houdini
  adapters and compare code complexity, runtime, replay, inspectability, and exact mesh
  preservation.

**If this existed, we would no longer need to** manually recreate or trust a DCC-specific
base head before testing procedural shape controls.

## Native deformation result

| Node | Unlocks | Edge | Source | Contract | Work deleted | Verifier | Challenge set | Promotion gate |
|---|---|---|---|---|---|---|---|---|
| Semantic local deformation | Editable shape controls | SEQUENCE | Adapt | Tool-neutral region command becomes a native non-destructive graph | Manual point movement | Independent expected-position, topology, region, protection, and graph-parameter checks | Baseline/normal and broad-jaw/strong | Both fresh scenes pass without code changes |

- Blender Geometry Nodes and Houdini SOPs both pass the normal and held-out commands.
- Houdini is the local-deformation champion: 80 adapter lines versus Blender's 108, and
  lower elapsed time in both final comparisons.
- Promotion is limited to this leaf. It does not prove facial anatomy, arbitrary
  operations, surface composition, topology generation, rigging, or visual quality.
- The next frontier was one versioned attachment socket joining a head envelope to a neck
  envelope with measured continuity. The result is recorded below.

## Head-to-neck socket result

| Node | Unlocks | Edge | Source | Contract | Work deleted | Verifier | Challenge set | Promotion gate |
|---|---|---|---|---|---|---|---|---|
| Head-to-neck socket | First generated module composition | SEQUENCE | Build | `module-socket/0` composes an accepted head and neck through one shared ring | Manual weld and first-seam repair | Independent preservation, manifold, duplicate-seam, tangent, dimension, semantic, and replay checks plus fresh Houdini readback | Baseline/short and broad-jaw/long-wide | Both variants replay twice and both derived Houdini scenes reopen without drift |

- Baseline and held-out compositions each emit 210 vertices and 224 faces, are closed
  manifolds, realize requested neck lengths, and report zero seam tangent-angle error.
- Houdini 22.0 freshly reopens both derived scenes and preserves positions, faces,
  semantic weights, module labels, and socket metadata within `1e-7`.
- Promotion is limited to an axial head-to-neck interface. It does not prove arbitrary
  local frames, branching anatomy, deformation across the seam, rigging, or visual
  anatomy.
- The next frontier was expressing sockets in local coordinate frames and attaching two
  mirrored branches to one parent module. The result is recorded below.

## Mirrored local-frame branch result

| Node | Unlocks | Edge | Source | Contract | Work deleted | Verifier | Challenge set | Promotion gate |
|---|---|---|---|---|---|---|---|---|
| Mirrored local-frame branches | Paired limbs, wings, horns, or other branches | SEQUENCE | Build | One branch recipe evaluates in explicit left/right orthonormal frames and reuses parent interface vertices | Separate left/right construction and manual seam welding | Independent preservation, manifold, seam, frame, mirror, length, semantic, and replay checks plus fresh Houdini readback | Baseline parent/three rings and held-out proportions/four rings | Both variants replay twice; derived Houdini scenes reopen without drift |

- Baseline emits 34 vertices and 36 faces; the held-out topology emits 42 vertices and
  44 faces. Both are closed manifolds with exact mirror and branch-length error `0`.
- The held-out recipe changes parent width/depth/height, branch length, taper, and ring
  count without a compiler change.
- Houdini 22.0 freshly reopens both derived scenes and preserves geometry, topology,
  region weights, module labels, frames, interfaces, cap IDs, and mirror pairs.
- Promotion covers paired branches on opposite X-facing sockets. It does not yet prove
  an arbitrary body graph, non-mirrored branches, curved branch guides, anatomical
  transitions, deformation, rigging, or visual quality.
- The next frontier was a versioned `BodyPlan` graph producing a core, head, paired arms,
  and paired legs. The result is recorded below.

## BodyPlan graph result

| Node | Unlocks | Edge | Source | Contract | Work deleted | Verifier | Challenge set | Promotion gate |
|---|---|---|---|---|---|---|---|---|
| BodyPlan compiler | Complete structural humanoid and later arbitrary anatomy graphs | SEQUENCE | Build | `body-plan/0` resolves parent/child attachments into one connected labelled volume boundary | Hand placement and separate per-body structural meshes | Independent manifold, cell connectivity, parent contact, declared mirror, semantic, bounds, and replay checks plus fresh Houdini readback | 1.96 m baseline and 2.38 m tall/wide held-out body; open, disconnected, asymmetric, false-contact, overlap, and bad-label counterexamples | Both plans replay twice; both derived Houdini scenes reopen without drift |

- Baseline: six modules, 168 occupied cells, 302 vertices, 300 faces, one connected
  component, 1.96 m height/width, and zero mirror error.
- Held-out: the unchanged compiler accepts longer torso, arms, and legs, producing 204
  cells, 366 vertices, 364 faces, 2.38 m height, 2.24 m width, and zero mirror error.
- The compiler emits one closed voxel boundary rather than welding the earlier smooth
  head/neck fixture. This proves the structure/anatomy-graph and coarse-volume layer; it
  does not prove smooth module substitution or anatomical transition quality.
- Houdini 22.0 freshly reopens both derived scenes and preserves geometry, topology,
  semantic weights, module labels, placements, connections, source cells, and mirrors.
- The next frontier was smooth primitive materialization from the accepted BodyPlan. The
  result is recorded below.

## Smooth BodyPlan surface result

| Node | Unlocks | Edge | Source | Contract | Work deleted | Verifier | Challenge set | Promotion gate |
|---|---|---|---|---|---|---|---|---|
| Smooth BodyPlan surface | Rounded unified mannequin and later anatomy-volume fitting | SEQUENCE | Adapt | `body-surface/0` maps accepted module bounds to native ellipsoids, VDB union/smoothing, polygon output, and restored semantic labels | Manual primitive fusion and surface cleanup | Independent manifold, connected-component, bounds, non-axial-normal, face-budget, provenance, native-parameter, semantic, and replay checks | Baseline and tall BodyPlans; open mesh, metadata drift, parameter drift, and missing labels | Both BodyPlans build twice identically, reopen separately, pass checks, and emit actual-mesh previews |

- Baseline produces one connected 10,298-vertex / 10,726-face surface; the held-out tall
  plan produces 12,153 vertices / 12,713 faces without adapter changes.
- Both remain under the face budget, preserve every BodyPlan module label, stay within
  `0.056 m` of structural bounds, and have over `99%` non-axis-aligned face normals.
- The result visibly replaces blocks with rounded fused volumes, but it resembles an
  ellipsoid mannequin rather than a human. Structural promotion does not imply anatomy
  or style acceptance.
- The next frontier was a versioned anatomical guide shared by the surface and future
  skeleton. The result is recorded below.

## Anatomical guide result

| Node | Unlocks | Edge | Source | Contract | Work deleted | Verifier | Challenge set | Promotion gate |
|---|---|---|---|---|---|---|---|---|
| Anatomical guide | One measured source for surface and skeleton construction | SEQUENCE | Build | `anatomical-guide/0` derives named joint landmarks, skeleton segments, tapered surface segments, torso ellipsoids, symmetry pairs, and provenance from an accepted BodyPlan | Redefining anatomy separately in mesh and rig tools | Independent landmark, containment, attachment, mirror, taper, provenance, and replay checks | Baseline and tall BodyPlans; missing landmarks, broken symmetry, detached segments, invalid taper, and provenance drift | Both proportion sets compile twice identically and pass every guide check |

- Baseline and held-out tall guides compile without code changes. Their hashes differ as
  proportions change and remain identical across replay.
- The guide is tool-neutral authority. It does not claim that the chosen measurements
  are anatomically final.

## Guide-driven anatomical surface result

| Node | Unlocks | Edge | Source | Contract | Work deleted | Verifier | Challenge set | Promotion gate |
|---|---|---|---|---|---|---|---|---|
| Anatomical surface | A visibly body-shaped generated mesh and a shared path toward rigging | SEQUENCE | Adapt | `anatomical-surface/0` materializes guide ellipsoids and tapered segments through the existing Houdini VDB graph | Rebuilding surface proportions separately from the guide | Independent topology, bounds, semantic, guide-provenance, actual-mesh taper, face-budget, fresh-readback, preview, and replay checks | Baseline and held-out tall guides; metadata drift, missing labels, open mesh, false taper, bounds drift, and face excess | Both guide variants build twice identically, reopen separately, and pass all mechanical checks |

- Baseline produces 13,149 vertices / 13,822 faces; held-out tall produces 15,407 /
  16,070. Both are one connected surface and emit labelled previews from the actual mesh.
- The first taper evaluator sampled the ankle/foot union and falsely penalized valid foot
  width. The failed report remains archived; the corrected evaluator samples the lower
  calf above the foot and the same artifact passes.
- The visible result is a useful anatomical mannequin, not final anatomy, topology,
  hands, face, rig, motion, or art style.
- Next frontier: generate a bone hierarchy directly from the accepted guide, proving
  that surface and skeleton share the same measurements before adding skinning.

## Guide skeleton result

| Node | Unlocks | Edge | Source | Contract | Work deleted | Verifier | Challenge set | Promotion gate |
|---|---|---|---|---|---|---|---|---|
| Guide skeleton | Native rig input from the same measurements as the surface | SEQUENCE | Adapt | `guide-skeleton/0` compiles a rooted 16-joint tree, 15 hierarchy edges, and orthonormal world frames from `anatomical-guide/0`; Houdini materializes native KineFX point geometry | Placing the first skeleton separately from the generated body guide | Independent joint, landmark, hierarchy, frame, provenance, replay, native-scene, and fresh-readback checks | Baseline and tall guides; disconnected graph, parent drift, position drift, frame drift, and provenance drift | Both guides replay identically and both KineFX scenes reopen without joint, hierarchy, transform, or provenance drift |

- Baseline and held-out skeleton hashes differ with their source proportions and remain
  stable over two compiler runs.
- Houdini stores the result as named points with `transform` attributes and hierarchy
  polylines, matching SideFX's KineFX skeleton representation.
- This is a guide skeleton, not a production rig. Controls, joint limits, skinning,
  deformation tests, retargeting, and animation remain unproven.
- Next frontier: bind the accepted mannequin surface to this exact skeleton with fully
  generated initial weights, then test deformation using fixed diagnostic poses.

## Generated skin capture result

| Node | Unlocks | Edge | Source | Contract | Work deleted | Verifier | Challenge set | Promotion gate |
|---|---|---|---|---|---|---|---|---|
| Skin capture | First end-to-end generated surface-to-skeleton deformation | SEQUENCE | Adapt | `skin-capture/0` uses native Houdini proximity capture with normalized two-influence weights and applies a versioned diagnostic elbow pose through Joint Deform | Manual first-pass weight painting and subjective proof that a generated mesh can move | Independent rest-surface, coverage, normalization, influence-count, side-isolation, provenance, active-motion, protected-module, preview, and fresh-readback checks | Baseline and tall bodies; missing weights, bad sums, excess influences, cross-side leakage, surface drift, provenance drift, inactive pose, and protected-region movement | Both proportion sets capture and deform without code changes; active limb moves while all unposed modules remain numerically fixed |

- Baseline captures 13,149 vertices to 16 regions with at most two influences, maximum
  sum error `2.99e-8`, and zero cross-side weight. The elbow pose moves the active arm
  `0.347 m`; other modules move at most `1.20e-7 m`.
- Held-out tall captures 15,407 vertices under the same recipe. The active arm moves
  `0.401 m`; other modules move at most `1.34e-7 m`.
- The actual posed preview exposes a sharp elbow collapse. The pipeline is valid, but
  deformation topology and weight smoothing are not production-ready.
- Next frontier: add elbow/knee deformation loops or helper joints, then compare native
  proximity and biharmonic capture using fixed bend-volume metrics.

## Skin-capture challenger result

| Node | Unlocks | Edge | Source | Contract | Work deleted | Verifier | Challenge set | Promotion gate |
|---|---|---|---|---|---|---|---|---|
| Biharmonic skin capture | Better organic bend weights without manual painting | OR | Borrow | Native Joint Capture Biharmonic followed by native Capture Correct limiting to four influences | Custom weight solver and first-pass manual smoothing | Same mechanical capture verifier plus tenth-percentile elbow thickness retention and labelled same-input comparison | Baseline and tall surfaces under identical skeleton and 55° pose; raw five-influence output; four-influence correction | Challenger exceeds `88%` retention and improves over proximity by at least `5%` on both proportion sets without violating the four-influence budget |

- Baseline: proximity retains `78.8%`; biharmonic retains `91.2%` (`+12.4%`).
- Tall held-out: proximity retains `70.1%`; biharmonic retains `92.6%` (`+22.6%`).
- Houdini Capture Correct reduces the raw five-influence result to four while preserving
  the measured improvement. Biharmonic replaces proximity as champion; all runs remain.
- Production deformation is still open. One elbow angle does not cover knees, shoulders,
  twists, extreme poses, self-intersection, or animation.
- Next frontier: generate a compact deformation-pose suite for every major joint before
  changing topology or adding helper joints.

## Major-joint pose suite result

| Node | Unlocks | Edge | Source | Contract | Work deleted | Verifier | Challenge set | Promotion gate |
|---|---|---|---|---|---|---|---|---|
| Deformation pose suite | Evidence-driven topology and rig changes plus later motion validation | SEQUENCE | Build | One accepted captured skin receives versioned elbow, knee, shoulder, hip, and torso-twist poses without recapture | Manually posing and visually judging every generated body | Independent active-motion, protected-module, bounded seam-transition, thickness-retention, provenance, fixed-view, and held-out-proportion checks | Baseline and tall bodies; missing poses, inactive joints, protected motion, excessive seam motion, and bend/twist collapse | All five poses pass declared thickness and isolation thresholds on both proportion sets |

- Baseline/tall thickness retention: elbow `91.2% / 92.6%`, knee `89.9% / 89.3%`,
  shoulder `91.9% / 90.2%`, hip `90.4% / 87.7%`, torso twist `97.9% / 98.0%`.
- The first suite report correctly failed shoulder, hip, and twist because it required
  adjacent seam modules to remain completely static. That was an evaluator error: smooth
  capture must move those seam bands. The failed run remains archived; the corrected
  contract separately bounds transition-module motion and rejects excessive leakage.
- This suite proves isolated static poses only. Pose sequences, joint limits, controls,
  contacts, self-intersection, animation curves, and runtime behavior remain open.
- Next frontier: compile versioned joint-limit and motion-curve recipes into a short
  generated animation while reusing this suite as the deformation guard.

## Bounded generated-motion result

| Node | Unlocks | Edge | Source | Contract | Work deleted | Verifier | Challenge set | Promotion gate |
|---|---|---|---|---|---|---|---|---|
| Bounded motion sequence | First generated native animation and later rig/control work | SEQUENCE | Build | Tool-neutral joint limits and curves compile complete skeleton frames; Houdini materializes them as one timeline | Manual range entry, first keyframing, and visual-only DCC checking | Independent limit, mirror, pose containment, curve, replay, continuity, topology, native skeleton, endpoint, provenance, and duplicate-readback checks | Baseline/tall bodies plus limit, timing, frame, topology, skeleton, endpoint, and provenance counterexamples | Both proportions compile twice identically and both Houdini timelines reopen twice with identical valid readbacks |

- The shared recipe generates 13 frames at 8 FPS. Baseline/tall peak surface motion is
  `0.693 m / 0.803 m`; both return to the exact rest surface and retain topology.
- Houdini uses a frame-driven KineFX skeleton Switch and one Joint Deform node. The
  labelled filmstrips are rendered from fresh readback, not imagined animation.
- This does not prove anatomy-final ranges, controls, contacts, balance,
  self-intersection, locomotion, retargeting, visual quality, Unreal import, or runtime.
- Next frontier: generate a contact-aware step intent with planted-foot constraints,
  then evaluate sliding and balance before adding more motion vocabulary.
