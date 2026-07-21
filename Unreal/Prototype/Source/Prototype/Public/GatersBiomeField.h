#pragma once

#include "CoreMinimal.h"
#include "GatersEnvironment.h"
#include "GatersWorldIntent.h"

enum class EGatersElevationBand : uint8
{
	Low,
	Mid,
	High
};

struct PROTOTYPE_API FGatersBiomeQuery
{
	float PadRadius = 0.f;
	FVector2D RouteTarget = FVector2D::ZeroVector;
	float NormalSampleDistance = 200.f;
	float WaterSearchRadius = 4000.f;
};

struct PROTOTYPE_API FGatersBiomeSample
{
	bool operator==(const FGatersBiomeSample& Other) const = default;

	float Height = 0.f;
	float NormalZ = 1.f;
	float WaterProximity = 0.f;
	float Moisture = 0.f;
	float Exposure = 0.f;
	EGatersElevationBand ElevationBand = EGatersElevationBand::Low;
	FString BiomeKey;
	int32 IntentVersion = 0;
	FString IntentRegionId;
	float IntentInfluence = 0.f;
};

// Pure coordinate query. It owns no cells, Actors, assets, or generated sites.
struct PROTOTYPE_API FGatersBiomeField
{
	static FGatersBiomeSample Query(
		const FGatersEnvironment& Environment,
		const FVector2D& Point,
		const FGatersBiomeQuery& Query = {});
	static FGatersBiomeSample Query(
		const FGatersEnvironment& Environment,
		const FGatersWorldIntentRecipe& Intent,
		const FVector2D& Point,
		const FGatersBiomeQuery& Query = {});
};
