#include "GatersEnvironmentRecipe.h"

#include "GatersEnvironmentBrief.h"
#include "GatersLandformProcessField.h"
#include "GatersRegionalWaterEvaluator.h"
#include "GatersTerrainSemanticField.h"

namespace
{
FGatersDrainageSettings DrainageSettingsFromRecipe(
	const FGatersDrainageRecipe& Recipe)
{
	FGatersDrainageSettings Settings;
	Settings.CellsPerAxis = Recipe.CellsPerAxis;
	Settings.Extent = Recipe.Extent;
	Settings.ChannelAccumulationThreshold =
		Recipe.ChannelAccumulationThreshold;
	Settings.WaterfallDropThreshold = Recipe.WaterfallDropThreshold;
	return Settings;
}
}

bool FGatersEnvironmentRecipe::Validate(TArray<FString>& OutErrors) const
{
	OutErrors.Reset();
	if (Version != CurrentVersion
		|| CompilerVersion != CurrentCompilerVersion
		|| WorldSize <= 0.f)
	{
		OutErrors.Add(TEXT("environment recipe root metadata is invalid"));
	}
	if (Terrain.Seed != Seed || Intent.Seed != Seed)
	{
		OutErrors.Add(TEXT("environment recipe seed provenance does not match"));
	}
	if (!FMath::IsNearlyEqual(Terrain.ChunkSize, WorldSize) ||
		!FMath::IsNearlyEqual(Intent.WorldSize, WorldSize) ||
		!FMath::IsNearlyEqual(EnvironmentBrief.WorldSize, WorldSize) ||
		!FMath::IsNearlyEqual(Climate.WorldSize, WorldSize))
	{
		OutErrors.Add(TEXT("environment recipe world-size provenance does not match"));
	}
	if (EnvironmentBrief.Seed != Seed)
	{
		OutErrors.Add(TEXT("environment recipe environment brief provenance does not match"));
	}

	TArray<FString> IntentErrors;
	if (!Intent.Validate(IntentErrors))
	{
		OutErrors.Append(IntentErrors);
	}

	if (RegionalWater.Version != 1)
	{
		OutErrors.Add(TEXT("environment recipe water version is unsupported"));
	}
	TSet<FString> SurfaceIds;
	for (const FGatersRegionalWaterSurface& Surface : RegionalWater.Surfaces)
	{
		const FGatersWorldRegionIntent* Region = Intent.Regions.FindByPredicate(
			[&Surface](const FGatersWorldRegionIntent& Candidate)
			{
				return Candidate.Id == Surface.RegionId;
			});
		if (!Region)
		{
			OutErrors.Add(FString::Printf(
				TEXT("environment recipe water region is unknown: %s"), *Surface.RegionId));
		}
		else if (!Surface.RegionCenter.Equals(Region->Center) ||
			!FMath::IsNearlyEqual(Surface.RegionRadius, Region->Radius))
		{
			OutErrors.Add(FString::Printf(
				TEXT("environment recipe water region provenance does not match: %s"),
				*Surface.RegionId));
		}
		if (Surface.Id.IsEmpty() || SurfaceIds.Contains(Surface.Id))
		{
			OutErrors.Add(FString::Printf(
				TEXT("environment recipe water identity is invalid: %s"), *Surface.Id));
		}
		SurfaceIds.Add(Surface.Id);
		if (Surface.Center.ContainsNaN() || Surface.RegionCenter.ContainsNaN() ||
			!FMath::IsFinite(Surface.RegionRadius) || Surface.RegionRadius <= 0.f ||
			!FMath::IsFinite(Surface.HalfExtent) || Surface.HalfExtent <= 0.f ||
			!FMath::IsFinite(Surface.Height))
		{
			OutErrors.Add(FString::Printf(
				TEXT("environment recipe water bounds are invalid: %s"), *Surface.Id));
		}
	}
	const FGatersRegionalWaterEvaluation WaterEvaluation =
		FGatersRegionalWaterEvaluator::Evaluate(
			Terrain,
			Intent,
			RegionalWater,
			[this](const FVector2D& Point)
			{
				return QueryTerrain(Point).Height;
			});
	OutErrors.Append(WaterEvaluation.Diagnostics);

	const FGatersClimateCompileResult ClimateResult = FGatersClimateField::Compile(
		Terrain, EnvironmentBrief, Climate.Landform);
	if (!ClimateResult.IsValid() || ClimateResult.Recipe != Climate)
	{
		OutErrors.Add(TEXT("environment recipe climate provenance is invalid"));
	}

	const FGatersDrainageBuildResult DrainageResult = FGatersDrainageNetwork::Build(
		*this, DrainageSettingsFromRecipe(Drainage));
	if (!DrainageResult.IsValid() || DrainageResult.Recipe != Drainage)
	{
		OutErrors.Add(TEXT("environment recipe drainage provenance is invalid"));
	}
	const FGatersDrainageWaterFitResult WaterFitResult =
		FGatersDrainageNetwork::FitRegionalWater(
			Drainage, RegionalWater, DrainageWaterFit.DatumTolerance);
	if (!WaterFitResult.IsValid() || WaterFitResult != DrainageWaterFit)
	{
		OutErrors.Add(TEXT("environment recipe drainage water fit is invalid"));
	}
	const FGatersDrainageFeatureCompileResult FeatureResult =
		FGatersDrainageNetwork::CompileFeatureCandidates(
			Drainage,
			RegionalWater,
			DrainageWaterFit,
			DrainageFeatures.Settings);
	if (!FeatureResult.IsValid() || FeatureResult.Recipe != DrainageFeatures)
	{
		OutErrors.Add(TEXT("environment recipe drainage features are invalid"));
	}
	const FGatersSurfaceConditionCompileResult SurfaceResult =
		FGatersSurfaceConditionField::Compile(*this, SurfaceConditions.Settings);
	if (!SurfaceResult.IsValid() || SurfaceResult.Recipe != SurfaceConditions)
	{
		OutErrors.Add(TEXT("environment recipe surface conditions are invalid"));
	}
	const FGatersBiomeOpportunityCompileResult OpportunityResult =
		FGatersBiomeOpportunityField::Compile(*this);
	if (!OpportunityResult.IsValid()
		|| OpportunityResult.Recipe != BiomeOpportunities)
	{
		OutErrors.Add(TEXT("environment recipe biome opportunities are invalid"));
	}
	return OutErrors.IsEmpty();
}

