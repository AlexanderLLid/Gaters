#pragma once

#include "CoreMinimal.h"

struct PROTOTYPE_API FGatersContentDistributionObservation
{
	FString Id;
	float VegetationOpportunity = 0.f;
	float StoneOpportunity = 0.f;
	int32 TreeCount = 0;
	int32 RockCount = 0;
	int32 Capacity = 0;
};

struct PROTOTYPE_API FGatersContentDistributionSettings
{
	float MaxMeanDensityError = 0.20f;
	float MaxMeanKindMixError = 0.25f;
	float MinDensityCorrelation = 0.35f;
	int32 MinObservations = 8;
};

struct PROTOTYPE_API FGatersContentDistributionIssue
{
	FString RuleId;
	FString ObservationId;
	float Observed = 0.f;
	float Expected = 0.f;
	float Limit = 0.f;
};

struct PROTOTYPE_API FGatersContentDistributionEvaluation
{
	bool IsValid() const { return Issues.IsEmpty(); }

	int32 Version = 1;
	int32 ObservationCount = 0;
	float MeanDensityError = 0.f;
	float MeanKindMixError = 0.f;
	float DensityCorrelation = 0.f;
	TArray<FGatersContentDistributionIssue> Issues;
};

struct PROTOTYPE_API FGatersContentDistributionEvaluator
{
	static FGatersContentDistributionEvaluation Evaluate(
		const TArray<FGatersContentDistributionObservation>& Observations,
		const FGatersContentDistributionSettings& Settings = {});
};
