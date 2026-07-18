#pragma once

#include "CoreMinimal.h"
#include "GatersEnvironment.h"

struct PROTOTYPE_API FGatersTerrainEvaluation
{
	float Relief() const { return MaxHeight - MinHeight; }

	int32 EvaluatorVersion = 3;
	float MinHeight = 0.f;
	float MaxHeight = 0.f;
	float MeanHeight = 0.f;
	float BelowDatumFraction = 0.f;
	float WaterFraction = 0.f;
	float MeanNeighborStep = 0.f;
	float MaxNeighborStep = 0.f;
	float BuildableFraction = 0.f;
};

struct PROTOTYPE_API FGatersTerrainEvaluator
{
	static FGatersTerrainEvaluation Evaluate(
		const FGatersEnvironment& Environment,
		float EvaluationSize = 0.f);
};