FGatersIntentTerrainSample FGatersEnvironmentRecipe::QueryTerrain(
	const FVector2D& Point) const
{
	return FGatersIntentTerrainField::Query(Terrain, Intent, Point);
}

float FGatersEnvironmentRecipe::MaterializedHeightAt(
	const FVector2D& Point,
	const float PadRadius,
	const FVector2D& RouteTarget) const
{
	return FGatersTerrainSemanticField::MaterializedHeightAt(
		Terrain, Intent, Point, PadRadius, RouteTarget);
}

FVector FGatersEnvironmentRecipe::MaterializedNormalAt(
	const FVector2D& Point,
	const float SampleDistance,
	const float PadRadius,
	const FVector2D& RouteTarget) const
{
	return FGatersTerrainSemanticField::MaterializedNormalAt(
		Terrain, Intent, Point, SampleDistance, PadRadius, RouteTarget);
}

FGatersBiomeSample FGatersEnvironmentRecipe::QueryBiome(
	const FVector2D& Point,
	const FGatersBiomeQuery& Query) const
{
	return FGatersBiomeField::Query(Terrain, Intent, Point, Query);
}

FGatersBiomeOpportunitySample FGatersEnvironmentRecipe::QueryOpportunities(
	const FVector2D& Point,
	const FGatersBiomeQuery& Query) const
{
	return FGatersBiomeOpportunityField::Evaluate(
		QueryBiome(Point, Query),
		QueryClimate(Point),
		QuerySurfaceConditions(Point));
}

