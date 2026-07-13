#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GatersTestSpawner.generated.h"

// Owns the one-island prototype flow in Lvl_GateGreybox:
//  - removes the legacy BP prototype actors (old dial network, old far site) — those
//    flows are proven/superseded and no longer part of the play path
//  - spawns the C++ island chunk (seed override via Saved/TestSeed.txt if present)
//  - teleports the player onto the island's gate pad
// Delete when a real gate loop is rebuilt against AGatersChunk.
UCLASS()
class PROTOTYPE_API UGatersTestSpawner : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
	UFUNCTION()
	void PlacePlayerOnIsland();

	FVector IslandOrigin = FVector(18500.f, 40000.f, 0.f);
};
