#if WITH_DEV_AUTOMATION_TESTS

#include "GatersWorldIntent.h"
#include "GatersEnvironment.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersWorldIntentContractTest,
	"Gaters.Worldgen.WorldIntent.Contract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersWorldIntentContractTest::RunTest(const FString& Parameters)
{
	const FGatersWorldIntentRecipe A = FGatersWorldIntentRecipe::Generate(73, 400000.f);
	const FGatersWorldIntentRecipe B = FGatersWorldIntentRecipe::Generate(73, 400000.f);
	const FGatersWorldIntentRecipe Other = FGatersWorldIntentRecipe::Generate(74, 400000.f);
	TestEqual(TEXT("intent contract is versioned"), A.Version, 2);
	TestEqual(TEXT("one global and two regional profiles are declared"), A.Regions.Num(), 3);
	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(73, 400000.f);
	TestEqual(TEXT("global intent preserves the existing terrain family"),
		A.Regions[0].TerrainTendency, Environment.Type);
	TestEqual(TEXT("global intent preserves the existing hydrology"),
		A.Regions[0].HydrologyTendency, Environment.Hydrology);
	TestEqual(TEXT("same seed reproduces exact intent"), A, B);
	TestNotEqual(TEXT("different seed changes intent"), A, Other);

	TArray<FString> Errors;
	TestTrue(TEXT("generated intent validates"), A.Validate(Errors));
	TestEqual(TEXT("valid intent has no diagnostics"), Errors.Num(), 0);
	for (const FGatersWorldRegionIntent& Region : A.Regions)
	{
		TestFalse(TEXT("region identity is present"), Region.Id.IsEmpty());
		TestTrue(TEXT("region radius is positive"), Region.Radius > 0.f);
		TestTrue(TEXT("vegetation opportunity is bounded"),
			Region.VegetationOpportunity >= 0.f && Region.VegetationOpportunity <= 1.f);
		TestTrue(TEXT("stone opportunity is bounded"),
			Region.StoneOpportunity >= 0.f && Region.StoneOpportunity <= 1.f);
		TestTrue(TEXT("landmark opportunity is bounded"),
			Region.LandmarkOpportunity >= 0.f && Region.LandmarkOpportunity <= 1.f);
		TestTrue(TEXT("travel friction is bounded"),
			Region.TravelFriction >= 0.f && Region.TravelFriction <= 1.f);
	}

	const FVector2D Point(87000.f, -42000.f);
	TestEqual(TEXT("coordinate query is stable"), A.At(Point), B.At(Point));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersWorldIntentSparseWorldTest,
	"Gaters.Worldgen.WorldIntent.SparseWorld",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersWorldIntentSparseWorldTest::RunTest(const FString& Parameters)
{
	bool bFoundSparseRegion = false;
	for (int32 Seed = 0; Seed < 256 && !bFoundSparseRegion; ++Seed)
	{
		const FGatersWorldIntentRecipe Intent =
			FGatersWorldIntentRecipe::Generate(Seed, 400000.f);
		TArray<FString> Errors;
		if (!Intent.Validate(Errors))
		{
			AddError(FString::Printf(TEXT("seed %d intent is invalid"), Seed));
			return false;
		}
		for (const FGatersWorldRegionIntent& Region : Intent.Regions)
		{
			bFoundSparseRegion |= Region.VegetationOpportunity <= 0.1f;
		}
	}
	TestTrue(TEXT("valid seed intent may declare almost no vegetation"), bFoundSparseRegion);
	return true;
}

#endif
