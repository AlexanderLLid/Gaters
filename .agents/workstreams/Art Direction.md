# Art Direction

## Current objective

- Keep final visual style open while isolated studies test recognizability and production
  feasibility without changing character-generation machinery.

## Owned outputs

- Style studies, reference boards, comparisons, and proposed style-contract changes
  under `research/character-style-exploration/` or another agreed research folder.
- Isolated Blender look-development source and derived previews under
  `research/character-style-exploration/`; these are visual-fidelity evidence only
  (`ART-3`).
- Visual requirements for Character Generation & Animation; no rig, motion, or runtime
  character implementation ownership.

## Boundaries

- Do not edit `research/embodied-species-lab/`, `SourceAssets/Blender/`, or Unreal
  character assets for an Art Direction feasibility proof.
- Body families, character variation, rigging, skinning, animation, physics, and runtime
  adapters remain owned by Character Generation & Animation.

## Exchanges

- [`CHAR-6 — CreatureDNA Houdini guide proof`](../exchanges/CHAR-6-creature-dna-houdini-proof.md)
  — resolved; Character owns the isolated structural evidence and Houdini remains a
  challenger without capability promotion.
- [`CHAR-7 — Procedural head and deformation proof`](../exchanges/CHAR-7-procedural-head-deformation-handoff.md)
  — resolved; Character may consume the verified mechanical contracts while all visual
  and production-body claims remain open.
- [`ART-1 — Complete BodyPlan ownership handoff`](../exchanges/ART-1-complete-body-plan-handoff.md)
  — resolved; Character owns the verified 11-node structural envelope, Art retains all
  visual decisions, and the shared registry correction is routed through `CHAR-8`.
- [`ART-2 — Style-neutral requirement integration`](../exchanges/ART-2-style-neutral-requirement.md)
  — resolved; the shared registry now requires recognizable coherent output without
  choosing realism, proportions, shading, detail, or final style.

## Evidence

- `research/procedural-head-machine/` is the first promoted dependency-free leaf of the
  zero-manual character-generation graph. Baseline run
  `Runs/baseline-20260721-150140-151922/` and held-out broad-jaw run
  `Runs/broad-jaw-20260721-150140-635004/` each rebuild twice with independent geometry
  verification and identical within-recipe mesh hashes. They share topology hash
  `60ae50e52c54ae03b787822d29ba90c8c3b69c86fce45f582e529b4f91a8d09c` while producing
  distinct mesh hashes. Eleven tests cover topology, declared dimensions, normalized
  semantic regions, deterministic receipts, and causal rejection of malformed geometry.
  A rejected broad-jaw run preserves the discovered maximum-width contract ambiguity;
  the compiler now rejects ratios outside the declared envelope. This proves only a
  tool-neutral procedural head envelope, not face anatomy, visual style, deformation
  topology, rigging, or motion. Adapter runs `AdapterRuns/20260721-171111-815/` and
  `AdapterRuns/20260721-171139-932/` prove Blender 5.2 and Houdini 22.0 each save, freshly
  reopen, and preserve baseline and held-out topology, positions, and semantic weights
  within `1e-7`; all four reports have zero failures. Houdini is faster in both observed
  comparisons, but the adapters only materialize accepted geometry, so procedural shape
  generation remains an untested differentiator. Next frontier: run one semantic local
  deformation through native procedural graphs in both backends.
- Native semantic-deformation runs `DeformationRuns/20260721-173457-953/` and
  `DeformationRuns/20260721-173509-099/` apply normal and strong jaw widening to baseline
  and broad-jaw inputs through editable Blender Geometry Nodes and Houdini SOP graphs.
  All four fresh-readback reports preserve topology and region weights, keep protected
  skull displacement below `3.81e-9 m`, and match independently expected positions below
  `8.24e-9 m`. Houdini is the isolated leaf champion: its adapter is smaller and faster
  in both final comparisons. This does not promote Houdini as the complete body, topology,
  rig, motion, or style backend. Next frontier: measured head-to-neck socket continuity.
