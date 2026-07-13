#include "GatersScatter.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
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

	// deterministic variety from the stable id — never from the chunk's stream, which
	// must keep the exact same draw sequence for existing seeds
	const uint32 Hash = ::GetTypeHash(InId) * 2654435761u;

	if (bInTree)
	{
		// spruce-dominant nordic mix; four baked variants per species
		static const TCHAR* Spruce[] = { TEXT("A"), TEXT("B"), TEXT("C"), TEXT("D") };
		const bool bSpruce = (Hash % 4u) != 0u; // 75/25 spruce/hazel
		const TCHAR* Var = Spruce[(Hash >> 3) % 4u];
		const FString Path = bSpruce
			? FString::Printf(TEXT("/Game/Megaplant_Library/Tree_Norway_Spruce/Tree_Norway_Spruce_01/Tree_Norway_Spruce_01_%s.Tree_Norway_Spruce_01_%s"), Var, Var)
			: FString::Printf(TEXT("/Game/Megaplant_Library/Tree_Common_Hazel/Tree_Common_Hazel_01/Tree_Common_Hazel_01_%s.Tree_Common_Hazel_01_%s"), Var, Var);
		if (USkeletalMesh* TreeMesh = LoadObject<USkeletalMesh>(nullptr, *Path))
		{
			// cone stays as the invisible touch trigger around the trunk
			Mesh->SetVisibility(false);
			TreeComp = NewObject<USkeletalMeshComponent>(this, TEXT("Tree"));
			TreeComp->SetSkeletalMesh(TreeMesh);
			TreeComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			TreeComp->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);
			TreeComp->RegisterComponent();

			// normalize by measured bounds, never assumed size (mesh families differ)
			const float NaturalHeight = TreeMesh->GetBounds().BoxExtent.Z * 2.f;
			// hazel is a multi-stem bush, not a tree — stretching it past ~6 m reads as poles
			const float TargetHeight = (bSpruce ? 1200.f : 450.f) + (Hash % 100u) * (bSpruce ? 5.f : 1.5f);
			const float S = NaturalHeight > 1.f ? TargetHeight / NaturalHeight : 1.f;
			TreeComp->SetRelativeScale3D(FVector(S));
			// cone pivot is its center; drop the tree base to the cone's base
			TreeComp->SetRelativeLocation(FVector(0.f, 0.f, -50.f));
			TreeComp->SetRelativeRotation(FRotator(0.f, (Hash >> 8) % 360u, 0.f));
			return;
		}
	}
	else
	{
		static const TCHAR* Rocks[] = {
			TEXT("/Game/Fab/Megascans/3D/Nordic_Beach_Rock_Formation_vflrejtfa/Raw/vflrejtfa_tier_0/StaticMeshes/vflrejtfa_tier_0.vflrejtfa_tier_0"),
			TEXT("/Game/Fab/Megascans/3D/Tundra_Mossy_Boulder_vivvecldw/Raw/vivvecldw_tier_0/StaticMeshes/vivvecldw_tier_0.vivvecldw_tier_0") };
		if (UStaticMesh* RockMesh = LoadObject<UStaticMesh>(nullptr, Rocks[Hash % 2u]))
		{
			Mesh->SetStaticMesh(RockMesh);
			const FVector Extent = RockMesh->GetBoundingBox().GetExtent();
			const float Widest = FMath::Max(Extent.X, Extent.Y) * 2.f;
			const float Target = 250.f + (Hash % 100u) * 2.f;
			SetActorScale3D(FVector(Widest > 1.f ? Target / Widest : 1.f));
			SetActorRotation(FRotator(0.f, (Hash >> 8) % 360u, 0.f));
			return;
		}
	}

	// fallback greybox look when the pack assets are absent
	SetActorScale3D(bInTree ? FVector(0.6f, 0.6f, 4.f) : FVector(1.5f, 1.5f, 0.6f));
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
