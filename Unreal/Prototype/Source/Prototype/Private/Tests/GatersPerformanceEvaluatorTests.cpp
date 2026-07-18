#if WITH_DEV_AUTOMATION_TESTS

#include "GatersPerformanceEvaluator.h"
#include "Misc/AutomationTest.h"

namespace
{
const FGatersPerformanceIssue* FindIssue(
	const FGatersPerformanceEvaluation& Evaluation, const TCHAR* RuleId)
{
	return Evaluation.Issues.FindByPredicate([RuleId](const FGatersPerformanceIssue& Issue)
	{
		return Issue.RuleId == RuleId;
	});
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersPerformanceEvaluatorTest,
	"Gaters.Evaluation.Performance.Budgets",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersPerformanceEvaluatorTest::RunTest(const FString& Parameters)
{
	FGatersPerformanceBudget Budget;
	Budget.MaxGenerationMs = 5000.0;
	Budget.MaxMeanFrameMs = 50.0;
	Budget.MaxActors = 2000;
	Budget.MaxTerrainCells = 100;
	Budget.MaxInstances = 1000;
	Budget.MaxStaticMeshComponents = 1000;
	Budget.MaxLod0Triangles = 1000000;

	FGatersPerformanceSample Cheap;
	Cheap.GenerationMs = 1800.0;
	Cheap.MeanFrameMs = 16.0;
	Cheap.UsedPhysicalMB = 1200.0;
	Cheap.TotalActors = 900;
	Cheap.ScatterActors = 200;
	Cheap.ClaimActors = 300;
	Cheap.BaseActors = 40;
	Cheap.LoadedTerrainCells = 81;
	Cheap.StaticMeshInstances = 500;
	Cheap.StaticMeshComponents = 400;
	Cheap.InstancedStaticMeshComponents = 2;
	Cheap.StaticMeshTriangles = 100000;
	Cheap.DynamicMeshTriangles = 50000;
	Cheap.Lod0Triangles = 150000;
	TestTrue(TEXT("representative world stays within budget"),
		FGatersPerformanceEvaluator::Evaluate(Cheap, Budget).IsWithinBudget());

	FGatersPerformanceSample Excessive = Cheap;
	Excessive.GenerationMs = 6000.0;
	Excessive.MeanFrameMs = 80.0;
	Excessive.TotalActors = 4000;
	Excessive.LoadedTerrainCells = 121;
	Excessive.StaticMeshInstances = 2000;
	Excessive.StaticMeshComponents = 1500;
	Excessive.Lod0Triangles = 2000000;
	const FGatersPerformanceEvaluation Failed =
		FGatersPerformanceEvaluator::Evaluate(Excessive, Budget);
	TestFalse(TEXT("intentionally excessive world fails"), Failed.IsWithinBudget());
	const FGatersPerformanceIssue* ActorIssue = FindIssue(Failed, TEXT("performance.actor_budget"));
	TestNotNull(TEXT("actor overflow has a causal rule"), ActorIssue);
	if (ActorIssue)
	{
		TestEqual(TEXT("actor overflow records measurement"), ActorIssue->Measured, 4000.0);
		TestEqual(TEXT("actor overflow records limit"), ActorIssue->Limit, 2000.0);
	}
	TestNotNull(TEXT("generation overflow is reported"),
		FindIssue(Failed, TEXT("performance.generation_budget")));
	TestNotNull(TEXT("frame overflow is reported"),
		FindIssue(Failed, TEXT("performance.frame_budget")));
	TestNotNull(TEXT("terrain-cell overflow is reported"),
		FindIssue(Failed, TEXT("performance.terrain_cell_budget")));
	TestNotNull(TEXT("instance overflow is reported"),
		FindIssue(Failed, TEXT("performance.instance_budget")));
	TestNotNull(TEXT("static-component overflow is reported"),
		FindIssue(Failed, TEXT("performance.static_component_budget")));
	TestNotNull(TEXT("triangle overflow is reported"),
		FindIssue(Failed, TEXT("performance.triangle_budget")));
	TestEqual(TEXT("runtime report is stable and machine-readable"),
		FGatersPerformanceEvaluator::Report(Excessive, Failed),
		FString(TEXT("PERF v=2 generation=6000.0 frame=80.000 memory=1200.0 actors=4000 scatter=200 claims=300 base=40 cells=121 static_components=1500 instanced_components=2 instances=2000 static_tris=100000 dynamic_tris=50000 lod0_triangles=2000000 valid=no issues=7")));
	return true;
}

#endif