- Head-to-neck socket runs `SocketRuns/head-neck-20260721-154234-243310/` and
  `SocketRuns/head-neck-long-20260721-154234-616857/` each replay twice with identical
  composition hashes. Both produce a 210-vertex, 224-face closed manifold, preserve the
  upstream head, realize the requested neck length, carry semantic/module labels, and
  measure zero seam tangent-angle error. Houdini adapter runs
  `SocketAdapterRuns/20260721-174612-938/` and `SocketAdapterRuns/20260721-174617-202/`
  freshly reopen the derived scenes and preserve geometry, labels, and socket metadata
  within `1e-7`. This promotes only one axial composition interface. Next frontier:
  local-frame mirrored branches on one parent module.
- Mirrored-branch runs `BranchRuns/torso-side-branches-20260721-161314-577521/` and
  `BranchRuns/torso-side-branches-held-out-20260721-161314-730080/` replay twice with
  identical within-recipe hashes. Baseline and held-out outputs are closed manifolds,
  reuse parent interfaces, preserve orthonormal local frames, and measure zero mirror
  and branch-length error. The held-out recipe changes parent proportions, length,
  taper, and ring count without code changes. Houdini runs
  `CompositionAdapterRuns/20260721-181354-980/` and
  `CompositionAdapterRuns/20260721-181358-939/` freshly reopen both outputs without
  geometry, topology, semantic, or socket-metadata drift. This promotes paired X-facing
  local-frame branches only. Next frontier: the first complete `BodyPlan` graph fixture.
- BodyPlan runs `BodyPlanRuns/stick-humanoid-20260721-162041-004309/` and
  `BodyPlanRuns/stick-humanoid-tall-20260721-162041-209553/` each replay twice with
  stable hashes. The baseline six-module fixture is one closed 1.96 m structural body;
  the held-out plan changes torso, arm, and leg proportions into a closed 2.38 m body
  without code changes. Independent checks reject open boundaries, disconnected cells,
  overlaps, asymmetry, false contacts, and invalid labels. Houdini runs
  `BodyPlanAdapterRuns/20260721-182213-911/` and
  `BodyPlanAdapterRuns/20260721-182218-075/` freshly reopen without geometry, semantic,
  or graph-metadata drift. This promotes the coarse structure/volume graph only. Next
  frontier: smooth BodyPlan module materialization with preserved graph evidence.
- Complete BodyPlan runs `BodyPlanRuns/stick-humanoid-20260721-203847-795559/` and
  `BodyPlanRuns/stick-humanoid-tall-20260721-203849-167982/` replace hidden terminal
  synthesis with 11 explicit graph modules: torso, neck, head, paired arms/hands, and
  paired legs/feet. Both replay twice with stable hashes and ten verified contacts.
  Guide runs `AnatomyRuns/humanoid-guide-stick-humanoid-20260721-203941-305591/` and
  `AnatomyRuns/humanoid-guide-stick-humanoid-tall-20260721-203941-599873/` preserve the
  new terminal module labels. Houdini surface runs
  `AnatomicalSurfaceRuns/anatomical-mannequin-20260721-223948-724/` and
  `AnatomicalSurfaceRuns/anatomical-mannequin-20260721-224006-728/` each build twice,
  freshly reopen, and pass independent verification. This completes the structural
  humanoid envelope only; fingers, toes, face anatomy, deformation topology, and art
  quality remain outside the claim.
- Profiled mid-poly surface runs
  `AnatomicalSurfaceRuns/anatomical-mannequin-20260721-235834-702/` and
  `AnatomicalSurfaceRuns/anatomical-mannequin-20260722-000008-545/` generate baseline
  and tall bodies through one recipe at 7,672 and 8,914 faces. Each rebuilds twice,
  freshly reopens, preserves guide/module provenance, and emits semantic plus clay-shaded
  fixed views. Rejected run `anatomical-mannequin-20260721-235644-330/` preserves the
  diagnosed mushroom head and oversized-deltoid failure. The retained geometry reads as
  a humanoid mannequin, but lacks facial structure, fingers, toes, clothing, production
  topology, and human visual acceptance. Mid-poly remains a challenger target rather
  than the selected style. Next visual frontier: generated head/face structure attached
  through the existing head-to-neck contract.
