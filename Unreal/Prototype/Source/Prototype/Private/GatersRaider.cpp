#include "GatersRaider.h"
#include "GatersDebugMessages.h"

#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GatersWorldDiff.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Navigation/PathFollowingComponent.h"
#include "UObject/ConstructorHelpers.h"

AGatersRaider::AGatersRaider()
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// foundation lips and jump-through-hole entry: bases sit on stilted foundations, so
	// the raider needs step + jump headroom a default character lacks
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->MaxStepHeight = 100.f;
	GetCharacterMovement()->JumpZVelocity = 900.f;

	UStaticMeshComponent* Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(GetCapsuleComponent());
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
	if (Cone.Succeeded())
	{
		Body->SetStaticMesh(Cone.Object);
		Body->SetRelativeLocation(FVector(0.f, 0.f, -40.f));
		Body->SetRelativeScale3D(FVector(0.9f, 0.9f, 1.6f));
	}
}

void AGatersRaider::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> Goals;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("RaidLoot"), Goals);
	float Best = TNumericLimits<float>::Max();
	for (AActor* G : Goals)
	{
		const float D = FVector::DistSquared(G->GetActorLocation(), GetActorLocation());
		if (D < Best)
		{
			Best = D;
			Loot = G;
		}
	}
	if (!Loot.IsValid())
	{
		Finish(false, TEXT("no-loot"));
		return;
	}
	StartApproach();
}

void AGatersRaider::StartApproach()
{
	State = EState::Approach;
	StallTime = 0.f;
	AAIController* AI = Cast<AAIController>(GetController());
	// controller possession can lag spawn by a frame; Direct mode covers until MoveTo works
	if (!AI || !Loot.IsValid() ||
		AI->MoveToActor(Loot.Get(), 120.f) == EPathFollowingRequestResult::Failed)
	{
		State = EState::Direct;
		return;
	}
	bPathfindingWorked = true;
}

void AGatersRaider::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (State == EState::Done)
	{
		return;
	}

	Elapsed += DeltaSeconds;
	if (Elapsed > TimeLimit)
	{
		Finish(false, TEXT("timeout"));
		return;
	}
	if (Loot.IsValid() && FVector::Dist(Loot->GetActorLocation(), GetActorLocation()) < 300.f)
	{
		Finish(true, TEXT("reached"));
		return;
	}

	const bool bStalled = !GetCharacterMovement()->IsFalling() && GetVelocity().Size2D() < 30.f;
	StallTime = bStalled ? StallTime + DeltaSeconds : 0.f;

	switch (State)
	{
	case EState::Approach:
		if (StallTime > 1.5f)
		{
			EnterBreach();
		}
		break;
	case EState::Direct:
		if (Loot.IsValid())
		{
			AddMovementInput((Loot->GetActorLocation() - GetActorLocation()).GetSafeNormal2D());
		}
		if (StallTime > 1.0f)
		{
			EnterBreach();
		}
		break;
	case EState::Attack:
		if (!Victim.IsValid())
		{
			EnterBreach();
			break;
		}
		AttackTime += DeltaSeconds;
		if (AttackTime > AttackSeconds)
		{
			Victim->Destroy();
			++Broken;
			Jump();
			StartApproach();
		}
		break;
	default:
		break;
	}
}

void AGatersRaider::EnterBreach()
{
	StallTime = 0.f;
	if (AActor* Target = NearestBreakable(AttackRange))
	{
		Victim = Target;
		AttackTime = 0.f;
		State = EState::Attack;
		return;
	}
	// nothing to break in reach: probably a ledge — jump and keep pushing straight
	Jump();
	State = EState::Direct;
}

AActor* AGatersRaider::NearestBreakable(float MaxDist) const
{
	TArray<AActor*> Pieces;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("Breakable"), Pieces);
	AActor* Best = nullptr;
	float BestD = MaxDist * MaxDist;
	for (AActor* P : Pieces)
	{
		const float D = FVector::DistSquared(P->GetActorLocation(), GetActorLocation());
		if (D < BestD)
		{
			BestD = D;
			Best = P;
		}
	}
	return Best;
}

void AGatersRaider::Finish(bool bSuccess, const TCHAR* Why)
{
	State = EState::Done;
	const float DistLeft = Loot.IsValid()
		? FVector::Dist(Loot->GetActorLocation(), GetActorLocation()) : -1.f;
	const FString Line = FString::Printf(
		TEXT("{\"context\":\"%s\",\"genversion\":%d,\"success\":%s,\"why\":\"%s\",\"time_s\":%.1f,\"broken\":%d,\"pathfinding\":%s,\"dist_left\":%.0f,\"ts\":\"%s\"}"),
		*Context, GatersGenVersion, bSuccess ? TEXT("true") : TEXT("false"), Why, Elapsed, Broken,
		bPathfindingWorked ? TEXT("true") : TEXT("false"), DistLeft, *FDateTime::UtcNow().ToIso8601());
	FFileHelper::SaveStringToFile(Line + LINE_TERMINATOR,
		*(FPaths::ProjectSavedDir() / TEXT("RaidResults.jsonl")),
		FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);

	UE_LOG(LogTemp, Display, TEXT("[GatersRaider] RAID %s"), *Line);
	FGatersDebugMessages::Show(
		FGatersDebugMessages::RaidKey,
		30.f,
		bSuccess ? FColor::Green : FColor::Red,
		FString::Printf(TEXT("RAID %s why=%s time=%.1fs broken=%d"),
			bSuccess ? TEXT("SUCCESS") : TEXT("FAIL"), Why, Elapsed, Broken));
	GetCharacterMovement()->DisableMovement();
	SetLifeSpan(5.f);
}

// user/headless entry: `Gaters.Raid seed=3` in the game console or -ExecCmds; spawns the
// raider next to the player, remaining args become the result context string
static FAutoConsoleCommandWithWorldAndArgs GatersRaidCmd(
	TEXT("Gaters.Raid"),
	TEXT("Spawn a probe raider at the player; args are recorded as the result context."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
		[](const TArray<FString>& Args, UWorld* World)
		{
			APawn* Player = UGameplayStatics::GetPlayerPawn(World, 0);
			const FVector At = Player ? Player->GetActorLocation() + FVector(200.f, 0.f, 50.f)
									  : FVector::ZeroVector;
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			if (AGatersRaider* Raider = World->SpawnActor<AGatersRaider>(At, FRotator::ZeroRotator, Params))
			{
				Raider->Context = FString::Join(Args, TEXT(" "));
			}
		}));
