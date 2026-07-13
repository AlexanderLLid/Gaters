#include "GatersTestSpawner.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GatersChunk.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "TimerManager.h"

void UGatersTestSpawner::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	if (!InWorld.IsGameWorld() || !InWorld.GetMapName().Contains(TEXT("Lvl_GateGreybox")))
	{
		return;
	}

	// legacy prototype actors: proven or superseded, out of the play path
	static const TCHAR* LegacyClasses[] = {
		TEXT("BP_TerrainChunk_C"), TEXT("BP_Gate_C"), TEXT("BP_RunClock_C"),
		TEXT("BP_LanePad_C"), TEXT("BP_BuildPlacer_C"), TEXT("BP_BuildPiece_C"),
		TEXT("BP_LootCube_C") };
	int32 Removed = 0;
	for (TActorIterator<AActor> It(&InWorld); It; ++It)
	{
		const FString ClassName = It->GetClass()->GetName();
		for (const TCHAR* Legacy : LegacyClasses)
		{
			if (ClassName == Legacy)
			{
				It->Destroy();
				++Removed;
				break;
			}
		}
	}

	int32 Seed = 7;
	FString SeedText;
	if (FFileHelper::LoadFileToString(SeedText, *(FPaths::ProjectSavedDir() / TEXT("TestSeed.txt"))))
	{
		Seed = FCString::Atoi(*SeedText);
	}

	bool bHaveChunk = false;
	for (TActorIterator<AGatersChunk> It(&InWorld); It; ++It)
	{
		bHaveChunk = true;
		break;
	}
	if (!bHaveChunk)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AGatersChunk* Chunk = InWorld.SpawnActorDeferred<AGatersChunk>(
			AGatersChunk::StaticClass(), FTransform(IslandOrigin), nullptr, nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (Chunk)
		{
			Chunk->Seed = Seed;
			Chunk->FinishSpawning(FTransform(IslandOrigin));
		}
	}
	UE_LOG(LogTemp, Display, TEXT("[GatersTestSpawner] island ready seed=%d legacy_removed=%d"), Seed, Removed);

	// player pawn spawns after world begin play; give it a beat, then move it over
	FTimerHandle Handle;
	InWorld.GetTimerManager().SetTimer(Handle, this, &UGatersTestSpawner::PlacePlayerOnIsland, 0.5f, false);
}

void UGatersTestSpawner::PlacePlayerOnIsland()
{
	if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		// gate pad plinth top is at +500 local; drop the player just above it
		Pawn->TeleportTo(IslandOrigin + FVector(0.f, 0.f, 640.f), FRotator(0.f, 0.f, 0.f));
	}
}