- `BodyPlanRuns/stick-humanoid-20260721-162041-004309/run-1/stick-humanoid-preview.png`
  is the labelled front/three-quarter preview rendered directly from the accepted mesh.
  Future accepted BodyPlan runs emit the same diagnostic automatically (`CHAR-5`).
- Smooth-surface runs `SmoothBodyRuns/smooth-mannequin-20260721-185933-316/` and
  `SmoothBodyRuns/smooth-mannequin-20260721-185952-066/` each build twice identically,
  freshly reopen, preserve graph provenance and all module labels, and pass independent
  manifold, connectivity, bounds, smoothness, and face-budget checks. The baseline actual
  mesh has 10,298 vertices / 10,726 faces; the held-out tall body has 12,153 / 12,713.
  Both emit labelled previews from the generated meshes. Visual result: rounded and
  connected, but still an ellipsoid mannequin rather than anatomy. Next frontier: one
  shared anatomical guide for tapered surface segments and the later skeleton.
- Anatomical-guide runs
  `AnatomyRuns/humanoid-guide-stick-humanoid-20260721-174142-274786/` and
  `AnatomyRuns/humanoid-guide-stick-humanoid-tall-20260721-174142-558352/` derive named
  landmarks, skeleton segments, tapered surface segments, torso volumes, symmetry, and
  BodyPlan provenance from both accepted proportion sets. Anatomical-surface runs
  `AnatomicalSurfaceRuns/anatomical-mannequin-20260721-194548-295/` and
  `AnatomicalSurfaceRuns/anatomical-mannequin-20260721-194708-795/` each build twice
  identically and freshly reopen. The meshes are one connected surface with 13,149 /
  13,822 baseline vertices/faces and 15,407 / 16,070 held-out vertices/faces. This proves
  guide-driven taper and proportions, not finished anatomy or style. Next frontier: a
  guide skeleton generated from the same accepted landmarks and segments.
- Guide-skeleton runs
  `SkeletonRuns/humanoid-guide-skeleton-stick-humanoid-20260721-175401-425940/` and
  `SkeletonRuns/humanoid-guide-skeleton-stick-humanoid-tall-20260721-175401-688763/`
  each compile twice identically into 16 joints and 15 bones. Houdini KineFX adapter runs
  `SkeletonAdapterRuns/20260721-195557-780/` and
  `SkeletonAdapterRuns/20260721-195607-297/` freshly reopen without joint name,
  hierarchy, position, transform, or provenance drift. The surface and skeleton now
  share one anatomical guide. Next frontier: generated skin weights plus fixed
  deformation poses.
- Skin-capture runs `SkinCaptureRuns/20260721-200258-918/` and
  `SkinCaptureRuns/20260721-200425-877/` use native Houdini Joint Capture Proximity and
  Joint Deform on the baseline and held-out bodies. All weights normalize within
  `2.99e-8`, use at most two joints, and have zero cross-side leakage. A generated 55°
  elbow pose moves only the intended arm (`0.347 m` / `0.401 m` maximum); protected
  modules remain within `1.34e-7 m`. Actual posed previews reveal sharp elbow collapse,
  so the end-to-end bind is promoted mechanically while deformation quality is rejected.
  Next frontier: deformation loops/helper joints and proximity-versus-biharmonic tests.
- Skin-capture comparisons `SkinCaptureComparisons/20260721-201606-732/` and
  `SkinCaptureComparisons/20260721-201708-665/` hold mesh, skeleton, and 55° pose fixed.
  Four-influence biharmonic capture retains `91.2%` / `92.6%` tenth-percentile elbow
  thickness on baseline/held-out bodies versus proximity's `78.8%` / `70.1%`.
  Biharmonic becomes the capture champion. This promotes one elbow fixture only; the
  next frontier is a generated major-joint pose suite before topology changes.
