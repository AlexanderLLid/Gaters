# Uneven-Terrain Foot Placement Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:executing-plans` to
> implement this plan inline. Do not branch, commit, push, edit shared Unreal source,
> alter `Prototype.uproject`, inspect Unreal processes, or promote a machine.

**Goal:** Prove native UE 5.8 Foot Placement and Two Bone IK can ground the generated
humanoid on deterministic uneven terrain while CharacterMovement remains authoritative.

**Architecture:** Extend the isolated generated skeleton with four mechanical target
roles, then reuse the existing editor intake and runtime-only CharacterMovement plugin.
A custom native test AnimInstance composes reference/source pose, Foot Placement, and
bilateral Two Bone IK. An independent Python policy judges native world-space contact
evidence from a versioned terrain contract.

**Tech Stack:** Blender Python, FBX, UE 5.8 `AnimationWarpingRuntime`,
`AnimGraphRuntime`, `ACharacter`, Automation Framework, Python `unittest`, PowerShell.

## Global Constraints

- Character pipeline only; no art direction, appearance, proportions, materials, or clothing.
- Blender owns offline bones and source export. Unreal owns live floor queries, IK, and contact.
- Keep all implementation under `research/embodied-species-lab/`.
- The Foot Placement node is an experimental challenger behind a replaceable adapter.
- All Unreal launches and builds go through Unreal Runner task
  `019f7c9d-e7c3-7970-844e-ef2640d5a411`.
- Requirements checked: `CHAR-1`, `CHAR-2`, `CHAR-3`, `ART-1`, `ART-2`, generated-content
  boundary. Exceptions: `ART-1` and `ART-2` are intentionally outside this mechanical proof.

---

### Task 1: Add generated mechanical foot roles

**Files:**

- Create: `research/embodied-species-lab/tests/test_humanoid_skeleton.py`
- Create: `research/embodied-species-lab/humanoid_skeleton.py`
- Modify: `research/embodied-species-lab/generate_humanoid.py`

**Interfaces:**

- Produces: bone specs for `ball_l`, `ball_r`, `ik_foot_l`, and `ik_foot_r`; each spec
  declares whether it deforms and the Blender exporter preserves non-deforming roles.

- [ ] Write tests requiring the four exact roles, parents, deform flags, and unique names.
- [ ] Run the focused tests and confirm RED because the roles are absent.
- [ ] Add the minimum bone specs, apply `use_deform` from each spec, and export all bones.
- [ ] Run all local Python tests and confirm GREEN.

### Task 2: Prove generated roles survive Unreal intake

**Files:**

- Modify: `research/embodied-species-lab/tests/test_unreal_intake_policy.py`
- Modify: `research/embodied-species-lab/unreal_intake_policy.py`
- Modify: `research/embodied-species-lab/unreal/import_generated_humanoid.py`

**Interfaces:**

- Consumes: regenerated manifest bone hierarchy.
- Produces: native report fields identifying all Foot Placement roles and a causal
  `intake.foot-placement-roles` rejection when any role is missing.

- [ ] Add a failing policy test that removes one target role from native readback.
- [ ] Run it and confirm RED because the policy accepts the malformed report.
- [ ] Add the role readback and minimal independent policy rule.
- [ ] Regenerate and run the complete intake harness through Unreal Runner.
- [ ] Confirm imported Skeleton, Physics Asset, IK Rig, and eight clips still pass.

### Task 3: Define uneven-contact evidence independently

**Files:**

- Create: `research/embodied-species-lab/species/foot-placement-route.json`
- Create: `research/embodied-species-lab/foot_placement_policy.py`
- Create: `research/embodied-species-lab/tests/test_foot_placement_policy.py`

**Interfaces:**

- Produces: `evaluate(report, contract) -> {passed, issues, metrics}` and stable causal
  rules for native-node identity, role mapping, walkable contacts, bilateral gap and
  penetration, pelvis bounds, route order, no NaNs, and unloaded editor adapter.

