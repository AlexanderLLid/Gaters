#pragma once

#include "CoreMinimal.h"
#include "Templates/Function.h"

struct PROTOTYPE_API FGatersTerrainMorphologyEvaluation
{
	int32 EvaluatorVersion = 1;
	int32 ClosedContourCount = 0;
	float MaximumClosedContourCircularity = 0.f;
	int32 MostCircularContourSamples = 0;
	int32 LargestClosedContourSamples = 0;
	float LargestClosedContourCircularity = 0.f;
};

// Pure evidence over height contours. Policy decides whether a radial form is explained
// by the requested environment (for example, a crater or atoll).
struct PROTOTYPE_API FGatersTerrainMorphologyEvaluator
{
	static FGatersTerrainMorphologyEvaluation Evaluate(
		TFunctionRef<float(const FVector2D&)> HeightAt,
		float HalfExtent,
		const TArray<float>& ContourHeights);
};
