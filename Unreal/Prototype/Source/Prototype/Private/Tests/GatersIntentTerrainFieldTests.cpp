#if WITH_DEV_AUTOMATION_TESTS

#include "GatersIntentTerrainField.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersIntentTerrainFieldContractTest,
	"Gaters.Worldgen.IntentTerrainField.Contract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersIntentTerrainFieldContractTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(73, 400000.f);
	const FGatersWorldIntentRecipe Intent = FGatersWorldIntentRecipe::Generate(73, 400000.f);
	const FGatersIntentTerrainSample A = FGatersIntentTerrainField::Query(
		Environment, Intent, FVector2D::ZeroVector);
	const FGatersIntentTerrainSample B = FGatersIntentTerrainField::Query(
		Environment, Intent, FVector2D::ZeroVector);

	TestEqual(TEXT("same input reproduces the exact sample"), A, B);
	TestEqual(TEXT("arrival terrain remains the existing terrain"),
		A.Height, Environment.HeightAt(FVector2D::ZeroVector));
	TestEqual(TEXT("arrival remains under the global profile"),
		A.RegionId, Intent.Regions[0].Id);
	TestEqual(TEXT("regional influence is absent at arrival"), A.Influence, 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersIntentTerrainFieldRegionTest,
	"Gaters.Worldgen.IntentTerrainField.Region",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersIntentTerrainFieldRegionTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(73, 400000.f);
	FGatersWorldIntentRecipe Intent = FGatersWorldIntentRecipe::Generate(73, 400000.f);
	FGatersWorldRegionIntent& Region = Intent.Regions[1];
	Region.Center = FVector2D(80000.f, 0.f);
	Region.Radius = 30000.f;
	Region.TerrainTendency = Environment.Type == EGatersEnvironment::Mountains
		? EGatersEnvironment::Lowlands : EGatersEnvironment::Mountains;
	Region.HydrologyTendency = EGatersHydrology::Lakes;

	const FGatersIntentTerrainSample Core = FGatersIntentTerrainField::Query(
		Environment, Intent, Region.Center);
	TestEqual(TEXT("region core records its responsible profile"), Core.RegionId, Region.Id);
	TestEqual(TEXT("region core reaches full influence"), Core.Influence, 1.f);
	TestEqual(TEXT("region core consumes declared terrain"), Core.Terrain, Region.TerrainTendency);
	TestEqual(TEXT("region core consumes declared hydrology"), Core.Hydrology, Region.HydrologyTendency);
	TestNotEqual(TEXT("regional profile changes terrain height"),
		Core.Height, Environment.HeightAt(Region.Center));

	const FVector2D Direction(1.f, 0.f);
	const float Inside = FGatersIntentTerrainField::Query(
		Environment, Intent, Region.Center + Direction * (Region.Radius - 10.f)).Height;
	const float Outside = FGatersIntentTerrainField::Query(
		Environment, Intent, Region.Center + Direction * (Region.Radius + 10.f)).Height;
	TestTrue(TEXT("region boundary fades continuously"), FMath::Abs(Inside - Outside) < 100.f);
	return true;
}

#endif
