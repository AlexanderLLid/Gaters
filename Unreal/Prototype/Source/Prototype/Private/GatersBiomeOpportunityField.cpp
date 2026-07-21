#include "GatersBiomeOpportunityField.h"

#include "GatersEnvironmentRecipe.h"

FGatersBiomeOpportunityCompileResult FGatersBiomeOpportunityField::Compile(
	const FGatersEnvironmentRecipe& Environment)
{
	FGatersBiomeOpportunityCompileResult Result;
	Result.Recipe.Seed = Environment.Seed;
	Result.Recipe.WorldSize = Environment.WorldSize;
	Result.Recipe.EnvironmentVersion = Environment.Version;
	Result.Recipe.ClimateVersion = Environment.Climate.Version;
	Result.Recipe.SurfaceConditionVersion = Environment.SurfaceConditions.Version;
	if (Environment.Version != FGatersEnvironmentRecipe::CurrentVersion
		|| Environment.CompilerVersion
			!= FGatersEnvironmentRecipe::CurrentCompilerVersion
		|| !FMath::IsFinite(Environment.WorldSize)
		|| Environment.WorldSize <= 0.f
		|| Environment.Seed != Environment.Climate.Seed
		|| Environment.Seed != Environment.SurfaceConditions.Seed
		|| !FMath::IsNearlyEqual(
			Environment.WorldSize, Environment.Climate.WorldSize)
		|| !FMath::IsNearlyEqual(
			Environment.WorldSize, Environment.SurfaceConditions.WorldSize)
		|| Environment.Climate.Version != FGatersClimateRecipe::CurrentVersion
		|| Environment.SurfaceConditions.Version
			!= FGatersSurfaceConditionRecipe::CurrentVersion
		|| Environment.SurfaceConditions.EnvironmentVersion != Environment.Version
		|| Environment.SurfaceConditions.ClimateVersion != Environment.Climate.Version)
	{
		Result.Issues.Add({TEXT("opportunity.environment"),
			TEXT("Biome opportunities require matching accepted physical evidence.")});
	}
	return Result;
}

FGatersBiomeOpportunitySample FGatersBiomeOpportunityField::Evaluate(
	const FGatersBiomeSample& Biome,
	const FGatersClimateSample& Climate,
	const FGatersSurfaceConditionSample& Surface)
{
	const bool bWater = Biome.BiomeKey == TEXT("water")
		|| Surface.Evidence.WaterDepth > SMALL_NUMBER;
	const float Temperature = FMath::Clamp(Climate.Temperature, 0.f, 1.f);
	const float Precipitation = FMath::Clamp(Climate.Precipitation, 0.f, 1.f);
	const float ThermalSuitability = FMath::Clamp(
		1.f - FMath::Abs(Temperature - 0.55f) / 0.55f, 0.f, 1.f);
	const float GroundSupport = FMath::Clamp(
		Surface.Soil * 0.65f + Surface.Sediment * 0.2f
			+ Surface.Saturation * 0.15f,
		0.f,
		1.f);
	const float GrowthStress = FMath::Clamp(
		Surface.Rock * 0.35f + Surface.Cliff * 0.65f
			+ Surface.Snow * 0.6f + Surface.Ice,
		0.f,
		1.f);

	FGatersBiomeOpportunitySample Result;
	Result.Vegetation = bWater ? 0.f : FMath::Clamp(
		GroundSupport * (0.25f + Precipitation * 0.75f)
			* ThermalSuitability * (1.f - GrowthStress * 0.8f),
		0.f,
		1.f);
	Result.Stone = bWater ? 0.f : FMath::Clamp(
		Surface.Rock * 0.65f + Surface.Cliff * 0.35f
			+ Surface.Ridge * 0.15f - Surface.Sediment * 0.25f,
		0.f,
		1.f);
	Result.Landmark = FMath::Clamp(
		Surface.Cliff * 0.45f + Surface.Ridge * 0.3f
			+ Surface.Shore * 0.25f + Biome.Exposure * 0.1f,
		0.f,
		1.f);
	Result.TravelFriction = bWater ? 1.f : FMath::Clamp(
		Surface.Cliff * 0.55f + Surface.Ice * 0.45f
			+ Surface.Snow * 0.25f + Surface.Saturation * 0.25f
			+ Biome.Exposure * 0.1f,
		0.f,
		1.f);
	return Result;
}

FGatersBiomeOpportunitySample FGatersBiomeOpportunityField::Query(
	const FGatersBiomeSample& Biome)
{
	const bool bWater = Biome.BiomeKey == TEXT("water");
	const float Slope = 1.f - FMath::Clamp(Biome.NormalZ, 0.f, 1.f);
	const float Moisture = FMath::Clamp(Biome.Moisture, 0.f, 1.f);
	const float Exposure = FMath::Clamp(Biome.Exposure, 0.f, 1.f);
	const float WaterProximity = FMath::Clamp(Biome.WaterProximity, 0.f, 1.f);
	const float HighGround = Biome.ElevationBand == EGatersElevationBand::High ? 1.f : 0.f;

	FGatersBiomeOpportunitySample Result;
	Result.Vegetation = bWater ? 0.f : FMath::Clamp(
		Moisture * Biome.NormalZ * (1.f - Exposure * 0.65f), 0.f, 1.f);
	Result.Stone = bWater ? 0.f : FMath::Clamp(
		0.1f + Slope * 0.45f + Exposure * 0.45f + HighGround * 0.2f,
		0.f, 1.f);
	Result.Landmark = bWater ? 0.05f : FMath::Clamp(
		Exposure * 0.45f + WaterProximity * 0.3f + HighGround * 0.15f,
		0.f, 1.f);
	Result.TravelFriction = bWater ? 1.f : FMath::Clamp(
		Slope * 0.7f + Exposure * 0.15f + WaterProximity * 0.15f,
		0.f, 1.f);
	return Result;
}
