#include "GatersClimateField.h"

#include "GatersEnvironment.h"

namespace
{
float SeedUnit(const int32 Seed, const uint32 Salt)
{
	uint32 Hash = static_cast<uint32>(Seed) + 0x9e3779b9u * (Salt + 1u);
	Hash = (Hash ^ (Hash >> 16)) * 0x7feb352du;
	Hash = (Hash ^ (Hash >> 15)) * 0x846ca68bu;
	Hash ^= Hash >> 16;
	return (Hash & 0x00ffffffu) / static_cast<float>(0x01000000u);
}

void AddIssue(
	FGatersClimateCompileResult& Result,
	const TCHAR* RuleId,
	const TCHAR* Message)
{
	Result.Issues.Add({RuleId, Message});
}

bool SameWorldSize(const float A, const float B)
{
	return FMath::IsFinite(A) && FMath::IsFinite(B)
		&& FMath::IsNearlyEqual(A, B, 0.01f);
}
}

FGatersClimateCompileResult FGatersClimateField::Compile(
	const FGatersEnvironment& Environment,
	const FGatersCompiledEnvironmentBrief& Intent,
	const FGatersLandformProcessRecipe& Landform)
{
	FGatersClimateCompileResult Result;
	Result.Recipe.Seed = Intent.Seed;
	Result.Recipe.WorldSize = Intent.WorldSize;
	Result.Recipe.Landform = Landform;
	const float WindAngle = SeedUnit(Intent.Seed, 37u) * 2.f * PI;
	Result.Recipe.PrevailingWind = FVector2D(
		FMath::Cos(WindAngle), FMath::Sin(WindAngle));
	Result.Recipe.SeasonalityBias = FMath::Lerp(
		0.25f, 0.8f, SeedUnit(Intent.Seed, 38u));

	if (Intent.CompilerVersion != 2 || Intent.BriefVersion != 2)
	{
		AddIssue(Result, TEXT("climate.intent_version"),
			TEXT("Climate requires environment brief compiler and brief version 2."));
	}
	if (Landform.Version != FGatersLandformProcessRecipe::CurrentVersion)
	{
		AddIssue(Result, TEXT("climate.landform_version"),
			TEXT("Climate requires the current landform process recipe."));
	}
	const bool bWorldSizeValid = FMath::IsFinite(Intent.WorldSize)
		&& Intent.WorldSize > 0.f
		&& SameWorldSize(Intent.WorldSize, Environment.ChunkSize)
		&& SameWorldSize(Intent.WorldSize, Landform.WorldSize);
	const bool bProvenanceMatches = Intent.Seed == Environment.Seed
		&& Intent.Seed == Landform.Seed
		&& bWorldSizeValid
		&& Landform.bHasWater == Environment.HasWater()
		&& FMath::IsNearlyEqual(
			Landform.WaterHeight, Environment.WaterHeight, 0.01f)
		&& Landform.Global == Intent.Global
		&& Landform.Regions == Intent.Regions;
	if (!bProvenanceMatches)
	{
		AddIssue(Result, TEXT("climate.provenance"),
			TEXT("Environment intent and landform must describe the same seed and world."));
	}
	return Result;
}

FGatersClimateSample FGatersClimateField::Evaluate(
	const FGatersClimateRecipe& Recipe,
	const FGatersEnvironmentTargetProfile& Profile,
	const FGatersClimateTerrainEvidence& Evidence)
{
	FGatersClimateSample Result;
	Result.Height = Evidence.Height;
	const float Elevation = FMath::Clamp(
		FMath::Max(Evidence.Height, 0.f) / 6000.f, 0.f, 1.f);
	Result.Temperature = FMath::Clamp(
		Profile.Temperature - Elevation * 0.35f, 0.f, 1.f);

	const float OrographicRise = FMath::Clamp(
		(Evidence.Height - Evidence.UpwindHeight) / 3000.f, -1.f, 1.f);
	const float LocalWater = Evidence.bHasWater
		? 1.f - FMath::SmoothStep(
			Evidence.WaterHeight + 250.f,
			Evidence.WaterHeight + 2500.f,
			Evidence.Height)
		: 0.f;
	const float BasePrecipitation = Profile.Moisture * 0.72f
		+ Profile.SurfaceWater * 0.16f + LocalWater * 0.12f;
	Result.Precipitation = FMath::Clamp(
		BasePrecipitation + OrographicRise * 0.2f, 0.f, 1.f);

	const float RelativeHeight = FMath::Clamp(
		(Evidence.Height - Evidence.NeighborhoodMeanHeight) / 3000.f,
		-1.f, 1.f);
	Result.WindExposure = FMath::Clamp(
		0.35f + RelativeHeight * 0.35f + Elevation * 0.15f, 0.f, 1.f);
	Result.Seasonality = FMath::Clamp(
		Recipe.SeasonalityBias + (0.5f - Result.Temperature) * 0.15f
			+ (1.f - Profile.Moisture) * 0.08f - LocalWater * 0.18f,
		0.f, 1.f);

	const float FreezeBand = 1.f - FMath::Clamp(
		FMath::Abs(Result.Temperature - 0.42f) / 0.28f, 0.f, 1.f);
	Result.FreezeThaw = FMath::Clamp(
		FreezeBand * Result.Precipitation, 0.f, 1.f);
	return Result;
}

FGatersClimateSample FGatersClimateField::Query(
	const FGatersClimateRecipe& Recipe,
	const FGatersEnvironment& Environment,
	const FVector2D& Point)
{
	return Query(
		Recipe,
		Point,
		[&Environment](const FVector2D& SamplePoint)
		{
			return Environment.HeightAt(SamplePoint);
		},
		Environment.HasWater(),
		Environment.WaterHeight);
}

FGatersClimateSample FGatersClimateField::Query(
	const FGatersClimateRecipe& Recipe,
	const FVector2D& Point,
	TFunctionRef<float(const FVector2D&)> HeightAt,
	const bool bHasWater,
	const float WaterHeight)
{
	constexpr float UpwindDistance = 4000.f;
	constexpr float NeighborhoodDistance = 3000.f;
	FGatersClimateTerrainEvidence Evidence;
	Evidence.Height = HeightAt(Point);
	Evidence.UpwindHeight = HeightAt(
		Point - Recipe.PrevailingWind * UpwindDistance);
	Evidence.NeighborhoodMeanHeight = 0.f;
	for (const FVector2D Direction : {
		FVector2D(1.f, 0.f), FVector2D(-1.f, 0.f),
		FVector2D(0.f, 1.f), FVector2D(0.f, -1.f)})
	{
		Evidence.NeighborhoodMeanHeight += HeightAt(
			Point + Direction * NeighborhoodDistance) * 0.25f;
	}
	Evidence.bHasWater = bHasWater;
	Evidence.WaterHeight = WaterHeight;
	float RegionInfluence = 0.f;
	const FGatersEnvironmentTargetProfile Profile =
		FGatersLandformProcessField::QueryProfile(
			Recipe.Landform, Point, &RegionInfluence);
	FGatersClimateSample Result = Evaluate(Recipe, Profile, Evidence);
	Result.RegionInfluence = RegionInfluence;
	return Result;
}
