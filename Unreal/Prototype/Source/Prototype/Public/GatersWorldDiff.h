#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GatersWorldDiff.generated.h"

// Bump when generation logic changes what a seed produces. Diffs only make sense against
// the generator version that recorded them; mismatched diffs are discarded on load
// instead of mis-replaying against a different layout (the cheap half of questions #26).
inline constexpr int32 GatersGenVersion = 10;

// Per-seed change list: the world is row + seed + diff. Entries carry the full recipe
// identity, for example "chop:content:7:4:-3:11" or "claim:plot:3".
UCLASS()
class PROTOTYPE_API UGatersWorldDiff : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 GenVersion = 0;

	UPROPERTY()
	TArray<FString> Entries;
};
