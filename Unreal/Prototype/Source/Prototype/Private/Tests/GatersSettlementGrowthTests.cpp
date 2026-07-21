#if WITH_DEV_AUTOMATION_TESTS

#include "GatersSettlementGrowth.h"
#include "Misc/AutomationTest.h"

namespace
{
FGatersTerrainSemanticField MakeGrowthField()
{
	FGatersTerrainSemanticField Field;
	Field.CellsPerAxis = 31;
	Field.CellSize = 500.f;
	Field.Cells.SetNum(Field.CellsPerAxis * Field.CellsPerAxis);
	for (int32 X = 0; X < Field.CellsPerAxis; ++X)
	{
		for (int32 Y = 0; Y < Field.CellsPerAxis; ++Y)
		{
			FGatersTerrainSemanticSample& Sample = Field.Cells[X * Field.CellsPerAxis + Y];
			Sample.Type = EGatersTerrainSemantic::Flat;
			Sample.Height = X * 3.f + Y;
		}
	}
	return Field;
}

FGatersPlannedSite GrowthSite(const FGatersTerrainSemanticField& Field)
{
	const FIntPoint Cell(Field.CellsPerAxis / 2, Field.CellsPerAxis / 2);
	return {TEXT("site:village:0"), EGatersPlannedSiteKind::Village, Cell,
		FVector(0.f, 0.f, Field.At(Cell.X, Cell.Y).Height)};
}

bool HasGrowthIssue(const FGatersSettlementGrowthEvaluation& Evaluation, const TCHAR* RuleId)
{
	return Evaluation.Issues.ContainsByPredicate([RuleId](const FGatersSettlementIssue& Issue)
	{
		return Issue.RuleId == RuleId;
	});
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersSettlementGrowthPatchContractTest,
	"Gaters.Worldgen.SettlementGrowth.Contract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersSettlementGrowthPatchContractTest::RunTest(const FString& Parameters)
{
	const FGatersTerrainSemanticField Field = MakeGrowthField();
	const FGatersPlannedSite Site = GrowthSite(Field);
	const FGatersSettlementPlan Hamlet =
		FGatersSettlementGenerator::Generate(Field, 73, Site, 0);
	const FGatersSettlementGrowthPatch A =
		FGatersSettlementGrowthPlanner::Plan(Field, 73, Site, Hamlet, 1);
	const FGatersSettlementGrowthPatch B =
		FGatersSettlementGrowthPlanner::Plan(Field, 73, Site, Hamlet, 1);

	TestTrue(TEXT("valid source produces an expansion patch"), A.bPlanned);
	TestEqual(TEXT("growth patch is versioned"), A.Version, 1);
	TestEqual(TEXT("growth is one stage"), A.TargetStage, A.SourceStage + 1);
	TestTrue(TEXT("growth adds buildings"), A.AddedBuildings.Num() > 0);
	TestEqual(TEXT("same inputs reproduce the patch"), A.CanonicalText(), B.CanonicalText());

	const FGatersSettlementGrowthEvaluation Evaluation =
		FGatersSettlementGrowthEvaluator::Evaluate(Field, Hamlet, A);
	TestTrue(TEXT("independent evaluator accepts the patch"), Evaluation.IsValid());
	TestEqual(TEXT("expanded stage is recorded"), Evaluation.ExpandedPlan.GrowthStage, 1);
	TestEqual(TEXT("expanded building count matches the stage contract"),
		Evaluation.ExpandedPlan.Buildings.Num(),
		FGatersSettlementGenerator::ExpectedBuildingCount(1));
	bool bPrefixPreserved = true;
	for (int32 Index = 0; Index < Hamlet.Buildings.Num(); ++Index)
	{
		bPrefixPreserved &= Hamlet.Buildings[Index].Id
			== Evaluation.ExpandedPlan.Buildings[Index].Id;
	}
	TestTrue(TEXT("applying the patch preserves the source prefix"), bPrefixPreserved);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersSettlementGrowthCounterexamplesTest,
	"Gaters.Worldgen.SettlementGrowth.Counterexamples",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersSettlementGrowthCounterexamplesTest::RunTest(const FString& Parameters)
{
	const FGatersTerrainSemanticField Field = MakeGrowthField();
	const FGatersPlannedSite Site = GrowthSite(Field);
	const FGatersSettlementPlan Hamlet =
		FGatersSettlementGenerator::Generate(Field, 73, Site, 0);

	const FGatersSettlementGrowthPatch Skipped =
		FGatersSettlementGrowthPlanner::Plan(Field, 73, Site, Hamlet, 2);
	TestFalse(TEXT("a patch cannot skip a growth stage"), Skipped.bPlanned);
	TestTrue(TEXT("skipped stage has a causal diagnostic"),
		Skipped.Diagnostics.Contains(TEXT("growth must advance exactly one stage")));

	FGatersSettlementPlan Tampered = Hamlet;
	Tampered.Buildings[0].Cell.X += 1;
	const FGatersSettlementGrowthPatch FromTampered =
		FGatersSettlementGrowthPlanner::Plan(Field, 73, Site, Tampered, 1);
	TestFalse(TEXT("planner rejects a source that no longer matches its seed"),
		FromTampered.bPlanned);
	TestTrue(TEXT("source mismatch is diagnosed"),
		FromTampered.Diagnostics.Contains(TEXT("source plan does not match its deterministic stage")));

	FGatersSettlementGrowthPatch Duplicate =
		FGatersSettlementGrowthPlanner::Plan(Field, 73, Site, Hamlet, 1);
	Duplicate.AddedBuildings[0].Id = Hamlet.Buildings[0].Id;
	const FGatersSettlementGrowthEvaluation DuplicateEvaluation =
		FGatersSettlementGrowthEvaluator::Evaluate(Field, Hamlet, Duplicate);
	TestTrue(TEXT("duplicate addition is rejected causally"),
		HasGrowthIssue(DuplicateEvaluation, TEXT("settlement.growth.identity.duplicate")));

	FGatersSettlementGrowthPatch Disconnected =
		FGatersSettlementGrowthPlanner::Plan(Field, 73, Site, Hamlet, 1);
	Disconnected.AddedBuildings[0].EntranceCell = FIntPoint(-1, -1);
	const FGatersSettlementGrowthEvaluation DisconnectedEvaluation =
		FGatersSettlementGrowthEvaluator::Evaluate(Field, Hamlet, Disconnected);
	TestTrue(TEXT("unreachable expansion retains the settlement diagnostic"),
		HasGrowthIssue(DisconnectedEvaluation, TEXT("settlement.entrance.unreachable")));
	return true;
}

#endif
