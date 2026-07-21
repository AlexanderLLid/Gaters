# Profiled Mid-Poly Body Implementation Plan

> **For agentic workers:** Execute inline with TDD. Steps use checkbox syntax for tracking.

**Goal:** Generate a more recognizable humanoid silhouette at a configurable mid-poly
face budget without replacing the BodyPlan, guide, rig, or animation contracts.

**Architecture:** The existing anatomical-guide recipe gains surface-only profile knobs.
The compiler emits additional labelled ellipsoids. Houdini reuses its VDB graph and adds
native polygon reduction before semantic relabelling.

**Tech Stack:** Python standard library, unittest, Houdini 22.0 Python/SOP nodes, JSON.

## Global constraints

- Final art style remains open; mid-poly is a challenger target only.
- Tool-neutral recipes and compiled guide remain authoritative.
- No manual modelling, rigging, skinning, animation, or Unreal work.
- Preserve baseline and held-out BodyPlan support.

### Task 1: Profile contract

**Files:**
- Modify: `research/procedural-head-machine/tests/test_anatomical_guide_compiler.py`
- Modify: `research/procedural-head-machine/recipes/humanoid-anatomical-guide.json`
- Modify: `research/procedural-head-machine/src/anatomical_guide_compiler.py`

**Interface:** `compile_anatomical_guide(body, recipe)` emits named `cranium`,
`lower_face`, bilateral `deltoid`, hand, and foot volumes without changing landmarks.

- [ ] Add tests asserting the named volumes, bilateral mirror geometry, and slimmer
  terminal proportions.
- [ ] Run the test and observe failure because those volumes do not exist.
- [ ] Add the minimum recipe fields and compiler logic.
- [ ] Run guide tests and confirm they pass.

### Task 2: Polygon target

**Files:**
- Modify: `research/procedural-head-machine/tests/test_anatomical_surface_verifier.py`
- Modify: `research/procedural-head-machine/recipes/anatomical-mannequin-surface.json`
- Modify: `research/procedural-head-machine/adapters/houdini_smooth_body_build.py`
- Modify: `research/procedural-head-machine/src/smooth_body_verifier.py`

**Interface:** `body-surface/0.target_faces` is a positive integer; Houdini applies a
native PolyReduce SOP and verification rejects output above the declared target tolerance.

- [ ] Add a test rejecting a readback above `target_faces`.
- [ ] Run the test and observe failure because the contract is ignored.
- [ ] Add the recipe field, PolyReduce node, persisted parameter, and verifier rule.
- [ ] Run surface verifier tests and confirm they pass.

### Task 3: Evidence

**Files:**
- Derived runs under `research/procedural-head-machine/AnatomyRuns/` and
  `AnatomicalSurfaceRuns/`.
- Modify: `research/procedural-head-machine/README.md`
- Modify: `.agents/workstreams/Art Direction.md`

- [ ] Compile baseline and tall guides.
- [ ] Build each Houdini surface twice and freshly reopen it.
- [ ] Inspect labelled previews and compare against the accepted mannequin.
- [ ] Run the complete unit suite and shared-document validator.
- [ ] Record truthful evidence and the remaining visual gap; do not promote on mechanical
  tests alone.