- Pose-suite runs `SkinCaptureRuns/20260721-203829-025/` and
  `SkinCaptureRuns/20260721-203902-380/` reuse one four-influence biharmonic capture for
  elbow, knee, shoulder, hip, and torso-twist poses on baseline and held-out bodies. All
  ten fixtures pass active-motion, protected-module, bounded seam-transition, and joint
  thickness checks. The earlier `SkinCaptureRuns/20260721-203609-297/` failure exposed
  an invalid evaluator assumption that smooth shoulder/hip/waist seams stay perfectly
  static; the corrected contract bounds rather than forbids seam motion. Next frontier:
  versioned joint limits and a generated motion curve guarded by this pose suite.
- `JointLimitRuns/`, `MotionRuns/`, and
  `MotionSequenceRuns/20260721-213301-311/` / `20260721-211007-322/` close that frontier:
  one recipe compiles nine bounded controls and 13 full skeleton frames, then Houdini
  replays them through one frame-driven KineFX sequence. Baseline and tall bodies pass
  two exact fresh readbacks, return to rest, and reach `0.693 m / 0.803 m` peak surface
  motion. This is source-motion feasibility only. Next frontier: contact-aware step
  intent with planted-foot and balance evidence.

- `research/character-style-exploration/procedural-reptile-head-proof/Derived/reptile-head-v4/`
  is an Apprentice-only Houdini creature-head proof with replayable parameters, SDF skin
  fusion, procedural painted colour, three fixed third-person renders, fresh reopen, and
  mechanical verification. It proves the blocking machinery but visually rejects direct
  primitive/SDF construction as a Wartales-level finishing path; eye integration, mouth
  anatomy, controlled planes, and authored surface character remain missing.

- `research/character-style-exploration/blender-face-proof/` contains one isolated,
  reproducible Blender bust and a three-view verifier (`ART-1`, `ART-2`, `ART-3`).
- Current result proves native Blender construction, procedural painted materials,
  asymmetric landmarks, fixed renders, save/reopen, and absence of armature/animation.
- Current result does **not** prove concept-A production fidelity. Integrated eye and
  mouth topology in strict profile is the largest remaining art-feasibility gap.
- `research/character-style-exploration/blender-face-proof/Derived/still-v7/` proves a
  close, unlit front still can be held and freshly rendered in Blender by projecting the
  approved painting onto the MPFB mesh. It does **not** prove view-independent geometry,
  material, profile, or motion; those are the reverse-engineering target.
- `research/character-style-exploration/blender-face-proof/Derived/sculpt-v13/` is the
  view-independent clay challenger: one editable 19,158-vertex head, an active direct
  sculpt shape key, and freshly reopened front / three-quarter / profile renders with no
  visible image textures, rig, or animation. It proves the projection can be replaced by
  geometry; likeness and authored surface character remain below the approved painting
  and require human acceptance before any production claim.
- Current face-sculpt search: `face-search-20260720-032234` at
  `research/character-style-exploration/blender-face-proof/Runs/face-search-20260720-032234/`;
  initial and retained final candidate:
  `9e7d3f47b84eb6eb3f015cd078dfbdab6747321a49f832b47746a7990f618c44`.
  The search stopped at three evaluator disagreements; no challenger was promoted.
- Held-out audit: post-search counterexample
  `c842c4a217388531088a5abfec7d603269d38ddb9cb722c81527cbed4037583e` was excluded
  from the search history and rejected by both blind evaluators; the retained candidate
  tied its fresh replay. `humanAcceptance` is `false`.
- Largest unresolved visible mismatch: the retained face remains too tall and narrow,
  with an over-tapered lower face rather than the reference's compact broad mature mass
  and squared jaw/chin silhouette.
- Deterministic macro calibration: corrected run `macro-grid-20260720-071000` rebuilt and
  freshly verified all eight combinations of head width, head height, and jaw taper. Its
  then-current body-contour score nominated
  `eaec583dac7c9adf295510e82a06dd721dceb2aea371ced31bfdcaa88b884d5b`, which changes
  only `jaw_taper` from `0.085` to `0.00`. Later blind visible-commissure annotations
  invalidated the metric's mouth-width component and therefore its aggregate score and
  nomination margin. The candidate remains an experiment parent, not a champion.
  Three-quarter/profile show no obvious new regression; `humanAcceptance` remains
  `false`.
