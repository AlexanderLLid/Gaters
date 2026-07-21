#if WITH_DEV_AUTOMATION_TESTS

#include "GatersBuildingEvaluator.h"
#include "GatersBuildingGenerator.h"
#include "Misc/AutomationTest.h"

namespace
{
FGatersTerrainSemanticField MakeBuildingField(
	const EGatersTerrainSemantic Fill = EGatersTerrainSemantic::Flat)
{
	FGatersTerrainSemanticField Field;
	Field.CellsPerAxis = 17;
	Field.CellSize = 500.f;
	Field.Cells.SetNum(Field.CellsPerAxis * Field.CellsPerAxis);
	for (int32 X = 0; X < Field.CellsPerAxis; ++X)
	{
		for (int32 Y = 0; Y < Field.CellsPerAxis; ++Y)
		{
			FGatersTerrainSemanticSample& Sample = Field.Cells[X * Field.CellsPerAxis + Y];
			Sample.Type = Fill;
			Sample.Height = 100.f;
		}
	}
	return Field;
}

FGatersSettlementBuilding BuildingFixture()
{
	FGatersSettlementBuilding Building;
	Building.Id = TEXT("settlement:building:home:0");
	Building.Role = EGatersSettlementRole::Home;
	Building.ContentKey = TEXT("settlement.home");
	Building.Cell = FIntPoint(8, 8);
	Building.EntranceCell = FIntPoint(9, 8);
	Building.Location = FVector(0.f, 0.f, 100.f);
	Building.Yaw = 0.f;
	Building.FootprintWidthCells = 2;
	Building.FootprintDepthCells = 1;
	Building.FloorCount = 2;
	return Building;
}

bool HasBuildingIssue(const FGatersBuildingEvaluation& Evaluation, const TCHAR* RuleId)
{
	return Evaluation.Issues.ContainsByPredicate([RuleId](const FGatersBuildingIssue& Issue)
	{
		return Issue.RuleId == RuleId;
	});
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersBuildingAssemblyContractTest,
	"Gaters.Worldgen.Buildings.Contract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersBuildingAssemblyContractTest::RunTest(const FString& Parameters)
{
	const FGatersTerrainSemanticField Field = MakeBuildingField();
	const FGatersSettlementBuilding Building = BuildingFixture();
	const FGatersBuildingAssembly A = FGatersBuildingGenerator::Generate(Field, Building);
	const FGatersBuildingAssembly B = FGatersBuildingGenerator::Generate(Field, Building);

	TestEqual(TEXT("building generator is versioned"), A.GeneratorVersion, 2);
	TestTrue(TEXT("valid anchor produces an assembly"), A.bGenerated);
	TestEqual(TEXT("same input preserves module identity"), A.CanonicalText(), B.CanonicalText());
	TestTrue(TEXT("assembly has a foundation"), A.Count(EGatersBuildingModuleKind::Foundation) == 1);
	const FGatersBuildingModule* Foundation = A.Modules.FindByPredicate(
		[](const FGatersBuildingModule& Module)
		{
			return Module.Kind == EGatersBuildingModuleKind::Foundation;
		});
	TestTrue(TEXT("foundation skirt extends below the terrain anchor"),
		Foundation && Foundation->Transform.GetLocation().Z < Building.Location.Z);
	TestTrue(TEXT("assembly has floor modules"), A.Count(EGatersBuildingModuleKind::Floor) > 0);
	TestTrue(TEXT("assembly has wall modules"), A.Count(EGatersBuildingModuleKind::Wall) > 0);
	TestEqual(TEXT("assembly has one semantic entrance"),
		A.Count(EGatersBuildingModuleKind::DoorWall), 1);
	TestTrue(TEXT("assembly has a roof"), A.Count(EGatersBuildingModuleKind::Roof) == 1);
	TestEqual(TEXT("assembly proves one usable volume per floor"),
		A.UsableSpaces.Num(), Building.FloorCount);
	TestEqual(TEXT("assembly proves one doorway opening"), A.Openings.Num(), 1);
	if (A.UsableSpaces.Num() > 0)
	{
		TestTrue(TEXT("usable volume has positive interior extent"),
			A.UsableSpaces[0].Extent.X > 0.f && A.UsableSpaces[0].Extent.Y > 0.f
			&& A.UsableSpaces[0].Extent.Z > 0.f);
		TestTrue(TEXT("usable volume has stable provenance"),
			A.UsableSpaces[0].SourceIds.Contains(Building.Id));
	}
	if (A.Openings.Num() > 0)
	{
		TestTrue(TEXT("doorway proves positive width and headroom"),
			A.Openings[0].Width > 0.f && A.Openings[0].Headroom > 0.f
			&& A.Openings[0].Depth > 0.f);
		TestTrue(TEXT("doorway names its source module"),
			A.Openings[0].SourceIds.Contains(A.Openings[0].SourceModuleId));
	}

	const FGatersBuildingEvaluation Evaluation =
		FGatersBuildingEvaluator::Evaluate(Field, A);
	TestEqual(TEXT("building evaluator is versioned"), Evaluation.EvaluatorVersion, 1);
	TestTrue(TEXT("valid assembly passes independent evaluation"), Evaluation.IsValid());
	TestEqual(TEXT("every module has stable identity"),
		Evaluation.UniqueModuleCount, A.Modules.Num());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersBuildingAssemblyCounterexampleTest,
	"Gaters.Worldgen.Buildings.Counterexamples",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersBuildingAssemblyCounterexampleTest::RunTest(const FString& Parameters)
{
	const FGatersTerrainSemanticField Field = MakeBuildingField();
	FGatersBuildingAssembly MissingDoor =
		FGatersBuildingGenerator::Generate(Field, BuildingFixture());
	MissingDoor.Modules.RemoveAll([](const FGatersBuildingModule& Module)
	{
		return Module.Kind == EGatersBuildingModuleKind::DoorWall;
	});
	TestTrue(TEXT("missing entrance is causal"), HasBuildingIssue(
		FGatersBuildingEvaluator::Evaluate(Field, MissingDoor), TEXT("building.entrance.missing")));

	FGatersBuildingAssembly Duplicate =
		FGatersBuildingGenerator::Generate(Field, BuildingFixture());
	const FGatersBuildingModule DuplicateModule = Duplicate.Modules[0];
	Duplicate.Modules.Add(DuplicateModule);
	TestTrue(TEXT("duplicate module identity is causal"), HasBuildingIssue(
		FGatersBuildingEvaluator::Evaluate(Field, Duplicate), TEXT("building.identity.duplicate")));

	FGatersBuildingAssembly InvalidTransform =
		FGatersBuildingGenerator::Generate(Field, BuildingFixture());
	InvalidTransform.Modules[0].Transform.SetScale3D(FVector(0.f, 1.f, 1.f));
	TestTrue(TEXT("invalid module transform is causal"), HasBuildingIssue(
		FGatersBuildingEvaluator::Evaluate(Field, InvalidTransform),
		TEXT("building.transform.invalid")));

	FGatersBuildingAssembly Unsupported =
		FGatersBuildingGenerator::Generate(Field, BuildingFixture());
	Unsupported.Modules.RemoveAll([](const FGatersBuildingModule& Module)
	{
		return Module.Kind == EGatersBuildingModuleKind::Floor;
	});
	TestTrue(TEXT("unsupported upper storey is causal"), HasBuildingIssue(
		FGatersBuildingEvaluator::Evaluate(Field, Unsupported), TEXT("building.support.missing")));

	FGatersBuildingAssembly Overlapped =
		FGatersBuildingGenerator::Generate(Field, BuildingFixture());
	const FGatersBuildingModule* Wall = Overlapped.Modules.FindByPredicate(
		[](const FGatersBuildingModule& Module)
		{
			return Module.Kind == EGatersBuildingModuleKind::Wall;
		});
	if (!TestNotNull(TEXT("overlap fixture has a wall"), Wall))
	{
		return false;
	}
	FGatersBuildingModule OverlapModule = *Wall;
	OverlapModule.Id += TEXT(":overlap");
	Overlapped.Modules.Add(OverlapModule);
	TestTrue(TEXT("exact module overlap is causal"), HasBuildingIssue(
		FGatersBuildingEvaluator::Evaluate(Field, Overlapped), TEXT("building.module.overlap")));

	FGatersBuildingAssembly MissingUsableSpace =
		FGatersBuildingGenerator::Generate(Field, BuildingFixture());
	MissingUsableSpace.UsableSpaces.Reset();
	TestTrue(TEXT("missing usable volume is causal"), HasBuildingIssue(
		FGatersBuildingEvaluator::Evaluate(Field, MissingUsableSpace),
		TEXT("building.space.missing")));

	FGatersBuildingAssembly InvalidOpening =
		FGatersBuildingGenerator::Generate(Field, BuildingFixture());
	InvalidOpening.Openings[0].Width = 0.f;
	TestTrue(TEXT("invalid opening clearance is causal"), HasBuildingIssue(
		FGatersBuildingEvaluator::Evaluate(Field, InvalidOpening),
		TEXT("building.opening.invalid")));

	FGatersTerrainSemanticField Blocked = MakeBuildingField();
	Blocked.Cells[8 * Blocked.CellsPerAxis + 8].Type = EGatersTerrainSemantic::Steep;
	const FGatersBuildingAssembly Rejected =
		FGatersBuildingGenerator::Generate(Blocked, BuildingFixture());
	TestFalse(TEXT("blocked terrain rejects building generation"), Rejected.bGenerated);
	return true;
}

#endif
