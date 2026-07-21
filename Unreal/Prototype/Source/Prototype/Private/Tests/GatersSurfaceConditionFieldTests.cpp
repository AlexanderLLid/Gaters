#if WITH_DEV_AUTOMATION_TESTS

#include "GatersSurfaceConditionField.h"

#include "GatersEnvironmentRecipe.h"
#include "Misc/AutomationTest.h"

namespace
{
bool IsBounded(const float Value)
{
	return FMath::IsFinite(Value) && Value >= 0.f && Value <= 1.f;
}

bool HasIssue(
	const FGatersSurfaceConditionCompileResult& Result,
	const TCHAR* RuleId)
{
	return Result.Issues.ContainsByPredicate(
		[RuleId](const FGatersSurfaceConditionIssue& Issue)
		{
			return Issue.RuleId == RuleId;
		});
}

void TestBounded(
	FAutomationTestBase& Test,
	const TCHAR* Label,
	const FGatersSurfaceConditionSample& Sample)
{
	for (const float Value : {
		Sample.Soil, Sample.Rock, Sample.Sediment, Sample.Sand,
		Sample.Snow, Sample.Ice, Sample.Saturation, Sample.Shore,
		Sample.Ridge, Sample.Valley, Sample.Cliff})
	{
		Test.TestTrue(Label, IsBounded(Value));
	}
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersSurfaceConditionPhysicalControlsTest,
	"Gaters.Worldgen.SurfaceConditionField.PhysicalControls",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersSurfaceConditionPhysicalControlsTest::RunTest(
	const FString& Parameters)
{
	FGatersSurfaceConditionEvidence Base;
	Base.Temperature = 0.65f;
	Base.Precipitation = 0.2f;
	const FGatersSurfaceConditionSample Flat =
		FGatersSurfaceConditionField::Evaluate(Base);

	FGatersSurfaceConditionEvidence Steep = Base;
	Steep.NormalZ = 0.25f;
	Steep.WindExposure = 0.9f;
	const FGatersSurfaceConditionSample Cliff =
		FGatersSurfaceConditionField::Evaluate(Steep);
	TestTrue(TEXT("steep exposed terrain increases cliff evidence"),
		Cliff.Cliff > Flat.Cliff);
	TestTrue(TEXT("steep exposed terrain increases rock evidence"),
		Cliff.Rock > Flat.Rock);
	FGatersSurfaceConditionEvidence Thawing = Base;
	Thawing.FreezeThaw = 1.f;
	const FGatersSurfaceConditionSample Weathered =
		FGatersSurfaceConditionField::Evaluate(Thawing);
	TestTrue(TEXT("freeze-thaw increases loose sediment evidence"),
		Weathered.Sediment > Flat.Sediment);

	FGatersSurfaceConditionEvidence LowWet = Base;
	LowWet.Height = -600.f;
	LowWet.NeighborhoodMeanHeight = 600.f;
	LowWet.Precipitation = 0.9f;
	LowWet.DrainageAccumulation = 24.f;
	LowWet.DrainageChannel = 1.f;
	LowWet.WaterProximity = 0.4f;
	const FGatersSurfaceConditionSample Valley =
		FGatersSurfaceConditionField::Evaluate(LowWet);
	TestTrue(TEXT("low relief increases valley evidence"), Valley.Valley > Flat.Valley);
	TestTrue(TEXT("wet accumulated valleys increase saturation"),
		Valley.Saturation > Flat.Saturation);
	TestTrue(TEXT("wet accumulated valleys increase sediment"),
		Valley.Sediment > Flat.Sediment);

	FGatersSurfaceConditionEvidence Coast = Base;
	Coast.ShoreProximity = 1.f;
	Coast.WaterProximity = 0.8f;
	const FGatersSurfaceConditionSample Shore =
		FGatersSurfaceConditionField::Evaluate(Coast);
	TestTrue(TEXT("declared shoreline increases shore evidence"),
		Shore.Shore > Flat.Shore);
	TestTrue(TEXT("gentle shoreline increases sand evidence"),
		Shore.Sand > Flat.Sand);

	FGatersSurfaceConditionEvidence Cold = LowWet;
	Cold.Temperature = 0.05f;
	Cold.FreezeThaw = 0.2f;
	Cold.WaterProximity = 1.f;
	Cold.WaterDepth = 1.f;
	const FGatersSurfaceConditionSample Frozen =
		FGatersSurfaceConditionField::Evaluate(Cold);
	TestTrue(TEXT("cold precipitation increases snow evidence"),
		Frozen.Snow > Valley.Snow);
	TestTrue(TEXT("cold surface water increases ice evidence"),
		Frozen.Ice > Valley.Ice);

	for (const FGatersSurfaceConditionSample* Sample :
		{&Flat, &Cliff, &Weathered, &Valley, &Shore, &Frozen})
	{
		TestBounded(*this, TEXT("all physical surface weights are bounded"), *Sample);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersSurfaceConditionRootContractTest,
	"Gaters.Worldgen.SurfaceConditionField.RootContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersSurfaceConditionRootContractTest::RunTest(
	const FString& Parameters)
{
	const FGatersEnvironmentRecipe Environment =
		FGatersEnvironmentRecipeCompiler::Compile(83, 120000.f);
	const FGatersSurfaceConditionCompileResult A =
		FGatersSurfaceConditionField::Compile(Environment);
	const FGatersSurfaceConditionCompileResult B =
		FGatersSurfaceConditionField::Compile(Environment);
	TestTrue(TEXT("accepted environment compiles a surface recipe"), A.IsValid());
	TestEqual(TEXT("same root compiles the same surface recipe"), A, B);
	TestEqual(TEXT("surface recipe is versioned"), A.Recipe.Version,
		FGatersSurfaceConditionRecipe::CurrentVersion);
	TestEqual(TEXT("surface recipe retains root seed"), A.Recipe.Seed, Environment.Seed);
	TestEqual(TEXT("surface recipe retains drainage version"),
		A.Recipe.DrainageVersion, Environment.Drainage.Version);

	for (const FVector2D Point : {
		FVector2D::ZeroVector,
		FVector2D(17000.f, -9000.f),
		Environment.Intent.Regions[1].Center})
	{
		const FGatersSurfaceConditionSample First =
			FGatersSurfaceConditionField::Query(A.Recipe, Environment, Point);
		const FGatersSurfaceConditionSample Second =
			FGatersSurfaceConditionField::Query(A.Recipe, Environment, Point);
		TestEqual(TEXT("same surface query repeats exactly"), First, Second);
		TestEqual(TEXT("surface query preserves point"), First.Point, Point);
		TestEqual(TEXT("surface query preserves root height"),
			First.Evidence.Height, Environment.QueryTerrain(Point).Height);
		TestEqual(TEXT("surface query records recipe version"),
			First.RecipeVersion, A.Recipe.Version);
		TestEqual(TEXT("drainage indices and weights align"),
			First.DrainageCellIndices.Num(), First.DrainageCellWeights.Num());
		float WeightSum = 0.f;
		for (int32 Index = 0; Index < First.DrainageCellIndices.Num(); ++Index)
		{
			TestTrue(TEXT("surface query references accepted drainage cells"),
				Environment.Drainage.Cells.IsValidIndex(
					First.DrainageCellIndices[Index]));
			WeightSum += First.DrainageCellWeights[Index];
		}
		TestTrue(TEXT("covered surface query has normalized drainage weights"),
			First.DrainageCellIndices.IsEmpty()
				|| FMath::IsNearlyEqual(WeightSum, 1.f, 0.001f));
		TestBounded(*this, TEXT("root surface weights are bounded"), First);
	}

	const float BoundaryX = -Environment.Drainage.Extent
		+ Environment.Drainage.CellSize * 40.f;
	const FGatersSurfaceConditionSample Left = FGatersSurfaceConditionField::Query(
		A.Recipe, Environment, FVector2D(BoundaryX - 1.f, 0.f));
	const FGatersSurfaceConditionSample Right = FGatersSurfaceConditionField::Query(
		A.Recipe, Environment, FVector2D(BoundaryX + 1.f, 0.f));
	TestTrue(TEXT("drainage accumulation is continuous across cell boundaries"),
		FMath::Abs(Left.Evidence.DrainageAccumulation
			- Right.Evidence.DrainageAccumulation) < 0.05f);
	TestTrue(TEXT("surface saturation does not snap at drainage cell boundaries"),
		FMath::Abs(Left.Saturation - Right.Saturation) < 0.05f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersSurfaceConditionCounterexamplesTest,
	"Gaters.Worldgen.SurfaceConditionField.Counterexamples",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersSurfaceConditionCounterexamplesTest::RunTest(
	const FString& Parameters)
{
	const FGatersEnvironmentRecipe Environment =
		FGatersEnvironmentRecipeCompiler::Compile(47, 120000.f);
	FGatersSurfaceConditionSettings Invalid;
	Invalid.NormalSampleDistance = 0.f;
	TestTrue(TEXT("invalid settings are causal"), HasIssue(
		FGatersSurfaceConditionField::Compile(Environment, Invalid),
		TEXT("surface.settings")));

	FGatersEnvironmentRecipe WrongEnvironment = Environment;
	WrongEnvironment.Version = 99;
	TestTrue(TEXT("invalid root provenance is causal"), HasIssue(
		FGatersSurfaceConditionField::Compile(WrongEnvironment),
		TEXT("surface.environment")));
	WrongEnvironment = Environment;
	WrongEnvironment.Drainage.Version = 99;
	TestTrue(TEXT("invalid drainage provenance is causal"), HasIssue(
		FGatersSurfaceConditionField::Compile(WrongEnvironment),
		TEXT("surface.environment")));

	FGatersSurfaceConditionEvidence Dry;
	Dry.Temperature = 0.7f;
	const FGatersSurfaceConditionSample Sparse =
		FGatersSurfaceConditionField::Evaluate(Dry);
	TestEqual(TEXT("dry scarcity invents no shore"), Sparse.Shore, 0.f);
	TestEqual(TEXT("warm dry scarcity invents no snow"), Sparse.Snow, 0.f);
	TestEqual(TEXT("warm dry scarcity invents no ice"), Sparse.Ice, 0.f);
	TestBounded(*this, TEXT("dry scarcity remains bounded"), Sparse);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersSurfaceConditionHeldOutTest,
	"Gaters.Worldgen.SurfaceConditionField.HeldOutRoots",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersSurfaceConditionHeldOutTest::RunTest(const FString& Parameters)
{
	for (const int32 Seed : {11, 29, 47, 83, 131})
	{
		const FGatersEnvironmentRecipe Environment =
			FGatersEnvironmentRecipeCompiler::Compile(Seed, 120000.f);
		const FGatersSurfaceConditionCompileResult Compiled =
			FGatersSurfaceConditionField::Compile(Environment);
		TestTrue(FString::Printf(TEXT("seed %d surface recipe compiles"), Seed),
			Compiled.IsValid());
		for (int32 X = -2; X <= 2; ++X)
		{
			for (int32 Y = -2; Y <= 2; ++Y)
			{
				const FVector2D Point(X * 12000.f, Y * 12000.f);
				const FGatersSurfaceConditionSample A =
					FGatersSurfaceConditionField::Query(
						Compiled.Recipe, Environment, Point);
				const FGatersSurfaceConditionSample B =
					FGatersSurfaceConditionField::Query(
						Compiled.Recipe, Environment, Point);
				TestEqual(FString::Printf(TEXT("seed %d surface query repeats"), Seed),
					A, B);
				TestBounded(*this, TEXT("held-out surface weights are bounded"), A);
				for (const int32 CellIndex : A.DrainageCellIndices)
				{
					TestTrue(TEXT("held-out provenance references a drainage cell"),
						Environment.Drainage.Cells.IsValidIndex(CellIndex));
				}
			}
		}
	}
	return true;
}

#endif
