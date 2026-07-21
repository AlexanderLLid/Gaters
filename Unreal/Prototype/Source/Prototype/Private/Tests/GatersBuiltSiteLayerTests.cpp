#if WITH_DEV_AUTOMATION_TESTS

#include "GatersBuiltSiteLayer.h"
#include "GatersSettlementEvaluator.h"
#include "GatersSettlementRecipeAdapter.h"
#include "Misc/AutomationTest.h"

namespace
{
FGatersTerrainSemanticField MakeBuiltSiteLayerField()
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

FGatersSiteRoutePlan MakeBuiltSiteLayerSites(const FGatersTerrainSemanticField& Field)
{
	const FIntPoint Cell(Field.CellsPerAxis / 2, Field.CellsPerAxis / 2);
	FGatersSiteRoutePlan Sites;
	Sites.bValid = true;
	Sites.Sites.Add({
		TEXT("site:village:0"),
		EGatersPlannedSiteKind::Village,
		Cell,
		FVector(0.f, 0.f, Field.At(Cell.X, Cell.Y).Height)});
	return Sites;
}

bool SameNode(const FGatersRecipeNode& A, const FGatersRecipeNode& B)
{
	return A.Id == B.Id && A.Kind == B.Kind && A.Location == B.Location
		&& A.Rotation == B.Rotation && A.Scale == B.Scale
		&& A.ContentKey == B.ContentKey;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersBuiltSiteLayerContractTest,
	"Gaters.Worldgen.BuiltSiteLayer.Contract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersBuiltSiteLayerContractTest::RunTest(const FString& Parameters)
{
	const FGatersBuiltSiteLayerResult Empty;
	TestEqual(TEXT("layer contract is versioned"), Empty.ContractVersion, 1);
	TestTrue(TEXT("default layer is valid and empty"), Empty.IsValid() && Empty.IsEmpty());

	const FGatersTerrainSemanticField Field = MakeBuiltSiteLayerField();
	const FGatersSiteRoutePlan Sites = MakeBuiltSiteLayerSites(Field);
	const FGatersPlannedSite& Site = Sites.Sites[0];
	const FGatersSettlementPlan LegacyPlan =
		FGatersSettlementGenerator::Generate(Field, 73, Site, 1);
	const FGatersSettlementEvaluation LegacyEvaluation =
		FGatersSettlementEvaluator::Evaluate(Field, LegacyPlan);
	const FGatersSettlementRecipeCompilation Legacy =
		FGatersSettlementRecipeAdapter::Compile(Field, LegacyPlan);
	TestTrue(TEXT("legacy comparison fixture is accepted"),
		LegacyEvaluation.IsValid() && Legacy.bCompiled);

	const FGatersBuiltSiteLayerResult A =
		FGatersBuiltSiteLayer::Generate(Field, 73, Sites, 1);
	const FGatersBuiltSiteLayerResult B =
		FGatersBuiltSiteLayer::Generate(Field, 73, Sites, 1);
	TestTrue(TEXT("accepted settlement produces a valid layer"), A.IsValid());
	TestFalse(TEXT("accepted settlement produces recipe nodes"), A.IsEmpty());
	TestEqual(TEXT("settlement generator version is recorded"),
		A.SettlementGeneratorVersion, LegacyPlan.GeneratorVersion);
	TestEqual(TEXT("settlement evaluator version is recorded"),
		A.SettlementEvaluatorVersion, LegacyEvaluation.EvaluatorVersion);
	TestEqual(TEXT("one accepted source site is counted"), A.SiteCount, 1);
	TestEqual(TEXT("building count is preserved"), A.BuildingCount, LegacyPlan.Buildings.Num());
	TestEqual(TEXT("parcel count is preserved"), A.ParcelCount, LegacyPlan.Parcels.Num());
	TestEqual(TEXT("path count is preserved"), A.PathCount, LegacyPlan.PathCells.Num());
	TestEqual(TEXT("valid assembly count is preserved"),
		A.ValidAssemblyCount, Legacy.ValidAssemblyCount);
	TestEqual(TEXT("module count is preserved"), A.ModuleCount, Legacy.ModuleCount);
	TestEqual(TEXT("accepted settlement exposes one semantic site recipe"),
		A.SiteRecipes.Num(), 1);
	TestEqual(TEXT("semantic site recipe preserves the source site"),
		A.SiteRecipes[0].SiteId, Site.Id);
	TestEqual(TEXT("layer preserves legacy recipe node count"),
		A.Nodes.Num(), Legacy.Nodes.Num());
	TArray<FString> ExpectedSourceIds{Site.Id};
	for (const FGatersSettlementBuilding& Building : LegacyPlan.Buildings)
	{
		ExpectedSourceIds.Add(Building.Id);
	}
	for (const FGatersSettlementParcel& Parcel : LegacyPlan.Parcels)
	{
		ExpectedSourceIds.Add(Parcel.Id);
	}
	for (const FGatersSettlementGrowthFront& Front : LegacyPlan.GrowthFronts)
	{
		ExpectedSourceIds.Add(Front.Id);
	}
	for (const FIntPoint& Cell : LegacyPlan.PathCells)
	{
		ExpectedSourceIds.Add(FGatersSettlementPlan::StablePathId(Cell));
	}
	TestTrue(TEXT("layer preserves every legacy source id in order"),
		A.SourceIds == ExpectedSourceIds);

	bool bLegacyNodesPreserved = A.Nodes.Num() == Legacy.Nodes.Num();
	for (int32 Index = 0; bLegacyNodesPreserved && Index < A.Nodes.Num(); ++Index)
	{
		bLegacyNodesPreserved = SameNode(A.Nodes[Index], Legacy.Nodes[Index]);
	}
	TestTrue(TEXT("layer preserves every legacy node fact"), bLegacyNodesPreserved);

	bool bDeterministic = A.Nodes.Num() == B.Nodes.Num() && A.SourceIds == B.SourceIds
		&& A.SiteRecipes.Num() == B.SiteRecipes.Num()
		&& A.SiteRecipes[0].Checksum() == B.SiteRecipes[0].Checksum();
	for (int32 Index = 0; bDeterministic && Index < A.Nodes.Num(); ++Index)
	{
		bDeterministic = SameNode(A.Nodes[Index], B.Nodes[Index]);
	}
	TestTrue(TEXT("fixed inputs reproduce the layer"), bDeterministic);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersBuiltSiteLayerOptionalTest,
	"Gaters.Worldgen.BuiltSiteLayer.Optional",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersBuiltSiteLayerOptionalTest::RunTest(const FString& Parameters)
{
	const FGatersTerrainSemanticField Field = MakeBuiltSiteLayerField();
	FGatersSiteRoutePlan NoVillage;
	NoVillage.bValid = true;
	const FGatersBuiltSiteLayerResult Empty =
		FGatersBuiltSiteLayer::Generate(Field, 73, NoVillage, 0);
	TestTrue(TEXT("missing optional site produces a valid empty layer"),
		Empty.IsValid() && Empty.IsEmpty());
	TestEqual(TEXT("empty layer has no source sites"), Empty.SiteCount, 0);
	TestTrue(TEXT("empty layer has no source ids or diagnostics"),
		Empty.SourceIds.IsEmpty() && Empty.Diagnostics.IsEmpty());
	TestTrue(TEXT("empty layer has no semantic site recipes"), Empty.SiteRecipes.IsEmpty());
	TestTrue(TEXT("empty layer records no settlement work"),
		Empty.SettlementGeneratorVersion == 0
		&& Empty.SettlementEvaluatorVersion == 0
		&& Empty.BuildingCount == 0
		&& Empty.ParcelCount == 0
		&& Empty.PathCount == 0
		&& Empty.ValidAssemblyCount == 0
		&& Empty.ModuleCount == 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersBuiltSiteLayerCounterexampleTest,
	"Gaters.Worldgen.BuiltSiteLayer.Counterexample",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersBuiltSiteLayerCounterexampleTest::RunTest(const FString& Parameters)
{
	const FGatersTerrainSemanticField Field = MakeBuiltSiteLayerField();
	const FGatersBuiltSiteLayerResult Invalid = FGatersBuiltSiteLayer::Generate(
		Field, 73, MakeBuiltSiteLayerSites(Field), 3);
	TestFalse(TEXT("unsupported stage is rejected"), Invalid.IsValid());
	TestTrue(TEXT("rejected layer emits no recipe nodes"), Invalid.IsEmpty());
	TestTrue(TEXT("unsupported stage preserves a causal diagnostic"),
		Invalid.Diagnostics.Contains(TEXT("growth stage is outside the supported contract")));
	return true;
}

#endif
