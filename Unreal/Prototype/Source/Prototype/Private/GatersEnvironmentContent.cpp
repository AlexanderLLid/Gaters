#include "GatersEnvironmentContent.h"

#include "Engine/StaticMesh.h"
#include "GatersContentCatalog.h"

namespace
{
FGatersAssetContract EnvironmentContract(
	const TCHAR* AssetId,
	const int32 Version,
	const TCHAR* ContentKey,
	const FVector& BoundsCenter,
	const FVector& BoundsExtent)
{
	FGatersAssetContract Contract;
	Contract.AssetId = AssetId;
	Contract.Version = Version;
	Contract.ContentKey = ContentKey;
	Contract.StyleId = TEXT("gaters.clean-midpoly-painted");
	Contract.BoundsCenter = BoundsCenter;
	Contract.BoundsExtent = BoundsExtent;
	Contract.ClearanceExtent = BoundsExtent;
	Contract.Collision = EGatersAssetCollision::Simple;
	Contract.RenderClass = EGatersAssetRenderClass::InstancedStatic;
	Contract.Contacts.Add({TEXT("ground"),
		BoundsCenter - FVector(0.f, 0.f, BoundsExtent.Z), FVector::UpVector});
	return Contract;
}
}

bool FGatersEnvironmentContent::Register(
	FGatersContentCatalog& Catalog,
	TArray<FString>& OutErrors)
{
	Catalog.AddPlaceholder(EnvironmentContract(
		TEXT("runtime.scatter.tree"), 1, TEXT("environment.tree"),
		FVector::ZeroVector, FVector(100.f)), OutErrors);
	Catalog.AddPlaceholder(EnvironmentContract(
		TEXT("runtime.scatter.rock"), 1, TEXT("environment.rock"),
		FVector::ZeroVector, FVector(100.f)), OutErrors);

	UStaticMesh* GeneratedRock = LoadObject<UStaticMesh>(
		nullptr, TEXT("/Game/Gaters/Generated/Candidates/SM_NeutralRock.SM_NeutralRock"));
	return GeneratedRock && Catalog.AddStaticMesh(*GeneratedRock, EnvironmentContract(
		TEXT("generated.neutral-rock"), 2, TEXT("environment.rock"),
		FVector(0.f, 0.f, 75.f), FVector(120.f, 90.f, 75.f)), OutErrors);
}
