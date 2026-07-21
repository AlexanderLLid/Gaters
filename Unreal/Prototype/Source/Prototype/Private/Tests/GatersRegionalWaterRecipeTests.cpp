#if WITH_DEV_AUTOMATION_TESTS

#include "GatersRegionalWaterRecipe.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersRegionalWaterRecipeTest,
	"Gaters.Worldgen.RegionalWater.Recipe",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersRegionalWaterRecipeTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(73, 400000.f);
	FGatersWorldIntentRecipe Intent = FGatersWorldIntentRecipe::Generate(73, 400000.f);
	for (int32 Index = 1; Index < Intent.Regions.Num(); ++Index)
	{
		Intent.Regions[Index].HydrologyTendency = EGatersHydrology::Dry;
	}
	const FGatersRegionalWaterRecipe Dry = FGatersRegionalWaterRecipe::Generate(
		Environment, Intent);
	TestEqual(TEXT("regional water contract is versioned"), Dry.Version, 1);
	TestTrue(TEXT("dry regional intent emits no water"), Dry.Surfaces.IsEmpty());

	Intent.Regions[1].TerrainTendency = EGatersEnvironment::Lowlands;
	Intent.Regions[1].HydrologyTendency = EGatersHydrology::Lakes;
	Intent.Regions[2].TerrainTendency = EGatersEnvironment::Archipelago;
	Intent.Regions[2].HydrologyTendency = EGatersHydrology::Ocean;
	const FGatersRegionalWaterRecipe A = FGatersRegionalWaterRecipe::Generate(
		Environment, Intent);
	const FGatersRegionalWaterRecipe B = FGatersRegionalWaterRecipe::Generate(
		Environment, Intent);
	TestEqual(TEXT("same inputs reproduce exact regional water"), A, B);
	TestEqual(TEXT("two lake footprints plus one ocean footprint"), A.Surfaces.Num(), 3);

	TSet<FString> Ids;
	for (const FGatersRegionalWaterSurface& Surface : A.Surfaces)
	{
		TestFalse(TEXT("regional water has stable identity"), Surface.Id.IsEmpty());
		TestFalse(TEXT("regional water identity is unique"), Ids.Contains(Surface.Id));
		Ids.Add(Surface.Id);
		TestTrue(TEXT("regional water has positive bounded extent"),
			Surface.HalfExtent > 0.f && Surface.HalfExtent <= Surface.RegionRadius * 0.5f);
		TestTrue(TEXT("regional water remains inside its region"),
			FVector2D::Distance(Surface.Center, Surface.RegionCenter) + Surface.HalfExtent
				<= Surface.RegionRadius + 1.f);
	}
	return true;
}

#endif
