#pragma once

#include "CoreMinimal.h"
#include "GatersTerrainNavigation.h"

struct PROTOTYPE_API FGatersTraversabilityEvaluation
{
	int32 EvaluatorVersion = 1;
	FGatersTerrainRegion Region;
	FGatersTerrainPath GoalPath;
	float WalkableFraction = 0.f;
	float ReachableFraction = 0.f;
	bool bEscapesStart = false;
	bool bGoalReachable = false;
};

struct PROTOTYPE_API FGatersTraversabilityEvaluator
{
	static FGatersTraversabilityEvaluation Evaluate(
		const FGatersTerrainSemanticField& Field,
		const FIntPoint& Start,
		const FIntPoint& Goal,
		int32 EscapeDistanceCells);
};
