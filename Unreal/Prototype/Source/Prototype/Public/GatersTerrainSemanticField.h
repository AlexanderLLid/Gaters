#pragma once

#include "CoreMinimal.h"
#include "GatersEnvironment.h"

enum class EGatersTerrainSemantic : uint8
{
	Flat,
	Slope,
	Steep,
	Water
};

struct PROTOTYPE_API FGatersTerrainSemanticSample
{
	float Height = 0.f;
	float NormalZ = 1.f;
	EGatersTerrainSemantic Type = EGatersTerrainSemantic::Flat;
};

// Pure, deterministic terrain evidence shared by placement and evaluation code.
struct PROTOTYPE_API FGatersTerrainSemanticField
{
	static FGatersTerrainSemanticField Build(
		const FGatersEnvironment& Environment,
		int32 CellsPerAxis,
		float CellSize,
		float PadRadius,
		float FlatNormalZ,
		float SlopeNormalZ,
		const FVector2D& RouteTarget = FVector2D::ZeroVector);

	static float MaterializedHeightAt(
		const FGatersEnvironment& Environment,
		const FVector2D& Point,
		float PadRadius,
		const FVector2D& RouteTarget = FVector2D::ZeroVector);
	static FVector MaterializedNormalAt(
		const FGatersEnvironment& Environment,
		const FVector2D& Point,
		float SampleDistance,
		float PadRadius,
		const FVector2D& RouteTarget = FVector2D::ZeroVector);

	const FGatersTerrainSemanticSample& At(int32 X, int32 Y) const;
	int32 Count(EGatersTerrainSemantic Type) const;
	int32 WalkableCount() const;

	int32 Version = 1;
	int32 CellsPerAxis = 0;
	float CellSize = 0.f;
	TArray<FGatersTerrainSemanticSample> Cells;
};
