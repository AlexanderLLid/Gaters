# Environment Recipe Compiler v1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use
> `superpowers:subagent-driven-development` or `superpowers:executing-plans` to execute
> this plan task-by-task. Do not commit unless the human asks; `AGENTS.md` overrides the
> generic skill workflow.

**Goal:** Compile one authoritative pure-data environment recipe from the existing
terrain, regional intent, water, biome, and opportunity machines so downstream systems
do not reconstruct those layers independently.

**Architecture:** Add a small `FGatersEnvironmentRecipe` facade and
`FGatersEnvironmentRecipeCompiler`. The compiler composes existing verified machines;
the recipe exposes deterministic terrain, biome, and neutral-opportunity queries plus
precomputed regional water. `AGatersChunk` and streamed terrain cells consume this root,
while Unreal Actors, assets, the Built Site layer, raids, and characters stay outside it.

**Tech Stack:** Unreal Engine 5.8 C++, existing pure structs, Unreal automation.

## Global Constraints

- `world.environment-generator` remains the top World & Terrain capability; this v1 is
  one prerequisite, not a claim of complete Earth terrain coverage.
- Preserve the verified terrain generator as champion.
- Do not add climate, geology, ice, caves, beaches, resources, sites, Actors, or assets
  to this first compiler slice.
- Generated recipes and semantic queries are authoritative; Unreal objects are derived.
- No edits to Settlements, Bases & Dungeons, Raids & Dungeons, characters, combat, or canon.
- Requirements checked: global generated-content boundary; World & Terrain ownership;
  no exceptions.

---

### Task 1: Pure environment recipe contract

**Files:**

- Create: `Unreal/Prototype/Source/Prototype/Public/GatersEnvironmentRecipe.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersEnvironmentRecipe.cpp`
- Create: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersEnvironmentRecipeTests.cpp`

**Interfaces:**

- Consumes: `FGatersEnvironment::FromSeed`, `FGatersWorldIntentRecipe::Generate`,
  `FGatersRegionalWaterRecipe::Generate`, `FGatersIntentTerrainField`,
  `FGatersTerrainSemanticField`, `FGatersBiomeField`, and
  `FGatersBiomeOpportunityField`.
- Produces:
  `FGatersEnvironmentRecipeCompiler::Compile(int32 Seed, float WorldSize)`, returning a
  recipe with version/provenance, global terrain, regional intent, regional water, and
  pure coordinate-query methods.

- [x] **Step 1: Write the failing contract test**

  Cover exact repeated-seed identity across seeds `0`, `2`, `4`, `7`, and `53`; changed
  seed variation; component provenance; exact parity with direct terrain/biome/
  opportunity queries; bounded query results; and regional-water parity.

- [x] **Step 2: Verify RED**

  Run:

  ```powershell
  & 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat' `
    PrototypeEditor Win64 Development `
    'C:\repos\Gaters\Unreal\Prototype\Prototype.uproject' -WaitMutex -NoHotReload
  ```

  Expected: compilation fails because `GatersEnvironmentRecipe.h` does not exist.

- [x] **Step 3: Implement the minimum compiler**

  `Compile` constructs exactly one global environment, one world-intent recipe, and one
  regional-water recipe. Query methods delegate to the existing field machines. Do not
  copy their generation formulas into the compiler.

- [x] **Step 4: Verify GREEN**

  Build with the command above, then run:

  ```powershell
  & 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
    'C:\repos\Gaters\Unreal\Prototype\Prototype.uproject' `
    -unattended -nop4 -nosplash -nullrhi `
    '-ExecCmds=Automation RunTests Gaters.Worldgen.EnvironmentRecipe;Quit' `
    '-TestExit=Automation Test Queue Empty' -log
  ```

  Expected: all `Gaters.Worldgen.EnvironmentRecipe` tests pass.

### Task 2: Runtime consumes the compiled root

**Files:**

- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersChunk.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersChunk.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersTerrainCell.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersTerrainCell.cpp`

**Interfaces:**

- Consumes: `FGatersEnvironmentRecipeCompiler::Compile` and recipe query methods.
- Produces: one runtime-owned environment recipe copied into streamed terrain cells;
  no cell reconstructs global environment intent from the seed.

- [x] **Step 1: Add a failing runtime-consumption assertion**

  Extend the environment-recipe test with a compile counter or an explicit terrain-cell
  configuration contract proving cells receive the recipe rather than calling
  `FromSeed`/`Generate` themselves. Prefer signature evidence if a global counter would
  add mutable test-only state.

- [x] **Step 2: Route runtime through the recipe**

  Replace `AGatersChunk`'s parallel `Environment` and `WorldIntent` members with one
  `FGatersEnvironmentRecipe`. Use its precompiled regional-water result. Change
  `AGatersTerrainCell::Configure` to receive the recipe and copy only the immutable
  environment and intent values needed while building its mesh.

- [x] **Step 3: Run focused regressions**

  Run `Gaters.Worldgen.EnvironmentRecipe`, `Gaters.Worldgen.IntentTerrain`,
  `Gaters.Worldgen.Biomes`, `Gaters.Worldgen.RegionalWater`,
  `Gaters.Worldgen.ContentCells`, and `Gaters.Runtime.Terrain` in separate Unreal command
  invocations. Expected: all pass with no output identity changes.

### Task 3: Machine truth and full verification

**Files:**

- Modify: `research/machines.json`
- Modify: `.agents/workstreams/Primary Builder — World & Terrain.md`

**Interfaces:**

- Consumes: fresh compiler and runtime evidence.
- Produces: an honest `world.environment-recipe-compiler` node; the existing
  `world.environment-generator` depends on it and remains the top planned World & Terrain
  capability.

- [x] **Step 1: Record the machine**

  Record inputs, output guarantee, work deleted, limitation, verifier, held-out seeds,
  failure artifact, and promotion gate. Mark only the implemented compiler slice as
  verified; keep complete Earth-like environment generation active/planned according to
  evidence.

- [x] **Step 2: Run all checks**

  Run the full `Gaters` Unreal automation suite, `research/Test-MachineRegistry.ps1`, and
  `research/Test-SharedAgentDocs.ps1`.

- [x] **Step 3: Close truthful status**

  Record exact counts and explicitly state that climate, substrate, drainage, landform
  processes, ice, and complete resource loops remain future environment dependencies.
