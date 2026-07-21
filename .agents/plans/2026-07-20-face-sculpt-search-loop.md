# Face Sculpt Search Loop Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use
> `superpowers:subagent-driven-development` to implement Tasks 1–3 and 5–6. Task 4 is
> the bounded multi-agent experiment. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build and run a bounded sculpt → blind evaluation → promotion loop that moves
an editable Blender face closer to the approved front reference through named parameter
search, while preserving the complete process as replayable evidence for the later face
generator.

**Architecture:** The existing MPFB face and clay sculpt remain the geometry foundation.
A small standard-library search controller produces symmetric `-delta` and `+delta`
candidate recipes for one named facial axis. Blender rebuilds both candidates, two
independent evaluator agents compare blinded fixed-view renders, and the root agent
promotes a candidate only when both evaluators agree and every hard geometry check passes.
Every candidate and verdict is preserved under one run identity: small authoritative
recipes, agent exchanges, evaluations, and decisions live under `Runs/`; reproducible
Blender files, renders, and logs live under ignored `Derived/search-runs/`.

**Tech Stack:** Python 3 standard library, Blender 5.2 LTS Python API, MPFB 2.x,
PowerShell, JSON, Codex subagents.

## Global Constraints

- `ART-1`: every promoted face remains grounded, mature, plausible, smooth, and
  restrained; cartoon, toy, cel-shaded, and deliberately faceted results are hard fails.
- `ART-2`: evaluation rewards plausible individual age, ancestry, proportion, and natural
  asymmetry; conventional attractiveness is not a target.
- `ART-3`: all source and evidence remain under
  `research/character-style-exploration/blender-face-proof/`; no character-generation,
  rigging, animation, Unreal, or runtime capability is touched or claimed.
- `ART-4`: every round records the exact recipe and delta, selected axis and reason,
  generator/tool/evaluator identities, agent instructions and responses, artifacts,
  ballots, promotion or stop decision, failure diagnosis, and next experiment. Failed
  challengers are training evidence and are never discarded from the run lineage.
- The approved painting supplies front-view likeness evidence only. Three-quarter and
  profile views are checked for plausible three-dimensional continuity, not invented
  likeness certainty.
- This loop evaluates facial geometry only. Every candidate uses the same neutral-clay
  material, cameras, exposure, and diagnostic lights; lighting, skin, hair, eyebrows,
  texture, and presentation quality cannot improve its rank. Evaluators compare the
  reference directly with each candidate's front view, then use the other views only to
  reject three-dimensional regressions.
- The evaluator never edits geometry. The sculptor never decides promotion. The root
  agent owns search state and promotion.
- A candidate with failed reopen, projection/material contamination, armature, animation,
  missing view, or invalid parameter provenance cannot be promoted.
- The overnight prototype runs at most sixteen rounds and stops after four consecutive
  rounds without promotion, three consecutive evaluator disagreements, or any repeated
  Blender hard failure. These values live in `face-search-space.json`.
- No dependency additions. No branch, commit, push, or Unreal operation.

## Generator-learning record

Each run owns these authoritative source records:

- `Runs/<run-id>/run.json`: reference hash, initial champion, search-space hash, source
  script hashes, Blender/MPFB versions, agent/model identities, limits, and stop reason.
- `Runs/<run-id>/candidates/<candidate-id>.json`: complete parameters, parent, searched
  axis, signed delta, and derived artifact identities.
- `Runs/<run-id>/rounds/round-<n>.json`: champion before/after, axis selection and evidence,
  step before/after, both candidate IDs, verbatim sculptor/evaluator instructions and
  responses, validated ballots, promotion decision, failures, and next experiment.
- `Runs/<run-id>/generator-learning.jsonl`: one canonical row per completed round,
  including rejected candidates, deterministically projected from the records above.

The later generator may learn from these records; it must not treat a promoted candidate
as human-approved unless a later record explicitly contains human acceptance.

## Approach decision

- **Selected — adaptive coordinate search:** produces two controlled challengers around
  the champion, narrows steps after ties, and revisits axes when coupled features change.
- **Rejected — strict binary search:** assumes one monotonic independent variable; facial
  controls interact, so an earlier optimum can become wrong after another feature moves.
- **Rejected — unconstrained agent sculpting:** may improve a render, but cannot attribute
  the improvement, replay it, or learn a reliable process from the result.

---

### Task 1: Define the bounded search contract

**Files:**

- Create: `research/character-style-exploration/blender-face-proof/face-search-space.json`
- Create: `research/character-style-exploration/blender-face-proof/face_search.py`
- Create: `research/character-style-exploration/blender-face-proof/test_face_search.py`
- Create during execution:
  `research/character-style-exploration/blender-face-proof/Runs/<run-id>/run.json`

**Interfaces:**

- Consumes: the accepted `sculpt-v13` parameter values as the initial champion.
- Produces:
  - `load_space(path: Path) -> dict`
  - `new_state(space: dict, run_id: str) -> dict`
  - `propose_pair(state: dict, axis: str, round_index: int) -> tuple[dict, dict]`
  - `validate_candidate(space: dict, candidate: dict) -> None`
  - `validate_round_record(record: dict) -> None`
  - `project_learning_row(record: dict) -> dict`
  - `decide_promotion(state: dict, ballots: list[dict]) -> dict`
  - CLI commands `init`, `propose`, `validate`, and `promote`.

