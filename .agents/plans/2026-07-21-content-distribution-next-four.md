# Content Distribution Next Four Implementation Plan

> **For agentic workers:** execute inline with test-driven development. Do not commit;
> the human reviews the shared dirty checkout first.

**Goal:** Make streamed vegetation and stone density respond measurably to accepted
physical opportunity evidence, then expose the comparison in Unreal debug drawing.

**Architecture:** Keep Environment Recipe as the root, Content Cell as the pure semantic
placement recipe, and Unreal as a derived adapter. Add one pure evaluator that consumes
external observations rather than querying or generating its own expectations.

**Tech stack:** Unreal Engine 5.8, C++20, Unreal Automation Tests, native debug drawing.

## Global constraints

- Preserve intentional empty regions under `WORLD-1`.
- Do not add asset, settlement, raid, character, or terrain-family decisions.
- Do not duplicate the Content Cell generator.
- Version recipes and preserve causal rejection evidence.

---

### Todo 1: Independent content-distribution evaluator

**Files:**
- Create: `Unreal/Prototype/Source/Prototype/Public/GatersContentDistributionEvaluator.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersContentDistributionEvaluator.cpp`
- Create: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersContentDistributionEvaluatorTests.cpp`

**Produces:** `FGatersContentDistributionEvaluator::Evaluate(Observations, Settings)`;
observations contain external opportunity values, actual tree/rock counts, and capacity.

- [x] Write synthetic passing, constant-density failure, and scarcity-violation tests.
- [x] Run focused Automation and confirm the missing evaluator is the RED cause.
- [x] Implement aggregate density error, kind-mix error, correlation, and causal issues.
- [x] Run focused Automation and confirm evaluator tests pass.

### Todo 2: Content Cell v6 density response

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersContentCellRecipe.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersContentCellRecipe.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersContentCellRecipeTests.cpp`

**Produces:** deterministic opportunity rejection and lower placement density when the
accepted combined opportunity is lower.

- [x] Add a multi-cell test proving high opportunity produces more placements than low
  opportunity and zero opportunity stays empty.
- [x] Run focused Content Cells and confirm constant density fails the new assertion.
- [x] Add one deterministic opportunity gate and `OpportunityRejectedCount`.
- [x] Run focused Content Cells and preserve identity, bounds, and causal accounting.

### Todo 3: Held-out Environment Recipe sweep

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersContentDistributionEvaluatorTests.cpp`

**Produces:** independent root-scale evidence over seeds `11`, `29`, `47`, `83`, and
`131` using external Environment Recipe queries and generated Content Cell recipes.

- [x] Add a held-out sweep that builds evaluator observations from both sources.
- [x] Run it and retain the first causal failure if the v6 challenger misses tolerance.
- [x] Make only the smallest generator correction supported by the failure.
- [x] Run evaluator, Content Cells, Worldgen, and complete Gaters suites.

### Todo 4: Runtime opportunity diagnostic

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersChunk.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersChunk.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersSeedCommandTests.cpp`

**Produces:** `Gaters.ContentOpportunities [seconds]`, drawing loaded cell opportunity
strength and actual tree/rock placements without changing recipes or assets.

- [x] Add a failing console-registration test.
- [x] Run it and confirm the command is absent.
- [x] Add the smallest native debug-draw adapter over loaded content cells.
- [x] Run focused command tests and repository validators.

Requirements checked: WORLD-1; exceptions: none.
