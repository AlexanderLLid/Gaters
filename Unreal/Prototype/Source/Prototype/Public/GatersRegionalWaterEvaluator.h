#pragma once

#include "CoreMinimal.h"
#include "GatersEnvironment.h"
#include "GatersRegionalWaterRecipe.h"
#include "GatersWorldIntent.h"
#include "Templates/Function.h"

struct PROTOTYPE_API FGatersRegionalWaterEvaluation
{
	int32 Version = 1;
	bool bValid = false;
	int32 ExpectedSurfaceCount = 0;
	int32 SubmergedSurfaceCount = 0;
	TArray<FString> Diagnostics;
};

struct PROTOTYPE_API FGatersRegionalWaterEvaluator
{
	static FGatersRegionalWaterEvaluation Evaluate(
		const FGatersEnvironment& Environment,
		const FGatersWorldIntentRecipe& Intent,
		const FGatersRegionalWaterRecipe& Recipe,
		float DatumTolerance = 1.f);

	static FGatersRegionalWaterEvaluation Evaluate(
		const FGatersEnvironment& Environment,
		const FGatersWorldIntentRecipe& Intent,
		const FGatersRegionalWaterRecipe& Recipe,
		TFunctionRef<float(const FVector2D&)> HeightAt,
		float DatumTolerance = 1.f);
};
