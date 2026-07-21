#pragma once

#include "CoreMinimal.h"
#include "GatersEnvironmentBrief.h"

struct FGatersEnvironment;

struct PROTOTYPE_API FGatersLandformProtectedRegion
{
	bool operator==(const FGatersLandformProtectedRegion& Other) const = default;

	FString Id;
	FVector2D Center = FVector2D::ZeroVector;
	float InnerRadius = 0.f;
	float OuterRadius = 0.f;
};

struct PROTOTYPE_API FGatersLandformProcessRecipe
{
	bool operator==(const FGatersLandformProcessRecipe& Other) const = default;

	static constexpr int32 CurrentVersion = 6;

	int32 Version = CurrentVersion;
	int32 Seed = 0;
	int32 CandidateIndex = 0;
	float FeatureScale = 1.f;
	float DissectionScale = 0.f;
	float RuggednessScale = 0.f;
	float WorldSize = 0.f;
	bool bHasWater = false;
	float WaterHeight = 0.f;
	FGatersEnvironmentTargetProfile Global;
	TArray<FGatersCompiledEnvironmentRegion> Regions;
	TArray<FGatersLandformProtectedRegion> ProtectedRegions;
};

struct PROTOTYPE_API FGatersLandformProcessSample
{
	bool operator==(const FGatersLandformProcessSample& Other) const = default;

	float BaseHeight = 0.f;
	float ReliefContribution = 0.f;
	float UpliftContribution = 0.f;
	float VolcanicContribution = 0.f;
	float GlacialContribution = 0.f;
	float DissectionContribution = 0.f;
	float RuggednessContribution = 0.f;
	float RegionInfluence = 0.f;
	float ProcessInfluence = 1.f;
	float Height = 0.f;
};

struct PROTOTYPE_API FGatersLandformProcessIssue
{
	FString RuleId;
	FString Message;
};

struct PROTOTYPE_API FGatersLandformProcessCompileResult
{
	bool IsValid() const { return Issues.IsEmpty(); }

	FGatersLandformProcessRecipe Recipe;
	TArray<FGatersLandformProcessIssue> Issues;
};

// Pure challenger field. It composes physical processes over an accepted base height
// but owns no Actors, assets, sites, water simulation, or runtime promotion decision.
struct PROTOTYPE_API FGatersLandformProcessField
{
	static FGatersLandformProcessCompileResult Compile(
		const FGatersEnvironment& Environment,
		const FGatersCompiledEnvironmentBrief& Intent,
		const TArray<FGatersLandformProtectedRegion>& ProtectedRegions = {},
		int32 CandidateIndex = 0);
	static FGatersEnvironmentTargetProfile QueryProfile(
		const FGatersLandformProcessRecipe& Recipe,
		const FVector2D& Point,
		float* OutRegionInfluence = nullptr);

	static FGatersLandformProcessSample Query(
		const FGatersLandformProcessRecipe& Recipe,
		const FVector2D& Point,
		float BaseHeight);
};