- `macro-grid-20260720-064000` is preserved but invalid for promotion: its first metric
  compared full reference contours with internal Blender control regions. The corrected
  evaluator fixed cheek and jaw with body-only non-ear contour bands, but its replacement
  mouth anchor required the separate correction below.
- `macro-grid-20260720-071000/mouth-commissure-semantic-incident.json` records that those
  replacement signed-displacement mouth vertices still sit outside the rendered visible
  commissures. Task 10 replaces that field with stable vertices mapped from two blind
  visible-corner annotations; the old field is excluded from selection and regression.
- Largest remaining geometry gap after the macro nomination: the eye apertures remain
  pinched and the nose/lip structure remains generic relative to the approved front
  reference; the single front image still cannot support profile-likeness claims.
- Deterministic feature calibration: clean run `feature-grid-20260720-094000` rebuilt,
  freshly reopened, and measured all eight combinations of eye aperture, nose length,
  and nose width under one static scene lock. Aperture `0.12` moved toward the target;
  larger tested nose length and width moved away. The raw-best improvement was only
  `0.010401788246`, below both required margins, so the all-low replay remains retained.
  The run is independently approved as evidence, not as human acceptance or proven
  likeness. Its measured response supports one bounded calibration pass next.
- Measured response calibration: run `feature-calibration-20260720-100000` tested exactly
  four brackets derived from the clean feature receipts. Candidate
  `9032fd4dc9db2de64564b3f19e32ca5e6199881df111c2fa2d37feb1c7cc12f8`
  (`eye_aperture=0.52`, `nose_length=0.08`, `nose_width=0.27`) reduced the three-feature
  score by `89.66%` with no Task 7 macro change and no obvious 3D continuity break. It is
  independently approved only as the best bounded calibration for human review;
  `humanAcceptance` and machine promotion remain `false`.
- Visible-detail pass: valid run `detail-grid-20260720-063059` rebuilt and freshly
  reopened all eight eye-corner, mouth, upper-lip, and nose-tip combinations after
  invalidating two failed runs and adding guards for frontmost landmark mapping and
  duplicate Blender targets. Receipt rank chose `c3cbf8d3...`, but both independent
  visual audits found no clear improvement over the Task 9 parent and disagreed on the
  marginal winner. No hard 3D regression appeared. Task 9 remains retained; Task 10 is
  review-only evidence that better eye, alar/tip, and lip topology or controls are needed.
- Front-only picture-to-mesh seed proof:
  `research/character-style-exploration/picture-to-mesh-proof/Derived/proof-v1/` molds
  one fixed 1,536-vertex topology from a single raster silhouette. Fresh Blender reopen
  improves front-mask IoU from `0.771655` to `0.991838` while preserving depth, vertex
  order, face connectivity, finite coordinates, and non-degenerate faces. Side and
  three-quarter renders are plausibility evidence only; this proves silhouette fitting,
  not inferred depth, anatomy, likeness, rigging, or animation.
- Single-picture internal-depth proof:
  `research/character-style-exploration/picture-to-mesh-proof/Derived/depth-v1/` converts
  one grayscale front depth picture into a local `FittedDepth` shape key on a fixed
  1,681-vertex grid. Fresh reopen measures depth RMSE `5.01e-09`, maximum edge stretch
  `1.3028`, unchanged image-plane coordinates and connectivity, finite geometry, and no
  degenerate faces. This proves local relief molding once depth is supplied; it does not
  prove that an ordinary painted reference can be interpreted into correct depth.
- Recognizable closed-mask proof:
  `research/character-style-exploration/picture-to-mesh-proof/Derived/mask-v8/` applies
  one fixed topology and recipe to balanced, broad, and crooked grayscale front masks.
  All three freshly reopen and pass front IoU `>= 0.9934`, named brow/eye/nose/cheek/
  mouth/chin depth ordering, exact topology and rear preservation, finite geometry,
  positive face area, and edge stretch `<= 1.75`. Visual review still rejects the output
  as a production head: it proves front bas-relief fitting, while profile, back, eyelid,
  lip, nostril, ear, and skull anatomy remain unsupported. The narrow valid smoothing
  interval also shows the next fitter must search per input against geometry and semantic
  gates rather than depend on one global scalar.
