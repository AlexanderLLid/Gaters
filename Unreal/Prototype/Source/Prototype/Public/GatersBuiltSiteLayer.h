#pragma once

#include "CoreMinimal.h"
#include "GatersBuiltSiteRecipe.h"
#include "GatersWorldRecipe.h"

struct FGatersSiteRoutePlan;
struct FGatersTerrainSemanticField;

struct PROTOTYPE_API FGatersBuiltSiteLayerResult
{
	bool IsValid() const { return Diagnostics.IsEmpty(); }
	bool IsEmpty() const { return Nodes.IsEmpty(); }

	int32 ContractVersion = 1;
	int32 SettlementGeneratorVersion = 0;
	int32 SettlementEvaluatorVersion = 0;
	int32 SiteCount = 0;
	int32 BuildingCount = 0;
	int32 ParcelCount = 0;
	int32 PathCount = 0;
	int32 ValidAssemblyCount = 0;
	int32 ModuleCount = 0;
	TArray<FString> SourceIds;
	TArray<FGatersBuiltSiteRecipe> SiteRecipes;
	TArray<FGatersRecipeNode> Nodes;
	TArray<FString> Diagnostics;
};

struct PROTOTYPE_API FGatersBuiltSiteLayer
{
	static FGatersBuiltSiteLayerResult Generate(
		const FGatersTerrainSemanticField& Field,
		int32 Seed,
		const FGatersSiteRoutePlan& Sites,
		int32 GrowthStage);
};
