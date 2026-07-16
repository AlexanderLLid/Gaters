#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GatersScatter.generated.h"

class AGatersChunk;
class UStaticMeshComponent;

// Greybox harvestable stand-in (tree/rock). Identity carrier: ScatterId is the
// seed-derived grid cell index, stable across regeneration. Touch = chop.
UCLASS()
class PROTOTYPE_API AGatersScatter : public AActor
{
	GENERATED_BODY()

public:
	AGatersScatter();

	void Setup(int32 InId, AGatersChunk* InChunk, bool bInTree);

	UPROPERTY(VisibleAnywhere)
	int32 ScatterId = -1;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

protected:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	UPROPERTY()
	TObjectPtr<AGatersChunk> OwnerChunk;
};
