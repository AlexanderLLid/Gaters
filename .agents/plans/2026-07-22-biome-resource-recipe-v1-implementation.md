# Biome Resource Recipe v1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use
> `superpowers:subagent-driven-development` or `superpowers:executing-plans` to implement
> this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Compile a bounded accepted Environment Recipe into one deterministic recipe
containing existing Content Cell placements plus neutral landmark and travel-friction
evidence.

**Architecture:** Add one pure compiler and recipe. It delegates all tree/rock placement
to `FGatersContentCellRecipe`, queries the accepted environment root for biome and
opportunity evidence, and owns only bounded aggregation, validation, identity, and
provenance. Existing evaluators judge its output.

**Tech Stack:** Unreal Engine 5.8, C++20, Unreal Automation Tests, native `FIntRect`,
`TArray`, `TSet`, and `FCrc`.

## Global Constraints

- Preserve optional scarcity under `WORLD-1`.
- Do not add an asset, site, route, settlement, raid, character, or gameplay-resource
  decision.
- Do not add another placement algorithm.
- Environment Recipe and Content Cell Recipe remain authoritative inputs.
- Use Unreal Runner for every Unreal build or Automation command.
- Do not commit; the human reviews the dirty shared checkout.

---

### Task 1: Define the failing recipe contract

**Files:**

- Create: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersBiomeResourceRecipeTests.cpp`

**Interfaces:**

- Consumes: `FGatersEnvironmentRecipeCompiler::Compile`,
  `FGatersContentCellRecipe::Generate`, and environment-root queries.
- Specifies: `FGatersBiomeResourceCompiler::Compile(Environment, CellBounds, CellSize,
  Settings)` and `FGatersBiomeResourceRecipe::Validate`.

```cpp
const FGatersBiomeResourceCompileResult Result =
	FGatersBiomeResourceCompiler::Compile(
		Environment, FIntRect(2, -1, 4, 1), 10000.f);
TestTrue(TEXT("bounded recipe compiles"), Result.IsValid());
TestEqual(TEXT("half-open bounds emit four cells"), Result.Recipe.Cells.Num(), 4);
TestEqual(TEXT("same input has stable identity"),
	Result.Recipe.Checksum(), Replay.Recipe.Checksum());
```

- [x] Add `Gaters.Worldgen.BiomeResources.Contract` using a bounded `2 x 2` cell range.
  It requires contract/compiler/environment provenance, stable cell IDs, deterministic
  canonical text/checksum, exact Content Cell reuse, exact biome/opportunity parity, and
  aggregate coverage/budget accounting.
- [x] Add `Gaters.Worldgen.BiomeResources.Scarcity` by setting every region's vegetation,
  stone, and landmark opportunity to zero. It requires a valid recipe with zero
  placements and no forced landmark opportunity.
- [x] Add `Gaters.Worldgen.BiomeResources.Counterexamples` covering invalid environment,
  empty/inverted bounds, non-finite/non-positive cell size, compile-budget overflow,
  mutated cell ID, duplicate coordinate, invalid opportunity, source-provenance drift,
  and aggregate-coverage drift. Every failure asserts its stable rule ID.
- [x] Add `Gaters.Worldgen.BiomeResources.HeldOutRoots` for seeds `11`, `29`, `47`, `83`,
  and `131`. It requires deterministic replay, exact independent source parity, valid
  scarcity, bounded totals, and at least two distinct biome/opportunity signatures across
  the held-out set.
- [x] Ask Unreal Runner to build and run `Gaters.Worldgen.BiomeResources`; record RED only
  when the first causal failure is the missing `GatersBiomeResourceRecipe.h` contract.

### Task 2: Implement the minimal pure compiler

**Files:**

- Create: `Unreal/Prototype/Source/Prototype/Public/GatersBiomeResourceRecipe.h`
- Create: `Unreal/Prototype/Source/Prototype/Private/GatersBiomeResourceRecipe.cpp`

**Interfaces:**

- Produces:
  - `FGatersBiomeResourceCell` with stable ID, coordinate, biome key, declared/effective
    landmark opportunity, declared/effective travel friction, and the unchanged
    `FGatersContentCellRecipe`.
  - `FGatersBiomeResourceRecipe` with version/provenance, half-open cell bounds, cell
    size, aggregate placement budget, aggregate coverage, cells, `CanonicalText`,
    `Checksum`, and `Validate`.
  - `FGatersBiomeResourceCompileResult` with `Recipe`, causal `Issues`, and `IsValid`.
  - `FGatersBiomeResourceCompiler::Compile`.

```cpp
struct FGatersBiomeResourceSettings
{
	int32 MaxCellCount = 256;
	FGatersContentCellSemantics ContentSemantics;
};

