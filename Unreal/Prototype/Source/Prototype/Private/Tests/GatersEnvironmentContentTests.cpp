#if WITH_DEV_AUTOMATION_TESTS

#include "GatersEnvironmentContent.h"

#include "Engine/StaticMesh.h"
#include "GatersContentCatalog.h"
#include "Misc/AutomationTest.h"
#include "StaticMeshResources.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersEnvironmentContentTest,
	"Gaters.Content.Environment.GeneratedRock",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersEnvironmentContentTest::RunTest(const FString& Parameters)
{
	UStaticMesh* GeneratedRock = LoadObject<UStaticMesh>(
		nullptr, TEXT("/Game/Gaters/Generated/Candidates/SM_NeutralRock.SM_NeutralRock"));
	TestNotNull(TEXT("generated Blender rock fixture exists"), GeneratedRock);
	if (!GeneratedRock)
	{
		return false;
	}
	const FStaticMeshRenderData* RenderData = GeneratedRock->GetRenderData();
	TestNotNull(TEXT("generated rock has native render data"), RenderData);
	if (!RenderData)
	{
		return false;
	}
	TestEqual(TEXT("generated rock has three native render LODs"),
		RenderData->LODResources.Num(), 3);
	for (int32 LodIndex = 0; LodIndex < RenderData->LODResources.Num(); ++LodIndex)
	{
		const FPositionVertexBuffer& Positions =
			RenderData->LODResources[LodIndex].VertexBuffers.PositionVertexBuffer;
		FBox LodBounds(ForceInit);
		for (uint32 VertexIndex = 0; VertexIndex < Positions.GetNumVertices(); ++VertexIndex)
		{
			LodBounds += FVector(Positions.VertexPosition(VertexIndex));
		}
		TestTrue(FString::Printf(TEXT("LOD%d preserves source size"), LodIndex),
			LodBounds.GetSize().Equals(FVector(240.f, 180.f, 150.f), 0.1f));
		TestTrue(FString::Printf(TEXT("LOD%d preserves grounded pivot"), LodIndex),
			LodBounds.GetCenter().Equals(FVector(0.f, 0.f, 75.f), 0.1f));
	}

	FGatersContentCatalog Catalog;
	TArray<FString> Errors;
	TestTrue(TEXT("accepted generated rock replaces the optional fallback"),
		FGatersEnvironmentContent::Register(Catalog, Errors));
	TestEqual(TEXT("generated environment registration has no intake errors"), Errors.Num(), 0);

	const TOptional<FGatersCatalogAsset> Rock = Catalog.Find(
		TEXT("environment.rock"), TEXT("gaters.clean-midpoly-painted"));
	TestTrue(TEXT("rock semantic key resolves"), Rock.IsSet());
	TestTrue(TEXT("rock semantic key carries the generated mesh"),
		Rock && Rock->Mesh.Get() == GeneratedRock);
	TestEqual(TEXT("generated art outranks fallback by contract version"),
		Rock ? Rock->Contract.Version : 0, 2);

	const TOptional<FGatersCatalogAsset> Tree = Catalog.Find(
		TEXT("environment.tree"), TEXT("gaters.clean-midpoly-painted"));
	TestTrue(TEXT("tree semantic key retains its fallback"), Tree.IsSet());
	TestTrue(TEXT("tree remains a placeholder until its own candidate exists"),
		Tree && Tree->Mesh.IsNull());
	return true;
}

#endif
