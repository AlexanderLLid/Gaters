#if WITH_DEV_AUTOMATION_TESTS

#include "GatersContentCellRecipe.h"
#include "GatersEnvironment.h"
#include "GatersWorldRecipe.h"
#include "Misc/AutomationTest.h"

namespace
{
constexpr float WorldSize = 200000.f;
constexpr float CellSize = 10000.f;

FGatersEnvironment FindEnvironment(EGatersEnvironment Type)
{
	for (int32 Seed = 0; Seed < 256; ++Seed)
	{
		const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(Seed, WorldSize);
		if (Environment.Type == Type)
		{
			return Environment;
		}
	}
	return {};
}

FGatersEnvironment FindDryLowlands(int32 ExcludedSeed = INDEX_NONE)
{
	for (int32 Seed = 0; Seed < 256; ++Seed)
	{
		const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(Seed, WorldSize);
		if (Seed != ExcludedSeed
			&& Environment.Type == EGatersEnvironment::Lowlands
			&& !Environment.HasWater())
		{
			return Environment;
		}
	}
	return {};
}

bool SamePlacement(
	const FGatersContentCellPlacement& A,
	const FGatersContentCellPlacement& B)
{
	return A.Id == B.Id
		&& A.Kind == B.Kind
		&& A.ContentKey == B.ContentKey
		&& A.Transform.Equals(B.Transform);
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersContentCellDeterminismTest,
	"Gaters.Worldgen.ContentCells.Determinism",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersContentCellDeterminismTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment Environment = FindEnvironment(EGatersEnvironment::Lowlands);
	const FGatersContentCellRecipe A = FGatersContentCellRecipe::Generate(
		FIntPoint(4, -3), CellSize, Environment);
	const FGatersContentCellRecipe B = FGatersContentCellRecipe::Generate(
		FIntPoint(4, -3), CellSize, Environment);

	TestEqual(TEXT("content-cell contract is versioned"), A.Version, 2);
	TestEqual(TEXT("same input preserves placement count"), A.Placements.Num(), B.Placements.Num());
	TestEqual(TEXT("same input preserves coverage evidence"), A.Coverage, B.Coverage);
	for (int32 Index = 0; Index < A.Placements.Num(); ++Index)
	{
		TestTrue(TEXT("same input preserves every placement"),
			SamePlacement(A.Placements[Index], B.Placements[Index]));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersContentCellIdentityAndBoundsTest,
	"Gaters.Worldgen.ContentCells.IdentityAndBounds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersContentCellIdentityAndBoundsTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment Environment = FindDryLowlands();
	FGatersContentCellSemantics IdentitySemantics = {};
	IdentitySemantics.MinGroundNormalZ = 0.f;
	const FIntPoint Cell(4, -3);
	const FGatersContentCellRecipe Recipe = FGatersContentCellRecipe::Generate(
		Cell, CellSize, Environment, IdentitySemantics);
	const FGatersContentCellRecipe Adjacent = FGatersContentCellRecipe::Generate(
		Cell + FIntPoint(1, 0), CellSize, Environment, IdentitySemantics);
	const FGatersEnvironment DifferentEnvironment = FindDryLowlands(Environment.Seed);
	const FGatersContentCellRecipe OtherSeed = FGatersContentCellRecipe::Generate(
		Cell, CellSize, DifferentEnvironment, IdentitySemantics);

	TestTrue(TEXT("placement budget is bounded"), Recipe.Placements.Num() <= Recipe.MaxPlacements);
	TestTrue(TEXT("identity fixture has adjacent-cell placements"),
		Adjacent.Placements.Num() > 0);
	TestTrue(TEXT("identity fixture has different-world placements"),
		OtherSeed.Placements.Num() > 0);
	TestEqual(TEXT("every opportunity has an outcome"),
		Recipe.Coverage.CandidateCount,
		Recipe.Coverage.PlacedCount + Recipe.Coverage.ReservedRejectedCount
			+ Recipe.Coverage.WaterRejectedCount
			+ Recipe.Coverage.SteepRejectedCount + Recipe.Coverage.BudgetRejectedCount);

	const FVector2D Center(Cell.X * CellSize, Cell.Y * CellSize);
	TSet<FString> Ids;
	for (const FGatersContentCellPlacement& Placement : Recipe.Placements)
	{
		TestTrue(TEXT("IDs are unique inside a cell"), !Ids.Contains(Placement.Id));
		Ids.Add(Placement.Id);
		TestTrue(TEXT("placement X stays inside its cell"),
			FMath::Abs(Placement.Transform.GetLocation().X - Center.X) < CellSize * 0.5f);
		TestTrue(TEXT("placement Y stays inside its cell"),
			FMath::Abs(Placement.Transform.GetLocation().Y - Center.Y) < CellSize * 0.5f);
		TestTrue(TEXT("placement keeps a semantic content key"),
			Placement.ContentKey == TEXT("environment.tree")
				|| Placement.ContentKey == TEXT("environment.rock"));
		TestTrue(TEXT("placement keeps a semantic content kind"),
			Placement.Kind == EGatersRecipeNodeKind::ScatterTree
				|| Placement.Kind == EGatersRecipeNodeKind::ScatterRock);
		const FVector BaseScale = Placement.Kind == EGatersRecipeNodeKind::ScatterTree
			? FVector(0.8f, 0.8f, 5.f)
			: FVector(1.8f, 1.4f, 0.8f);
		const FVector Scale = Placement.Transform.GetScale3D();
		const float Variation = Scale.X / BaseScale.X;
		TestTrue(TEXT("recipe owns bounded scatter scale variation"),
			Variation >= 0.85f && Variation <= 1.15f);
		TestTrue(TEXT("recipe preserves the existing tree or rock scale shape"),
			Scale.Equals(BaseScale * Variation));
	}
	for (const FGatersContentCellPlacement& Placement : Adjacent.Placements)
	{
		TestFalse(TEXT("adjacent cells cannot collide in identity"), Ids.Contains(Placement.Id));
	}
	for (const FGatersContentCellPlacement& Placement : OtherSeed.Placements)
	{
		TestFalse(TEXT("different worlds cannot collide in identity"), Ids.Contains(Placement.Id));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersContentCellSpatialCoverageTest,
	"Gaters.Worldgen.ContentCells.SpatialCoverage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersContentCellSpatialCoverageTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment Environment = FindEnvironment(EGatersEnvironment::Lowlands);
	const FIntPoint Cell(4, -3);
	FGatersContentCellSemantics FullLand = {};
	FullLand.MinGroundNormalZ = 0.f;
	const FGatersContentCellRecipe A = FGatersContentCellRecipe::Generate(
		Cell, CellSize, Environment, FullLand);
	const FGatersContentCellRecipe B = FGatersContentCellRecipe::Generate(
		Cell, CellSize, Environment, FullLand);
	const FVector2D Center(Cell.X * CellSize, Cell.Y * CellSize);
	bool bNegativeX = false;
	bool bPositiveX = false;
	bool bNegativeY = false;
	bool bPositiveY = false;

	TestEqual(TEXT("full dry land fills the bounded placement budget"),
		A.Placements.Num(), A.MaxPlacements);
	TestEqual(TEXT("full dry land rejects remaining opportunities only by budget"),
		A.Coverage.BudgetRejectedCount, A.Coverage.CandidateCount - A.MaxPlacements);
	for (int32 Index = 0; Index < A.Placements.Num(); ++Index)
	{
		const FVector Location = A.Placements[Index].Transform.GetLocation();
		bNegativeX |= Location.X < Center.X;
		bPositiveX |= Location.X > Center.X;
		bNegativeY |= Location.Y < Center.Y;
		bPositiveY |= Location.Y > Center.Y;
		TestTrue(TEXT("ranked selection remains deterministic"),
			SamePlacement(A.Placements[Index], B.Placements[Index]));
	}
	TestTrue(TEXT("placements span both X halves"), bNegativeX && bPositiveX);
	TestTrue(TEXT("placements span both Y halves"), bNegativeY && bPositiveY);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersContentCellCoverageTest,
	"Gaters.Worldgen.ContentCells.Coverage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersContentCellCoverageTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment Land = FindEnvironment(EGatersEnvironment::Lowlands);
	const FGatersContentCellRecipe DistantLand = FGatersContentCellRecipe::Generate(
		FIntPoint(4, -3), CellSize, Land);
	TestTrue(TEXT("a distant valid land cell exposes resource opportunities"),
		DistantLand.Placements.Num() > 0);

	const FGatersEnvironment Islands = FindEnvironment(EGatersEnvironment::Archipelago);
	const FGatersContentCellRecipe DistantWater = FGatersContentCellRecipe::Generate(
		FIntPoint(8, 8), CellSize, Islands);
	TestEqual(TEXT("distant ocean may be empty"), DistantWater.Placements.Num(), 0);
	TestEqual(TEXT("empty ocean records water as the cause"),
		DistantWater.Coverage.WaterRejectedCount, DistantWater.Coverage.CandidateCount);

	FGatersContentCellSemantics Strict = {};
	Strict.MinGroundNormalZ = 1.f;
	const FGatersContentCellRecipe StrictSlope = FGatersContentCellRecipe::Generate(
		FIntPoint(4, -3), CellSize, Land, Strict);
	TestTrue(TEXT("terrain rejected by slope records the cause"),
		StrictSlope.Coverage.SteepRejectedCount > 0);

	FGatersContentCellSemantics Reserved = {};
	Reserved.PadRadius = CellSize;
	const FGatersContentCellRecipe ReservedFootprint = FGatersContentCellRecipe::Generate(
		FIntPoint::ZeroValue, CellSize, Land, Reserved);
	TestEqual(TEXT("reserved arrival footprint contains no resources"),
		ReservedFootprint.Placements.Num(), 0);
	TestEqual(TEXT("reserved arrival footprint records the cause"),
		ReservedFootprint.Coverage.ReservedRejectedCount,
		ReservedFootprint.Coverage.CandidateCount);
	return true;
}

#endif
