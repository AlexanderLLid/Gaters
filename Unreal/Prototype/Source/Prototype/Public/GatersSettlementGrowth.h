#pragma once

#include "CoreMinimal.h"
#include "GatersSettlementEvaluator.h"

struct PROTOTYPE_API FGatersSettlementGrowthPatch
{
	FString CanonicalText() const;

	int32 Version = 1;
	bool bPlanned = false;
	int32 SourceStage = 0;
	int32 TargetStage = 0;
	FString SourceCanonicalText;
	TArray<FGatersSettlementParcel> AddedParcels;
	TArray<FGatersSettlementBuilding> AddedBuildings;
	TArray<FIntPoint> AddedPathCells;
	TArray<FString> Diagnostics;
};

struct PROTOTYPE_API FGatersSettlementGrowthEvaluation
{
	bool IsValid() const { return Issues.IsEmpty(); }

	int32 EvaluatorVersion = 1;
	FGatersSettlementPlan ExpandedPlan;
	TArray<FGatersSettlementIssue> Issues;
};

struct PROTOTYPE_API FGatersSettlementGrowthPlanner
{
	static FGatersSettlementGrowthPatch Plan(
		const FGatersTerrainSemanticField& Field,
		int32 Seed,
		const FGatersPlannedSite& VillageSite,
		const FGatersSettlementPlan& Existing,
		int32 TargetStage);
};

struct PROTOTYPE_API FGatersSettlementGrowthEvaluator
{
	static FGatersSettlementGrowthEvaluation Evaluate(
		const FGatersTerrainSemanticField& Field,
		const FGatersSettlementPlan& Existing,
		const FGatersSettlementGrowthPatch& Patch);
};
