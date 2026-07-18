#if WITH_DEV_AUTOMATION_TESTS

#include "GatersWorldCompiler.h"
#include "GatersContentCatalog.h"
#include "GatersWorldRecipe.h"
#include "Engine/StaticMesh.h"
#include "Misc/AutomationTest.h"

namespace
{
FGatersAssetContract Contract(
	const TCHAR* AssetId,
	const TCHAR* ContentKey,
	const int32 Version,
	const EGatersAssetRenderClass RenderClass,
	const bool bPersistent = false)
{
	FGatersAssetContract Result;
	Result.AssetId = AssetId;
	Result.ContentKey = ContentKey;
	Result.Version = Version;
	Result.StyleId = TEXT("gaters.clean-midpoly-painted");
	Result.BoundsExtent = FVector(100);
	Result.ClearanceExtent = FVector(100);
	Result.Collision = EGatersAssetCollision::Simple;
	Result.RenderClass = RenderClass;
	Result.bInstanceStatePersistent = bPersistent;
	Result.Contacts.Add({TEXT("ground"), FVector(0, 0, -100), FVector::UpVector});
	return Result;
}

const FGatersCompiledNode* FindCompiled(const FGatersCompiledWorld& World, const TCHAR* NodeId)
{
	return World.Nodes.FindByPredicate([NodeId](const FGatersCompiledNode& Node)
	{
		return Node.NodeId == NodeId;
	});
}

bool HasDiagnostic(const FGatersCompiledWorld& World, const TCHAR* RuleId)
{
	return World.Diagnostics.ContainsByPredicate([RuleId](const FGatersCompilerDiagnostic& Diagnostic)
	{
		return Diagnostic.RuleId == RuleId;
	});
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersWorldCompilerTest,
	"Gaters.Runtime.WorldCompiler.Manifest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersWorldCompilerTest::RunTest(const FString& Parameters)
{
	FGatersWorldRecipe Recipe = FGatersWorldRecipe::Generate(
		73, 30000.f, 6000.f, 10800.f, 900.f, 350.f);
	FGatersRecipeNode Piece{
		TEXT("piece:0"), EGatersRecipeNodeKind::BasePiece, FVector(100, 200, 300)};
	Piece.Rotation = FRotator(0, 90, 0);
	Piece.ContentKey = TEXT("building.foundation.wood");
	Recipe.Nodes.Add(Piece);
	FGatersRecipeNode Scatter{
		TEXT("scatter:0"), EGatersRecipeNodeKind::ScatterTree, FVector(400, 500, 600)};
	Scatter.ContentKey = TEXT("environment.tree");
	Scatter.Scale = FVector(0.8f, 0.8f, 5.f);
	Recipe.Nodes.Add(Scatter);
	FGatersRecipeNode Route{
		TEXT("route:0"), EGatersRecipeNodeKind::RouteWaypoint, FVector(700, 800, 900)};
	Route.ContentKey = TEXT("route:arrival-village");
	Recipe.Nodes.Add(Route);
	Recipe.Links.Add({TEXT("link:0"), TEXT("base:0"), TEXT("site"), TEXT("piece:0"), TEXT("root")});

	FGatersContentCatalog Catalog;
	TArray<FString> Errors;
	TestTrue(TEXT("foundation placeholder enters fixture catalog"), Catalog.AddPlaceholder(Contract(
		TEXT("foundation.1"), TEXT("building.foundation.wood"), 1,
		EGatersAssetRenderClass::InstancedStatic), Errors));
	const FGatersCompiledWorld Compiled = FGatersWorldCompiler::Compile(
		Recipe, Catalog, TEXT("gaters.clean-midpoly-painted"));
	TestEqual(TEXT("compiler preserves recipe checksum"), Compiled.RecipeChecksum, Recipe.Checksum());
	TestTrue(TEXT("compiled manifest identifies its source recipe"), Compiled.MatchesRecipe(Recipe));
	TestEqual(TEXT("compiler preserves every node"), Compiled.Nodes.Num(), Recipe.Nodes.Num());
	TestEqual(TEXT("compiler preserves every link"), Compiled.Links.Num(), Recipe.Links.Num());
	const FGatersCompiledNode* CompiledPiece = FindCompiled(Compiled, TEXT("piece:0"));
	TestNotNull(TEXT("compiled piece retains stable ID"), CompiledPiece);
	if (CompiledPiece)
	{
		TestTrue(TEXT("compiled piece retains transform"),
			CompiledPiece->Transform.Equals(FTransform(Piece.Rotation, Piece.Location, Piece.Scale)));
		TestEqual(TEXT("instanced contract selects instances"),
			CompiledPiece->Representation, EGatersCompiledRepresentation::InstancedStatic);
		TestEqual(TEXT("selected artifact is recorded"), CompiledPiece->AssetId, FString(TEXT("foundation.1")));
		TestTrue(TEXT("compiled node carries the selected mechanical contract"),
			CompiledPiece->AssetContract.IsSet());
		if (CompiledPiece->AssetContract.IsSet())
		{
			const FGatersAssetContract& SelectedContract = CompiledPiece->AssetContract.GetValue();
			TestEqual(TEXT("compiled contract preserves the selected asset"),
				SelectedContract.AssetId, FString(TEXT("foundation.1")));
			TestEqual(TEXT("compiled contract preserves the semantic key"),
				SelectedContract.ContentKey, FString(TEXT("building.foundation.wood")));
			TestEqual(TEXT("compiled contract preserves contacts"),
				SelectedContract.Contacts.Num(), 1);
			TestTrue(TEXT("compiled contract preserves bounds"),
				SelectedContract.BoundsExtent.Equals(FVector(100.f)));
		}
	}
	TestTrue(TEXT("missing optional scatter is a warning"),
		HasDiagnostic(Compiled, TEXT("compiler.optional_placeholder")));
	TestTrue(TEXT("route grouping never queries the asset catalog"),
		FindCompiled(Compiled, TEXT("route:0"))->Representation == EGatersCompiledRepresentation::Semantic);
	TestTrue(TEXT("compiled manifest exposes semantic nodes to runtime materializers"),
		Compiled.FindNode(TEXT("route:0")) == FindCompiled(Compiled, TEXT("route:0")));
	TestEqual(TEXT("runtime materializers can select a semantic node kind"),
		Compiled.FindNodes(EGatersRecipeNodeKind::RouteWaypoint).Num(), 1);
	int32 StableIndex = INDEX_NONE;
	TestTrue(TEXT("runtime materializers can recover a stable node index"),
		Compiled.FindNode(TEXT("route:0"))->TryGetStableIndex(TEXT("route"), StableIndex));
	TestEqual(TEXT("stable node index is preserved"), StableIndex, 0);
	TestFalse(TEXT("stable node index rejects the wrong prefix"),
		Compiled.FindNode(TEXT("route:0"))->TryGetStableIndex(TEXT("plot"), StableIndex));

	UStaticMesh* TreeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone.Cone"));
	FGatersAssetContract TreeContract = Contract(
		TEXT("tree.1"), TEXT("environment.tree"), 1,
		EGatersAssetRenderClass::InstancedStatic);
	TreeContract.BoundsCenter = TreeMesh->GetBoundingBox().GetCenter();
	TreeContract.BoundsExtent = TreeMesh->GetBoundingBox().GetExtent();
	TreeContract.ClearanceExtent = TreeContract.BoundsExtent;
	TreeContract.Contacts[0].Location = TreeContract.BoundsCenter -
		FVector(0, 0, TreeContract.BoundsExtent.Z);
	TestTrue(TEXT("catalog accepts a contracted tree mesh"),
		Catalog.AddStaticMesh(*TreeMesh, TreeContract, Errors));
	const FGatersCompiledWorld WithTree = FGatersWorldCompiler::Compile(
		Recipe, Catalog, TEXT("gaters.clean-midpoly-painted"));
	const FGatersCompiledNode* CompiledTree = FindCompiled(WithTree, TEXT("scatter:0"));
	TestEqual(TEXT("catalog tree selects native instancing"),
		CompiledTree->Representation, EGatersCompiledRepresentation::InstancedStatic);
	TestTrue(TEXT("compiler carries the selected mesh to materialization"),
		CompiledTree->Mesh.Get() == TreeMesh);
	const FVector CompiledTreeContact = CompiledTree->Transform.TransformPosition(
		TreeContract.Contacts[0].Location * TreeContract.CentimetersPerUnit);
	TestTrue(TEXT("compiled scatter terrain contact lands on the recipe ground anchor"),
		CompiledTreeContact.Equals(Scatter.Location));

	FGatersWorldRecipe MissingRequired = Recipe;
	MissingRequired.Nodes[2].ContentKey = TEXT("building.missing");
	TestFalse(TEXT("compiled manifest rejects a mutated recipe"), Compiled.MatchesRecipe(MissingRequired));
	const FGatersCompiledWorld Failed = FGatersWorldCompiler::Compile(
		MissingRequired, Catalog, TEXT("gaters.clean-midpoly-painted"));
	TestFalse(TEXT("missing required content invalidates compilation"), Failed.IsValid());
	TestTrue(TEXT("required-content failure is causal"),
		HasDiagnostic(Failed, TEXT("compiler.required_content")));

	TestTrue(TEXT("persistent replacement enters fixture catalog"), Catalog.AddPlaceholder(Contract(
		TEXT("foundation.2"), TEXT("building.foundation.wood"), 2,
		EGatersAssetRenderClass::InstancedStatic, true), Errors));
	const FGatersCompiledWorld Recompiled = FGatersWorldCompiler::Compile(
		Recipe, Catalog, TEXT("gaters.clean-midpoly-painted"));
	TestEqual(TEXT("catalog swap preserves recipe identity"),
		Recompiled.RecipeChecksum, Compiled.RecipeChecksum);
	const FGatersCompiledNode* RecompiledPiece = FindCompiled(Recompiled, TEXT("piece:0"));
	TestEqual(TEXT("persistent contract forces unique actor"),
		RecompiledPiece->Representation, EGatersCompiledRepresentation::UniqueActor);
	TestEqual(TEXT("catalog swap records replacement artifact"),
		RecompiledPiece->AssetId, FString(TEXT("foundation.2")));
	return true;
}

#endif
