#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GatersTestSpawner.generated.h"

// Owns the one-island prototype flow in Lvl_GateGreybox:
//  - removes the legacy BP prototype actors (old dial network, old far site) — those
//    flows are proven/superseded and no longer part of the play path
//  - spawns the C++ island chunk (seed override via Saved/TestSeed.txt if present)
//  - teleports the player onto the island's arrival marker
// Delete when a real Rift arrival loop is rebuilt against AGatersChunk.
UCLASS()
class PROTOTYPE_API UGatersTestSpawner : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
	UFUNCTION()
	void PlacePlayerOnIsland();
	void CaptureTraversalGallery();

	FVector IslandOrigin = FVector(18500.f, 40000.f, 0.f);
	int32 CurrentSeed = 7;
	int32 CurrentVillageStage = 0;
	bool bCurrentBuiltSites = true;
	bool bCurrentLandforms = false;
	FString CurrentArtifactLabel;
};
