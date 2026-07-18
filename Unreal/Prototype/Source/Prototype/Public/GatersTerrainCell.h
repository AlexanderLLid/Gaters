#pragma once

#include "CoreMinimal.h"
#include "DynamicMeshActor.h"
#include "GatersEnvironment.h"
#include "GatersTerrainCell.generated.h"

UCLASS()
class PROTOTYPE_API AGatersTerrainCell : public ADynamicMeshActor
{
	GENERATED_BODY()

public:
	AGatersTerrainCell();

	static FVector2D GlobalSamplePosition(
		const FIntPoint& Cell, const FVector2D& LocalPosition, float CellSize);

	void Configure(
		int32 InSeed,
		float InWorldSize,
		const FIntPoint& InCell,
		float InCellSize,
		int32 InResolution,
		float InPadRadius,
		const FVector2D& InRouteTarget);
	void Build();

private:
	int32 Seed = 0;
	float WorldSize = 0.f;
	FIntPoint Cell = FIntPoint::ZeroValue;
	float CellSize = 10000.f;
	int32 Resolution = 32;
	float PadRadius = 0.f;
	FVector2D RouteTarget = FVector2D::ZeroVector;
	FGatersEnvironment Environment;
};
