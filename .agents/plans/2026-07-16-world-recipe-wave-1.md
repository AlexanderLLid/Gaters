# World Recipe Wave 1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:executing-plans` to implement this plan task-by-task. Subagents are intentionally not used because the repository instructions do not authorize delegation.

**Goal:** Make one existing Gaters seed produce a canonical, testable World Recipe and make `AGatersChunk` consume that recipe for environment and base-site placement.

**Architecture:** Add one pure C++ value type between seed generation and the Unreal actor. The value contains only the minimum stable world facts already generated today; it neither spawns actors nor serializes diffs. `AGatersChunk` remains the materializer and reports the recipe checksum so batch runs can identify exact inputs.

**Tech Stack:** Unreal Engine 5.8 C++, Unreal Automation Tests, `FCrc`.

## Global Constraints

- Preserve immutable ground and seed-plus-diff reconstruction.
- Do not change seed meaning or `GatersGenVersion`.
- Do not add JSON, reflection, PCG, new plugins, or a general graph framework.
- Do not change EBS stamping, scatter draw order, terrain math, or persistence keys.
- Do not commit, branch, or push; the human reviews first.
- Preserve unrelated dirty-worktree changes made by the human or Claude.

---

### Task 1: Canonical World Recipe

**Files:**
- Create: `Unreal/Prototype/Source/Prototype/Public/GatersWorldRecipe.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersWorldRecipe.cpp`
- Create: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersWorldRecipeTests.cpp`

**Interfaces:**
- Consumes: `FGatersEnvironment::FromSeed()` and `FGatersEnvironment::FindBaseSite()`.
- Produces: `FGatersWorldRecipe::Generate(...)`, `CanonicalText()`, and `Checksum()`.

- [ ] **Step 1: Write the failing automation test**

  Test that identical inputs yield identical canonical text/checksum and the same environment/base-site facts as the existing direct generator. Test that another seed changes the canonical identity.

- [ ] **Step 2: Build to verify RED**

  Run:

  ```powershell
  & 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat' PrototypeEditor Win64 Development 'C:\repos\Gaters\Unreal\Prototype\Prototype.uproject' -WaitMutex -NoHotReload
  ```

  Expected: failure because `GatersWorldRecipe.h` does not exist.

- [ ] **Step 3: Implement the minimum value type**

  Store schema version, generator version, seed, chunk size, environment type/name, water height, base validity, and base location. Generate those facts through the existing `FGatersEnvironment`; construct a fixed-order canonical string and hash it with `FCrc::StrCrc32`.

- [ ] **Step 4: Build and run the focused test**

  Run the build command above, then:

  ```powershell
  & 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\repos\Gaters\Unreal\Prototype\Prototype.uproject' -unattended -nop4 -nosplash -nullrhi '-ExecCmds=Automation RunTests Gaters.Worldgen.WorldRecipe;Quit' -TestExit='Automation Test Queue Empty' -log
  ```

  Expected: build exit `0`; focused automation tests report success.

---

### Task 2: Chunk Consumes and Reports the Recipe

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersChunk.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersChunk.cpp`
- Modify: `Unreal/Prototype/Scripts/RunEnvironmentSweep.ps1`

**Interfaces:**
- Consumes: `FGatersWorldRecipe::Generate(...)` and recipe fields.
- Produces: existing chunk output plus `RECIPE schema=<n> checksum=<hex>`.

- [ ] **Step 1: Write a failing integration assertion**

  Extend the automation test to assert that a recipe-created environment reproduces the recipe environment type, water height, and explicit base location without rerolling placement.

- [ ] **Step 2: Run the focused test to verify RED**

  Expected: failure because the recipe has no `CreateEnvironment()` adapter yet.

- [ ] **Step 3: Add the minimal adapter and chunk wiring**

  Add `CreateEnvironment()` to the recipe. In `RollSite()`, generate one recipe, construct the environment from it, and use the recipe's explicit base site. Keep the existing fallback behavior for an invalid site. Emit the recipe report line and include it in sweep filtering.

- [ ] **Step 4: Build, run focused tests, then regression tests**

  Run the build and focused command from Task 1, then:

  ```powershell
  & 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\repos\Gaters\Unreal\Prototype\Prototype.uproject' -unattended -nop4 -nosplash -nullrhi '-ExecCmds=Automation RunTests Gaters.Worldgen;Quit' -TestExit='Automation Test Queue Empty' -log
  ```

  Expected: all `Gaters.Worldgen` tests report success.

- [ ] **Step 5: Run a real seed smoke test**

  Run:

  ```powershell
  & 'C:\repos\Gaters\Unreal\Prototype\Scripts\RunEnvironmentSweep.ps1' -Seeds 7 -SecondsPerSeed 12
  ```

  Expected: one `RECIPE` line plus unchanged valid `SITE`, `BASE`, `SCATTER`, `STAMP`, and `GEN` lines.

## Self-review

- Scope covers only the first recipe seam, not the eventual full semantic scene graph.
- Every new non-trivial method is exercised by a focused automation test.
- The actor does not become responsible for canonicalization.
- Existing generation algorithms remain the sole source of seed meaning.
- No placeholders, migrations, new dependencies, or speculative extension points are included.

