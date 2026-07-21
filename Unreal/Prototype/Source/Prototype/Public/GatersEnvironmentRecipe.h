#pragma once

#include "CoreMinimal.h"
#include "GatersBiomeField.h"
#include "GatersBiomeOpportunityField.h"
#include "GatersClimateField.h"
#include "GatersDrainageNetwork.h"
#include "GatersIntentTerrainField.h"
#include "GatersRegionalWaterRecipe.h"
#include "GatersSurfaceConditionField.h"

// Authoritative pure-data root for one generated environment. Downstream systems query
// this recipe instead of reconstructing terrain, intent, water, or biome state.
struct PROTOTYPE_API FGatersEnvironmentRecipe
{
	static constexpr int32 CurrentVersion = 5;
	static constexpr int32 CurrentCompilerVersion = 5;

	bool Validate(TArray<FString>& OutErrors) const;
	FGatersIntentTerrainSample QueryTerrain(const FVector2D& Point) const;
	float MaterializedHeightAt(
		const FVector2D& Point,
		float PadRadius,
		const FVector2D& RouteTarget = FVector2D::ZeroVector) const;
	FVector MaterializedNormalAt(
		const FVector2D& Point,
		float SampleDistance,
		float PadRadius,
		const FVector2D& RouteTarget = FVector2D::ZeroVector) const;
	FGatersBiomeSample QueryBiome(
		const FVector2D& Point,
		const FGatersBiomeQuery& Query = {}) const;
	FGatersBiomeOpportunitySample QueryOpportunities(
		const FVector2D& Point,
		const FGatersBiomeQuery& Query = {}) const;
	FGatersClimateSample QueryClimate(const FVector2D& Point) const;
	FGatersSurfaceConditionSample QuerySurfaceConditions(
		const FVector2D& Point) const;

	int32 Version = CurrentVersion;
	int32 CompilerVersion = CurrentCompilerVersion;
	int32 Seed = 0;
	float WorldSize = 0.f;
	FGatersEnvironment Terrain;
	FGatersWorldIntentRecipe Intent;
	FGatersRegionalWaterRecipe RegionalWater;
	FGatersCompiledEnvironmentBrief EnvironmentBrief;
	FGatersClimateRecipe Climate;
	FGatersDrainageRecipe Drainage;
	FGatersDrainageWaterFitResult DrainageWaterFit;
	FGatersDrainageFeatureRecipe DrainageFeatures;
	FGatersSurfaceConditionRecipe SurfaceConditions;
	FGatersBiomeOpportunityRecipe BiomeOpportunities;
};

struct PROTOTYPE_API FGatersEnvironmentRecipeCompiler
{
	static FGatersEnvironmentRecipe Compile(int32 Seed, float WorldSize);
	static FGatersEnvironmentRecipe Compile(const FGatersEnvironment& Terrain);
	static FGatersEnvironmentRecipe Compile(
		const FGatersEnvironment& Terrain,
		const FGatersCompiledEnvironmentBrief& EnvironmentBrief,
		const FGatersLandformProcessRecipe& Landform);
};
