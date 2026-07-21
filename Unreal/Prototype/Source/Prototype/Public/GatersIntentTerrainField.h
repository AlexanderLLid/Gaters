#pragma once

#include "CoreMinimal.h"
#include "GatersEnvironment.h"
#include "GatersWorldIntent.h"

struct PROTOTYPE_API FGatersIntentTerrainSample
{
	bool operator==(const FGatersIntentTerrainSample& Other) const = default;

	float Height = 0.f;
	FString RegionId;
	float Influence = 0.f;
	EGatersEnvironment Terrain = EGatersEnvironment::Lowlands;
	EGatersHydrology Hydrology = EGatersHydrology::Dry;
};

// Pure adapter over the existing terrain families. It owns no cells, Actors, or assets.
struct PROTOTYPE_API FGatersIntentTerrainField
{
	static FGatersIntentTerrainSample Query(
		const FGatersEnvironment& Environment,
		const FGatersWorldIntentRecipe& Intent,
		const FVector2D& Point);
};
