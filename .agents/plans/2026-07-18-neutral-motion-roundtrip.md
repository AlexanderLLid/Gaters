# Neutral Motion Round-trip Implementation Plan

> **For agentic workers:** use `superpowers:executing-plans` and TDD. Do not commit,
> branch, or push; the human reviews the shared worktree first.

**Goal:** Prove that a style-neutral motion brief can reproducibly generate a tiny rig,
weighted mesh, and animation in Blender, then import as native skeletal assets and an
animation sequence in Unreal.

**Architecture:** A small JSON brief and Blender Python generator are authoritative.
Blender `.blend` and FBX files are derived transports. A focused Unreal Python adapter
imports the FBX, measures native asset evidence, and writes a disposable report. Tests
compare semantic facts rather than nondeterministic binary transport bytes.

**Tech Stack:** Blender 5.2 Python, FBX, Unreal Engine 5.8 Python editor scripting,
native `USkeletalMesh`/`USkeleton`/`UAnimSequence`, PowerShell harnesses.

## Global constraints

- Keep `selectedStyle` null; this fixture proves machinery, not art direction.
- Blender Python produces curves; Unreal owns playback and later IK retargeting.
- Unreal PCG does not generate bone animation.
- No custom animation runtime, procedural character framework, or batch optimizer yet.
- Generated `.blend`, FBX, `.uasset`, previews, and reports are disposable evidence.

### Task 1: Reproducible Blender motion fixture

**Files:**
- Create test first: `SourceAssets/Blender/Test-MotionFixture.ps1`
- Create: `SourceAssets/Blender/Build-MotionFixture.ps1`
- Create: `SourceAssets/Blender/contracts/neutral-motion.json`
- Create: `SourceAssets/Blender/generate_motion_fixture.py`

**Interface:** The brief declares bone hierarchy, clip timing, root samples, and contact
events. The manifest records reopened Blender evidence, exact semantic samples, and one
FBX transport explicitly marked as byte-nondeterministic.

- [x] Write the harness and run RED against the missing generator.
- [x] Generate the armature, weighted primitive mesh, clip, markers, `.blend`, and FBX.
- [x] Reopen the `.blend`, sample the declared frames, and fail on hierarchy/timing/root
  motion/contact differences.
- [x] Build twice and require identical semantic manifests plus stale-output cleanup.

### Task 2: Native Unreal import fixture

**Files:**
- Create test first: `Unreal/Prototype/Scripts/Test-MotionFixtureImport.ps1`
- Create: `Unreal/Prototype/Scripts/ImportMotionFixture.py`
- Output: `Unreal/Prototype/Content/Gaters/Generated/Motion/`
- Output: `Unreal/Prototype/Saved/AssetImport/neutral-motion.json`

**Interface:** One automated import produces a native skeletal mesh, skeleton, and
animation sequence. The report records paths, classes, reference bones, duration, frame
count, and root-track/contact-event evidence available through Unreal.

- [x] Write the harness and run RED against the missing importer.
- [x] Validate the Blender manifest and FBX before mutating Unreal content.
- [x] Import, save, inspect, and report the native skeletal artifacts.
- [x] Import twice and require contract-equivalent evidence.

### Task 3: Mechanical evaluator boundary

**Files:**
- Modify: `research/machines.json`
- Create: `.agents/reports/neutral-motion-roundtrip.md`

- [x] Record what Blender and Unreal can measure without rendering.
- [x] Keep motion factory/evaluators unpromoted until counterexamples exist for broken
  hierarchy, timing, root path, contacts, penetration, and foot sliding.
- [x] Run relevant harnesses, full Unreal automation, registry validation, and whitespace
  verification.

## Self-review

- The fixture proves one path only; it is not a character generator or animation set.
- Semantic evidence is deterministic even when FBX bytes are not.
- Any later LLM loop changes briefs/generators and competes through evaluator evidence;
  it never edits promoted binaries as source truth.
