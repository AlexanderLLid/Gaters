#pragma once

#include "CoreMinimal.h"
#include "GatersEnvironment.h"
#include "GatersWorldRecipe.h"

struct PROTOTYPE_API FGatersContentCellSemantics
{
	float PadRadius = 0.f;
	FVector2D RouteTarget = FVector2D::ZeroVector;
	float RouteTargetClearance = 0.f;
	float WaterClearance = 50.f;
	float MinGroundNormalZ = 0.77f;
};

struct PROTOTYPE_API FGatersContentCellCoverage
{
	bool operator==(const FGatersContentCellCoverage& Other) const = default;

	int32 CandidateCount = 0;
	int32 PlacedCount = 0;
	int32 ReservedRejectedCount = 0;
	int32 WaterRejectedCount = 0;
	int32 SteepRejectedCount = 0;
	int32 BudgetRejectedCount = 0;
};

struct PROTOTYPE_API FGatersContentCellPlacement
{
	FString Id;
	EGatersRecipeNodeKind Kind = EGatersRecipeNodeKind::ScatterTree;
	FString ContentKey;
	FTransform Transform = FTransform::Identity;
};

// Pure generated content facts for one bounded terrain cell. It owns no actors or assets.
struct PROTOTYPE_API FGatersContentCellRecipe
{
	static FGatersContentCellRecipe Generate(
		const FIntPoint& Cell,
		float CellSize,
		const FGatersEnvironment& Environment,
		const FGatersContentCellSemantics& Semantics = {});

	int32 Version = 2;
	int32 WorldSeed = 0;
	FIntPoint Cell = FIntPoint::ZeroValue;
	float CellSize = 0.f;
	int32 MaxPlacements = 8;
	TArray<FGatersContentCellPlacement> Placements;
	FGatersContentCellCoverage Coverage;
};
