# Unreal Prototype Guide

Entry point for running and inspecting the prototype. Current capability status lives in
`research/machines.json`; source, automation output, and archived runs are the evidence.
This page intentionally carries no implementation history.

## Open

- Project: `Unreal/Prototype/Prototype.uproject` using Unreal 5.8.
- Test map: `/Game/Gaters/Maps/Lvl_GateGreybox`.
- Generated motion imports: `/Game/Gaters/Generated/Motion`.

## Test seeds

- In PIE, open the Unreal console and run `Gaters.Seed 53` to reload the current map with
  another seed.
- Headless runs may pass `-GatersSeed=N`.
- `Unreal/Prototype/Scripts/RunEnvironmentSweep.ps1 -CaptureGallery` runs a seed set and
  writes comparable captures under the project `Saved` directory.

## Architecture entry points

- `FGatersWorldRecipe` owns pure generated identity.
- `FGatersEnvironment` and terrain semantic/navigation types own ground facts.
- `FGatersWorldCompiler` and the content catalog translate recipes into contracted
  runtime plans.
- Terrain and content cell streaming materialize a bounded neighborhood.
- Structural, traversability, terrain, physical-fit, and performance evaluators provide
  independent evidence.
- `AGatersChunk` remains the prototype coordinator, not the long-term home for every
  generator.

## Verify

- Build `PrototypeEditor Win64 Development` with Unreal closed when native code changed.
- Run the relevant `Gaters.*` automation filter, then the full `Gaters` filter before
  integration.
- Run `research/Test-MachineRegistry.ps1` whenever registry evidence or status changes.
- Record visual claims with a reproducible gallery or video; a successful compile is not
  visual evidence.

Exact commands for specialized asset round trips live beside their scripts under
`SourceAssets/Blender/` and `Unreal/Prototype/Scripts/`.
