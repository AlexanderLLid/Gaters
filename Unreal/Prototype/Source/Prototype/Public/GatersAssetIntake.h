#pragma once

#include "CoreMinimal.h"

class UStaticMesh;
class USkeletalMesh;
struct FGatersAssetContract;

struct PROTOTYPE_API FGatersAssetIntake
{
	static bool ValidateStaticMesh(
		const UStaticMesh& Mesh,
		const FGatersAssetContract& Contract,
		TArray<FString>& OutErrors,
		float BoundsToleranceCm = 0.1f);

	static bool ValidateSkeletalMesh(
		const USkeletalMesh& Mesh,
		const FGatersAssetContract& Contract,
		TArray<FString>& OutErrors,
		float BoundsToleranceCm = 0.1f);
};
