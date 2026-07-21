#if WITH_DEV_AUTOMATION_TESTS

#include "GatersSettlementRecipeAdapter.h"
#include "Misc/AutomationTest.h"

namespace
{
FGatersTerrainSemanticField MakeRecipeAdapterField()
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
			Sample.Height = X * 2.f + Y;
		}
	}
	return Field;
}

FGatersPlannedSite RecipeAdapterSite(const FGatersTerrainSemanticField& Field)
{
	const FIntPoint Cell(Field.CellsPerAxis / 2, Field.CellsPerAxis / 2);
	return {TEXT("site:village:0"), EGatersPlannedSiteKind::Village, Cell,
		FVector(0.f, 0.f, Field.At(Cell.X, Cell.Y).Height)};
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersSettlementRecipeAdapterContractTest,
	"Gaters.Worldgen.SettlementRecipeAdapter.Contract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersSettlementRecipeAdapterContractTest::RunTest(const FString& Parameters)
{
	const FGatersTerrainSemanticField Field = MakeRecipeAdapterField();
	const FGatersPlannedSite Site = RecipeAdapterSite(Field);
	const FGatersSettlementPlan Hamlet =
		FGatersSettlementGenerator::Generate(Field, 73, Site, 0);
	const FGatersSettlementPlan Village =
		FGatersSettlementGenerator::Generate(Field, 73, Site, 1);
	const FGatersSettlementRecipeCompilation A =
		FGatersSettlementRecipeAdapter::Compile(Field, Hamlet);
	const FGatersSettlementRecipeCompilation B =
		FGatersSettlementRecipeAdapter::Compile(Field, Village);

	TestTrue(TEXT("accepted hamlet compiles to recipe nodes"), A.bCompiled);
	TestTrue(TEXT("accepted village compiles to recipe nodes"), B.bCompiled);
	TestEqual(TEXT("every building receives a valid assembly"),
		B.ValidAssemblyCount, Village.Buildings.Num());
	TestTrue(TEXT("compiled village contains modules"), B.ModuleCount > 0);
	TestEqual(TEXT("compiled village carries all parcel semantics"),
		B.Count(EGatersRecipeNodeKind::SettlementParcel), Village.Parcels.Num());
	TestEqual(TEXT("compiled village carries immutable growth fronts"),
		B.Count(EGatersRecipeNodeKind::SettlementGrowthFront), Village.GrowthFronts.Num());

	TSet<FString> VillageNodeIds;
	for (const FGatersRecipeNode& Node : B.Nodes)
	{
		VillageNodeIds.Add(Node.Id);
	}
	bool bPrefixPreserved = true;
	for (const FGatersRecipeNode& Node : A.Nodes)
	{
		bPrefixPreserved &= VillageNodeIds.Contains(Node.Id);
	}
	TestTrue(TEXT("larger stage preserves every earlier recipe identity"), bPrefixPreserved);
	for (const FIntPoint& Cell : Hamlet.PathCells)
	{
		TestTrue(TEXT("path recipe identity is coordinate-stable"),
			VillageNodeIds.Contains(FGatersSettlementPlan::StablePathId(Cell)));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersSettlementRecipeAdapterCounterexampleTest,
	"Gaters.Worldgen.SettlementRecipeAdapter.Counterexample",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersSettlementRecipeAdapterCounterexampleTest::RunTest(const FString& Parameters)
{
	const FGatersTerrainSemanticField Field = MakeRecipeAdapterField();
	FGatersSettlementPlan Invalid = FGatersSettlementGenerator::Generate(
		Field, 73, RecipeAdapterSite(Field), 0);
	Invalid.Parcels.Reset();
	const FGatersSettlementRecipeCompilation Compilation =
		FGatersSettlementRecipeAdapter::Compile(Field, Invalid);
	TestFalse(TEXT("adapter rejects a structurally invalid village"), Compilation.bCompiled);
	TestTrue(TEXT("adapter preserves causal settlement diagnostics"),
		Compilation.Diagnostics.ContainsByPredicate([](const FString& Diagnostic)
		{
			return Diagnostic.Contains(TEXT("settlement.parcel"));
		}));
	return true;
}

#endif
