#pragma once

#include "CoreMinimal.h"
#include "GatersEnvironmentBrief.h"
#include "GatersLandformProcessField.h"
#include "Templates/Function.h"

struct FGatersEnvironment;

struct PROTOTYPE_API FGatersClimateRecipe
{
	bool operator==(const FGatersClimateRecipe& Other) const = default;

	static constexpr int32 CurrentVersion = 1;

	int32 Version = CurrentVersion;
	int32 Seed = 0;
	float WorldSize = 0.f;
	FVector2D PrevailingWind = FVector2D(1.f, 0.f);
	float SeasonalityBias = 0.5f;
	FGatersLandformProcessRecipe Landform;
};

struct PROTOTYPE_API FGatersClimateTerrainEvidence
{
	bool operator==(const FGatersClimateTerrainEvidence& Other) const = default;

	float Height = 0.f;
	float UpwindHeight = 0.f;
	float NeighborhoodMeanHeight = 0.f;
	bool bHasWater = false;
	float WaterHeight = 0.f;
};

struct PROTOTYPE_API FGatersClimateSample
{
	bool operator==(const FGatersClimateSample& Other) const = default;

	float Temperature = 0.f;
	float Precipitation = 0.f;
	float WindExposure = 0.f;
	float Seasonality = 0.f;
	float FreezeThaw = 0.f;
	float RegionInfluence = 0.f;
	float Height = 0.f;
};

struct PROTOTYPE_API FGatersClimateIssue
{
	FString RuleId;
	FString Message;
};

struct PROTOTYPE_API FGatersClimateCompileResult
{
	bool IsValid() const { return Issues.IsEmpty(); }

	FGatersClimateRecipe Recipe;
	TArray<FGatersClimateIssue> Issues;
};

// Pure coordinate field. It emits physical climate evidence without choosing
// biomes, resources, assets, sites, or runtime materialization.
struct PROTOTYPE_API FGatersClimateField
{
	static FGatersClimateCompileResult Compile(
		const FGatersEnvironment& Environment,
		const FGatersCompiledEnvironmentBrief& Intent,
		const FGatersLandformProcessRecipe& Landform);

	static FGatersClimateSample Evaluate(
		const FGatersClimateRecipe& Recipe,
		const FGatersEnvironmentTargetProfile& Profile,
		const FGatersClimateTerrainEvidence& Evidence);

	static FGatersClimateSample Query(
		const FGatersClimateRecipe& Recipe,
		const FGatersEnvironment& Environment,
		const FVector2D& Point);

	static FGatersClimateSample Query(
		const FGatersClimateRecipe& Recipe,
		const FVector2D& Point,
		TFunctionRef<float(const FVector2D&)> HeightAt,
		bool bHasWater,
		float WaterHeight);
};
