#pragma once

#include "CoreMinimal.h"

struct FGatersEnvironmentRecipe;

struct PROTOTYPE_API FGatersSurfaceConditionSettings
{
	bool operator==(const FGatersSurfaceConditionSettings& Other) const = default;

	float NormalSampleDistance = 1000.f;
	float ReliefScale = 1200.f;
	float ShoreDistance = 3000.f;
	float AccumulationScale = 12.f;
	float CliffFullNormalZ = 0.45f;
	float CliffStartNormalZ = 0.82f;
};

struct PROTOTYPE_API FGatersSurfaceConditionEvidence
{
	bool operator==(const FGatersSurfaceConditionEvidence& Other) const = default;

	float Height = 0.f;
	float NormalZ = 1.f;
	float NeighborhoodMeanHeight = 0.f;
	float Temperature = 0.5f;
	float Precipitation = 0.f;
	float WindExposure = 0.f;
	float FreezeThaw = 0.f;
	float DrainageAccumulation = 0.f;
	float DrainageChannel = 0.f;
	float DrainageDrop = 0.f;
	float WaterProximity = 0.f;
	float ShoreProximity = 0.f;
	float WaterDepth = 0.f;
};

struct PROTOTYPE_API FGatersSurfaceConditionSample
{
	bool operator==(const FGatersSurfaceConditionSample& Other) const = default;

	static constexpr int32 CurrentVersion = 1;

	int32 Version = CurrentVersion;
	int32 RecipeVersion = 0;
	int32 Seed = 0;
	FVector2D Point = FVector2D::ZeroVector;
	FGatersSurfaceConditionEvidence Evidence;
	TArray<int32> DrainageCellIndices;
	TArray<float> DrainageCellWeights;
	float Soil = 0.f;
	float Rock = 0.f;
	float Sediment = 0.f;
	float Sand = 0.f;
	float Snow = 0.f;
	float Ice = 0.f;
	float Saturation = 0.f;
	float Shore = 0.f;
	float Ridge = 0.f;
	float Valley = 0.f;
	float Cliff = 0.f;
};

struct PROTOTYPE_API FGatersSurfaceConditionRecipe
{
	bool operator==(const FGatersSurfaceConditionRecipe& Other) const = default;

	static constexpr int32 CurrentVersion = 1;

	int32 Version = CurrentVersion;
	int32 Seed = 0;
	float WorldSize = 0.f;
	int32 EnvironmentVersion = 0;
	int32 ClimateVersion = 0;
	int32 DrainageVersion = 0;
	int32 RegionalWaterVersion = 0;
	FGatersSurfaceConditionSettings Settings;
};

struct PROTOTYPE_API FGatersSurfaceConditionIssue
{
	bool operator==(const FGatersSurfaceConditionIssue& Other) const = default;

	FString RuleId;
	FString Message;
};

struct PROTOTYPE_API FGatersSurfaceConditionCompileResult
{
	bool operator==(const FGatersSurfaceConditionCompileResult& Other) const = default;
	bool IsValid() const { return Issues.IsEmpty(); }

	FGatersSurfaceConditionRecipe Recipe;
	TArray<FGatersSurfaceConditionIssue> Issues;
};

// Pure physical surface evidence. It owns no biome names, resources, art, sites,
// encounters, terrain mutation, or runtime Actors.
struct PROTOTYPE_API FGatersSurfaceConditionField
{
	static FGatersSurfaceConditionCompileResult Compile(
		const FGatersEnvironmentRecipe& Environment,
		const FGatersSurfaceConditionSettings& Settings = {});

	static FGatersSurfaceConditionSample Evaluate(
		const FGatersSurfaceConditionEvidence& Evidence,
		const FGatersSurfaceConditionSettings& Settings = {});

	static FGatersSurfaceConditionSample Query(
		const FGatersSurfaceConditionRecipe& Recipe,
		const FGatersEnvironmentRecipe& Environment,
		const FVector2D& Point);
};
