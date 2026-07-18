#pragma once

#include "CoreMinimal.h"
#include "GatersEnvironment.h"

enum class EGatersRecipeNodeKind : uint8
{
	Gate,
	BaseSite,
	ScatterTree,
	ScatterRock,
	BuildPlot,
	BasePiece,
	RaidLoot,
	VillageSite,
	LandmarkSite,
	RouteWaypoint
};

struct PROTOTYPE_API FGatersRecipeNode
{
	FString Id;
	EGatersRecipeNodeKind Kind = EGatersRecipeNodeKind::Gate;
	FVector Location = FVector::ZeroVector;
	FRotator Rotation = FRotator::ZeroRotator;
	FVector Scale = FVector::OneVector;
	FString ContentKey;
};

struct PROTOTYPE_API FGatersRecipeLink
{
	FString Id;
	FString FromNodeId;
	FString FromPort;
	FString ToNodeId;
	FString ToPort;
};

// Pure generated facts consumed by materializers and evaluators. It owns no actors,
// assets, or world diffs; CanonicalText is the stable identity for one generator input.
struct PROTOTYPE_API FGatersWorldRecipe
{
	static FGatersWorldRecipe Generate(
		int32 Seed,
		float ChunkSize,
		float MinBaseDistance,
		float MaxBaseDistance,
		float BaseFootprintRadius,
		float MaxFoundationDrop);

	FString CanonicalText() const;
	uint32 Checksum() const;
	FGatersEnvironment CreateEnvironment() const;
	const FGatersRecipeNode* FindNode(const FString& Id) const;
	bool Validate(TArray<FString>& OutErrors) const;

	int32 SchemaVersion = 6;
	int32 GeneratorVersion = 0;
	int32 Seed = 0;
	float ChunkSize = 0.f;
	EGatersEnvironment EnvironmentType = EGatersEnvironment::Lowlands;
	EGatersHydrology Hydrology = EGatersHydrology::Dry;
	FString EnvironmentName;
	float WaterHeight = -100000.f;
	bool bHasBaseSite = false;
	FVector2D BaseSite = FVector2D::ZeroVector;
	TArray<FGatersRecipeNode> Nodes;
	TArray<FGatersRecipeLink> Links;
};
