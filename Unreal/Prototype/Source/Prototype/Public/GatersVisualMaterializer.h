#pragma once

#include "CoreMinimal.h"
#include "GatersWorldCompiler.h"

class AActor;
class UInstancedStaticMeshComponent;
class USceneComponent;

struct PROTOTYPE_API FGatersVisualInstance
{
	FString StableId;
	FTransform Transform = FTransform::Identity;
};

enum class EGatersVisualBatch : uint8
{
	Trees,
	Rocks,
	OpenClaims,
	ClaimedClaims
};

enum class EGatersVisualInteractionKind : uint8
{
	None,
	Chop,
	Claim
};

struct PROTOTYPE_API FGatersVisualInteraction
{
	bool IsValid() const { return Kind != EGatersVisualInteractionKind::None; }

	EGatersVisualInteractionKind Kind = EGatersVisualInteractionKind::None;
	FString StableId;
	FString DiffEntry;
};

struct PROTOTYPE_API FGatersVisualBatchPlan
{
	int32 NumInstances() const;

	TArray<FGatersVisualInstance> Trees;
	TArray<FGatersVisualInstance> Rocks;
	TArray<FGatersVisualInstance> OpenClaims;
	TArray<FGatersVisualInstance> ClaimedClaims;
	TArray<FGatersVisualInstance> VillageFoundations;
	TArray<FGatersVisualInstance> VillageWalls;
	TArray<FGatersVisualInstance> VillageDoors;
	TArray<FGatersVisualInstance> VillageRoofs;
	TArray<FGatersVisualInstance> VillagePaths;
	TArray<FGatersVisualInstance> VillageSpaces;
	TSoftObjectPtr<UStaticMesh> TreeMesh;
	TSoftObjectPtr<UStaticMesh> RockMesh;
};

// Thin adapter boundary: Gaters groups stable recipe identities; Unreal owns rendering.
struct PROTOTYPE_API FGatersVisualMaterializer
{
	static FGatersVisualBatchPlan Plan(
		const FGatersCompiledWorld& World,
		const TArray<FString>& DiffEntries);

	static bool Materialize(
		AActor& Owner,
		USceneComponent& Parent,
		const FGatersVisualBatchPlan& Plan,
		TArray<TObjectPtr<UInstancedStaticMeshComponent>>& InOutComponents,
		TArray<FString>& OutErrors);

	static FGatersVisualInteraction InteractionAt(
		const FGatersVisualBatchPlan& Plan,
		EGatersVisualBatch Batch,
		int32 InstanceIndex);
};
