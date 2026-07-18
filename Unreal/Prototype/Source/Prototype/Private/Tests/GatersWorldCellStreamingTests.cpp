#if WITH_DEV_AUTOMATION_TESTS

#include "GatersWorldCellStreaming.h"
#include "GatersTerrainCell.h"
#include "GatersTerrainSemanticField.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersWorldCellCoordinatesTest,
	"Gaters.Worldgen.Streaming.Cells.Coordinates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersWorldCellCoordinatesTest::RunTest(const FString& Parameters)
{
	constexpr float CellSize = 10000.f;
	TestEqual(TEXT("origin is centered in cell zero"),
		FGatersWorldCellStreaming::CellAt(FVector2D::ZeroVector, CellSize), FIntPoint(0, 0));
	TestEqual(TEXT("positive half-edge enters the next cell"),
		FGatersWorldCellStreaming::CellAt(FVector2D(5000.f, 0.f), CellSize), FIntPoint(1, 0));
	TestEqual(TEXT("negative coordinates floor symmetrically"),
		FGatersWorldCellStreaming::CellAt(FVector2D(-5001.f, 0.f), CellSize), FIntPoint(-1, 0));
	TestEqual(TEXT("cell center round-trips"),
		FGatersWorldCellStreaming::CellAt(
			FGatersWorldCellStreaming::CellCenter(FIntPoint(-3, 4), CellSize), CellSize),
		FIntPoint(-3, 4));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersWorldCellSetTest,
	"Gaters.Worldgen.Streaming.Cells.DesiredSet",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersWorldCellSetTest::RunTest(const FString& Parameters)
{
	constexpr float CellSize = 10000.f;
	constexpr float WorldSize = 60000.f;
	const TArray<FIntPoint> Origin = FGatersWorldCellStreaming::DesiredCells(
		{ FVector2D::ZeroVector }, CellSize, 1, WorldSize);
	TestEqual(TEXT("radius one loads nine cells"), Origin.Num(), 9);
	TestEqual(TEXT("cell order is deterministic"), Origin[0], FIntPoint(-1, -1));
	TestEqual(TEXT("cell order ends at positive corner"), Origin.Last(), FIntPoint(1, 1));

	const TArray<FIntPoint> TwoSources = FGatersWorldCellStreaming::DesiredCells(
		{ FVector2D::ZeroVector, FVector2D(10000.f, 0.f) }, CellSize, 1, WorldSize);
	TestEqual(TEXT("overlapping source sets are deduplicated"), TwoSources.Num(), 12);

	const TArray<FIntPoint> Edge = FGatersWorldCellStreaming::DesiredCells(
		{ FVector2D(19000.f, 0.f) }, CellSize, 1, WorldSize);
	TestEqual(TEXT("world bounds clip cells whose mesh would cross the edge"), Edge.Num(), 6);
	for (const FIntPoint Cell : Edge)
	{
		TestTrue(TEXT("bounded cells stay within the logical world"),
			FMath::Abs(FGatersWorldCellStreaming::CellCenter(Cell, CellSize).X) <= 25000.f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersWorldCellPrefetchTest,
	"Gaters.Worldgen.Streaming.Cells.DirectionalPrefetch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersWorldCellPrefetchTest::RunTest(const FString& Parameters)
{
	const TArray<FIntPoint> Cells = FGatersWorldCellStreaming::DesiredCells(
		{ FVector2D::ZeroVector }, 10000.f, 1, 100000.f, FIntPoint(1, 0));
	TestEqual(TEXT("one forward row extends the bounded three-by-three set"), Cells.Num(), 12);
	for (int32 Y = -1; Y <= 1; ++Y)
	{
		TestTrue(TEXT("forward row is prefetched before the boundary"), Cells.Contains(FIntPoint(2, Y)));
	}
	TestFalse(TEXT("prefetch does not expand behind the player"), Cells.Contains(FIntPoint(-2, 0)));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersTerrainCellSeamTest,
	"Gaters.Worldgen.Streaming.Terrain.Seams",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersTerrainCellSeamTest::RunTest(const FString& Parameters)
{
	constexpr float CellSize = 10000.f;
	const FVector2D LeftEdge = AGatersTerrainCell::GlobalSamplePosition(
		FIntPoint(0, 0), FVector2D(CellSize * 0.5f, 1250.f), CellSize);
	const FVector2D RightEdge = AGatersTerrainCell::GlobalSamplePosition(
		FIntPoint(1, 0), FVector2D(-CellSize * 0.5f, 1250.f), CellSize);
	TestEqual(TEXT("adjacent meshes sample the exact same shared edge"), LeftEdge, RightEdge);

	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(53, 100000.f);
	const FVector LeftNormal = FGatersTerrainSemanticField::MaterializedNormalAt(
		Environment, LeftEdge, CellSize / 32.f, 1200.f, FVector2D(5000.f, 0.f));
	const FVector RightNormal = FGatersTerrainSemanticField::MaterializedNormalAt(
		Environment, RightEdge, CellSize / 32.f, 1200.f, FVector2D(5000.f, 0.f));
	TestEqual(TEXT("adjacent meshes derive the same shared-edge normal"), LeftNormal, RightNormal);
	TestTrue(TEXT("derived terrain normal remains normalized"), LeftNormal.IsNormalized());
	return true;
}

#endif
