#pragma once

#include "CoreMinimal.h"
#include "GatersBiomeField.h"
#include "GatersClimateField.h"
#include "GatersSurfaceConditionField.h"

struct FGatersEnvironmentRecipe;

struct PROTOTYPE_API FGatersBiomeOpportunitySample
{
	bool operator==(const FGatersBiomeOpportunitySample& Other) const = default;

	float Vegetation = 0.f;
	float Stone = 0.f;
	float Landmark = 0.f;
	float TravelFriction = 0.f;
};

struct PROTOTYPE_API FGatersBiomeOpportunityRecipe
{
	bool operator==(const FGatersBiomeOpportunityRecipe& Other) const = default;

	static constexpr int32 CurrentVersion = 2;

	int32 Version = CurrentVersion;
	int32 Seed = 0;
	float WorldSize = 0.f;
	int32 EnvironmentVersion = 0;
	int32 ClimateVersion = 0;
	int32 SurfaceConditionVersion = 0;
};

struct PROTOTYPE_API FGatersBiomeOpportunityIssue
{
	bool operator==(const FGatersBiomeOpportunityIssue& Other) const = default;

	FString RuleId;
	FString Message;
};

struct PROTOTYPE_API FGatersBiomeOpportunityCompileResult
{
	bool operator==(const FGatersBiomeOpportunityCompileResult& Other) const = default;
	bool IsValid() const { return Issues.IsEmpty(); }

	FGatersBiomeOpportunityRecipe Recipe;
	TArray<FGatersBiomeOpportunityIssue> Issues;
};

// Converts biome evidence into neutral opportunity weights. It does not decide assets,
// resource items, placement counts, routes, or sites.
struct PROTOTYPE_API FGatersBiomeOpportunityField
{
	static FGatersBiomeOpportunityCompileResult Compile(
		const FGatersEnvironmentRecipe& Environment);

	static FGatersBiomeOpportunitySample Evaluate(
		const FGatersBiomeSample& Biome,
		const FGatersClimateSample& Climate,
		const FGatersSurfaceConditionSample& Surface);

	static FGatersBiomeOpportunitySample Query(const FGatersBiomeSample& Biome);
};
