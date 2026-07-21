#pragma once

#include "CoreMinimal.h"
#include "Templates/Function.h"

struct FGatersEnvironment;

struct PROTOTYPE_API FGatersLandformResponseEvaluation
{
	bool IsValid() const { return Diagnostics.IsEmpty(); }

	int32 EvaluatorVersion = 1;
	float RmsHeightDifference = 0.f;
	float PositiveChangeFraction = 0.f;
	float NegativeChangeFraction = 0.f;
	float ReliefDelta = 0.f;
	float MaximumHeightDelta = 0.f;
	float MeanHeightDelta = 0.f;
	TArray<FString> Diagnostics;
};

struct PROTOTYPE_API FGatersLandformResponseEvaluator
{
	static FGatersLandformResponseEvaluation Evaluate(
		const FGatersEnvironment& Environment,
		TFunctionRef<float(const FVector2D&)> BaselineHeightAt,
		TFunctionRef<float(const FVector2D&)> ChallengerHeightAt,
		float EvaluationSize = 0.f);
};