- [ ] **Step 1: Write the failing symmetric-pair and bounds tests**

```python
from copy import deepcopy
from pathlib import Path
import unittest

from face_search import (
    load_space,
    new_state,
    project_learning_row,
    propose_pair,
    validate_candidate,
)


ROOT = Path(__file__).resolve().parent


class FaceSearchTests(unittest.TestCase):
    def setUp(self):
        self.space = load_space(ROOT / "face-search-space.json")
        self.state = new_state(self.space, "test-run")

    def test_pair_is_symmetric_around_champion(self):
        lower, upper = propose_pair(self.state, "nose_width", 1)
        current = self.state["champion"]["parameters"]["nose_width"]
        self.assertAlmostEqual(current - lower["parameters"]["nose_width"], 0.07)
        self.assertAlmostEqual(upper["parameters"]["nose_width"] - current, 0.07)
        self.assertEqual(lower["parentId"], self.state["champion"]["candidateId"])
        self.assertEqual(upper["parentId"], self.state["champion"]["candidateId"])

    def test_candidate_outside_axis_bounds_is_rejected(self):
        candidate = deepcopy(self.state["champion"])
        candidate["parameters"]["nose_width"] = 9.0
        with self.assertRaisesRegex(ValueError, "nose_width outside"):
            validate_candidate(self.space, candidate)


if __name__ == "__main__":
    unittest.main()
```

- [ ] **Step 2: Run RED**

Run:

```powershell
python research/character-style-exploration/blender-face-proof/test_face_search.py
```

Expected: fail because `face_search` or `face-search-space.json` does not exist.

- [ ] **Step 3: Add the exact first-wave search space**

```json
{
  "schemaVersion": 1,
  "maxRounds": 16,
  "stopAfterNoPromotion": 4,
  "stopAfterDisagreement": 3,
  "axes": {
    "head_width":       { "minimum": -0.25, "maximum": 0.10, "initial": -0.08, "step": 0.05, "minimumStep": 0.0125 },
    "head_height":      { "minimum": -0.20, "maximum": 0.20, "initial":  0.00, "step": 0.05, "minimumStep": 0.0125 },
    "jaw_taper":        { "minimum":  0.00, "maximum": 0.20, "initial":  0.085, "step": 0.03, "minimumStep": 0.0075 },
    "chin_projection":  { "minimum": -0.30, "maximum": 0.15, "initial": -0.12, "step": 0.05, "minimumStep": 0.0125 },
    "nose_width":       { "minimum":  0.00, "maximum": 0.55, "initial":  0.26, "step": 0.07, "minimumStep": 0.0175 },
    "nose_length":      { "minimum":  0.00, "maximum": 0.65, "initial":  0.30, "step": 0.07, "minimumStep": 0.0175 },
    "nose_projection":  { "minimum":  0.00, "maximum": 0.45, "initial":  0.14, "step": 0.06, "minimumStep": 0.0150 },
    "eye_spacing":      { "minimum": -0.20, "maximum": 0.20, "initial":  0.00, "step": 0.05, "minimumStep": 0.0125 },
    "cheek_definition": { "minimum":  0.00, "maximum": 0.70, "initial":  0.36, "step": 0.08, "minimumStep": 0.0200 },
    "lip_volume":       { "minimum": -0.15, "maximum": 0.35, "initial":  0.08, "step": 0.07, "minimumStep": 0.0175 },
    "eye_bag":          { "minimum":  0.00, "maximum": 0.75, "initial":  0.44, "step": 0.08, "minimumStep": 0.0200 }
  }
}
```

- [ ] **Step 4: Implement the minimum pure search state**

Use JSON only. Candidate IDs are the SHA-256 of canonical JSON containing
`schemaVersion`, `parentId`, `round`, `axis`, `direction`, and `parameters`. Clamp the
two proposed values to the axis bounds and reject a pair if clamping makes either value
equal to the champion. `new_state` records one champion, per-axis step sizes, no-promotion
count, disagreement count, and an empty immutable history.

`validate_round_record` requires `championBefore`, `selectedAxis`, `axisReason`,
`stepBefore`, both complete candidates, source/tool/evaluator identities, verbatim agent
instructions and responses, both validated ballots, failures, decision, `championAfter`,
`stepAfter`, and `nextExperiment`. `project_learning_row` retains accepted and rejected
candidate deltas plus diagnoses; it removes only redundant image bytes and Blender logs,
which remain referenced by hash and relative path.

- [ ] **Step 5: Write and run promotion tests**

Add tests proving:

