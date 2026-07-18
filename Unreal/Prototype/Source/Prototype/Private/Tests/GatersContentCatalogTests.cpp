#if WITH_DEV_AUTOMATION_TESTS

#include "GatersContentCatalog.h"
#include "Engine/StaticMesh.h"
#include "Misc/AutomationTest.h"

namespace
{
FGatersAssetContract FoundationContract(const int32 Version = 1)
{
	FGatersAssetContract Contract;
	Contract.AssetId = FString::Printf(TEXT("fixture.wood-foundation.%d"), Version);
	Contract.Version = Version;
	Contract.ContentKey = TEXT("building.foundation.wood");
	Contract.StyleId = TEXT("gaters.clean-midpoly-painted");
	Contract.BoundsCenter = FVector(0, 0, 25);
	Contract.BoundsExtent = FVector(200, 200, 25);
	Contract.ClearanceExtent = FVector(200, 200, 25);
	Contract.Collision = EGatersAssetCollision::Simple;
	Contract.RenderClass = EGatersAssetRenderClass::InstancedStatic;
	Contract.Contacts.Add({TEXT("ground"), FVector::ZeroVector, FVector::UpVector});
	return Contract;
}

bool HasCatalogIssue(const FGatersCatalogSelection& Selection, const TCHAR* RuleId)
{
	return Selection.Issues.ContainsByPredicate([RuleId](const FGatersCatalogIssue& Issue)
	{
		return Issue.RuleId == RuleId;
	});
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersContentCatalogTest,
	"Gaters.Content.Catalog.StaticMesh",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersContentCatalogTest::RunTest(const FString& Parameters)
{
	UStaticMesh* Mesh = LoadObject<UStaticMesh>(
		nullptr, TEXT("/Game/Gaters/Generated/Fixtures/SM_WoodFoundation.SM_WoodFoundation"));
	TestNotNull(TEXT("catalog fixture exists"), Mesh);
	if (!Mesh)
	{
		return false;
	}

	FGatersContentCatalog Catalog;
	TArray<FString> Errors;
	TestTrue(TEXT("linted mesh enters catalog"), Catalog.AddStaticMesh(*Mesh, FoundationContract(), Errors));
	TOptional<FGatersCatalogAsset> Found = Catalog.Find(
		TEXT("building.foundation.wood"), TEXT("gaters.clean-midpoly-painted"));
	TestTrue(TEXT("semantic query finds accepted mesh"), Found.IsSet());
	if (Found)
	{
		TestEqual(TEXT("native soft reference preserves mesh"), Found->Mesh.Get(), Mesh);
		TestEqual(TEXT("first accepted contract version"), Found->Contract.Version, 1);
	}
	TestFalse(TEXT("unknown semantic key has no result"),
		Catalog.Find(TEXT("building.wall.wood"), TEXT("gaters.clean-midpoly-painted")).IsSet());

	FGatersAssetContract Rejected = FoundationContract(9);
	Rejected.BoundsExtent.X += 100.f;
	TestFalse(TEXT("failed intake never enters catalog"), Catalog.AddStaticMesh(*Mesh, Rejected, Errors));
	Found = Catalog.Find(TEXT("building.foundation.wood"), TEXT("gaters.clean-midpoly-painted"));
	TestEqual(TEXT("rejected higher version cannot replace champion"), Found ? Found->Contract.Version : 0, 1);

	TestTrue(TEXT("valid higher version enters catalog"),
		Catalog.AddStaticMesh(*Mesh, FoundationContract(2), Errors));
	Found = Catalog.Find(TEXT("building.foundation.wood"), TEXT("gaters.clean-midpoly-painted"));
	TestEqual(TEXT("highest accepted version wins deterministically"), Found ? Found->Contract.Version : 0, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersContentCatalogQueryTest,
	"Gaters.Content.Catalog.Query",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersContentCatalogQueryTest::RunTest(const FString& Parameters)
{
	FGatersContentCatalog Catalog;
	TArray<FString> Errors;
	TestTrue(TEXT("valid contract-only placeholder enters catalog"),
		Catalog.AddPlaceholder(FoundationContract(), Errors));

	FGatersCatalogQuery Query;
	Query.ContentKey = TEXT("building.foundation.wood");
	Query.StyleId = TEXT("gaters.clean-midpoly-painted");
	Query.Collision = EGatersAssetCollision::Simple;
	Query.RenderClass = EGatersAssetRenderClass::InstancedStatic;
	Query.MaxBoundsExtent = FVector(250, 250, 50);
	FGatersCatalogSelection Selection = Catalog.Select(Query);
	TestTrue(TEXT("compatible placeholder resolves"), Selection.Asset.IsSet());
	if (Selection.Asset)
	{
		TestTrue(TEXT("placeholder has no mesh dependency"), Selection.Asset->Mesh.IsNull());
	}

	Query.Collision = EGatersAssetCollision::Complex;
	Selection = Catalog.Select(Query);
	TestFalse(TEXT("incompatible collision is rejected"), Selection.Asset.IsSet());
	TestTrue(TEXT("collision rejection is causal"),
		HasCatalogIssue(Selection, TEXT("catalog.collision")));

	Query.Collision = EGatersAssetCollision::Simple;
	Query.RenderClass = EGatersAssetRenderClass::UniqueStatic;
	Selection = Catalog.Select(Query);
	TestTrue(TEXT("render-class rejection is causal"),
		HasCatalogIssue(Selection, TEXT("catalog.render_class")));

	Query.RenderClass = EGatersAssetRenderClass::InstancedStatic;
	Query.MaxBoundsExtent = FVector(100, 100, 20);
	Selection = Catalog.Select(Query);
	TestTrue(TEXT("bounds rejection is causal"),
		HasCatalogIssue(Selection, TEXT("catalog.bounds")));

	Query.MaxBoundsExtent = FVector(250, 250, 50);
	Query.RequiredPorts.Add(TEXT("snap"));
	Selection = Catalog.Select(Query);
	TestTrue(TEXT("required-port rejection is causal"),
		HasCatalogIssue(Selection, TEXT("catalog.port")));

	Query.ContentKey = TEXT("building.missing");
	Selection = Catalog.Select(Query);
	TestTrue(TEXT("missing semantic key is explicit"),
		HasCatalogIssue(Selection, TEXT("catalog.no_match")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersContentCatalogReplacementTest,
	"Gaters.Content.Catalog.Replacement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersContentCatalogReplacementTest::RunTest(const FString& Parameters)
{
	UStaticMesh* Mesh = LoadObject<UStaticMesh>(
		nullptr, TEXT("/Game/Gaters/Generated/Fixtures/SM_WoodFoundation.SM_WoodFoundation"));
	TestNotNull(TEXT("replacement fixture exists"), Mesh);
	if (!Mesh)
	{
		return false;
	}

	FGatersContentCatalog Catalog;
	TArray<FString> Errors;
	TestTrue(TEXT("placeholder v1 enters catalog"), Catalog.AddPlaceholder(FoundationContract(1), Errors));
	TestTrue(TEXT("linted art v2 enters catalog"), Catalog.AddStaticMesh(*Mesh, FoundationContract(2), Errors));
	TOptional<FGatersCatalogAsset> Found = Catalog.Find(
		TEXT("building.foundation.wood"), TEXT("gaters.clean-midpoly-painted"));
	TestEqual(TEXT("art replaces placeholder by contract version"), Found ? Found->Contract.Version : 0, 2);
	TestTrue(TEXT("selected replacement carries mesh"), Found && !Found->Mesh.IsNull());

	TestTrue(TEXT("champion can be withdrawn"), Catalog.Withdraw(TEXT("fixture.wood-foundation.2")));
	Found = Catalog.Find(TEXT("building.foundation.wood"), TEXT("gaters.clean-midpoly-painted"));
	TestEqual(TEXT("withdrawal exposes compatible fallback"), Found ? Found->Contract.Version : 0, 1);
	TestTrue(TEXT("fallback is the placeholder"), Found && Found->Mesh.IsNull());

	FGatersAssetContract Invalid = FoundationContract(3);
	Invalid.BoundsExtent = FVector::ZeroVector;
	TestFalse(TEXT("invalid placeholder never enters catalog"), Catalog.AddPlaceholder(Invalid, Errors));
	return true;
}

#endif
