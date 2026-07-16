#include "GatersTestSpawner.h"

#include "Camera/CameraActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GatersChunk.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/Parse.h"
#include "TimerManager.h"
#include "UnrealClient.h"

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
	FParse::Value(FCommandLine::Get(), TEXT("GatersSeed="), Seed);
	CurrentSeed = Seed;

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
		if (FParse::Param(FCommandLine::Get(), TEXT("GatersGallery")))
		{
			const FVector CameraLocation = IslandOrigin + FVector(22000.f, -22000.f, 18000.f);
			const FRotator CameraRotation(-30.f, 135.f, 0.f);
			if (ACameraActor* Camera = GetWorld()->SpawnActor<ACameraActor>(CameraLocation, CameraRotation))
			{
				if (APlayerController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0))
				{
					Controller->SetViewTarget(Camera);
					Pawn->SetActorHiddenInGame(true);
				}
			}
			const FString GalleryDir = FPaths::ProjectSavedDir() / TEXT("EnvironmentGallery");
			IFileManager::Get().MakeDirectory(*GalleryDir, true);
			const FString Screenshot = GalleryDir / FString::Printf(TEXT("seed-%d.png"), CurrentSeed);
			FScreenshotRequest::RequestScreenshot(Screenshot, false, false);
			UE_LOG(LogTemp, Display, TEXT("[GatersTestSpawner] gallery=%s"), *Screenshot);
			return;
		}

		// gate pad plinth top is at +500 local; drop the player just above it
		Pawn->TeleportTo(IslandOrigin + FVector(0.f, 0.f, 640.f), FRotator(0.f, 0.f, 0.f));
	}
}
