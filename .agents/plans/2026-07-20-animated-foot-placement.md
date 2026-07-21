# Animated Foot Placement Implementation Plan

> **For agentic workers:** execute inline with test-driven development. Do not branch,
> commit, or push; the human reviews the dirty worktree.

**Goal:** Prove the generated walk clip advances and changes the evaluated pose through
the native uneven-terrain Foot Placement graph without regressing terrain contact.

**Architecture:** Keep the current runtime-only plugin and route. Replace only the graph
source node with native `FAnimNode_SequencePlayer_Standalone`, configure it with the
already imported `A_Walk`, and emit a separate challenger report evaluated by an
independent Python policy.

**Tech stack:** UE 5.8 C++, native Animation Runtime/AnimationWarping nodes, Python
`unittest`, PowerShell harness, Unreal Automation through Unreal Runner.

## Global constraints

- Blender owns offline generation; Unreal owns live playback, movement, floor contact,
  IK, collision, and authority.
- Keep the current reference-pose Foot Placement and flat CharacterMovement harnesses as
  regressions.
- No editor module dependency, shared Unreal source edit, art work, registry edit,
  branch, commit, or push.

Requirements checked: `CHAR-1`, `CHAR-2`, `CHAR-3`, `ART-1`, `ART-2`, `ART-3`, `ART-4`,
generated-content boundary; exceptions: none.

---

### Task 1: Independent animated-contact policy

**Files:**
- Create: `research/embodied-species-lab/animated_foot_placement_policy.py`
- Create: `research/embodied-species-lab/tests/test_animated_foot_placement_policy.py`
- Modify: `research/embodied-species-lab/species/foot-placement-route.json`

**Interface:** `evaluate(report: dict, contract: dict) -> dict` returns deterministic
`passed`, metrics, and causal issues. The report adds `sourceAnimation` and
`animationSamples` while reusing the existing case/contact shape.

- [ ] Write fixtures and mutations for wrong native source, wrong generated clip,
  non-advancing asset time, static thigh pose, contact/floor regression, and invalid
  finish.
- [ ] Run the focused test and verify RED because the evaluator is missing.
- [ ] Implement the smallest evaluator and contract thresholds.
- [ ] Run the focused and complete Python suites and verify GREEN.

### Task 2: Native automation RED

**Files:**
- Modify: `research/embodied-species-lab/unreal/CharacterMovementChallenger/Source/CharacterMovementChallenger/Private/Tests/CharacterFootPlacementUnevenTests.cpp`
- Create: `research/embodied-species-lab/Test-AnimatedFootPlacementMachine.ps1`

**Interface:** automation name
`Gaters.CharacterLab.CharacterMovement.AnimatedUnevenFootPlacement`; environment inputs
reuse `GATERS_FOOT_PLACEMENT_CONTRACT` and add a separate evidence path.

- [ ] Add the failing native test first. Require generated `A_Walk`, native Sequence
  Player identity, advancing asset time, evaluated thigh delta, existing terrain-contact
  bounds, CharacterMovement route movement, and stable evidence output.
- [ ] Send the public harness to Unreal Runner and verify RED specifically because the
  current graph still uses `FAnimNode_RefPose`.

### Task 3: Native Sequence Player GREEN

**Files:**
- Modify: `research/embodied-species-lab/unreal/CharacterMovementChallenger/Source/CharacterMovementChallenger/Public/CharacterFootPlacementAnimInstance.h`
- Modify: `research/embodied-species-lab/unreal/CharacterMovementChallenger/Source/CharacterMovementChallenger/Private/CharacterFootPlacementAnimInstance.cpp`

**Interface:** `SetSourceSequence(UAnimSequenceBase*)`, `GetSourceSequence()`, and
`GetSourceAssetTimeSeconds()` on the isolated anim instance. Graph order becomes
`SequencePlayer -> LocalToComponent -> FootPlacement -> TwoBoneIK left/right -> Local`.

- [ ] Replace only `FAnimNode_RefPose` with
  `FAnimNode_SequencePlayer_Standalone`; loop at native play rate.
- [ ] Configure the generated walk clip in the native test before fixed-step evaluation.
- [ ] Send the harness to Unreal Runner and iterate only from causal compiler/runtime
  failures until native and independent policy runs pass twice byte-identically.

### Task 4: Regression and closeout

**Files:**
- Modify: `research/embodied-species-lab/README.md`
- Modify: `.agents/workstreams/Character Generation & Animation.md`
- Create after green: `.agents/exchanges/CHAR-6-animated-foot-placement-handoff.md`

- [ ] Run 54+ Python tests, Blender locomotion, animated challenger twice, reference-pose
  Foot Placement, flat CharacterMovement, shared-agent-doc checks, and scoped diff checks.
- [ ] Record exact hashes and measurements without changing `research/machines.json`.
- [ ] Hand narrow evidence to Primary Builder; do not request shared integration or
  promotion unless the challenger and all regressions pass.
