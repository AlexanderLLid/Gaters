# Procedural head machine

First dependency-free leaf of the zero-manual character pipeline:

```text
head-recipe/0 -> head-mesh/0 + head-regions/0 -> independent verification
```

Run the baseline twice:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File research/procedural-head-machine/Test-ProceduralHeadMachine.ps1
```

Run the held-out broad-jaw recipe:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File research/procedural-head-machine/Test-ProceduralHeadMachine.ps1 `
  -Recipe research/procedural-head-machine/recipes/broad-jaw.json
```

Each run preserves the exact recipe, mesh, region weights, OBJ preview, independent
verification, provenance receipt, and canonical hashes. The OBJ is diagnostic derived
output. This machine proves no face, likeness, deformation topology, rig, or animation.

Compare fresh native Blender and Houdini materializations of an accepted run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File research/procedural-head-machine/Test-HeadBackendAdapters.ps1 `
  -SourceRun research/procedural-head-machine/Runs/<accepted-run>/run-1
```

Both adapters save a native scene, reopen it in another process, export raw geometry and
region weights, and pass the same external topology/tolerance verifier.

Compare native editable deformation graphs:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File research/procedural-head-machine/Test-HeadNativeDeformation.ps1 `
  -SourceRun research/procedural-head-machine/Runs/<accepted-run>/run-1 `
  -Command research/procedural-head-machine/recipes/widen-jaw.json
```

The command remains authoritative. Blender stores it as Geometry Nodes; Houdini stores
it as a SOP graph. Fresh readback verifies graph identity and amount, exact expected
positions within tolerance, unchanged topology and semantic weights, a moved active jaw,
and a protected skull.

Compose an accepted head with a generated neck:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File research/procedural-head-machine/Test-HeadNeckSocket.ps1 `
  -SourceRun research/procedural-head-machine/Runs/<accepted-run>/run-1 `
  -Socket research/procedural-head-machine/recipes/head-neck-socket.json
```

Materialize and freshly reopen an accepted composition in Houdini:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File research/procedural-head-machine/Test-ModuleCompositionHoudiniAdapter.ps1 `
  -CompositionRun research/procedural-head-machine/SocketRuns/<accepted-run>/run-1
```

The socket compiler reuses one interface ring, preserves the upstream head, extrapolates
the incoming tangent into the first neck ring, closes the output, and carries semantic
region/module labels. This proves one head-to-neck structural interface only; it does not
prove arbitrary attachment frames, anatomy, seam deformation, rigging, or visual quality.

Generate one parent with mirrored local-frame branches:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File research/procedural-head-machine/Test-MirroredBranches.ps1
```

Run the held-out proportions and topology:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File research/procedural-head-machine/Test-MirroredBranches.ps1 `
  -Parent research/procedural-head-machine/recipes/torso-branch-parent-held-out.json `
  -Branch research/procedural-head-machine/recipes/torso-mirrored-branches-held-out.json
```

The parent emits explicit left/right socket frames. One branch recipe is evaluated in
those local frames, so the compiler does not contain separate left/right shape recipes.
The verifier checks shared interfaces, closed topology, orthonormal frames, mirror error,
realized length, labels, and reproducibility. This remains a structural box-and-branches
fixture, not a torso or arms quality claim.

Compile the first complete structural humanoid `BodyPlan`:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File research/procedural-head-machine/Test-BodyPlan.ps1
```

Compile the held-out tall proportions:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File research/procedural-head-machine/Test-BodyPlan.ps1 `
  -Plan research/procedural-head-machine/recipes/stick-humanoid-body-plan-held-out.json
```

Materialize and freshly reopen an accepted body in Houdini:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File research/procedural-head-machine/Test-BodyPlanHoudiniAdapter.ps1 `
  -BodyRun research/procedural-head-machine/BodyPlanRuns/<accepted-run>/run-1
```