struct FGatersBiomeResourceCell
{
	FString Id;
	FIntPoint Coordinate = FIntPoint::ZeroValue;
	FString BiomeKey;
	float DeclaredLandmarkOpportunity = 0.f;
	float LandmarkOpportunity = 0.f;
	float DeclaredTravelFriction = 0.f;
	float TravelFriction = 0.f;
	FGatersContentCellRecipe Content;
};

struct FGatersBiomeResourceIssue
{
	FString RuleId;
	FString SubjectId;
	FString Message;
};

struct FGatersBiomeResourceRecipe
{
	static constexpr int32 CurrentVersion = 1;
	static constexpr int32 CurrentCompilerVersion = 1;

	bool Validate(TArray<FGatersBiomeResourceIssue>& OutIssues) const;
	FString CanonicalText() const;
	uint32 Checksum() const;

	int32 Version = CurrentVersion;
	int32 CompilerVersion = CurrentCompilerVersion;
	int32 EnvironmentVersion = 0;
	int32 BiomeOpportunityVersion = 0;
	int32 Seed = 0;
	FIntRect CellBounds;
	float CellSize = 0.f;
	int32 PlacementBudget = 0;
	FGatersContentCellCoverage Coverage;
	TArray<FGatersBiomeResourceCell> Cells;
};

struct FGatersBiomeResourceCompileResult
{
	bool IsValid() const { return Issues.IsEmpty(); }
	FGatersBiomeResourceRecipe Recipe;
	TArray<FGatersBiomeResourceIssue> Issues;
};

struct FGatersBiomeResourceCompiler
{
	static FGatersBiomeResourceCompileResult Compile(
		const FGatersEnvironmentRecipe& Environment,
		const FIntRect& CellBounds,
		float CellSize,
		const FGatersBiomeResourceSettings& Settings = {});
};
```

- [ ] Define contract version `1`, compiler version `1`, default maximum cell count `256`,
  and stable issue records containing rule ID, subject ID, and message.
- [ ] Reject invalid source, bounds, cell size, or requested cell count before generation.
  Use stable rule IDs `biome-resource.environment.invalid`,
  `biome-resource.bounds.invalid`, `biome-resource.cell-size.invalid`, and
  `biome-resource.cell-budget.exceeded`.
- [ ] Iterate the half-open bounds in stable X-major/Y-minor order. For each center point,
  use the same normal-sample distance as Content Cell, generate the existing Content Cell
  recipe unchanged, and record:
  - landmark opportunity = declared landmark opportunity × physical landmark opportunity;
  - travel friction = max(declared travel friction, physical travel friction).
- [ ] Sum placement budget and every `FGatersContentCellCoverage` field.
- [ ] Validate exact coordinate coverage/order, identity, finite bounded values, source
  provenance, Content Cell coordinate/size, and aggregate totals.
  Use stable rule IDs `biome-resource.cell.identity`, `biome-resource.cell.duplicate`,
  `biome-resource.opportunity.invalid`, `biome-resource.source.provenance`, and
  `biome-resource.aggregate.coverage`.
- [ ] Canonicalize every authoritative field with stable numeric formatting and compute
  checksum through `FCrc::StrCrc32`.
- [ ] Ask Unreal Runner to build and run `Gaters.Worldgen.BiomeResources`; repair only
  causal compiler failures until all four tests pass.

### Task 3: Verify regressions and integrate machine truth

**Files:**

- Modify: `research/machines.json`
- Modify: `.agents/workstreams/Primary Builder — World & Terrain.md`
- Modify: this plan's checkboxes.

**Interfaces:**

- Consumes: focused recipe evidence and existing independent Content Distribution
  evaluator evidence.
- Produces: truthful current status for `world.biome-resource-generator` and its parent
  `world.environment-generator` without promoting visual assets or gameplay resources.

- [ ] Ask Unreal Runner to run focused `Gaters.Worldgen.BiomeResources` and
  `Gaters.Worldgen.ContentDistribution` suites.
- [ ] Ask Unreal Runner to run complete `Gaters.Worldgen`, then complete `Gaters`.
- [ ] Preserve every unrelated active failure exactly; stop and diagnose any new
  World-owned failure.
- [ ] Update only the evidence-supported registry fields. The machine may be `verified`
  only if focused and held-out gates pass; otherwise keep it `active` and record the first
  falsified guarantee.
- [ ] Run `research/Test-MachineRegistry.ps1`,
  `research/Test-SharedAgentDocs.ps1`, and scoped `git diff --check`.
- [ ] Re-read this plan and the design, confirm every requirement is evidenced, and report
  exact counts, logs, limitations, and the next frontier.

Requirements checked: `WORLD-1`; exceptions: none.
