#pragma once

#include "CoreMinimal.h"

struct PROTOTYPE_API FGatersWorldCellStreaming
{
	static FIntPoint CellAt(const FVector2D& Position, float CellSize);
	static FVector2D CellCenter(const FIntPoint& Cell, float CellSize);
	static TArray<FIntPoint> DesiredCells(
		const TArray<FVector2D>& Sources,
		float CellSize,
		int32 LoadRadius,
		float WorldSize,
		const FIntPoint& PrefetchDirection = FIntPoint::ZeroValue);
};