`body-plan/0` declares a root core plus explicit neck, head, paired arm/hand, and paired
leg/foot nodes. The compiler places
modules on an integer volume grid, removes internal contact faces, and emits one connected
closed boundary with placements, contact frames, source cells, mirror declarations, and
semantic labels. Every accepted run also emits a labelled front/three-quarter PNG from
the actual mesh. This proves structural body-plan compilation, not smooth anatomy,
deformation topology, rigging, animation, or art quality.

Materialize an accepted BodyPlan as one smooth Houdini surface:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File research/procedural-head-machine/Test-SmoothBody.ps1 `
  -BodyRun research/procedural-head-machine/BodyPlanRuns/<accepted-run>/run-1
```

The `body-surface/0` recipe converts one ellipsoid per BodyPlan module into a VDB signed
distance field, smooths and polygonizes it, restores nearest-module/role labels, reopens
the scene in a separate process, and emits a labelled front/three-quarter preview. It
proves a connected rounded surface and replaceable BodyPlan-to-Houdini contract. It does
not prove human anatomy, joint segmentation, deformation topology, or production art.

Compile the shared anatomical guide from an accepted BodyPlan:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File research/procedural-head-machine/Test-AnatomicalGuide.ps1 `
  -BodyRun research/procedural-head-machine/BodyPlanRuns/<accepted-run>/run-1
```

`anatomical-guide/0` resolves named joint landmarks, skeleton segments, tapered surface
segments, torso volumes, explicit terminal volumes, symmetry pairs, and exact BodyPlan
provenance. The guide is the
single tool-neutral anatomy input intended for both the surface and later guide skeleton.
It proves consistent measurements and replay, not correct anatomy or visual quality.

Materialize the accepted guide as one Houdini surface:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File research/procedural-head-machine/Test-AnatomicalSurface.ps1 `
  -BodyRun research/procedural-head-machine/BodyPlanRuns/<accepted-run>/run-1 `
  -GuideRun research/procedural-head-machine/AnatomyRuns/<accepted-run>/run-1
```

The anatomical surface uses the guide's ellipsoids and sampled tapered segments as the
authoritative inputs to the existing Houdini VDB graph. Independent verification checks
guide provenance, topology, bounds, labels, replay, and taper measured from the actual
mesh. This is an anatomical mannequin proof, not a finished human, deformation mesh,
rig, animation, or art-style result.

The current profiled mid-poly challenger extends that same recipe with cranium,
lower-face, jaw, deltoid, hand, and foot volumes. Houdini applies a native polygon target
before restoring semantic labels. Accepted runs emit both a semantic preview and a
clay-shaded preview from the actual mesh. Mid-poly is an experiment target, not a chosen
art direction. The first profile attempt is retained as rejected evidence because its
two-part head and oversized deltoids produced a mushroom skull and swollen shoulders.

Generate and materialize a KineFX guide skeleton from the same guide:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File research/procedural-head-machine/Test-GuideSkeleton.ps1 `
  -GuideRun research/procedural-head-machine/AnatomyRuns/<accepted-run>/run-1

powershell -NoProfile -ExecutionPolicy Bypass -File research/procedural-head-machine/Test-GuideSkeletonHoudiniAdapter.ps1 `
  -SkeletonRun research/procedural-head-machine/SkeletonRuns/<accepted-run>/run-1
```

`guide-skeleton/0` compiles a rooted 16-joint / 15-bone tree with orthonormal joint
frames and exact source-guide identity. Houdini stores it as native KineFX points with
`name` and `transform` attributes connected by hierarchy polylines, then reopens it in a
separate process. This proves guide-to-skeleton reuse, not controls, limits, skinning,
deformation quality, retargeting, or animation.

Generate initial skin weights and a diagnostic pose:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File research/procedural-head-machine/Test-SkinCapture.ps1 `
  -SurfaceRun research/procedural-head-machine/AnatomicalSurfaceRuns/<accepted-run>/run-1 `
  -SkeletonRun research/procedural-head-machine/SkeletonRuns/<accepted-run>/run-1
