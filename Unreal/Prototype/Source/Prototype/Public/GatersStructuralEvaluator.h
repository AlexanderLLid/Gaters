#pragma once

#include "CoreMinimal.h"
#include "GatersAssetContract.h"

struct FGatersWorldRecipe;

struct PROTOTYPE_API FGatersStructuralLimits
{
	int32 MaxNodes = MAX_int32;
};

struct PROTOTYPE_API FGatersStructuralIssue
{
	FString RuleId;
	FString Message;
	TArray<FString> NodeIds;
	TArray<FString> LinkIds;
	bool bHasMeasurement = false;
	double Measured = 0.0;
	double Limit = 0.0;
};

struct PROTOTYPE_API FGatersStructuralContext
{
	FGatersStructuralLimits Limits;
	TMap<FString, FGatersAssetContract> AssetContracts;
	float PortToleranceCm = 1.f;
	float OverlapToleranceCm = 1.f;
	bool bValidateAssetContracts = true;
};

struct PROTOTYPE_API FGatersStructuralEvaluation
{
	bool IsValid() const { return Issues.IsEmpty(); }

	TArray<FGatersStructuralIssue> Issues;
};

// Pure structural policy. It reports causal, stable diagnostics and never repairs,
// materializes, samples terrain, or loads content.
struct PROTOTYPE_API FGatersStructuralEvaluator
{
	static FGatersStructuralEvaluation Evaluate(
		const FGatersWorldRecipe& Recipe,
		const FGatersStructuralLimits& Limits = FGatersStructuralLimits());

	static FGatersStructuralEvaluation Evaluate(
		const FGatersWorldRecipe& Recipe,
		const FGatersStructuralContext& Context);
};
