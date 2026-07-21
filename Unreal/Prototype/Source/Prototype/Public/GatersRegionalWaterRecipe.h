#pragma once

#include "CoreMinimal.h"
#include "GatersEnvironment.h"
#include "GatersWorldIntent.h"

struct PROTOTYPE_API FGatersRegionalWaterSurface
{
	bool operator==(const FGatersRegionalWaterSurface& Other) const = default;

	FString Id;
	FString RegionId;
	FVector2D RegionCenter = FVector2D::ZeroVector;
	float RegionRadius = 0.f;
	FVector2D Center = FVector2D::ZeroVector;
	float HalfExtent = 0.f;
	float Height = 0.f;
	EGatersHydrology Hydrology = EGatersHydrology::Dry;
};

struct PROTOTYPE_API FGatersRegionalWaterRecipe
{
	bool operator==(const FGatersRegionalWaterRecipe& Other) const = default;

	static FGatersRegionalWaterRecipe Generate(
		const FGatersEnvironment& Environment,
		const FGatersWorldIntentRecipe& Intent);

	int32 Version = 1;
	TArray<FGatersRegionalWaterSurface> Surfaces;
};