```python
def test_promotion_requires_two_agreeing_blind_ballots(self):
    lower, upper = propose_pair(self.state, "nose_width", 1)
    ballots = [
        {"evaluatorId": "primary", "winnerId": upper["candidateId"], "hardFailures": [], "severeRegressions": []},
        {"evaluatorId": "audit", "winnerId": upper["candidateId"], "hardFailures": [], "severeRegressions": []}
    ]
    decision = decide_promotion(self.state, ballots)
    self.assertEqual(decision["winnerId"], upper["candidateId"])
    self.assertTrue(decision["promote"])

def test_disagreement_keeps_champion_and_halves_step(self):
    before = self.state["steps"]["nose_width"]
    ballots = [
        {"evaluatorId": "primary", "winnerId": "candidate-a", "hardFailures": [], "severeRegressions": []},
        {"evaluatorId": "audit", "winnerId": "candidate-b", "hardFailures": [], "severeRegressions": []}
    ]
    decision = decide_promotion(self.state, ballots)
    self.assertFalse(decision["promote"])
    self.assertEqual(decision["nextStep"], before / 2.0)

def test_learning_row_keeps_rejected_candidates_and_failures(self):
    record = {
        "championBefore": "champion",
        "selectedAxis": "nose_width",
        "axisReason": ["nose remains too narrow"],
        "stepBefore": 0.07,
        "candidates": [
            {"candidateId": "lower", "delta": -0.07},
            {"candidateId": "upper", "delta": 0.07}
        ],
        "agents": {"sculptor": {}, "primary": {}, "audit": {}},
        "ballots": [],
        "failures": ["primary-audit-disagreement"],
        "decision": {"promote": False, "humanAccepted": False},
        "championAfter": "champion",
        "stepAfter": 0.035,
        "nextExperiment": {"axis": "nose_projection"}
    }
    row = project_learning_row(record)
    self.assertEqual([item["candidateId"] for item in row["candidates"]], ["lower", "upper"])
    self.assertEqual(row["failures"], ["primary-audit-disagreement"])
    self.assertFalse(row["decision"]["humanAccepted"])
```

Run the single unittest file until all tests pass with no third-party package.

---

### Task 2: Make the Blender sculpt consume candidate parameters

**Files:**

- Modify: `research/character-style-exploration/blender-face-proof/sculpt_clay.py`
- Modify: `research/character-style-exploration/blender-face-proof/verify_sculpt.py`
- Modify: `research/character-style-exploration/blender-face-proof/test_face_search.py`
- Derived: `research/character-style-exploration/blender-face-proof/Derived/search-runs/<run-id>/round-<n>/<candidate-id>/`

**Interfaces:**

- Consumes: `--candidate <candidate.json>` and the existing
  `Derived/face-v2/face-proof.blend`.
- Produces: `face-sculpt.blend`, `front.png`, `three-quarter.png`, `profile.png`, and
  `manifest.json` containing the candidate ID, parent ID, axis, direction, complete
  parameter vector, candidate-file SHA-256, active shape keys, and hard-check results.

- [ ] **Step 1: Write RED for semantic-to-Blender target mapping**

Add a pure `candidate_to_controls(parameters)` function to `face_search.py` and tests
asserting these mappings:

```python
def test_nose_width_maps_to_all_width_controls(self):
    controls = candidate_to_controls({**self.state["champion"]["parameters"], "nose_width": 0.40})
    self.assertEqual(controls["nose.width"], 0.40)
    self.assertEqual(controls["sculpt.nose.tip"], 0.28)
    self.assertEqual(controls["sculpt.nose.nostrils"], 0.24)

def test_eye_spacing_is_bilateral(self):
    controls = candidate_to_controls({**self.state["champion"]["parameters"], "eye_spacing": 0.10})
    self.assertEqual(controls["sculpt.eye.left.out"], 0.10)
    self.assertEqual(controls["sculpt.eye.right.out"], 0.10)
```

Run RED because `candidate_to_controls` does not exist.

- [ ] **Step 2: Implement the fixed mapping**

Use existing MPFB target names. Positive and negative signed controls choose one target
with an absolute weight; never activate opposing targets simultaneously.

```python
TARGET_MAP = {
    "head_width": ("head", "head-scale-horiz-decr", "head-scale-horiz-incr"),
    "head_height": ("head", "head-scale-vert-decr", "head-scale-vert-incr"),
    "eye_spacing": (
        ("eyes", "l-eye-trans-in", "l-eye-trans-out"),
        ("eyes", "r-eye-trans-in", "r-eye-trans-out")
    )
}
```

Map the remaining axes to the already used MPFB keys and direct fields:

- `jaw_taper` → direct lower-face horizontal taper.
- `chin_projection` → `chin-prominent-decr/incr`.
- `nose_width` → `nose.width`; tip and nostril width are `0.70` and `0.60` of it.
- `nose_length` → `nose.length`.
- `nose_projection` → `nose.bridgeDepth`.
- `cheek_definition` → bilateral cheek-bone increase and cheek-volume decrease, retaining
  the current restrained left/right difference.
- `lip_volume` → upper/lower lip volume decrease/increase.
- `eye_bag` → bilateral eye-bag increase, retaining the current restrained difference.

- [ ] **Step 3: Replace hardcoded sculpt values with the candidate mapping**

Add `--candidate` to `arguments()`. Load and validate the candidate before touching
Blender. `apply_sculpt_targets`, the direct jaw field, and the eye/cheek fields consume
only mapped controls. Do not change topology, materials, cameras, or the projection ban.

- [ ] **Step 4: Prove two parameter values create different geometry**

Initialize a test run and propose the first pair:

