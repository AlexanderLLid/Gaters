#pragma once

#include "CoreMinimal.h"

// Gaters-only on-screen diagnostics. UE_LOG remains active and engine warnings are not
// suppressed.
struct PROTOTYPE_API FGatersDebugMessages
{
	static constexpr uint64 ReportKey = 0x4741544552530001ull;
	static constexpr uint64 TraversalKey = 0x4741544552530002ull;
	static constexpr uint64 RaidKey = 0x4741544552530003ull;

	static bool IsEnabled();
	static void SetEnabled(bool bEnabled);
	static void Show(uint64 Key, float Duration, const FColor& Color, const FString& Text);
};
