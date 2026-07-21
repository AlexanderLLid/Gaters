#pragma once

#include "CoreMinimal.h"
#include "GatersEnvironment.h"
#include "GatersWorldIntent.h"

struct PROTOTYPE_API FGatersIntentTerrainObservation
{
	FVector2D Point = FVector2D::ZeroVector;
	float Height = 0.f;
};

struct PROTOTYPE_API FGatersIntentTerrainFidelityEvaluation
{
	int32 Version = 1;
	bool bValid = false;
	int32 CoveredRegionCount = 0;
	float MaxHeightError = 0.f;
	TArray<FString> Diagnostics;
};

// Consumes external observations; it never invokes the terrain generator under test.
struct PROTOTYPE_API FGatersIntentTerrainFidelityEvaluator
{
	static FGatersIntentTerrainFidelityEvaluation Evaluate(
		const FGatersEnvironment& Environment,
		const FGatersWorldIntentRecipe& Intent,
		const TArray<FGatersIntentTerrainObservation>& Observations,
		float HeightTolerance = 0.01f);
};
