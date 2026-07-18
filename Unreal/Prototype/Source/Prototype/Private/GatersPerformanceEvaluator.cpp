#include "GatersPerformanceEvaluator.h"

FGatersPerformanceEvaluation FGatersPerformanceEvaluator::Evaluate(
	const FGatersPerformanceSample& Sample,
	const FGatersPerformanceBudget& Budget)
{
	FGatersPerformanceEvaluation Result;
	auto Check = [&Result](const TCHAR* RuleId, const double Measured, const double Limit)
	{
		if (Measured > Limit)
		{
			Result.Issues.Add({RuleId, Measured, Limit});
		}
	};
	Check(TEXT("performance.generation_budget"), Sample.GenerationMs, Budget.MaxGenerationMs);
	Check(TEXT("performance.frame_budget"), Sample.MeanFrameMs, Budget.MaxMeanFrameMs);
	Check(TEXT("performance.memory_budget"), Sample.UsedPhysicalMB, Budget.MaxUsedPhysicalMB);
	Check(TEXT("performance.actor_budget"), Sample.TotalActors, Budget.MaxActors);
	Check(TEXT("performance.terrain_cell_budget"), Sample.LoadedTerrainCells, Budget.MaxTerrainCells);
	Check(TEXT("performance.instance_budget"), Sample.StaticMeshInstances, Budget.MaxInstances);
	Check(TEXT("performance.static_component_budget"),
		Sample.StaticMeshComponents, Budget.MaxStaticMeshComponents);
	Check(TEXT("performance.triangle_budget"), Sample.Lod0Triangles, Budget.MaxLod0Triangles);
	return Result;
}

FString FGatersPerformanceEvaluator::Report(
	const FGatersPerformanceSample& Sample,
	const FGatersPerformanceEvaluation& Evaluation)
{
	return FString::Printf(
		TEXT("PERF v=2 generation=%.1f frame=%.3f memory=%.1f actors=%d scatter=%d claims=%d base=%d cells=%d static_components=%d instanced_components=%d instances=%d static_tris=%lld dynamic_tris=%lld lod0_triangles=%lld valid=%s issues=%d"),
		Sample.GenerationMs, Sample.MeanFrameMs, Sample.UsedPhysicalMB,
		Sample.TotalActors, Sample.ScatterActors, Sample.ClaimActors,
		Sample.BaseActors, Sample.LoadedTerrainCells, Sample.StaticMeshComponents,
		Sample.InstancedStaticMeshComponents, Sample.StaticMeshInstances,
		static_cast<long long>(Sample.StaticMeshTriangles),
		static_cast<long long>(Sample.DynamicMeshTriangles),
		static_cast<long long>(Sample.Lod0Triangles),
		Evaluation.IsWithinBudget() ? TEXT("yes") : TEXT("no"), Evaluation.Issues.Num());
}
