# Native Foot-Sliding Evidence Implementation Plan

> **For agentic workers:** use `superpowers:executing-plans` and TDD. Do not
> commit, branch, or push; the human reviews the shared worktree first.

**Goal:** Reject imported movement clips whose planted feet drift across component space,
then repair the neutral fixture until native Unreal evidence passes.

**Architecture:** Unreal's native `AnimPoseExtensions` samples each foot in component
space for every animation frame. The independent mechanical evaluator derives stance
windows from alternating contact events and measures horizontal drift. Blender remains
the authoritative motion source and is repaired only after the native evaluator fails.

**Tech Stack:** Blender 5.2 Python, Unreal 5.8 Python API, Python standard library,
PowerShell harnesses.

## Constraints

- Do not build IK, retargeting, runtime playback, or a custom pose sampler.
- Treat contact events as stance starts ending at the next contact event.
- Keep the drift limit in evaluator output, not prose canon.
- Do not touch Base, Combat, lore, or shared Unreal runtime code.

### Task 1: Native component-space evidence

**Files:**

- Modify test first: `Unreal/Prototype/Scripts/Test-MotionFixtureImport.ps1`
- Modify after RED: `Unreal/Prototype/Scripts/ImportMotionFixture.py`

- [x] Require `footWorldSamples` for `foot_l` and `foot_r`, with one component-space
  position per sampled frame, and verify repeated imports preserve them.
- [x] Run the import harness and verify RED because the field is missing.
- [x] Use `AnimPoseExtensions.get_anim_pose_at_frame` and
  `get_bone_pose(..., AnimPoseSpaces.WORLD)` to record the native positions.
- [x] Re-run the import harness and require deterministic evidence for all frames.

### Task 2: Independent slide rule and generator repair

**Files:**

- Modify test first: `Unreal/Prototype/Scripts/Test-MotionMechanicalEvaluator.ps1`
- Modify after RED: `Unreal/Prototype/Scripts/EvaluateMotionMechanical.py`
- Modify after native failure: `SourceAssets/Blender/generate_motion_fixture.py`

- [x] Require the positive candidate to report `checks.footSliding=true`, per-stance
  drift evidence, and a declared centimeter limit. Add a held-out report with one planted
  foot sample shifted beyond the limit and require `motion.contact.foot_sliding`.
- [x] Run the mechanical harness and verify RED because no slide rule exists.
- [x] Add the minimum rule: for each event, inspect that foot from its frame through the
  next event frame and measure maximum horizontal displacement from stance start.
- [x] Re-run and preserve the native failure showing the current fixture slides.
- [x] Repair Blender foot translation keys so each stance cancels root translation;
  make the root and foot translation curves linear.
- [x] Re-run the complete Blender-to-Unreal evaluator; require the repaired native clip
  to pass and the shifted-foot counterexample to fail.

### Task 3: Evidence and registry

**Files:**

- Modify: `.agents/reports/motion-mechanical-evaluator-v1.md`
- Modify: `research/machines.json`
- Modify: `.agents/workstreams/builder.md`

- [x] Record native component-space sampling, the falsified sliding fixture, generator
  repair, and held-out rejection without claiming runtime IK or retargeting.
- [x] Update the active machine's verifier/champion text in place.
- [x] Run the motion harness, full Gaters automation, registry/shared-doc validators,
  and `git diff --check`.

## Self-review

- The producer does not judge itself; Unreal measures, the evaluator judges, and Blender
  repairs source data.
- The test covers one alternating two-foot gait only. Reach, penetration, deformation,
  retargeting, combat timing, and runtime cost remain outside this slice.