- Anatomical front-fit machine v0: run `anatomical-front-fit-v0-20260720-1` executes one
  contract through fresh MPFB sculpt build, reopen verification, front-feature geometry
  measurement, and separate mechanical/visual decisions. Mechanical fit passes with
  total ratio error `0.006962` against `0.02`; visual review rejects likeness because the
  skull, jaw, eyes, nose, and mouth remain materially unlike the approved reference.
  This proves cheap end-to-end replay and falsifies recipe replay as picture-to-head
  compilation. The next frontier is front semantic landmarks to bounded anatomical
  control proposals; human visual acceptance remains `false`.
- Direct style-macro controls: `Derived/style-macro-v2/` retains one editable
  topology-preserving shape key that independently controls upper height, lower-face
  height, cheek contour, and jaw contour. A measured second calibration reduces the five
  macro-ratio error from `0.288546` to `0.069615`; every controlled ratio is within
  `0.001` of target, while unchanged mouth width accounts for nearly all remaining macro
  error. Fresh reopen verifies zero depth-axis change and maximum displacement `0.004489`.
  Retain the control for the future generator, but reject likeness: eye, nose, mouth, age,
  asymmetry, and material remain unresolved. Upper-skull compression is provisional
  because hair obscures the source skull.
- Mouth and eye calibration: `Derived/style-eyes-v2/` carries the retained macro shape,
  a separate mouth-narrow target, independent screen-side eye-span controls, and one
  eye-height control. Fresh reopen reduces mouth-width error to `0.000943` and combined
  eye width/aperture error from `0.050160` to `0.000253` while preserving topology and
  profile depth. Visual review still rejects likeness: matching the eye bounding box did
  not reproduce lid curvature, canthi, globe fit, or socket structure. The next isolated
  experiment measures those contours; width and aperture are no longer the active knobs.
- Large visible presentation pass: `Derived/style-presentation-v5/` retains the proven
  macro and eye measurements while adding replayable dark-skin, lip, brow, sclera, iris,
  pupil, eye-bag, and lip-volume controls. It is the presentation champion because it is
  materially easier to compare with the approved reference and passes fresh three-view
  verification without image textures. Likeness remains rejected. Three deliberately
  larger challengers showed that stock lip targets barely change the inflated silhouette
  and global lower-face scaling damages the jaw-neck transition; the next frontier is
  local semantic deformation for lids, sockets, lips, and nose structure.
- Direct anatomical head-boundary fit: `Derived/head-silhouette-v3/` corrects the
  centreline chin anchor, extracts the target outline relative to the fixed eye line,
  and applies one editable topology-preserving shape key. Fresh verification reduces
  complete front-boundary error by `94.29%`, with zero depth-axis change and maximum
  displacement `0.012273 m`. The raw exact fit was rejected because it copied raster
  edge noise; the retained fit smooths the target and propagates it through the head
  cross-section. Retain the control, reject likeness: the three-quarter temple/cheek
  indentation and all internal feature anatomy remain unresolved. Dependency order is
  head boundary, feature placement, local feature shape, then age/material detail.
- Head-evaluator correction: v3's `94.29%` promotion is invalid because lower raster
  extrema followed the connected neck rather than the jaw-to-chin contour, and one mean
  hid regional errors. `Derived/head-silhouette-v4/` limits automatic deformation and
  scoring to semantically valid skull, temple, and face-side boundaries; it emits
  eye/IPD-aligned target/generated crops, a sharp centre wipe, and a full-opacity blink.
  Mechanical checks can no longer
  promote a head. The required region review rejects skull, temples, ears, jaw/chin, and
  overall shape, so `promotionEligible=false`; the presentation champion remains
  `style-presentation-v5`.
