#pragma once

#include "CoreMinimal.h"
#include "GatersBuiltSiteRecipe.h"

struct PROTOTYPE_API FGatersBuiltSiteRecipeJson
{
	static bool Serialize(
		const TArray<FGatersBuiltSiteRecipe>& Recipes,
		FString& OutJson,
		TArray<FString>& OutDiagnostics);
	static bool Save(
		const TArray<FGatersBuiltSiteRecipe>& Recipes,
		const FString& OutputPath,
		TArray<FString>& OutDiagnostics);
	static bool GenerateSettlement(
		const FString& OutputPath,
		int32 Seed,
		int32 Stage,
		TArray<FString>& OutDiagnostics);
};