```powershell
python research/character-style-exploration/blender-face-proof/face_search.py init `
  --space research/character-style-exploration/blender-face-proof/face-search-space.json `
  --run research/character-style-exploration/blender-face-proof/Derived/search-runs/smoke
python research/character-style-exploration/blender-face-proof/face_search.py propose `
  --run research/character-style-exploration/blender-face-proof/Derived/search-runs/smoke `
  --axis nose_width
```

Run Blender once per emitted candidate using the exact shape:

```powershell
& 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe' --background `
  'C:\repos\Gaters\research\character-style-exploration\blender-face-proof\Derived\face-v2\face-proof.blend' `
  --python 'C:\repos\Gaters\research\character-style-exploration\blender-face-proof\sculpt_clay.py' -- `
  --candidate '<absolute-candidate-json>' --output '<absolute-candidate-output>'
```

Require different `face-sculpt.blend` hashes and different rendered nose silhouettes.
Then fresh-load both blends with `verify_sculpt.py` and require the existing projection,
editable-shape-key, armature, animation, and 768px checks to pass.

---

### Task 3: Define independent blind evaluator ballots

**Files:**

- Create: `research/character-style-exploration/blender-face-proof/evaluator-contract.json`
- Modify: `research/character-style-exploration/blender-face-proof/face_search.py`
- Modify: `research/character-style-exploration/blender-face-proof/test_face_search.py`

**Interfaces:**

- Consumes: target front image, champion three-view renders, two challenger three-view
  renders, and a randomized slot map not containing parameter values.
- Produces one ballot containing `evaluatorId`, `winnerSlot`, `confidence`, per-dimension
  judgments, `hardFailures`, `severeRegressions`, `diagnosis`, and `nextAxis`.

- [ ] **Step 1: Write RED for ballot validation and label resolution**

```python
def test_ballot_resolves_blind_slot_without_parameter_access(self):
    package = {"slots": {"left": "id-a", "center": "champion", "right": "id-b"}}
    ballot = {
        "schemaVersion": 1,
        "evaluatorId": "primary",
        "winnerSlot": "right",
        "confidence": 0.78,
        "dimensions": {
            "frontLikeness": "better",
            "threeDimensionalPlausibility": "same",
            "groundedMatureGeometry": "same",
            "individualCharacter": "better"
        },
        "hardFailures": [],
        "severeRegressions": [],
        "diagnosis": ["nose remains too narrow"],
        "nextAxis": "nose_projection"
    }
    resolved = validate_ballot(self.space, package, ballot)
    self.assertEqual(resolved["winnerId"], "id-b")