FGatersClimateSample FGatersEnvironmentRecipe::QueryClimate(
	const FVector2D& Point) const
{
	return FGatersClimateField::Query(
		Climate,
		Point,
		[this](const FVector2D& SamplePoint)
		{
			return QueryTerrain(SamplePoint).Height;
		},
		Terrain.HasWater(),
		Terrain.WaterHeight);
}

FGatersSurfaceConditionSample FGatersEnvironmentRecipe::QuerySurfaceConditions(
	const FVector2D& Point) const
{
	return FGatersSurfaceConditionField::Query(
		SurfaceConditions, *this, Point);
}

FGatersEnvironmentRecipe FGatersEnvironmentRecipeCompiler::Compile(
	const int32 Seed,
	const float WorldSize)
{
	check(WorldSize > 0.f);
	return Compile(FGatersEnvironment::FromSeed(Seed, WorldSize));
}

FGatersEnvironmentRecipe FGatersEnvironmentRecipeCompiler::Compile(
	const FGatersEnvironment& Terrain)
{
	check(Terrain.ChunkSize > 0.f);
	const FGatersEnvironmentBriefCompileResult Brief =
		FGatersEnvironmentBriefCompiler::Compile(
			FGatersEnvironmentBrief(), Terrain.Seed, Terrain.ChunkSize);
	const FGatersLandformProcessCompileResult Landform =
		FGatersLandformProcessField::Compile(Terrain, Brief.Intent);
	return Compile(Terrain, Brief.Intent, Landform.Recipe);
}

FGatersEnvironmentRecipe FGatersEnvironmentRecipeCompiler::Compile(
	const FGatersEnvironment& Terrain,
	const FGatersCompiledEnvironmentBrief& EnvironmentBrief,
	const FGatersLandformProcessRecipe& Landform)
{
	check(Terrain.ChunkSize > 0.f);
	FGatersEnvironmentRecipe Result;
	Result.Seed = Terrain.Seed;
	Result.WorldSize = Terrain.ChunkSize;
	Result.Terrain = Terrain;
	Result.Intent = FGatersWorldIntentRecipe::Generate(Result.Terrain);
	Result.RegionalWater = FGatersRegionalWaterRecipe::Generate(
		Result.Terrain, Result.Intent);
	Result.EnvironmentBrief = EnvironmentBrief;
	Result.Climate = FGatersClimateField::Compile(
		Result.Terrain, Result.EnvironmentBrief, Landform).Recipe;
	FGatersDrainageSettings DrainageSettings;
	DrainageSettings.CellsPerAxis = 129;
	const FGatersDrainageBuildResult Drainage =
		FGatersDrainageNetwork::Build(Result, DrainageSettings);
	Result.Drainage = Drainage.Recipe;
	if (!Drainage.IsValid())
	{
		return Result;
	}
	const FGatersDrainageWaterFitResult WaterFit =
		FGatersDrainageNetwork::FitRegionalWater(
			Result.Drainage, Result.RegionalWater);
	Result.DrainageWaterFit = WaterFit;
	if (!WaterFit.IsValid())
	{
		return Result;
	}
	const FGatersDrainageFeatureCompileResult Features =
		FGatersDrainageNetwork::CompileFeatureCandidates(
			Result.Drainage, Result.RegionalWater, Result.DrainageWaterFit);
	Result.DrainageFeatures = Features.Recipe;
	if (!Features.IsValid())
	{
		return Result;
	}
	Result.SurfaceConditions = FGatersSurfaceConditionField::Compile(Result).Recipe;
	Result.BiomeOpportunities =
		FGatersBiomeOpportunityField::Compile(Result).Recipe;
	return Result;
}
