#if WITH_DEV_AUTOMATION_TESTS

#include "GatersBiomeField.h"
#include "GatersIntentTerrainField.h"
#include "GatersWorldIntent.h"
#include "Misc/AutomationTest.h"

namespace
{
FGatersEnvironment FindWetEnvironment(FVector2D& OutWetPoint)
{
	for (int32 Seed = 0; Seed < 256; ++Seed)
	{
		const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(Seed, 200000.f);
		if (!Environment.HasWater())
		{
			continue;
		}
		for (int32 Ring = 0; Ring < 5; ++Ring)
		{
			const float Distance = 40000.f + Ring * 10000.f;
			for (int32 Direction = 0; Direction < 32; ++Direction)
			{
				const float Angle = 2.f * PI * Direction / 32.f;
				const FVector2D Point(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance);
				if (Environment.HeightAt(Point) <= Environment.WaterHeight + 20.f)
				{
					OutWetPoint = Point;
					return Environment;
				}
			}
		}
	}
	return {};
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersBiomeFieldContractTest,
	"Gaters.Worldgen.Biomes.Contract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersBiomeFieldContractTest::RunTest(const FString& Parameters)
{
	const TArray<int32> Seeds = {0, 2, 4, 7, 53};
	TSet<FString> SeenKeys;
	for (const int32 Seed : Seeds)
	{
		const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(Seed, 24000.f);
		for (const FVector2D Point : {
			FVector2D(0.f, 0.f), FVector2D(4200.f, -1700.f),
			FVector2D(-6800.f, 5100.f), FVector2D(11200.f, 3400.f)})
		{
			const FGatersBiomeSample A = FGatersBiomeField::Query(Environment, Point);
			const FGatersBiomeSample B = FGatersBiomeField::Query(Environment, Point);
			TestEqual(TEXT("same query is deterministic"), A, B);
			TestTrue(TEXT("normal is bounded"), A.NormalZ >= 0.f && A.NormalZ <= 1.f);
			TestTrue(TEXT("water proximity is bounded"),
				A.WaterProximity >= 0.f && A.WaterProximity <= 1.f);
			TestTrue(TEXT("moisture is bounded"), A.Moisture >= 0.f && A.Moisture <= 1.f);
			TestTrue(TEXT("exposure is bounded"), A.Exposure >= 0.f && A.Exposure <= 1.f);
			TestFalse(TEXT("biome key is present"), A.BiomeKey.IsEmpty());
			SeenKeys.Add(A.BiomeKey);
		}
	}
	TestTrue(TEXT("held-out worlds produce non-vacuous biome variety"), SeenKeys.Num() >= 4);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersBiomeFieldSpatialTest,
	"Gaters.Worldgen.Biomes.SpatialContinuity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersBiomeFieldSpatialTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(2, 24000.f);
	const FVector2D Boundary(12000.f, 1700.f);
	const FGatersBiomeSample Left = FGatersBiomeField::Query(
		Environment, Boundary - FVector2D(1.f, 0.f));
	const FGatersBiomeSample Right = FGatersBiomeField::Query(
		Environment, Boundary + FVector2D(1.f, 0.f));
	TestTrue(TEXT("height does not seam at a streaming-cell edge"),
		FMath::Abs(Left.Height - Right.Height) < 20.f);
	TestTrue(TEXT("moisture does not seam at a streaming-cell edge"),
		FMath::Abs(Left.Moisture - Right.Moisture) < 0.02f);
	TestTrue(TEXT("exposure does not seam at a streaming-cell edge"),
		FMath::Abs(Left.Exposure - Right.Exposure) < 0.02f);

	const FGatersEnvironment LakeWorld = FGatersEnvironment::FromSeed(53, 24000.f);
	const TArray<FGatersWaterSurface> Surfaces = LakeWorld.WaterSurfaces();
	TestTrue(TEXT("lake fixture exposes a surface"), Surfaces.Num() > 0);
	if (Surfaces.Num() > 0)
	{
		const FGatersBiomeSample Wet = FGatersBiomeField::Query(LakeWorld, Surfaces[0].Center);
		const FGatersBiomeSample Far = FGatersBiomeField::Query(
			LakeWorld, Surfaces[0].Center + FVector2D(9000.f, 0.f));
		TestTrue(TEXT("water evidence increases moisture nearby"), Wet.Moisture > Far.Moisture);
		TestTrue(TEXT("water proximity distinguishes lake from distant land"),
			Wet.WaterProximity > Far.WaterProximity);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersBiomeFieldIntentTest,
	"Gaters.Worldgen.Biomes.RegionalIntent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersBiomeFieldIntentTest::RunTest(const FString& Parameters)
{
	FVector2D WetPoint = FVector2D::ZeroVector;
	const FGatersEnvironment Environment = FindWetEnvironment(WetPoint);
	FGatersWorldIntentRecipe Intent = FGatersWorldIntentRecipe::Generate(
		Environment.Seed, Environment.ChunkSize);
	FGatersWorldRegionIntent& DryRegion = Intent.Regions[1];
	DryRegion.Center = WetPoint;
	DryRegion.Radius = 20000.f;
	DryRegion.TerrainTendency = EGatersEnvironment::Lowlands;
	DryRegion.HydrologyTendency = EGatersHydrology::Dry;
	Intent.Regions[2].Center = WetPoint + FVector2D(100000.f, 100000.f);

	const FGatersBiomeSample GlobalArrival = FGatersBiomeField::Query(
		Environment, FVector2D::ZeroVector);
	const FGatersBiomeSample IntentArrival = FGatersBiomeField::Query(
		Environment, Intent, FVector2D::ZeroVector);
	TestEqual(TEXT("regional intent preserves arrival height"),
		IntentArrival.Height, GlobalArrival.Height);
	TestEqual(TEXT("regional intent preserves arrival moisture"),
		IntentArrival.Moisture, GlobalArrival.Moisture);
	TestEqual(TEXT("regional intent preserves arrival biome"),
		IntentArrival.BiomeKey, GlobalArrival.BiomeKey);

	const FGatersBiomeSample GlobalWet = FGatersBiomeField::Query(Environment, WetPoint);
	const FGatersBiomeSample RegionalDry = FGatersBiomeField::Query(
		Environment, Intent, WetPoint);
	const FGatersIntentTerrainSample Terrain = FGatersIntentTerrainField::Query(
		Environment, Intent, WetPoint);
	TestEqual(TEXT("biome records responsible intent version"),
		RegionalDry.IntentVersion, Intent.Version);
	TestEqual(TEXT("biome records responsible intent region"),
		RegionalDry.IntentRegionId, DryRegion.Id);
	TestEqual(TEXT("regional core reaches full intent influence"),
		RegionalDry.IntentInfluence, 1.f);
	TestEqual(TEXT("biome consumes intent-aware height"), RegionalDry.Height, Terrain.Height);
	TestTrue(TEXT("dry regional core suppresses global water evidence"),
		RegionalDry.WaterProximity < GlobalWet.WaterProximity);
	TestTrue(TEXT("dry regional core is not classified as water"),
		RegionalDry.BiomeKey != TEXT("water"));

	const FVector2D Annulus = DryRegion.Center + FVector2D(DryRegion.Radius * 0.8f, 0.f);
	const FGatersBiomeSample Left = FGatersBiomeField::Query(
		Environment, Intent, Annulus - FVector2D(1.f, 0.f));
	const FGatersBiomeSample Right = FGatersBiomeField::Query(
		Environment, Intent, Annulus + FVector2D(1.f, 0.f));
	TestTrue(TEXT("intent biome height is continuous across the blend annulus"),
		FMath::Abs(Left.Height - Right.Height) < 20.f);
	TestTrue(TEXT("intent biome moisture is continuous across the blend annulus"),
		FMath::Abs(Left.Moisture - Right.Moisture) < 0.02f);
	return true;
}

#endif