```

Run RED because `validate_ballot` does not exist.

- [ ] **Step 2: Add the evaluator contract**

The contract permits only these dimensions and values:

```json
{
  "schemaVersion": 1,
  "winnerSlots": ["left", "center", "right", "tie"],
  "dimensionValues": ["better", "same", "worse", "unknown"],
  "dimensions": [
    "frontLikeness",
    "threeDimensionalPlausibility",
    "groundedMatureGeometry",
    "individualCharacter"
  ],
  "hardFailures": [
    "invalid-blend",
    "projection-or-visible-image-texture",
    "missing-view",
    "implausible-anatomy",
    "cartoon-or-toy-read",
    "armature-or-animation",
    "unknown-provenance"
  ]
}
```

Reject ballots with unknown dimensions, axes, slots, hard failures, candidate IDs, or
confidence outside `[0, 1]`. Evaluators name slots only; only the root resolves IDs.

- [ ] **Step 3: Generate opposite-order ballot packages**

`face_search.py package` writes `primary-package.json` and `audit-package.json`. Both
reference identical images, but their left/right candidate order is reversed. Neither
package contains parameter values, directions, or the producing agent's diagnosis.

- [ ] **Step 4: Add the promotion rule**

A challenger is promotable only when:

- both ballots resolve to the same challenger ID;
- both ballots have no hard failure or severe regression;
- neither marks `groundedMatureGeometry` or `threeDimensionalPlausibility` as `worse`;
- both have confidence at or above `0.60`;
- the candidate passed `verify_sculpt.py` after a fresh reopen.

Tie, disagreement, insufficient confidence, or champion selection keeps the champion and
halves the searched axis step down to its configured minimum.

Run the unittest file and require all pure tests to pass.

---

### Task 4: Run the bounded multi-agent search

**Files:**

- Derived only:
  `research/character-style-exploration/blender-face-proof/Derived/search-runs/<timestamp>/`
- Authoritative process evidence:
  `research/character-style-exploration/blender-face-proof/Runs/<timestamp>/`

**Agent roles:**

- Root: owns state, selects the next axis, validates ballots, and promotes.
- Sculptor agent: receives candidate JSON paths and exact Blender commands; it may repair
  build failures but cannot alter candidate values or decide quality.
- Primary evaluator agent: receives only the primary blind package and image paths.
- Audit evaluator agent: receives only the reversed audit package and image paths.

- [ ] **Step 1: Initialize the immutable run**

Seed the champion with the `sculpt-v13` parameter vector and hashes of its `.blend`,
three renders, reference, search space, generator, and verifier. Preserve `sculpt-v13`;
never overwrite it. Write `run.json` before dispatching an agent.

- [ ] **Step 2: Obtain a blind baseline diagnosis**

Send the reference and champion views to both evaluators without the sculpt script or
parameter values. They each return ranked mismatches using only configured axes. The root
selects the highest shared axis; if they share none, stop with `no-actionable-axis`.
Record both verbatim instructions, responses, identities, and the root's selection reason.

- [ ] **Step 3: Dispatch the sculptor**

For the selected axis, create bounded `-delta` and `+delta` recipes. Send both exact
Blender commands to the sculptor agent. Require the sculptor to fresh-load verify every
candidate and return artifact paths plus command output. The root independently checks
the manifests and hashes before evaluation. Record the sculptor instruction, response,
command, tool versions, candidate recipes, outputs, and failures even when construction
does not succeed.

- [ ] **Step 4: Dispatch both evaluators in parallel**

Use this role instruction, changing only package path and evaluator ID:

```text
You are an independent face-sculpt evaluator. Do not edit files or infer parameter
values. Compare the approved front reference with the blinded champion and challenger
renders in the supplied package. Front likeness is reference-backed; three-quarter and
profile are anatomy regression checks only. Ignore lighting, skin, hair, eyebrows,
texture, and presentation quality: they cannot improve a candidate's rank. Judge skull
and facial silhouette, feature placement and projection, age structure, asymmetry, and
anatomical continuity. Return one evaluator-contract ballot. Reject projection tricks,
implausible anatomy, cartoon/toy geometry, missing evidence, and generic smoothing that
removes individual character. Name the most important remaining geometric error and one
configured next axis. Do not promote anything.
```

- [ ] **Step 5: Promote or narrow**

Validate both ballots. Promote only through Task 3's gate. Otherwise retain the champion,
halve the current step, and record the exact disagreement or regression. Select the next
axis from the shared evaluator diagnosis, excluding axes already at minimum step unless a
later promotion changed an interacting region. Finalize the round record and append its
canonical `project_learning_row` result; append-only output must include rejected
challengers and may not be rewritten after a later round.

- [ ] **Step 6: Repeat until a stop condition**

Stop at the first configured condition: maximum rounds, consecutive non-promotions,
consecutive evaluator disagreements, repeated hard Blender failure, or no actionable axis.
Write the stop condition and causal evidence to `run.json`. No evaluator result is called
human acceptance.

---

### Task 5: Falsify the evaluator and compare the final champion

**Files:**

- Modify: `research/character-style-exploration/blender-face-proof/test_face_search.py`
- Source: final `Runs/<run-id>/` records.
- Derived: final `Derived/search-runs/<run-id>/` artifacts.

**Interfaces:**

- Consumes: initial champion, final machine champion, and a deliberate counterexample.
- Produces: held-out evidence showing whether the evaluator can reject an obvious failure
  and prefer the final result without knowing chronology.

- [ ] **Step 1: Generate one held-out counterexample**

Create a candidate at the permitted extreme of `head_height` and `eye_spacing`. It must
remain mechanically valid but visibly violate grounded mature anatomy. Do not include it
in search training or previous ballots.

- [ ] **Step 2: Run two final blind audits**

Package initial champion, final champion, and counterexample in opposite orders. Require
both evaluators to reject the counterexample. Record whether they agree that the final
champion beats the initial champion, tie, or disagree.

- [ ] **Step 3: Verify replayability**

Rebuild the final champion from its candidate JSON in a fresh derived folder, reopen it,
rerender all three views, and compare semantic manifest fields and image dimensions.
Byte-identical Blender/PNG output is not required; recipe, topology, views, parameters,
and hard checks must match.

- [ ] **Step 4: Verify the generator-learning projection**

Rebuild `generator-learning.jsonl` from the immutable round records and compare its hash
with the stored projection. Assert one row per completed round, both challengers in every
row, retained failures and disagreements, exact parameter deltas, evaluator identities,
artifact hashes, decisions, and next-experiment diagnoses. Reject any row that labels a
machine promotion as human acceptance.

- [ ] **Step 5: Run fresh checks**

```powershell
python research/character-style-exploration/blender-face-proof/test_face_search.py
& research/character-style-exploration/blender-face-proof/Test-FaceProof.ps1 -PreflightOnly
& research/Test-MachineRegistry.ps1
git diff --check
```

Expected: all commands exit `0`. The face search remains unaccepted until the human sees
the initial/final blind comparison.

---

### Task 6: Record machine truth and Art Direction status

**Files:**

- Modify: `research/machines.json`
- Modify: `.agents/workstreams/Art Direction.md`
- Modify: `.agents/workflow-feedback.md` only if the evaluator or loop fails its contract.

**Interfaces:**

- Consumes: immutable run evidence and held-out audit.
- Produces: current machine status without duplicating experiment details; the registry
  points to the run identity while `Runs/<run-id>/` owns the replayable process evidence.

- [ ] **Step 1: Add `evaluation.face-sculpt` as active**

Contract:

- Outcome: blind paired evaluation records front-reference likeness separately from
  three-dimensional plausibility, grounded mature style, and individual character.
- Requires: `content.style-contract`, `tool.blender-operator`, `research.run-archive`.
- Verifier: opposite-order evaluator ballots plus a held-out mechanically valid anatomy
  counterexample.
- Challenge set: front-only reference ambiguity, label-order bias, projection cheats,
  generic smoothing, exaggerated age, cartoon proportions, and plausible ties.
- Failure artifact: packages, renders, ballots, disagreement, evaluator versions, and
  implicated candidate IDs.
- Promotion gate: both blind evaluators reject held-out failures and agree on hidden
  champion comparisons without claiming unsupported profile likeness.
- Work deleted: unaided subjective claims that a face became closer.

- [ ] **Step 2: Add `research.face-sculpt-search` as active**

Contract:

- Outcome: named Blender facial axes iteratively produce, evaluate, narrow, and preserve
  champion/challenger face sculpts.
- Requires: `evaluation.face-sculpt`, `tool.blender-operator`,
  `research.promotion-gate`, `research.run-archive`.
- Verifier: replay the bounded run from initial champion through every proposal, ballot,
  step update, and promotion.
- Challenge set: bounds, ties, evaluator disagreement, coupled controls, minimum steps,
  invalid Blender output, interrupted runs, and local optima.
- Failure artifact: complete run state, candidate recipes, Blender logs, manifests,
  renders, ballots, and promotion decisions.
- Promotion gate: the final candidate is reproducible, passes hard checks, beats the
  initial champion in opposite-order held-out audits, and receives later human approval.
- Work deleted: manual parameter guessing, unrecorded sculpt iteration, and reconstructing
  generator training evidence after the experiment.

- [ ] **Step 3: Update the workstream with only current status**

Record the run path, initial/final candidate IDs, stop reason, held-out audit outcome, and
largest unresolved visible mismatch. Do not duplicate the machine graph or call the result
production-ready.

- [ ] **Step 4: Validate registry and shared files**

Run `research/Test-MachineRegistry.ps1`, `research/Test-SharedAgentDocs.ps1`, and
`git diff --check`. Preserve the dirty worktree and do not commit.

Requirements checked: `ART-1`, `ART-2`, `ART-3`, `ART-4`; exceptions: none. `CHAR-1`,
`CHAR-2`, and `CHAR-3` are excluded by `ART-3`.

---

### Task 7: Replace subtle visual voting with one deterministic macro grid

**Why:** The first run falsified its visual ranking guarantee: reversed packages exposed
screen-position sensitivity before any promotion. Both evaluators nevertheless diagnosed
the same large mismatch—tall/narrow facial mass and an over-tapered lower face. Test the
three existing controls that directly address that mismatch at visibly separated values
before adding more sculpt axes.

**Files:**

- Create: `research/character-style-exploration/blender-face-proof/target-front-geometry.json`
- Create: `research/character-style-exploration/blender-face-proof/macro_grid.py`
- Create: `research/character-style-exploration/blender-face-proof/measure_macro_geometry.py`
- Modify: `research/character-style-exploration/blender-face-proof/test_face_search.py`
- Create during execution: `Runs/<macro-run-id>/` and
  `Derived/search-runs/<macro-run-id>/`.

**Interfaces:**

- Consumes the retained candidate and the fixed current sculpt generator without changing
  either one.
- Produces all eight combinations of:
  - `head_width`: `-0.08`, `+0.10`
  - `head_height`: `0.00`, `-0.20`
  - `jaw_taper`: `0.085`, `0.00`
- Measures stable-topology front geometry from the Blender source, normalized by eye
  separation, against one immutable manual annotation of the approved reference.
- Ranks only front-supported cheek width, jaw width, mouth width, eye-to-chin height, and
  lower-face taper. Three-quarter/profile remain hard plausibility gates, never invented
  likeness evidence.

- [ ] **Step 1: Write RED for grid identity and scoring**

Require exactly eight unique, bounded candidate recipes; order-independent score output;
rejection of changed target hash, camera, topology, missing anchors, or non-finite values;
and a declared minimum improvement margin over the retained candidate.

- [ ] **Step 2: Add the immutable target annotation**

Record image hash, image size, the manually selected eye, cheek, jaw, mouth, and chin
front landmarks, their annotation method, and derived normalized ratios. Exclude hair,
skin, brows, materials, lighting, background, and the hair-covered skull top.

- [ ] **Step 3: Export candidate geometry metrics**

From each freshly reopened candidate blend, project stable MPFB surface regions through
the fixed orthographic front camera. Write the exact anchor vertex sets, camera hash,
topology count, raw coordinates, normalized ratios, and score inputs. Do not derive the
candidate score from raster color, edges, lighting, or texture.

- [ ] **Step 4: Build and verify the full grid**

Use the integrity-pinned `Derived/face-v2/face-proof.blend`, Blender
`--python-exit-code 1`, the current candidate-driven sculpt script, and root-owned fresh
verification. Preserve every recipe, build/verify log, manifest, render, metric record,
tool hash, and failure under the new run identity.

- [ ] **Step 5: Select or falsify**

Nominate the lowest deterministic score only if it beats the retained candidate by the
declared margin and passes all 3D hard checks. Otherwise retain the candidate. If the best
grid point lands on a bound while a large residual remains, record that as evidence for
missing independent jaw/chin, upper-face, eye-aperture, and nose-structure controls—not
as evaluator uncertainty. No nomination is human acceptance.

- [ ] **Step 6: Independently review and update current truth**

An agent not involved in generation checks annotation provenance, grid completeness,
metric reproducibility, selection arithmetic, and all hard-check receipts. Update the
Art Direction workstream and machine failure evidence with the result; do not mark either
machine verified or add a champion without human acceptance.

Requirements checked: `ART-1`, `ART-2`, `ART-3`, `ART-4`; exceptions: none. `CHAR-1`,
`CHAR-2`, and `CHAR-3` remain excluded by `ART-3`.

### Task 8: Calibrate eye aperture and front-supported nose structure

**Why:** Task 7 removed unsupported lower-face taper without a 3D regression. The largest
remaining feature mismatch is the pinched eye aperture, followed by the nose's short,
generic front read. A single front reference does not support searches over projection,
lip depth, eye-bag depth, profile shape, or lighting-dependent surface detail.

**Files:**

- Create: `research/character-style-exploration/blender-face-proof/target-feature-geometry.json`
- Create: `research/character-style-exploration/blender-face-proof/feature_grid.py`
- Create: `research/character-style-exploration/blender-face-proof/sculpt_features.py`
- Create: `research/character-style-exploration/blender-face-proof/measure_feature_geometry.py`
- Modify: `research/character-style-exploration/blender-face-proof/sculpt_integrity.py`
- Modify: `research/character-style-exploration/blender-face-proof/test_face_search.py`
- Create during execution: `Runs/<feature-run-id>/` and
  `Derived/search-runs/<feature-run-id>/`.

**Grid:**

- Parent: Task 7 nominee
  `eaec583dac7c9adf295510e82a06dd721dceb2aea371ced31bfdcaa88b884d5b`.
- `eye_aperture`: `0.00`, `0.12` through paired
  `l-eye-height1-incr` / `r-eye-height1-incr` targets.
- `nose_length`: `0.30`, `0.37` through the existing `nose.length` control.
- `nose_width`: `0.26`, `0.33` through the existing coupled bridge/tip/nostril mapping.
- All other parent parameters remain byte-for-byte unchanged. The all-low cell is a
  required semantic replay control.

- [ ] **Step 1: RED for complete recipes, semantic anchors, and replay**

Require exactly eight unique recipes whose identity includes the complete feature vector;
reject undeclared control differences. Add pure directional fixtures for paired upper/
lower lid anchors, nose root/base anchors, signed alare anchors, fixed scene state, finite
ratios, and replay equivalence to the Task 7 parent.

- [ ] **Step 2: Freeze the feature target annotation**

Against the approved target hash, annotate exposed upper/lower lid margins for both eyes,
nasion, subnasale, and left/right alare. Exclude lashes, eyebrows, eye bags, shadows,
texture, and nostril darkness. Score only mean eye-aperture height, nose root-to-base
height, and alar width, each normalized by the existing eye-center distance. Record manual
uncertainty; if the annotation cannot support the fixed margin, stop rather than invent
precision.

- [ ] **Step 3: Rebuild every candidate from the pinned source**

`sculpt_features.py` rebuilds from `Derived/face-v2/face-proof.blend`; it applies the
complete Task 7 parent vector plus exactly the three declared feature values before save
and render. Candidate recipes retain the existing complete parameter vector and add one
hashed `featureControls` record. Extend integrity checks only enough to bind that record,
the feature script, source blend, effective controls, scene lock, and output identity;
all prior sculpt receipts must still validate.

- [ ] **Step 4: Export semantically equivalent feature metrics**

Derive lid anchors from fixed signed vertical extrema in the paired eye-height targets;
nose root/base from fixed vertical role vertices; alare from fixed signed horizontal nose
targets. Store target-file hashes, topology indices, world/camera coordinates, scene-lock
hash, and raw/normalized ratios. The all-low cell must match the Task 7 parent topology,
camera, macro ratios, and new anchors within declared float tolerance.

- [ ] **Step 5: Nominate or retain**

The lowest feature-only geometry score must beat the all-low replay by `0.02`, retain the
Task 7 macro metrics within tolerance, and pass fresh hard checks. Three-quarter/profile
may reject broken lid/nose continuity, self-intersection, cartoon forms, or increased
forward projection; they cannot reward likeness. No result is human acceptance.

- [ ] **Step 6: Independent review and current-truth update**

Independently reproduce recipe identities, hashes, metric roles, replay equivalence,
score arithmetic, and three-view hard gates. Preserve all failures. Update current
Art Direction and active machine evidence only after approval; do not add a registry
champion or verified status.

Requirements checked: `ART-1`, `ART-2`, `ART-3`, `ART-4`; exceptions: none. `CHAR-1`,
`CHAR-2`, and `CHAR-3` remain excluded by `ART-3`.

---

### Task 9: Convert measured feature response into one bounded calibration set

**Why:** Task 8 established monotonic, isolated responses. Its best cell improved only
eye aperture and did not clear either promotion margin. The response predicts that the
target lies outside the tested eye range, below the tested nose-length range, and almost
exactly at the low nose-width value. Test that prediction once; do not resume subjective
visual voting or expand into an unbounded Cartesian search.

**Files:**

- Create: `research/character-style-exploration/blender-face-proof/feature_calibration.py`
- Create: `research/character-style-exploration/blender-face-proof/sculpt_calibrated.py`
- Modify: `research/character-style-exploration/blender-face-proof/sculpt_integrity.py`
- Modify: `research/character-style-exploration/blender-face-proof/test_face_search.py`
- Create during execution: `Runs/<calibration-run-id>/` and
  `Derived/search-runs/<calibration-run-id>/`.

**Calibration:**

- Parent geometry remains the Task 7 nominee.
- Task 8 low/high receipts, not rendered appearance, derive the predicted controls.
- Test exactly four combinations: `eye_aperture` in `{0.42, 0.52}`,
  `nose_length` in `{0.08, 0.14}`, and fixed `nose_width = 0.27`.
- All other controls remain byte-for-byte identical to the parent.

- [ ] **Step 1: RED for measured derivation and invalid-run exclusion**

Require calibration recipes to bind the source Task 8 receipt hashes, response slope,
predicted crossing, bracket values, complete control vector, and parent. Mechanically
reject every run named by an incident's `invalidatedRunIds` and any run whose own incident
marks it `invalid-for-promotion`.

- [ ] **Step 2: Build each candidate independently from the pinned source**

Use a new versioned sculpt script so Task 8 manifest script hashes remain valid. Preserve
one build log, fresh reopen log, feature receipt, macro receipt, manifest, blend, and three
views per recipe. The static scene lock must remain identical to Task 8.

- [ ] **Step 3: Select only a calibration triage result**

Rank the four candidates by the same feature metric and reject any macro or three-view
regression. Because Task 8's entire baseline error is smaller than the annotation
uncertainty margin, the result may be labeled `calibration-best-for-human-review`, never
machine-promoted, human-accepted, or proven likeness.

- [ ] **Step 4: Independently review the arithmetic and views**

An agent not involved in generation reproduces the slopes, brackets, hashes, scores,
replay constraints, and hard gates. If the response is non-monotonic or the best lies on
a tested bound, stop and record the missing control/span instead of extrapolating again.

Requirements checked: `ART-1`, `ART-2`, `ART-3`, `ART-4`; exceptions: none. `CHAR-1`,
`CHAR-2`, and `CHAR-3` remain excluded by `ART-3`.

---

### Task 10: Correct visible eye-corner, mouth, and nose-tip shape once

**Why:** Two independent original-resolution annotations show that the calibrated eye
span is close but its outer corners remain too upswept, while the visible mouth
commissures do not match the old MPFB mouth-width anchor. The earlier macro receipt used
maximum target displacement vertices outside the visible commissures. The current mouth
is narrower with a thinner upper lip than the approved front; the nose's lowest central
boundary is slightly high. Fix the semantic anchor before changing these shapes.

**Files:**

- Create: `research/character-style-exploration/blender-face-proof/target-detail-geometry.json`
- Create: `research/character-style-exploration/blender-face-proof/detail_grid.py`
- Create: `research/character-style-exploration/blender-face-proof/sculpt_details.py`
- Create: `research/character-style-exploration/blender-face-proof/measure_detail_geometry.py`
- Modify: `research/character-style-exploration/blender-face-proof/sculpt_integrity.py`
- Modify: `research/character-style-exploration/blender-face-proof/test_face_search.py`
- Create during execution: `Runs/<detail-run-id>/` and
  `Derived/search-runs/<detail-run-id>/`.

**Grid:**

- Parent feature controls: Task 9 review candidate `9032fd4d...`.
- Fixed paired outer-eye-corner down control: `0.08`.
- Existing absolute `mouth.width`: `{0.24, 0.36}`.
- Paired upper-lip-height increase: `{0.08, 0.18}`.
- Existing absolute `nose.pointDown`: `{0.15, 0.25}`.
- All other Task 7 and Task 9 controls remain exact.

- [x] **Step 1: Freeze equivalent visible landmarks**

Store both blind annotations verbatim, their consensus and uncertainty, and stable body
vertex indices mapped once from the Task 9 front to the visible eye corners, mouth
commissures, center upper/seam/lower lip, and nose lowest center. Mapping excludes back
surface and helper geometry. Reject the previous mouth target-extrema metric for visible
commissure scoring; preserve its incident.

- [x] **Step 2: RED for eight recipes and detail receipts**

Require exactly eight complete recipes whose identities bind the Task 9 parent, detail
controls, target annotation, mapped vertices, and all source metric hashes. Detail
receipts must reopen the blend, reproduce camera/topology/scene locks, and project only
the frozen equivalent vertices.

- [x] **Step 3: Build, verify, and measure independently**

Build each cell from the original pinned source with a new hashed sculpt script. Preserve
build, fresh reopen, detail, feature, and macro receipts plus all three views. Macro gates
use cheek, jaw, eye-to-chin, and taper only; the known-invalid old mouth anchor cannot
select or reject a candidate.

- [x] **Step 4: Stop after independent review**

Rank only front-supported mouth width, upper/lower lip height, nose-tip height, and eye
corner tilt within annotation uncertainty. Three-quarter/profile remain rejection-only.
Record the best as review-only, never human acceptance. Stop after this pass regardless
of endpoint placement; any remaining likeness gap needs new human feedback or new
independently annotated feature semantics.

Requirements checked: `ART-1`, `ART-2`, `ART-3`, `ART-4`; exceptions: none. `CHAR-1`,
`CHAR-2`, and `CHAR-3` remain excluded by `ART-3`.
