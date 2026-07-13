#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GatersClaimMarker.generated.h"

class AGatersChunk;
class UStaticMeshComponent;

// Greybox claim pad on a buildable plot. Touch = claim; claimed pads turn blue
// and replay as already-claimed on regeneration.
UCLASS()
class PROTOTYPE_API AGatersClaimMarker : public AActor
{
	GENERATED_BODY()

public:
	AGatersClaimMarker();

	void Setup(int32 InPlotIndex, AGatersChunk* InChunk, bool bAlreadyClaimed);

	UPROPERTY(VisibleAnywhere)
	int32 PlotIndex = -1;

	UPROPERTY(VisibleAnywhere)
	bool bClaimed = false;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

protected:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	void ApplyClaimedLook();

	UPROPERTY()
	TObjectPtr<AGatersChunk> OwnerChunk;
};
