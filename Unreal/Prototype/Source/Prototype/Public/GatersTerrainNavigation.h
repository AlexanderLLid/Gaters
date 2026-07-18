#pragma once

#include "CoreMinimal.h"
#include "GatersTerrainSemanticField.h"

struct PROTOTYPE_API FGatersTerrainPath
{
	bool bFound = false;
	int32 Cost = 0;
	TArray<FIntPoint> Cells;
};

struct PROTOTYPE_API FGatersTerrainRegion
{
	int32 CellsPerAxis = 0;
	int32 WalkableCount = 0;
	int32 ReachableCount = 0;
	int32 ComponentCount = 0;
	TBitArray<> Reachable;

	bool IsReachable(const FIntPoint& Cell) const;
};

// Terrain-only connectivity for generator validation. Runtime actors still use Unreal navigation.
struct PROTOTYPE_API FGatersTerrainNavigation
{
	static bool IsWalkable(EGatersTerrainSemantic Type);
	static FGatersTerrainRegion Analyze(
		const FGatersTerrainSemanticField& Field,
		const FIntPoint& Start);
	static FGatersTerrainPath FindPath(
		const FGatersTerrainSemanticField& Field,
		const FIntPoint& Start,
		const FIntPoint& Goal);
};
