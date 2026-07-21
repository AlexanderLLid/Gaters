#pragma once

#include "CoreMinimal.h"
#include "GatersAssetContract.h"

enum class EGatersLegacyBaseArchetype : uint8
{
	Hut,
	Compound,
	Tower
};

struct PROTOTYPE_API FGatersLegacyBaseModuleDefinition
{
	bool bAvailable = false;
	FGatersAssetContract Contract;
};

struct PROTOTYPE_API FGatersLegacyBaseTierDefinition
{
	FString Id;
	FGatersLegacyBaseModuleDefinition Foundation;
	FGatersLegacyBaseModuleDefinition Wall;
	FGatersLegacyBaseModuleDefinition DoorFrame;
	FGatersLegacyBaseModuleDefinition Window;
	FGatersLegacyBaseModuleDefinition Ceiling;
	FGatersLegacyBaseModuleDefinition Fence;
};

struct PROTOTYPE_API FGatersLegacyBaseLayerInput
{
	int32 Version = 1;
	FVector2D BaseCenter = FVector2D::ZeroVector;
	int32 RandomState = 0;
	float CellSize = 300.f;
	float MaxFoundationDrop = 350.f;
	TArray<FGatersLegacyBaseTierDefinition> Tiers;
	FGatersLegacyBaseModuleDefinition Door;
	TArray<FString> SourceIds;
};

struct PROTOTYPE_API FGatersLegacyBasePieceRecipe
{
	FString Id;
	FTransform Transform = FTransform::Identity;
	FString ContentKey;
	TArray<FString> SourceIds;
};

struct PROTOTYPE_API FGatersLegacyBaseLayerResult
{
	bool IsValid() const;

	int32 Version = 1;
	int32 GeneratorVersion = 1;
	EGatersLegacyBaseArchetype Archetype = EGatersLegacyBaseArchetype::Hut;
	FString MainTierId;
	int32 MainWidth = 0;
	int32 MainDepth = 0;
	int32 MainStories = 0;
	int32 BuildingCount = 0;
	TArray<FGatersLegacyBasePieceRecipe> Pieces;
	TArray<FGatersAssetContract> ContentRequirements;
	TArray<FString> Diagnostics;
};

class PROTOTYPE_API FGatersLegacyBaseLayer
{
public:
	static FGatersLegacyBaseLayerResult Generate(
		const FGatersLegacyBaseLayerInput& Input,
		TFunctionRef<float(const FVector2D&)> HeightAt);
};