```

Houdini's native Joint Capture Proximity SOP generates normalized two-influence weights;
Joint Deform applies a fixed 55-degree elbow pose. Verification checks rest-surface
preservation, influence count, normalization, side isolation, provenance, active-limb
movement, and protected-module stability after a fresh scene reopen. The labelled weight
and posed-mesh previews are derived from the actual captured geometry. This proves the
first automated bind and deformation path, not production deformation quality.

Compare proximity against the four-influence-limited biharmonic challenger:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File research/procedural-head-machine/Test-SkinCaptureComparison.ps1 `
  -ChampionRun research/procedural-head-machine/SkinCaptureRuns/<proximity-run> `
  -ChallengerRun research/procedural-head-machine/SkinCaptureRuns/<biharmonic-run>
```

The independent comparison measures the tenth-percentile surface thickness around the
posed elbow. Biharmonic capture is the current champion: it retains `91.2%` versus
`78.8%` on the baseline and `92.6%` versus `70.1%` on the tall held-out body while
remaining at four influences. This is one bend fixture, not production skinning proof.

Run the generated five-pose deformation suite from one captured skin:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File research/procedural-head-machine/Test-SkinCapture.ps1 `
  -SurfaceRun research/procedural-head-machine/AnatomicalSurfaceRuns/<accepted-run>/run-1 `
  -SkeletonRun research/procedural-head-machine/SkeletonRuns/<accepted-run>/run-1 `
  -Recipe research/procedural-head-machine/recipes/biharmonic-pose-suite.json
```

The suite applies elbow, knee, shoulder, hip, and torso-twist recipes without recapturing
the skin. It measures active motion, strict protected modules, bounded seam-transition
modules, and tenth-percentile thickness around every tested joint. Baseline and tall
fixtures pass. This proves a reusable deformation evaluator, not controls or animation.

Compile joint limits and a bounded motion sequence:

```powershell
python research/procedural-head-machine/src/run_joint_limits.py `
  research/procedural-head-machine/SkeletonRuns/<accepted-run>/run-1 `
  research/procedural-head-machine/recipes/humanoid-joint-limits.json `
  research/procedural-head-machine/recipes/biharmonic-pose-suite.json `
  --output-root research/procedural-head-machine/JointLimitRuns

python research/procedural-head-machine/src/run_motion_curve.py `
  research/procedural-head-machine/SkeletonRuns/<accepted-run>/run-1 `
  research/procedural-head-machine/JointLimitRuns/<accepted-run>/run-1 `
  research/procedural-head-machine/recipes/humanoid-motion-proof.json `
  --output-root research/procedural-head-machine/MotionRuns
```

Materialize and verify the generated timeline in Houdini:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File research/procedural-head-machine/Test-MotionSequence.ps1 `
  -SourceSkinRun research/procedural-head-machine/SkinCaptureRuns/<accepted-run> `
  -MotionRun research/procedural-head-machine/MotionRuns/<accepted-run>/run-1
```

The source contracts own nine joint controls and a 13-frame rest-to-peak-to-rest motion.
Houdini uses a frame-driven KineFX skeleton Switch and one Joint Deform node. Verification
reopens two fresh scenes, compares exact readback hashes, checks every skeleton frame,
rest topology, endpoints, continuity, and provenance, then renders five labelled frames
from the actual deformed mesh. Baseline and tall fixtures pass. This is a generated
motion proof, not final joint ranges, contact-aware locomotion, animation quality,
retargeting, or Unreal runtime evidence.

Create a rotatable inspection view from an accepted motion readback:

```powershell
python research/procedural-head-machine/src/render_model_inspector.py `
  research/procedural-head-machine/MotionSequenceRuns/<accepted-run>/run-1/readback.json `
  <output>/generated-humanoid-inspector.html
```

The viewer exposes the actual accepted vertices and faces with rest/peak pose,
solid/wireframe, orbit, and zoom controls. It is inspection evidence, not a new model.
