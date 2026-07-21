# Mechanical Motion Evaluator v1 Implementation Plan

> **For agentic workers:** use `superpowers:executing-plans` and TDD. Do not
> commit, branch, or push; the human reviews the shared worktree first.

**Goal:** Turn native Unreal import evidence plus its authoritative motion manifest into
a versioned pass/fail record that rejects broken hierarchy, timing, root motion, and
gameplay contact events.

**Architecture:** Keep Unreal as the native evidence producer. Add one standard-library
Python policy evaluator that consumes the existing manifest and import report, emits a
separate immutable-style evaluation record, and maps source contact frames into a
versioned event timeline rather than pretending FBX transported Unreal notifies.

**Tech Stack:** Python 3 standard library, PowerShell harness, Blender 5.2, Unreal 5.8.

## Global constraints

- Do not add a custom animation runtime, rig factory, character generator, or dependency.
- Do not treat the importer as its own judge; the evaluator is a separate process.
- Keep source manifests authoritative and Unreal assets/reports derived.
- Keep the machine `active`, not `verified`, until foot sliding, reach, penetration, and
  runtime combat fixtures exist.

---

### Task 1: Pure mechanical policy and counterexamples

**Files:**

- Create test first: `Unreal/Prototype/Scripts/Test-MotionMechanicalEvaluator.ps1`
- Create after RED: `Unreal/Prototype/Scripts/EvaluateMotionMechanical.py`

**Interfaces:**

- Consumes: `--manifest <manifest.json> --import-report <neutral-motion.json>`.
- Produces: `--output <motion-mechanical.json>` with `schemaVersion`,
  `evaluatorVersion`, `candidateId`, `passed`, `checks`, `issues`, and `gameplayEvents`.
- Exit `0` for a valid candidate and `2` for a mechanically invalid candidate.

- [x] Write a PowerShell harness that first runs the evaluator against the real neutral
  manifest/report and requires `passed=true` with the expected three mapped contact
  events.
- [x] Add held-out copies that independently remove `foot_l`, alter duration, alter the
  final root sample, and remove a contact event; require a named issue for each.
- [x] Run the harness before the evaluator exists and verify RED because the required
  policy executable is missing.
- [x] Implement the minimum evaluator with `json`, `hashlib`, `argparse`, and `pathlib`.
  Compare declared hierarchy, duration, key coverage, centimeter-converted root samples,
  and contact events. Map each valid event to seconds using `(frame-startFrame)/fps`.
- [x] Run the focused harness and require the positive candidate plus all four negative
  candidates to classify correctly.

### Task 2: End-to-end evidence and registry truth

**Files:**

- Modify: `Unreal/Prototype/Scripts/Test-MotionMechanicalEvaluator.ps1`
- Create: `.agents/reports/motion-mechanical-evaluator-v1.md`
- Modify: `research/machines.json`
- Modify: `.agents/workstreams/builder.md`

**Interfaces:**

- Consumes: reproducible Blender fixture and native Unreal import report.
- Produces: `Saved/AssetImport/motion-mechanical.json` and registry evidence for
  `evaluation.motion-mechanical`.

- [x] Make the harness invoke `Test-MotionFixtureImport.ps1` before evaluation so native
  evidence is always fresh.
- [x] Run the complete motion harness and require two reproducible imports followed by
  correct positive/negative mechanical classifications.
- [x] Record only verified checks and explicit missing checks in the short report.
- [x] Set `evaluation.motion-mechanical` to `active`, record the v1 champion and exact
  verifier/challenge evidence, and leave the promotion gate unchanged.
- [x] Update Primary Builder status with the current objective/evidence paths.
- [x] Run `research/Test-MachineRegistry.ps1`, full Gaters Unreal automation,
  `research/Test-SharedAgentDocs.ps1`, and `git diff --check`.

## Self-review

- The evaluator is independent from the Blender generator and Unreal importer.
- Missing Unreal notifies are not hidden: gameplay events are an explicit manifest-backed
  sidecar until a runtime consumer exists.
- Runtime foot contact, reach, penetration, deformation, and combat timing remain outside
  this slice and block verification.
