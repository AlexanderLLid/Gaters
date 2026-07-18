#pragma once

#include "CoreMinimal.h"

class FGatersContentCatalog;

class PROTOTYPE_API FGatersEnvironmentContent
{
public:
	// Registers placeholder-safe environment semantics and returns true only when
	// the generated rock passes native Unreal intake and becomes the champion.
	static bool Register(FGatersContentCatalog& Catalog, TArray<FString>& OutErrors);
};
