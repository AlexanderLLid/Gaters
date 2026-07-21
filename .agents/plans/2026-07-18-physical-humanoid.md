# Physical Humanoid Challenger Plan

**Goal:** Turn the isolated generated humanoid into a constrained physical body, strike
it in Blender, and bake the resulting reaction onto its generated skeleton.

**Machine:** A pure-Python body profile owns masses and joint limits. Blender is only
the solver adapter: rigid proxies, spring joints, collision, simulation, and animation
bake are reproducible derived output.

**Borrowed systems:** Blender 5.2 rigid bodies and generic spring constraints. Rigify is
available but deferred because control-rig generation does not remove the current
physics or bake work.

## Constraints

- Work only in `research/embodied-species-lab/` plus this execution snapshot.
- Preserve `Test-HumanoidMachine.ps1` as the known-good champion.
- Do not touch Unreal, `research/machines.json`, branches, commits, or remotes.
- Placeholder geometry is enough; evidence is mechanical, not artistic.

## Tasks

- [x] Add failing tests for deterministic mass distribution and joint safety limits.
- [x] Generate a physical profile covering every simulated body segment.
- [x] Build rigid proxies and constrained joints from that profile.
- [x] Generate a physical impact from the event definition and simulate it.
- [x] Bake the resulting transforms to an armature action.
- [x] Reopen the Blend file and validate masses, constraints, motion, and export.
- [x] Run the challenger twice and compare semantic manifests.
