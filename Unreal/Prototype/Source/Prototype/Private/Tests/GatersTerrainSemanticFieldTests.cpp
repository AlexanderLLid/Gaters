#if WITH_DEV_AUTOMATION_TESTS

#include "GatersEnvironment.h"
#include "GatersIntentTerrainField.h"
#include "GatersTerrainSemanticField.h"
#include "GatersWorldIntent.h"
#include "Misc/AutomationTest.h"

namespace
{
constexpr float ChunkSize = 30000.f;

FGatersEnvironment FindEnvironment(EGatersEnvironment Type)
{
	for (int32 Seed = 0; Seed < 256; ++Seed)
	{
		const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(Seed, ChunkSize);
		if (Environment.Type == Type)
		{
			return Environment;
		}
	}
	return {};
}

FGatersTerrainSemanticField BuildField(const FGatersEnvironment& Environment)
{
	return FGatersTerrainSemanticField::Build(Environment, 61, 500.f, 1000.f, 0.94f, 0.77f);
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersTerrainSemanticFieldTest,
	"Gaters.Worldgen.Environment.SemanticField",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersTerrainSemanticFieldTest::RunTest(const FString& Parameters)
{
	const FGatersTerrainSemanticField A = BuildField(FGatersEnvironment::FromSeed(73, ChunkSize));
	const FGatersTerrainSemanticField B = BuildField(FGatersEnvironment::FromSeed(73, ChunkSize));

	TestEqual(TEXT("semantic contract is versioned"), A.Version, 2);
	TestEqual(TEXT("grid dimensions are retained"), A.CellsPerAxis, 61);
	TestEqual(TEXT("every grid cell has one sample"), A.Cells.Num(), 61 * 61);
	TestEqual(TEXT("same seed has the same sample count"), A.Cells.Num(), B.Cells.Num());
	for (int32 Index = 0; Index < A.Cells.Num(); ++Index)
	{
		TestEqual(TEXT("same seed has deterministic height"), A.Cells[Index].Height, B.Cells[Index].Height);
		TestEqual(TEXT("same seed has deterministic normal"), A.Cells[Index].NormalZ, B.Cells[Index].NormalZ);
		TestEqual(TEXT("same seed has deterministic semantics"), A.Cells[Index].Type, B.Cells[Index].Type);
	}

	const FGatersTerrainSemanticSample& Arrival = A.At(30, 30);
	TestEqual(TEXT("materialized arrival center is flat at zero"), Arrival.Height, 0.f);
	TestEqual(TEXT("arrival center terrain is classified flat"), Arrival.Type, EGatersTerrainSemantic::Flat);
	const FGatersEnvironment Unmodified = FGatersEnvironment::FromSeed(73, ChunkSize);
	TestEqual(TEXT("zero pad radius preserves raw terrain at origin"),
		FGatersTerrainSemanticField::MaterializedHeightAt(
			Unmodified, FVector2D::ZeroVector, 0.f),
		Unmodified.HeightAt(FVector2D::ZeroVector));

	const int32 Classified = A.Count(EGatersTerrainSemantic::Flat)
		+ A.Count(EGatersTerrainSemantic::Slope)
		+ A.Count(EGatersTerrainSemantic::Steep)
		+ A.Count(EGatersTerrainSemantic::Water);
	TestEqual(TEXT("categories are exhaustive"), Classified, A.Cells.Num());

	const FGatersTerrainSemanticField Islands = BuildField(FindEnvironment(EGatersEnvironment::Archipelago));
	TestTrue(TEXT("archipelago exposes water semantics"), Islands.Count(EGatersTerrainSemantic::Water) > 0);
	TestTrue(TEXT("archipelago retains walkable terrain"), Islands.WalkableCount() > 0);

	const FGatersTerrainSemanticField Canyon = BuildField(FindEnvironment(EGatersEnvironment::Canyon));
	TestTrue(TEXT("canyon exposes steep semantics"), Canyon.Count(EGatersTerrainSemantic::Steep) > 0);
	TestTrue(TEXT("canyon retains walkable terrain"), Canyon.WalkableCount() > 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersTerrainSemanticIntentAdapterTest,
	"Gaters.Worldgen.Environment.SemanticFieldIntentAdapter",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersTerrainSemanticIntentAdapterTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(73, 400000.f);
	FGatersWorldIntentRecipe Intent = FGatersWorldIntentRecipe::Generate(73, 400000.f);
	const FVector2D Point = Intent.Regions[1].Center;
	const float Expected = FGatersIntentTerrainField::Query(Environment, Intent, Point).Height;
	TestEqual(TEXT("materialized height consumes explicit world intent"),
		FGatersTerrainSemanticField::MaterializedHeightAt(
			Environment, Intent, Point, 0.f), Expected);
	const FVector A = FGatersTerrainSemanticField::MaterializedNormalAt(
		Environment, Intent, Point, 100.f, 0.f);
	const FVector B = FGatersTerrainSemanticField::MaterializedNormalAt(
		Environment, Intent, Point, 100.f, 0.f);
	TestEqual(TEXT("intent-aware normal is deterministic"), A, B);

	FGatersWorldRegionIntent& Local = Intent.Regions[1];
	Local.Center = FVector2D(5000.f, 0.f);
	Local.Radius = 4000.f;
	Local.TerrainTendency = Environment.Type == EGatersEnvironment::Mountains
		? EGatersEnvironment::Lowlands : EGatersEnvironment::Mountains;
	Local.HydrologyTendency = EGatersHydrology::Dry;
	const FGatersTerrainSemanticField Field = FGatersTerrainSemanticField::Build(
		Environment, Intent, 61, 500.f, 0.f, 0.94f, 0.77f);
	const FGatersTerrainSemanticSample& RegionalCore = Field.At(40, 30);
	TestEqual(TEXT("semantic field samples the same regional terrain as rendering"),
		RegionalCore.Height,
		FGatersTerrainSemanticField::MaterializedHeightAt(
			Environment, Intent, Local.Center, 0.f));
	TestNotEqual(TEXT("regional semantic field does not fall back to global terrain"),
		RegionalCore.Height,
		FGatersTerrainSemanticField::MaterializedHeightAt(
			Environment, Local.Center, 0.f));
	return true;
}

#endif