- [ ] Write failing tests for a passing report and one mutation per causal rule.
- [ ] Run focused tests and confirm RED because the evaluator is absent.
- [ ] Implement the smallest evaluator and CLI.
- [ ] Run focused and full Python tests and confirm GREEN.

### Task 4: Add the static native Foot Placement falsifier

**Files:**

- Modify: `research/embodied-species-lab/unreal/CharacterMovementChallenger/Source/CharacterMovementChallenger/CharacterMovementChallenger.Build.cs`
- Create: `research/embodied-species-lab/unreal/CharacterMovementChallenger/Source/CharacterMovementChallenger/Public/CharacterFootPlacementAnimInstance.h`
- Create: `research/embodied-species-lab/unreal/CharacterMovementChallenger/Source/CharacterMovementChallenger/Private/CharacterFootPlacementAnimInstance.cpp`
- Create: `research/embodied-species-lab/unreal/CharacterMovementChallenger/Source/CharacterMovementChallenger/Private/Tests/CharacterFootPlacementUnevenTests.cpp`

**Interfaces:**

- Consumes: source pose and exact generated bone roles.
- Produces: runtime-only graph `pose -> FootPlacement -> TwoBoneIK left -> TwoBoneIK right`
  and test `Gaters.CharacterLab.CharacterMovement.UnevenFootPlacement`.

- [ ] Add a native test that creates split-height support and initially asserts missing
  evaluated contact evidence; run through Unreal Runner and confirm causal RED.
- [ ] Add only `AnimGraphRuntime` and `AnimationWarpingRuntime` runtime dependencies.
- [ ] Compose native graph nodes in a custom AnimInstance proxy; configure Graph speed
  mode, root/pelvis roles, bilateral leg definitions, and no stretching.
- [ ] Measure input IK targets, output deform feet, independent support traces, pelvis
  offset, native node/module identities, and NaNs after settling.
- [ ] Run through Unreal Runner and require the static split-height case to pass.

### Task 5: Traverse the deterministic uneven route

**Files:**

- Modify: `research/embodied-species-lab/unreal/CharacterMovementChallenger/Source/CharacterMovementChallenger/Private/Tests/CharacterFootPlacementUnevenTests.cpp`
- Create: `research/embodied-species-lab/Test-FootPlacementMachine.ps1`

**Interfaces:**

- Consumes: the static passing graph and terrain contract.
- Produces: two native reports and two independent policy reports for split stance,
  step traversal, slope traversal, and stopped grounded finish.

- [ ] Extend the native test with a short fixed-delta CharacterMovement route over lab-owned
  runtime collision components; confirm RED while traversal samples are absent.
- [ ] Drive the existing generated walk/stop clips without root motion.
- [ ] Record deterministic samples and stable JSON to `GATERS_FOOT_PLACEMENT_EVIDENCE`.
- [ ] Add a public harness that packages once, runs twice through Unreal Runner, invokes
  the independent policy, and requires byte-identical reports.
- [ ] Run the flat route afterward and require no regression.

### Task 6: Close evidence and handoff

**Files:**

- Modify: `research/embodied-species-lab/README.md`
- Modify: `.agents/workstreams/Character Generation & Animation.md`
- Create: `.agents/exchanges/CHAR-5-native-foot-placement-handoff.md`

**Interfaces:**

- Produces: exact commands, measured results, explicit experimental-node limitation,
  and the next isolated recovery experiment without registry promotion.

- [ ] Run all local Python tests.
- [ ] Run fresh intake, uneven-contact, and flat-route public harnesses through Unreal Runner.
- [ ] Run `research/Test-SharedAgentDocs.ps1`.
- [ ] Record hashes and measurements; preserve failures and do not replace the flat champion.
- [ ] Send CHAR-5 to Primary Builder for review without requesting shared integration.
