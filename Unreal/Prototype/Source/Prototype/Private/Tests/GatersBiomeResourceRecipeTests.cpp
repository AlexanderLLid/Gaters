#if WITH_DEV_AUTOMATION_TESTS

#include "GatersBiomeResourceRecipe.h"
#include "GatersEnvironmentRecipe.h"
#include "Misc/AutomationTest.h"

namespace
{
constexpr float WorldSize = 200000.f;
constexpr float CellSize = 10000.f;

bool HasIssue(
	const TArray<FGatersBiomeResourceIssue>& Issues,
	const FString& RuleId)
{
	return Issues.ContainsByPredicate(
		[&RuleId](const FGatersBiomeResourceIssue& Issue)
		{
			return Issue.RuleId == RuleId;
		});
}

bool SameContent(
	const FGatersContentCellRecipe& A,
	const FGatersContentCellRecipe& B)
{
	if (A.Version != B.Version
		|| A.WorldSeed != B.WorldSeed
		|| A.Cell != B.Cell
		|| !FMath::IsNearlyEqual(A.CellSize, B.CellSize)
		|| A.EnvironmentVersion != B.EnvironmentVersion
		|| A.BiomeOpportunityVersion != B.BiomeOpportunityVersion
		|| A.IntentVersion != B.IntentVersion
		|| A.IntentRegionId != B.IntentRegionId
		|| A.BiomeKey != B.BiomeKey
		|| A.Coverage != B.Coverage
		|| A.Placements.Num() != B.Placements.Num())
	{
		return false;
	}
	for (int32 Index = 0; Index < A.Placements.Num(); ++Index)
	{
		const FGatersContentCellPlacement& Left = A.Placements[Index];
		const FGatersContentCellPlacement& Right = B.Placements[Index];
		if (Left.Id != Right.Id
			|| Left.Kind != Right.Kind
			|| Left.ContentKey != Right.ContentKey
			|| !Left.Transform.Equals(Right.Transform))
		{
			return false;
		}
	}
	return true;
}

void AddCoverage(
	FGatersContentCellCoverage& Total,
	const FGatersContentCellCoverage& Cell)
{
	Total.CandidateCount += Cell.CandidateCount;
	Total.PlacedCount += Cell.PlacedCount;
	Total.ReservedRejectedCount += Cell.ReservedRejectedCount;
	Total.IntentRejectedCount += Cell.IntentRejectedCount;
	Total.WaterRejectedCount += Cell.WaterRejectedCount;
	Total.SteepRejectedCount += Cell.SteepRejectedCount;
	Total.OpportunityRejectedCount += Cell.OpportunityRejectedCount;
	Total.BudgetRejectedCount += Cell.BudgetRejectedCount;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersBiomeResourceContractTest,
	"Gaters.Worldgen.BiomeResources.Contract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersBiomeResourceContractTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironmentRecipe Environment =
		FGatersEnvironmentRecipeCompiler::Compile(11, WorldSize);
	const FIntRect Bounds(2, -1, 4, 1);
	const FGatersBiomeResourceCompileResult A =
		FGatersBiomeResourceCompiler::Compile(Environment, Bounds, CellSize);
	const FGatersBiomeResourceCompileResult B =
		FGatersBiomeResourceCompiler::Compile(Environment, Bounds, CellSize);

	TestTrue(TEXT("bounded recipe compiles"), A.IsValid());
	TestTrue(TEXT("same bounded recipe recompiles"), B.IsValid());
	if (!A.IsValid() || !B.IsValid())
	{
		return false;
	}
	TestEqual(TEXT("recipe contract version"), A.Recipe.Version, 1);
	TestEqual(TEXT("compiler version"), A.Recipe.CompilerVersion, 1);
	TestEqual(TEXT("environment provenance"),
		A.Recipe.EnvironmentVersion, Environment.Version);
	TestEqual(TEXT("opportunity provenance"),
		A.Recipe.BiomeOpportunityVersion, Environment.BiomeOpportunities.Version);
	TestEqual(TEXT("half-open bounds emit four cells"), A.Recipe.Cells.Num(), 4);
	TestEqual(TEXT("canonical identity repeats"),
		A.Recipe.CanonicalText(), B.Recipe.CanonicalText());
	TestEqual(TEXT("checksum repeats"), A.Recipe.Checksum(), B.Recipe.Checksum());

	FGatersContentCellCoverage ExpectedCoverage;
	int32 ExpectedBudget = 0;
	for (const FGatersBiomeResourceCell& Cell : A.Recipe.Cells)
	{
		const FVector2D Center(
			Cell.Coordinate.X * CellSize,
			Cell.Coordinate.Y * CellSize);
		FGatersBiomeQuery Query;
		Query.NormalSampleDistance = (CellSize / 4.f) * 0.25f;
		const FGatersBiomeOpportunitySample Opportunity =
			Environment.QueryOpportunities(Center, Query);
		const FGatersWorldRegionIntent& Region = Environment.Intent.At(Center);
		const FGatersContentCellRecipe ExpectedContent =
			FGatersContentCellRecipe::Generate(Cell.Coordinate, CellSize, Environment);

		TestEqual(TEXT("cell keeps stable identity"),
			Cell.Id,
			FString::Printf(TEXT("biome-resource:%d:%d:%d"),
				Environment.Seed, Cell.Coordinate.X, Cell.Coordinate.Y));
		TestEqual(TEXT("cell keeps root biome"),
			Cell.BiomeKey, Environment.QueryBiome(Center, Query).BiomeKey);
		TestTrue(TEXT("cell reuses exact content recipe"),
			SameContent(Cell.Content, ExpectedContent));
		TestEqual(TEXT("declared landmark opportunity is recorded"),
			Cell.DeclaredLandmarkOpportunity, Region.LandmarkOpportunity);
		TestEqual(TEXT("landmark opportunity combines intent and physical evidence"),
			Cell.LandmarkOpportunity,
			Region.LandmarkOpportunity * Opportunity.Landmark);
		TestEqual(TEXT("declared travel friction is recorded"),
			Cell.DeclaredTravelFriction, Region.TravelFriction);
		TestEqual(TEXT("travel friction preserves strongest cause"),
			Cell.TravelFriction,
			FMath::Max(Region.TravelFriction, Opportunity.TravelFriction));
		ExpectedBudget += Cell.Content.MaxPlacements;
		AddCoverage(ExpectedCoverage, Cell.Content.Coverage);
	}
	TestEqual(TEXT("placement budget aggregates"),
		A.Recipe.PlacementBudget, ExpectedBudget);
	TestEqual(TEXT("coverage aggregates"), A.Recipe.Coverage, ExpectedCoverage);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersBiomeResourceScarcityTest,
	"Gaters.Worldgen.BiomeResources.Scarcity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersBiomeResourceScarcityTest::RunTest(const FString& Parameters)
{
	FGatersEnvironmentRecipe Environment =
		FGatersEnvironmentRecipeCompiler::Compile(29, WorldSize);
	for (FGatersWorldRegionIntent& Region : Environment.Intent.Regions)
	{
		Region.VegetationOpportunity = 0.f;
		Region.StoneOpportunity = 0.f;
		Region.LandmarkOpportunity = 0.f;
	}
	const FGatersBiomeResourceCompileResult Result =
		FGatersBiomeResourceCompiler::Compile(
			Environment, FIntRect(2, -1, 5, 2), CellSize);

	TestTrue(TEXT("declared scarcity remains valid"), Result.IsValid());
	if (!Result.IsValid())
	{
		return false;
	}
	for (const FGatersBiomeResourceCell& Cell : Result.Recipe.Cells)
	{
		TestEqual(TEXT("scarcity emits no placements"),
			Cell.Content.Placements.Num(), 0);
		TestEqual(TEXT("scarcity emits no forced landmark"),
			Cell.LandmarkOpportunity, 0.f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersBiomeResourceCounterexampleTest,
	"Gaters.Worldgen.BiomeResources.Counterexamples",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersBiomeResourceCounterexampleTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironmentRecipe Environment =
		FGatersEnvironmentRecipeCompiler::Compile(47, WorldSize);
	FGatersEnvironmentRecipe InvalidEnvironment = Environment;
	InvalidEnvironment.Version = 0;
	TestTrue(TEXT("invalid environment fails causally"), HasIssue(
		FGatersBiomeResourceCompiler::Compile(
			InvalidEnvironment, FIntRect(0, 0, 1, 1), CellSize).Issues,
		TEXT("biome-resource.environment.invalid")));
	TestTrue(TEXT("empty bounds fail causally"), HasIssue(
		FGatersBiomeResourceCompiler::Compile(
			Environment, FIntRect(0, 0, 0, 1), CellSize).Issues,
		TEXT("biome-resource.bounds.invalid")));
	TestTrue(TEXT("non-finite cell size fails causally"), HasIssue(
		FGatersBiomeResourceCompiler::Compile(
			Environment, FIntRect(0, 0, 1, 1), NAN).Issues,
		TEXT("biome-resource.cell-size.invalid")));
	FGatersBiomeResourceSettings Tight;
	Tight.MaxCellCount = 1;
	TestTrue(TEXT("compile budget fails causally"), HasIssue(
		FGatersBiomeResourceCompiler::Compile(
			Environment, FIntRect(0, 0, 2, 2), CellSize, Tight).Issues,
		TEXT("biome-resource.cell-budget.exceeded")));

	const FGatersBiomeResourceCompileResult Valid =
		FGatersBiomeResourceCompiler::Compile(
			Environment, FIntRect(2, -1, 4, 1), CellSize);
	TestTrue(TEXT("mutation fixture compiles"), Valid.IsValid());
	if (!Valid.IsValid())
	{
		return false;
	}
	TArray<FGatersBiomeResourceIssue> Issues;
	FGatersBiomeResourceRecipe Mutated = Valid.Recipe;
	Mutated.Cells[0].Id = TEXT("wrong");
	TestFalse(TEXT("mutated identity is rejected"), Mutated.Validate(Issues));
	TestTrue(TEXT("identity issue is causal"),
		HasIssue(Issues, TEXT("biome-resource.cell.identity")));
	Mutated = Valid.Recipe;
	Mutated.Cells[1].Coordinate = Mutated.Cells[0].Coordinate;
	TestFalse(TEXT("duplicate coordinate is rejected"), Mutated.Validate(Issues));
	TestTrue(TEXT("duplicate issue is causal"),
		HasIssue(Issues, TEXT("biome-resource.cell.duplicate")));
	Mutated = Valid.Recipe;
	Mutated.Cells[0].LandmarkOpportunity = NAN;
	TestFalse(TEXT("invalid opportunity is rejected"), Mutated.Validate(Issues));
	TestTrue(TEXT("opportunity issue is causal"),
		HasIssue(Issues, TEXT("biome-resource.opportunity.invalid")));
	Mutated = Valid.Recipe;
	++Mutated.Cells[0].Content.EnvironmentVersion;
	TestFalse(TEXT("source drift is rejected"), Mutated.Validate(Issues));
	TestTrue(TEXT("source issue is causal"),
		HasIssue(Issues, TEXT("biome-resource.source.provenance")));
	Mutated = Valid.Recipe;
	++Mutated.Coverage.PlacedCount;
	TestFalse(TEXT("aggregate drift is rejected"), Mutated.Validate(Issues));
	TestTrue(TEXT("aggregate issue is causal"),
		HasIssue(Issues, TEXT("biome-resource.aggregate.coverage")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersBiomeResourceHeldOutRootsTest,
	"Gaters.Worldgen.BiomeResources.HeldOutRoots",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersBiomeResourceHeldOutRootsTest::RunTest(const FString& Parameters)
{
	TSet<FString> CompositionSignatures;
	for (const int32 Seed : {11, 29, 47, 83, 131})
	{
		const FGatersEnvironmentRecipe Environment =
			FGatersEnvironmentRecipeCompiler::Compile(Seed, WorldSize);
		const FIntRect Bounds(2, -2, 5, 1);
		const FGatersBiomeResourceCompileResult A =
			FGatersBiomeResourceCompiler::Compile(Environment, Bounds, CellSize);
		const FGatersBiomeResourceCompileResult B =
			FGatersBiomeResourceCompiler::Compile(Environment, Bounds, CellSize);
		TestTrue(TEXT("held-out root compiles"), A.IsValid());
		TestTrue(TEXT("held-out root repeats"), B.IsValid());
		if (!A.IsValid() || !B.IsValid())
		{
			continue;
		}
		TestEqual(TEXT("held-out identity repeats"),
			A.Recipe.CanonicalText(), B.Recipe.CanonicalText());
		TestTrue(TEXT("held-out placement total stays within budget"),
			A.Recipe.Coverage.PlacedCount <= A.Recipe.PlacementBudget);

		FString Signature;
		for (const FGatersBiomeResourceCell& Cell : A.Recipe.Cells)
		{
			const FVector2D Center(
				Cell.Coordinate.X * CellSize,
				Cell.Coordinate.Y * CellSize);
			FGatersBiomeQuery Query;
			Query.NormalSampleDistance = (CellSize / 4.f) * 0.25f;
			const FGatersBiomeOpportunitySample Opportunity =
				Environment.QueryOpportunities(Center, Query);
			const FGatersWorldRegionIntent& Region = Environment.Intent.At(Center);
			TestEqual(TEXT("held-out biome parity"),
				Cell.BiomeKey, Environment.QueryBiome(Center, Query).BiomeKey);
			TestEqual(TEXT("held-out landmark parity"),
				Cell.LandmarkOpportunity,
				Region.LandmarkOpportunity * Opportunity.Landmark);
			TestEqual(TEXT("held-out friction parity"),
				Cell.TravelFriction,
				FMath::Max(Region.TravelFriction, Opportunity.TravelFriction));
			Signature += FString::Printf(TEXT("%s:%d:%d:%d|"),
				*Cell.BiomeKey,
				FMath::RoundToInt(Cell.LandmarkOpportunity * 10.f),
				FMath::RoundToInt(Cell.TravelFriction * 10.f),
				Cell.Content.Placements.Num());
		}
		CompositionSignatures.Add(Signature);
	}
	TestTrue(TEXT("held-out roots expose distinct physical compositions"),
		CompositionSignatures.Num() >= 2);
	return true;
}

#endif
