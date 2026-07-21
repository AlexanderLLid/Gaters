# CharacterMovement Flat Route Implementation Plan

> **For Codex:** Execute this plan inline with the `executing-plans` skill. Do not branch,
> commit, push, edit shared Unreal source, alter `Prototype.uproject`, or promote a machine.

**Goal:** Prove that UE 5.8 native `ACharacter` and `CharacterMovement` drive the generated
humanoid and its eight cooked mechanical clips through the deterministic flat route
`idle -> walk -> run -> turn -> stop -> jump -> fall -> land`.

**Architecture:** Add one external runtime-only plugin under the embodied-species lab.
The plugin loads the existing cooked CharacterLab assets, runs the route in the existing
derived `L_CharacterLab` map, records native movement state, and never loads the
editor-only intake adapter. A small versioned JSON contract owns route tunables; a Python
policy independently evaluates the native JSON report. PowerShell packages the plugin,
runs the test twice, evaluates both reports, and requires byte-identical stable evidence.

**Native machinery:** UE 5.8 `ACharacter`, `UCharacterMovementComponent`, capsule
collision/floor finding, `UAnimSequence`, Automation Framework, JSON. No Anim Blueprint,
custom movement component, Mover, Motion Warping, Foot Placement, or art work.

**Requirements checked:** `CHAR-1`, `CHAR-2`, `CHAR-3`, `ART-1`, `ART-2`, generated-content
boundary. Exceptions: none.

---

## Task 1: Define the independent route contract and policy

**Files:**

- Create: `research/embodied-species-lab/species/character-movement-route.json`
- Create: `research/embodied-species-lab/character_movement_policy.py`
- Create: `research/embodied-species-lab/tests/test_character_movement_policy.py`

1. Write failing policy tests for:
   - exact ordered phases;
   - generated asset references and all eight clip references;
   - capsule ownership and `CharacterMovement` component class;
   - grounded walk/run/turn/stop observations;
   - falling followed by landing;
   - forward and lateral displacement minimums;
   - final grounded state;
   - editor adapter reported as not loaded;
   - stable causal rule IDs for malformed reports.
2. Run the focused tests and confirm RED because the evaluator is absent.
3. Implement the smallest JSON evaluator and CLI.
4. Run the focused tests and confirm GREEN.

## Task 2: Scaffold the isolated runtime plugin and prove its boundary

**Files:**

- Create: `research/embodied-species-lab/unreal/CharacterMovementChallenger/CharacterMovementChallenger.uplugin`
- Create: `research/embodied-species-lab/unreal/CharacterMovementChallenger/Config/FilterPlugin.ini`
- Create: `research/embodied-species-lab/unreal/CharacterMovementChallenger/Source/CharacterMovementChallenger/CharacterMovementChallenger.Build.cs`
- Create: `research/embodied-species-lab/unreal/CharacterMovementChallenger/Source/CharacterMovementChallenger/Private/CharacterMovementChallengerModule.cpp`
- Create: `research/embodied-species-lab/Build-CharacterMovementChallenger.ps1`

1. Define exactly one `Runtime` module with only `Core`, `CoreUObject`, `Engine`, and
   `Json` dependencies.
2. Add a build harness that refuses to run while interactive Unreal is open, cleans only
   the lab-owned package directory, packages with UE 5.8, and exposes the packaged plugin
   path.
3. Package once and inspect the descriptor/binaries to confirm no `UnrealEd`,
   `PhysicsUtilities`, or `CharacterPhysicsProfileAdapter` dependency.

## Task 3: Add the failing native flat-route automation test

**Files:**

- Create: `research/embodied-species-lab/unreal/CharacterMovementChallenger/Source/CharacterMovementChallenger/Private/Tests/CharacterMovementFlatRouteTests.cpp`

1. Add a native product-filter automation test named
   `Gaters.CharacterLab.CharacterMovement.FlatRoute`.
2. Load the fixed CharacterLab map and cooked mesh, skeleton, Physics Asset, IK Rig, and
   eight animation paths.
3. Spawn a stock `ACharacter`, attach the generated skeletal mesh and Physics Asset, and
   verify the editor adapter module is absent.
4. Initially record the undriven character and assert the route contract; run the test
   and confirm a causal RED result for missing movement phases.

## Task 4: Drive the route with native CharacterMovement

**Files:**

- Modify: `research/embodied-species-lab/unreal/CharacterMovementChallenger/Source/CharacterMovementChallenger/Private/Tests/CharacterMovementFlatRouteTests.cpp`

1. Configure the existing capsule and `UCharacterMovementComponent`; enable physics with
   no controller for the headless pawn.
2. At a fixed delta, drive:
   - idle with zero input;
   - walk forward with `AddMovementInput` and walk speed;
   - run forward with a higher native max speed;
   - turn left by changing the movement direction while native movement owns translation;
   - stop by removing input and allowing native braking;
   - jump through the character jump path;
   - fall under CharacterMovement gravity;
   - land when native floor state returns to walking.
3. Select the matching imported in-place `UAnimSequence` in single-node mode at each
   phase. Do not create an Anim Blueprint or use root motion.
4. Sample fixed-time position, velocity, acceleration, facing, movement mode, floor hit,
   active phase, and active clip. Round only report serialization, not simulation state.
5. Emit versioned stable JSON to the path in `GATERS_CHARACTER_MOVEMENT_EVIDENCE`.
6. Run the native test and confirm GREEN.

## Task 5: Add repeatability and the public challenger command

**Files:**

- Create: `research/embodied-species-lab/Test-CharacterMovementMachine.ps1`
- Modify: `research/embodied-species-lab/README.md`

1. Have the public command require the passing existing CharacterLab assets rather than
   rerunning Blender/import implicitly.
2. Package the runtime plugin, launch `UnrealEditor-Cmd` on `L_CharacterLab`, and run the
   native flat-route test twice with separate evidence/log paths.
3. Run the independent policy for each native report.
4. Require byte-identical native reports and policy reports.
5. Scan logs for build, load, automation, collision, and asset errors. Preserve all
   evidence on failure.
6. Document the command, proven boundary, and explicit deferral of uneven terrain.

## Task 6: Verify the complete isolated wave and hand it off

**Files:**

- Modify: `.agents/workstreams/Character Generation & Animation.md`
- Modify: `.agents/exchanges/CHAR-3-fullbodyik-motion-handoff.md`

1. Run focused Python policy tests.
2. Run the public CharacterMovement challenger twice.
3. Re-run the existing intake policy tests and native intake challenger as regression
   coverage if the interactive editor remains closed.
4. Run `research/Test-SharedAgentDocs.ps1` because the workstream/exchange files changed.
5. Record measured evidence and the next frontier: uneven-terrain Foot Placement.
6. Add the flat-route evidence to requester-owned CHAR-3 material without claiming
   integration or editing `research/machines.json`.

