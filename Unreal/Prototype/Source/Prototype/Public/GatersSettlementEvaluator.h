#pragma once

#include "CoreMinimal.h"
#include "GatersSettlementGenerator.h"

struct PROTOTYPE_API FGatersSettlementIssue
{
	FString RuleId;
	FString SubjectId;
	FString Message;
};

struct PROTOTYPE_API FGatersSettlementEvaluation
{
	bool IsValid() const { return Issues.IsEmpty(); }
	FString Summary() const;

	int32 EvaluatorVersion = 3;
	int32 BuildingCount = 0;
	int32 HomeCount = 0;
	int32 CoveredRoleCount = 0;
	int32 ReachableEntranceCount = 0;
	int32 PathCellCount = 0;
	int32 OrientationBucketCount = 0;
	int32 FloorBucketCount = 0;
	int32 FootprintBucketCount = 0;
	float MaxBuildingRadiusCells = 0.f;
	TArray<FGatersSettlementIssue> Issues;
};

struct PROTOTYPE_API FGatersSettlementEvaluator
{
	static FGatersSettlementEvaluation Evaluate(
		const FGatersTerrainSemanticField& Field,
		const FGatersSettlementPlan& Plan);
};
