#pragma once

#include "CoreMinimal.h"
#include "GatersWorldRecipe.h"

struct PROTOTYPE_API FGatersWorldRecipeLayer
{
	FString LayerId;
	int32 SchemaVersion = 0;
	int32 GeneratorVersion = 0;
	bool bGenerated = false;
	TArray<FGatersRecipeNode> Nodes;
	TArray<FString> Diagnostics;
};

struct PROTOTYPE_API FGatersWorldRecipeLayerComposition
{
	bool bComposed = false;
	int32 AppendedNodeCount = 0;
	TArray<FString> Diagnostics;
};

struct PROTOTYPE_API FGatersWorldRecipeLayerComposer
{
	static FGatersWorldRecipeLayerComposition Append(
		FGatersWorldRecipe& Recipe,
		const FGatersWorldRecipeLayer& Layer);
};
