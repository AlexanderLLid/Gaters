#if WITH_DEV_AUTOMATION_TESTS

#include "GatersTerrainNavigation.h"
#include "GatersTraversabilityEvaluator.h"
#include "GatersWorldRecipe.h"
#include "Misc/AutomationTest.h"

namespace
{
FGatersTerrainSemanticField MakeField(std::initializer_list<const TCHAR*> Rows)
{
	FGatersTerrainSemanticField Field;
	Field.CellsPerAxis = static_cast<int32>(Rows.size());
	Field.CellSize = 500.f;
	Field.Cells.SetNum(Field.CellsPerAxis * Field.CellsPerAxis);
	int32 Y = 0;
	for (const TCHAR* Row : Rows)
	{
		check(FCString::Strlen(Row) == Field.CellsPerAxis);
		for (int32 X = 0; X < Field.CellsPerAxis; ++X)
		{
			FGatersTerrainSemanticSample& Sample = Field.Cells[X * Field.CellsPerAxis + Y];
			switch (Row[X])
			{
			case TEXT('.'): Sample.Type = EGatersTerrainSemantic::Flat; break;
			case TEXT('s'): Sample.Type = EGatersTerrainSemantic::Slope; break;
			case TEXT('#'): Sample.Type = EGatersTerrainSemantic::Steep; break;
			case TEXT('~'): Sample.Type = EGatersTerrainSemantic::Water; break;
			default: checkNoEntry();
			}
		}
		++Y;
	}
	return Field;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersTerrainNavigationTest,
	"Gaters.Worldgen.Navigation.TerrainGrid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersTerrainNavigationTest::RunTest(const FString& Parameters)
{
	const FGatersTerrainSemanticField Corridor = MakeField({
		TEXT("#####"), TEXT("#####"), TEXT("..s.."), TEXT("#####"), TEXT("#####") });
	const FGatersTerrainPath CorridorPath = FGatersTerrainNavigation::FindPath(
		Corridor, FIntPoint(0, 2), FIntPoint(4, 2));
	TestTrue(TEXT("flat and slope corridor is reachable"), CorridorPath.bFound);
	TestEqual(TEXT("corridor cost is four cell steps"), CorridorPath.Cost, 4);
	TestEqual(TEXT("path includes both endpoints"), CorridorPath.Cells.Num(), 5);

	const FGatersTerrainRegion CorridorRegion = FGatersTerrainNavigation::Analyze(
		Corridor, FIntPoint(0, 2));
	TestEqual(TEXT("corridor has one walkable component"), CorridorRegion.ComponentCount, 1);
	TestEqual(TEXT("all corridor cells are reachable"), CorridorRegion.ReachableCount, 5);
	TestEqual(TEXT("corridor walkable count is exhaustive"), CorridorRegion.WalkableCount, 5);

	const FGatersTerrainSemanticField Barrier = MakeField({
		TEXT("..~.."), TEXT("..~.."), TEXT("..~.."), TEXT("..~.."), TEXT("..~..") });
	const FGatersTerrainRegion BarrierRegion = FGatersTerrainNavigation::Analyze(
		Barrier, FIntPoint(0, 2));
	TestEqual(TEXT("water barrier splits the terrain"), BarrierRegion.ComponentCount, 2);
	TestEqual(TEXT("only one side is reachable"), BarrierRegion.ReachableCount, 10);
	TestFalse(TEXT("goal across water is unreachable"),
		FGatersTerrainNavigation::FindPath(Barrier, FIntPoint(0, 2), FIntPoint(4, 2)).bFound);

	const FGatersTerrainSemanticField Trapped = MakeField({
		TEXT("###"), TEXT("#.#"), TEXT("###") });
	const FGatersTerrainRegion TrappedRegion = FGatersTerrainNavigation::Analyze(
		Trapped, FIntPoint(1, 1));
	TestEqual(TEXT("trapped start reaches only itself"), TrappedRegion.ReachableCount, 1);

	const FGatersTerrainSemanticField Open = MakeField({
		TEXT("..."), TEXT("..."), TEXT("...") });
	const FGatersTerrainPath A = FGatersTerrainNavigation::FindPath(Open, FIntPoint(0, 0), FIntPoint(2, 2));
	const FGatersTerrainPath B = FGatersTerrainNavigation::FindPath(Open, FIntPoint(0, 0), FIntPoint(2, 2));
	TestEqual(TEXT("same query has same cost"), A.Cost, B.Cost);
	TestEqual(TEXT("same query has same path"), A.Cells, B.Cells);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersTraversabilityEvaluatorTest,
	"Gaters.Worldgen.Navigation.TraversabilityEvaluator",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersTraversabilityEvaluatorTest::RunTest(const FString& Parameters)
{
	const FGatersTerrainSemanticField Open = MakeField({
		TEXT("....."), TEXT("....."), TEXT("....."), TEXT("....."), TEXT(".....") });
	const FGatersTraversabilityEvaluation OpenResult = FGatersTraversabilityEvaluator::Evaluate(
		Open, FIntPoint(2, 2), FIntPoint(4, 4), 2);
	TestEqual(TEXT("evaluation contract is versioned"), OpenResult.EvaluatorVersion, 1);
	TestEqual(TEXT("open terrain has full reachable coverage"), OpenResult.ReachableFraction, 1.f);
	TestTrue(TEXT("open terrain escapes the start region"), OpenResult.bEscapesStart);
	TestTrue(TEXT("open terrain reaches its critical goal"), OpenResult.bGoalReachable);

	const FGatersTerrainSemanticField Trapped = MakeField({
		TEXT("###"), TEXT("#.#"), TEXT("###") });
	const FGatersTraversabilityEvaluation TrappedResult = FGatersTraversabilityEvaluator::Evaluate(
		Trapped, FIntPoint(1, 1), FIntPoint(1, 1), 1);
	TestFalse(TEXT("isolated cell cannot escape"), TrappedResult.bEscapesStart);
	TestEqual(TEXT("isolated cell remains its own reachable region"), TrappedResult.Region.ReachableCount, 1);

	const FGatersTerrainSemanticField Barrier = MakeField({
		TEXT("..~.."), TEXT("..~.."), TEXT("..~.."), TEXT("..~.."), TEXT("..~..") });
	const FGatersTraversabilityEvaluation BarrierResult = FGatersTraversabilityEvaluator::Evaluate(
		Barrier, FIntPoint(0, 2), FIntPoint(4, 2), 2);
	TestFalse(TEXT("critical goal across water is rejected"), BarrierResult.bGoalReachable);
	TestEqual(TEXT("barrier reports two components"), BarrierResult.Region.ComponentCount, 2);
	TestEqual(TEXT("barrier exposes half its walkable terrain"), BarrierResult.ReachableFraction, 0.5f);

	for (const int32 Seed : { 0, 2, 4, 7 })
	{
		const FGatersWorldRecipe Recipe = FGatersWorldRecipe::Generate(
			Seed, 30000.f, 6000.f, 10800.f, 900.f, 350.f);
		const FGatersTerrainSemanticField Generated = FGatersTerrainSemanticField::Build(
			FGatersEnvironment::FromSeed(Seed, 30000.f), 61, 500.f, 1000.f, 0.94f, 0.77f,
			Recipe.BaseSite);
		const int32 Half = Generated.CellsPerAxis / 2;
		const FIntPoint BaseCell(
			FMath::RoundToInt(Recipe.BaseSite.X / Generated.CellSize) + Half,
			FMath::RoundToInt(Recipe.BaseSite.Y / Generated.CellSize) + Half);
		const FGatersTraversabilityEvaluation GeneratedResult = FGatersTraversabilityEvaluator::Evaluate(
			Generated, FIntPoint(Half, Half), BaseCell, 3);
		TestTrue(*FString::Printf(TEXT("seed %d escapes the Gate"), Seed), GeneratedResult.bEscapesStart);
		TestTrue(*FString::Printf(TEXT("seed %d reaches its generated base"), Seed), GeneratedResult.bGoalReachable);
	}

	// Held-out visual failure: seed 53 once produced a mountain field whose Gate pad
	// and base were each locally valid but disconnected by nearly continuous cliffs.
	const int32 ChallengeSeed = 53;
	const float RuntimeWorldSize = 400000.f;
	const FGatersWorldRecipe ChallengeRecipe = FGatersWorldRecipe::Generate(
		ChallengeSeed, RuntimeWorldSize, 6000.f, 10800.f, 900.f, 350.f);
	const FGatersTerrainSemanticField ChallengeField = FGatersTerrainSemanticField::Build(
		FGatersEnvironment::FromSeed(ChallengeSeed, RuntimeWorldSize),
		61, 500.f, 1000.f, 0.94f, 0.77f, ChallengeRecipe.BaseSite);
	const int32 ChallengeHalf = ChallengeField.CellsPerAxis / 2;
	const FIntPoint ChallengeBaseCell(
		FMath::RoundToInt(ChallengeRecipe.BaseSite.X / ChallengeField.CellSize) + ChallengeHalf,
		FMath::RoundToInt(ChallengeRecipe.BaseSite.Y / ChallengeField.CellSize) + ChallengeHalf);
	const FGatersTraversabilityEvaluation ChallengeResult = FGatersTraversabilityEvaluator::Evaluate(
		ChallengeField, FIntPoint(ChallengeHalf, ChallengeHalf), ChallengeBaseCell, 3);
	TestTrue(TEXT("seed 53 escapes the Gate at runtime dimensions"), ChallengeResult.bEscapesStart);
	TestTrue(TEXT("seed 53 reaches its generated base at runtime dimensions"), ChallengeResult.bGoalReachable);

	const int32 DryMountainSeed = 0;
	const FGatersWorldRecipe DryMountainRecipe = FGatersWorldRecipe::Generate(
		DryMountainSeed, RuntimeWorldSize, 6000.f, 10800.f, 900.f, 350.f);
	const FGatersTerrainSemanticField DryMountainField = FGatersTerrainSemanticField::Build(
		FGatersEnvironment::FromSeed(DryMountainSeed, RuntimeWorldSize),
		61, 500.f, 1000.f, 0.94f, 0.77f, DryMountainRecipe.BaseSite);
	const FIntPoint DryMountainBaseCell(
		FMath::RoundToInt(DryMountainRecipe.BaseSite.X / DryMountainField.CellSize) + ChallengeHalf,
		FMath::RoundToInt(DryMountainRecipe.BaseSite.Y / DryMountainField.CellSize) + ChallengeHalf);
	const FGatersTraversabilityEvaluation DryMountainResult = FGatersTraversabilityEvaluator::Evaluate(
		DryMountainField, FIntPoint(ChallengeHalf, ChallengeHalf), DryMountainBaseCell, 3);
	AddInfo(FString::Printf(TEXT("seed 0 reachable=%.3f components=%d"),
		DryMountainResult.ReachableFraction, DryMountainResult.Region.ComponentCount));
	TestTrue(TEXT("seed 0 keeps most walkable terrain connected"),
		DryMountainResult.ReachableFraction >= 0.75f);
	TestTrue(TEXT("seed 0 avoids fragmented micro-ridge regions"),
		DryMountainResult.Region.ComponentCount <= 48);
	TestTrue(TEXT("seed 0 escapes the Gate"), DryMountainResult.bEscapesStart);
	TestTrue(TEXT("seed 0 reaches its generated base"), DryMountainResult.bGoalReachable);
	return true;
}

#endif
