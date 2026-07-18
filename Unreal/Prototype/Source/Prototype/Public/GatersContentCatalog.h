#pragma once

#include "CoreMinimal.h"
#include "GatersAssetContract.h"

class UStaticMesh;

struct PROTOTYPE_API FGatersCatalogAsset
{
	FGatersAssetContract Contract;
	TSoftObjectPtr<UStaticMesh> Mesh;
};

struct PROTOTYPE_API FGatersCatalogQuery
{
	FString ContentKey;
	FString StyleId;
	TOptional<EGatersAssetCollision> Collision;
	TOptional<EGatersAssetRenderClass> RenderClass;
	FVector MaxBoundsExtent = FVector::ZeroVector;
	TArray<FString> RequiredPorts;
};

struct PROTOTYPE_API FGatersCatalogIssue
{
	FString RuleId;
	FString AssetId;
	FString Message;
};

struct PROTOTYPE_API FGatersCatalogSelection
{
	TOptional<FGatersCatalogAsset> Asset;
	TArray<FGatersCatalogIssue> Issues;
};

class PROTOTYPE_API FGatersContentCatalog
{
public:
	bool AddStaticMesh(
		UStaticMesh& Mesh,
		const FGatersAssetContract& Contract,
		TArray<FString>& OutErrors,
		float BoundsToleranceCm = 0.1f);
	bool AddPlaceholder(const FGatersAssetContract& Contract, TArray<FString>& OutErrors);
	bool Withdraw(const FString& AssetId);

	TOptional<FGatersCatalogAsset> Find(const FString& ContentKey, const FString& StyleId) const;
	FGatersCatalogSelection Select(const FGatersCatalogQuery& Query) const;

private:
	TArray<FGatersCatalogAsset> Assets;
};
