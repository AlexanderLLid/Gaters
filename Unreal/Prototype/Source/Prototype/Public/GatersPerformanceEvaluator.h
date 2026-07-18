#pragma once

#include "CoreMinimal.h"

struct PROTOTYPE_API FGatersPerformanceSample
{
	double GenerationMs = 0.0;
	double MeanFrameMs = 0.0;
	double UsedPhysicalMB = 0.0;
	int32 TotalActors = 0;
	int32 ScatterActors = 0;
	int32 ClaimActors = 0;
	int32 BaseActors = 0;
	int32 LoadedTerrainCells = 0;
	int32 StaticMeshComponents = 0;
	int32 InstancedStaticMeshComponents = 0;
	int32 StaticMeshInstances = 0;
	int64 StaticMeshTriangles = 0;
	int64 DynamicMeshTriangles = 0;
	int64 Lod0Triangles = 0;
};

struct PROTOTYPE_API FGatersPerformanceBudget
{
	double MaxGenerationMs = TNumericLimits<double>::Max();
	double MaxMeanFrameMs = TNumericLimits<double>::Max();
	double MaxUsedPhysicalMB = TNumericLimits<double>::Max();
	int32 MaxActors = MAX_int32;
	int32 MaxTerrainCells = MAX_int32;
	int32 MaxInstances = MAX_int32;
	int32 MaxStaticMeshComponents = MAX_int32;
	int64 MaxLod0Triangles = TNumericLimits<int64>::Max();
};

struct PROTOTYPE_API FGatersPerformanceIssue
{
	FString RuleId;
	double Measured = 0.0;
	double Limit = 0.0;
};

struct PROTOTYPE_API FGatersPerformanceEvaluation
{
	bool IsWithinBudget() const { return Issues.IsEmpty(); }

	TArray<FGatersPerformanceIssue> Issues;
};

// Pure policy over an already measured runtime sample. Collection remains an Unreal
// adapter concern so fixtures can prove budget behavior without launching a world.
struct PROTOTYPE_API FGatersPerformanceEvaluator
{
	static FGatersPerformanceEvaluation Evaluate(
		const FGatersPerformanceSample& Sample,
		const FGatersPerformanceBudget& Budget);
	static FString Report(
		const FGatersPerformanceSample& Sample,
		const FGatersPerformanceEvaluation& Evaluation);
};
