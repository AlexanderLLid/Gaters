# Procedural Head Envelope Implementation Plan

> **For agentic workers:** Implement task-by-task with test-first evidence. Do not commit,
> branch, or push; the human reviews workspace changes first.

**Goal:** Build the first dependency-free leaf of the zero-manual character generator:
one tool-neutral recipe compiles into a verified procedural head envelope with stable
topology and semantic region weights.

**Architecture:** Standard-library Python compiles JSON into canonical mesh and region
artifacts. A separate verifier consumes only those artifacts and the recipe. DCC adapters
are deliberately excluded until the contract passes twice.

**Tech Stack:** Python standard library, JSON, `unittest`, PowerShell.

## Global Constraints

- Authoritative inputs and outputs are tool-neutral files; DCC scenes are derived.
- No manual modelling or repair step may satisfy a promotion gate.
- No Blender, Houdini, Unreal, external package, face feature, rig, or animation work.
- Preserve recipe, artifacts, hashes, versions, and causal verification results.
- Requirements checked: `ART-1`, `ART-2`, `ART-3`, `ART-4`, `CHAR-1`, `CHAR-2`,
  `CHAR-3`, `CHAR-4`, generated-content boundary; exceptions: `ART-1` and `ART-2` are not
  visually evaluated and no downstream character capability is claimed.

---

### Task 1: Head recipe and compiler

**Files:**

- Create: `research/procedural-head-machine/recipes/baseline.json`
- Create: `research/procedural-head-machine/recipes/broad-jaw.json`
- Create: `research/procedural-head-machine/src/head_compiler.py`
- Create: `research/procedural-head-machine/tests/test_head_compiler.py`

**Interfaces:**

- Consumes: `compile_recipe(recipe: dict) -> dict`
- Produces: canonical `head-mesh/0` dictionary containing `vertices`, `faces`, and
  per-vertex `regions`.

- [x] Write tests first for stable topology, closed face indices, bounded normalized
  region weights, declared bounds, and jaw-only held-out variation.
- [x] Run `py -m unittest discover -s research/procedural-head-machine/tests -v` and
  observe failure because `head_compiler` does not exist.
- [x] Implement only the UV-ring envelope and four region-weight functions needed by the
  tests.
- [x] Rerun the test command and require all compiler tests to pass.

### Task 2: Independent verifier and repeatable runner

**Files:**

- Create: `research/procedural-head-machine/src/head_verifier.py`
- Create: `research/procedural-head-machine/src/run_machine.py`
- Create: `research/procedural-head-machine/tests/test_head_verifier.py`
- Create: `research/procedural-head-machine/Test-ProceduralHeadMachine.ps1`
- Create: `research/procedural-head-machine/README.md`

**Interfaces:**

- Consumes: recipe plus raw `head-mesh/0` dictionary.
- Produces: `verification.json` with causal rule IDs and `receipt.json` with canonical
  hashes and tool versions.

- [x] Write failing tests for finite coordinates, valid non-degenerate faces, closed
  manifold edge counts, normalized regions, bounds, and malformed counterexamples.
- [x] Run the focused verifier tests and observe the expected missing-module failure.
- [x] Implement the verifier without importing compiler functions.
- [x] Implement a runner that performs two fresh builds, writes immutable run folders,
  compares canonical hashes, and exits nonzero on verification failure.
- [x] Run the PowerShell entry point for baseline and broad-jaw recipes; require both
  repeated builds to pass and retain identical topology IDs across recipes.

### Task 3: Promotion decision

**Files:**

- Modify: `.agents/workstreams/Art Direction.md`

- [x] Record exact run paths, hashes, checks, and limitations.
- [x] Promote only the tool-neutral head leaf. Keep visual likeness, Blender, Houdini,
  topology-for-deformation, rigging, motion, and full-creature generation unproven.
- [x] Select the next frontier: implement the accepted artifact in Blender and Houdini
  without changing its vertices, faces, regions, or receipt identity.
