#include "GatersClaimMarker.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Pawn.h"
#include "GatersChunk.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AGatersClaimMarker::AGatersClaimMarker()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}
	Mesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Mesh->SetGenerateOverlapEvents(true);
}

void AGatersClaimMarker::Setup(int32 InPlotIndex, AGatersChunk* InChunk, bool bAlreadyClaimed)
{
	PlotIndex = InPlotIndex;
	OwnerChunk = InChunk;
	bClaimed = bAlreadyClaimed;
	if (bClaimed)
	{
		ApplyClaimedLook();
	}
}

void AGatersClaimMarker::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (bClaimed || !Pawn || !Pawn->IsPlayerControlled() || !OwnerChunk)
	{
		return;
	}
	bClaimed = true;
	ApplyClaimedLook();
	OwnerChunk->ClaimPlot(PlotIndex);
}

void AGatersClaimMarker::ApplyClaimedLook()
{
	if (UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Gaters/Materials/MI_Claimed.MI_Claimed")))
	{
		Mesh->SetMaterial(0, Material);
	}
}
