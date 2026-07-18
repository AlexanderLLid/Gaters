#if WITH_DEV_AUTOMATION_TESTS

#include "GatersEnvironment.h"
#include "GatersTerrainSemanticField.h"
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

	TestEqual(TEXT("semantic contract is versioned"), A.Version, 1);
	TestEqual(TEXT("grid dimensions are retained"), A.CellsPerAxis, 61);
	TestEqual(TEXT("every grid cell has one sample"), A.Cells.Num(), 61 * 61);
	TestEqual(TEXT("same seed has the same sample count"), A.Cells.Num(), B.Cells.Num());
	for (int32 Index = 0; Index < A.Cells.Num(); ++Index)
	{
		TestEqual(TEXT("same seed has deterministic height"), A.Cells[Index].Height, B.Cells[Index].Height);
		TestEqual(TEXT("same seed has deterministic normal"), A.Cells[Index].NormalZ, B.Cells[Index].NormalZ);
		TestEqual(TEXT("same seed has deterministic semantics"), A.Cells[Index].Type, B.Cells[Index].Type);
	}

	const FGatersTerrainSemanticSample& Gate = A.At(30, 30);
	TestEqual(TEXT("materialized gate center is flat at zero"), Gate.Height, 0.f);
	TestEqual(TEXT("gate center terrain is classified flat"), Gate.Type, EGatersTerrainSemantic::Flat);

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

#endif
