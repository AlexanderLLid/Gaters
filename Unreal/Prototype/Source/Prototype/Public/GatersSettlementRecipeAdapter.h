#pragma once

#include "CoreMinimal.h"
#include "GatersSettlementGenerator.h"
#include "GatersWorldRecipe.h"

struct PROTOTYPE_API FGatersSettlementRecipeCompilation
{
	int32 Count(EGatersRecipeNodeKind Kind) const;

	bool bCompiled = false;
	int32 ValidAssemblyCount = 0;
	int32 ModuleCount = 0;
	TArray<FGatersRecipeNode> Nodes;
	TArray<FString> Diagnostics;
};

// Converts an accepted semantic settlement into runtime-neutral recipe nodes.
// Habitat selection belongs before this adapter; rendering belongs after it.
struct PROTOTYPE_API FGatersSettlementRecipeAdapter
{
	static FGatersSettlementRecipeCompilation Compile(
		const FGatersTerrainSemanticField& Field,
		const FGatersSettlementPlan& Plan);
};
