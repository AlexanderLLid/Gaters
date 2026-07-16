#include "GatersScatter.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Pawn.h"
#include "GatersChunk.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AGatersScatter::AGatersScatter()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
	if (ConeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(ConeMesh.Object);
	}
	Mesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Mesh->SetGenerateOverlapEvents(true);
}

void AGatersScatter::Setup(int32 InId, AGatersChunk* InChunk, bool bInTree)
{
	ScatterId = InId;
	OwnerChunk = InChunk;

	// Native greybox primitives are deliberate asset slots. A later Blender tree or rock
	// replaces the mesh here without changing placement, ids, diffs, or generation.
	const uint32 Hash = ::GetTypeHash(InId) * 2654435761u;
	if (!bInTree)
	{
		if (UStaticMesh* RockMesh = LoadObject<UStaticMesh>(
			nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")))
		{
			Mesh->SetStaticMesh(RockMesh);
		}
	}

	const float Variation = 0.85f + (Hash % 31u) / 100.f;
	SetActorScale3D((bInTree ? FVector(0.8f, 0.8f, 5.f) : FVector(1.8f, 1.4f, 0.8f)) * Variation);
	SetActorRotation(FRotator(0.f, (Hash >> 8) % 360u, 0.f));
	const TCHAR* MaterialPath = bInTree
		? TEXT("/Game/Gaters/Materials/MI_ScatterTree.MI_ScatterTree")
		: TEXT("/Game/Gaters/Materials/MI_ScatterRock.MI_ScatterRock");
	if (UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, MaterialPath))
	{
		Mesh->SetMaterial(0, Material);
	}
}

void AGatersScatter::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	// player-only: AI pawns (probe raiders) must not mow the forest as they walk
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (Pawn && Pawn->IsPlayerControlled() && OwnerChunk)
	{
		OwnerChunk->ChopScatter(ScatterId, this);
	}
}
