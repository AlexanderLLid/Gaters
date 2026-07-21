#include "GatersBiomeField.h"

#include "GatersIntentTerrainField.h"
#include "GatersTerrainSemanticField.h"

namespace
{
float IsWet(const FGatersEnvironment& Environment, const FVector2D& Point)
{
	return Environment.HasWater() &&
		Environment.HeightAt(Point) <= Environment.WaterHeight + 20.f ? 1.f : 0.f;
}

template <typename WetFunction>
float WaterProximity(
	const FVector2D& Point,
	const float SearchRadius,
	WetFunction&& WetAt)
{
	if (SearchRadius <= 0.f)
	{
		return WetAt(Point);
	}

	constexpr int32 RingCount = 4;
	constexpr int32 DirectionCount = 12;
	float Result = WetAt(Point);
	for (int32 Ring = 1; Ring <= RingCount; ++Ring)
	{
		const float Distance = SearchRadius * Ring / RingCount;
		for (int32 Direction = 0; Direction < DirectionCount; ++Direction)
		{
			const float Angle = 2.f * PI * Direction / DirectionCount;
			const FVector2D Sample = Point + FVector2D(
				FMath::Cos(Angle), FMath::Sin(Angle)) * Distance;
			Result = FMath::Max(
				Result, WetAt(Sample) * (1.f - Distance / SearchRadius));
		}
	}
	return Result;
}

float LocalVariation(const FGatersEnvironment& Environment, const FVector2D& Point)
{
	const FVector2D SeedOffset(
		Environment.Seed * 137.3f, Environment.Seed * -91.7f);
	return FMath::PerlinNoise2D((Point + SeedOffset) * 0.00008f) * 0.5f + 0.5f;
}

FString Classify(
	const FGatersEnvironment& Environment,
	const FGatersBiomeSample& Sample,
	const bool bWet)
{
	if (bWet)
	{
		return TEXT("water");
	}
	if (Sample.WaterProximity >= 0.5f)
	{
		return Environment.Type == EGatersEnvironment::Canyon
			? TEXT("riverbank") : TEXT("shore");
	}
	if (Sample.ElevationBand == EGatersElevationBand::High)
	{
		return TEXT("alpine");
	}
	if (Environment.Type == EGatersEnvironment::Canyon)
	{
		return TEXT("badlands");
	}
	if (Environment.Type == EGatersEnvironment::Archipelago)
	{
		return TEXT("island-upland");
	}
	if (Sample.Moisture >= 0.58f && Sample.NormalZ >= 0.75f)
	{
		return TEXT("woodland");
	}
	return Sample.ElevationBand == EGatersElevationBand::Mid
		? TEXT("highland") : TEXT("grassland");
}

FGatersBiomeSample CompleteSample(
	const FGatersEnvironment& SeedEnvironment,
	const FGatersEnvironment& ClassificationEnvironment,
	const FVector2D& Point,
	const float Height,
	const float NormalZ,
	const float InWaterProximity,
	const float HydrologyStrength,
	const bool bWet)
{
	FGatersBiomeSample Result;
	Result.Height = Height;
	Result.NormalZ = FMath::Clamp(NormalZ, 0.f, 1.f);
	Result.WaterProximity = FMath::Clamp(InWaterProximity, 0.f, 1.f);
	const float Variation = LocalVariation(SeedEnvironment, Point);
	Result.Moisture = FMath::Clamp(
		0.16f + HydrologyStrength * 0.16f + Variation * 0.28f
			+ Result.WaterProximity * 0.55f,
		0.f, 1.f);
	const float ElevationExposure = FMath::Clamp((Result.Height - 300.f) / 2600.f, 0.f, 1.f);
	Result.Exposure = FMath::Clamp(
		(1.f - Result.NormalZ) * 0.55f + ElevationExposure * 0.35f
			+ (1.f - Variation) * 0.1f,
		0.f, 1.f);
	Result.ElevationBand = Result.Height >= 1800.f
		? EGatersElevationBand::High
		: Result.Height >= 400.f
			? EGatersElevationBand::Mid
			: EGatersElevationBand::Low;
	Result.BiomeKey = Classify(ClassificationEnvironment, Result, bWet);
	return Result;
}
}

FGatersBiomeSample FGatersBiomeField::Query(
	const FGatersEnvironment& Environment,
	const FVector2D& Point,
	const FGatersBiomeQuery& Query)
{
	check(Query.NormalSampleDistance > 0.f);
	check(Query.WaterSearchRadius >= 0.f);

	const float Height = FGatersTerrainSemanticField::MaterializedHeightAt(
		Environment, Point, Query.PadRadius, Query.RouteTarget);
	const float NormalZ = FGatersTerrainSemanticField::MaterializedNormalAt(
			Environment, Point, Query.NormalSampleDistance,
			Query.PadRadius, Query.RouteTarget).Z;
	const auto WetAt = [&Environment](const FVector2D& Sample)
	{
		return IsWet(Environment, Sample);
	};
	return CompleteSample(
		Environment, Environment, Point, Height, NormalZ,
		WaterProximity(Point, Query.WaterSearchRadius, WetAt),
		Environment.HasWater() ? 1.f : 0.f, WetAt(Point) >= 0.5f);
}

FGatersBiomeSample FGatersBiomeField::Query(
	const FGatersEnvironment& Environment,
	const FGatersWorldIntentRecipe& Intent,
	const FVector2D& Point,
	const FGatersBiomeQuery& Query)
{
	check(Query.NormalSampleDistance > 0.f);
	check(Query.WaterSearchRadius >= 0.f);
	const FGatersIntentTerrainSample Terrain = FGatersIntentTerrainField::Query(
		Environment, Intent, Point);
	const FGatersEnvironment RegionalEnvironment = Environment.WithProfile(
		Terrain.Terrain, Terrain.Hydrology);
	const auto WetAt = [&Environment, &Intent](const FVector2D& Sample)
	{
		const FGatersIntentTerrainSample LocalTerrain = FGatersIntentTerrainField::Query(
			Environment, Intent, Sample);
		const FGatersEnvironment LocalEnvironment = Environment.WithProfile(
			LocalTerrain.Terrain, LocalTerrain.Hydrology);
		const float GlobalWet = IsWet(Environment, Sample);
		const float RegionalWet = LocalEnvironment.HasWater()
			&& LocalTerrain.Height <= LocalEnvironment.WaterHeight + 20.f ? 1.f : 0.f;
		return FMath::Lerp(GlobalWet, RegionalWet, LocalTerrain.Influence);
	};
	const float Height = FGatersTerrainSemanticField::MaterializedHeightAt(
		Environment, Intent, Point, Query.PadRadius, Query.RouteTarget);
	const float NormalZ = FGatersTerrainSemanticField::MaterializedNormalAt(
		Environment, Intent, Point, Query.NormalSampleDistance,
		Query.PadRadius, Query.RouteTarget).Z;
	const float HydrologyStrength = FMath::Lerp(
		Environment.HasWater() ? 1.f : 0.f,
		RegionalEnvironment.HasWater() ? 1.f : 0.f,
		Terrain.Influence);
	FGatersBiomeSample Result = CompleteSample(
		Environment,
		Terrain.Influence >= 0.5f ? RegionalEnvironment : Environment,
		Point, Height, NormalZ,
		WaterProximity(Point, Query.WaterSearchRadius, WetAt),
		HydrologyStrength, WetAt(Point) >= 0.5f);
	Result.IntentVersion = Intent.Version;
	Result.IntentRegionId = Terrain.RegionId;
	Result.IntentInfluence = Terrain.Influence;
	return Result;
}
