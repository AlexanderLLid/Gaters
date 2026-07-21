#pragma once

#include "CoreMinimal.h"
#include "GatersBuiltSiteRecipe.h"
#include "GatersSettlementGenerator.h"

struct PROTOTYPE_API FGatersSettlementSiteRecipeCompilation
{
	bool bCompiled = false;
	FGatersBuiltSiteRecipe Recipe;
	TArray<FString> Diagnostics;
};

struct PROTOTYPE_API FGatersSettlementEvidenceSettings
{
	static FGatersSettlementEvidenceSettings Ground();

	int32 Version = 1;
	FString MovementModeId;
	float PathWidth = 0.f;
	float OutdoorHeadroom = 0.f;
	float MaxConnectionLength = 0.f;
	float SlotClearanceRadius = 0.f;
	float SlotClearanceHeight = 0.f;
};

struct PROTOTYPE_API FGatersSettlementSiteRecipeAdapter
{
	static FGatersSettlementSiteRecipeCompilation Compile(
		const FGatersTerrainSemanticField& Field,
		int32 Seed,
		const FGatersSettlementPlan& Plan,
		const FGatersSettlementEvidenceSettings& Settings =
			FGatersSettlementEvidenceSettings::Ground());
};
