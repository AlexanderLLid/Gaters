#if WITH_DEV_AUTOMATION_TESTS

#include "GatersWorldRecipeLayer.h"
#include "Misc/AutomationTest.h"

namespace
{
FGatersWorldRecipe LayerBaseRecipe()
{
	return FGatersWorldRecipe::Generate(7, 200000.f, 6000.f, 10800.f, 900.f, 180.f);
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersWorldRecipeLayerContractTest,
	"Gaters.Worldgen.WorldRecipeLayer.Contract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersWorldRecipeLayerContractTest::RunTest(const FString& Parameters)
{
	FGatersWorldRecipe Recipe = LayerBaseRecipe();
	const int32 BaseCount = Recipe.Nodes.Num();
	const uint32 BaseChecksum = Recipe.Checksum();

	const FGatersWorldRecipeLayer Empty;
	const FGatersWorldRecipeLayerComposition EmptyResult =
		FGatersWorldRecipeLayerComposer::Append(Recipe, Empty);
	TestTrue(TEXT("empty optional layer is accepted"), EmptyResult.bComposed);
	TestEqual(TEXT("empty layer changes no node count"), Recipe.Nodes.Num(), BaseCount);
	TestEqual(TEXT("empty layer changes no recipe identity"), Recipe.Checksum(), BaseChecksum);

	FGatersWorldRecipeLayer Sites;
	Sites.LayerId = TEXT("built-sites");
	Sites.SchemaVersion = 1;
	Sites.GeneratorVersion = 2;
	Sites.bGenerated = true;
	Sites.Nodes.Add({TEXT("test:site:0"), EGatersRecipeNodeKind::VillageSite,
		FVector(1000.f, 2000.f, 50.f)});
	const FGatersWorldRecipeLayerComposition ValidResult =
		FGatersWorldRecipeLayerComposer::Append(Recipe, Sites);
	TestTrue(TEXT("valid optional layer composes"), ValidResult.bComposed);
	TestNotNull(TEXT("composed node is discoverable"), Recipe.FindNode(TEXT("test:site:0")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersWorldRecipeLayerCounterexampleTest,
	"Gaters.Worldgen.WorldRecipeLayer.Counterexample",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersWorldRecipeLayerCounterexampleTest::RunTest(const FString& Parameters)
{
	FGatersWorldRecipe Recipe = LayerBaseRecipe();
	const int32 BaseCount = Recipe.Nodes.Num();
	const uint32 BaseChecksum = Recipe.Checksum();
	FGatersWorldRecipeLayer Duplicate;
	Duplicate.LayerId = TEXT("built-sites");
	Duplicate.SchemaVersion = 1;
	Duplicate.GeneratorVersion = 1;
	Duplicate.bGenerated = true;
	Duplicate.Nodes.Add({Recipe.Nodes[0].Id, EGatersRecipeNodeKind::VillageSite,
		FVector(1000.f, 0.f, 0.f)});
	Duplicate.Nodes.Add({TEXT("must:not:append"), EGatersRecipeNodeKind::VillageSite,
		FVector(2000.f, 0.f, 0.f)});

	const FGatersWorldRecipeLayerComposition Result =
		FGatersWorldRecipeLayerComposer::Append(Recipe, Duplicate);
	TestFalse(TEXT("duplicate identity rejects whole layer"), Result.bComposed);
	TestTrue(TEXT("duplicate identity has causal diagnostic"),
		Result.Diagnostics.ContainsByPredicate([](const FString& Diagnostic)
		{
			return Diagnostic.Contains(TEXT("duplicate"));
		}));
	TestEqual(TEXT("rejected layer appends no nodes"), Recipe.Nodes.Num(), BaseCount);
	TestEqual(TEXT("rejected layer preserves checksum"), Recipe.Checksum(), BaseChecksum);
	return true;
}

#endif
