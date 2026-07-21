#pragma once

#include "CoreMinimal.h"
#include "GatersEnvironment.h"

struct PROTOTYPE_API FGatersWorldRegionIntent
{
	bool operator==(const FGatersWorldRegionIntent& Other) const = default;

	FString Id;
	FVector2D Center = FVector2D::ZeroVector;
	float Radius = 0.f;
	EGatersEnvironment TerrainTendency = EGatersEnvironment::Lowlands;
	EGatersHydrology HydrologyTendency = EGatersHydrology::Dry;
	float VegetationOpportunity = 0.f;
	float StoneOpportunity = 0.f;
	float LandmarkOpportunity = 0.f;
	float TravelFriction = 0.f;
};

// Small authoritative declaration derived from a seed before detailed world cells exist.
struct PROTOTYPE_API FGatersWorldIntentRecipe
{
	bool operator==(const FGatersWorldIntentRecipe& Other) const = default;

	static FGatersWorldIntentRecipe Generate(int32 Seed, float WorldSize);
	static FGatersWorldIntentRecipe Generate(const FGatersEnvironment& Environment);

	const FGatersWorldRegionIntent& At(const FVector2D& Point) const;
	bool Validate(TArray<FString>& OutErrors) const;

	int32 Version = 2;
	int32 Seed = 0;
	float WorldSize = 0.f;
	TArray<FGatersWorldRegionIntent> Regions;
};
