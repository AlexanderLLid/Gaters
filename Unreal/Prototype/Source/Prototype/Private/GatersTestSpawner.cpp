#include "GatersTestSpawner.h"

#include "Camera/CameraActor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SkyLight.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GatersChunk.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/Parse.h"
#include "TimerManager.h"
#include "UnrealClient.h"

namespace
{
void ConfigureWorldLighting(UWorld& World)
{
	for (TActorIterator<ADirectionalLight> It(&World); It; ++It)
	{
		It->SetActorRotation(FRotator(-38.f, 135.f, 0.f));
		UDirectionalLightComponent* Sun =
			CastChecked<UDirectionalLightComponent>(It->GetLightComponent());
		Sun->SetIntensity(4.f);
		Sun->SetLightColor(FLinearColor(1.f, 0.95f, 0.9f), false);
		Sun->SetAtmosphereSunLight(true);
		break;
	}

	for (TActorIterator<ASkyLight> It(&World); It; ++It)
	{
		USkyLightComponent* Sky = It->GetLightComponent();
		Sky->SetIntensity(3.5f);
		Sky->SetLightColor(FLinearColor(0.85f, 0.9f, 1.f));
		Sky->SetLowerHemisphereColor(FLinearColor(0.18f, 0.2f, 0.24f));
		break;
	}

	for (TActorIterator<APostProcessVolume> It(&World); It; ++It)
	{
		It->bUnbound = true;
		It->Settings.bOverride_AutoExposureMinBrightness = true;
		It->Settings.bOverride_AutoExposureMaxBrightness = true;
		It->Settings.bOverride_AutoExposureBias = true;
		It->Settings.AutoExposureMinBrightness = 1.f;
		It->Settings.AutoExposureMaxBrightness = 1.f;
		It->Settings.AutoExposureBias = 1.35f;
		break;
	}

	UE_LOG(LogTemp, Display,
		TEXT("[GatersTestSpawner] LIGHTING sun=(-38,135) sun_lux=4 sky=3.5 exposure=1 bias=1.35"));
}

void SwitchSeed(const TArray<FString>& Args, UWorld* World)
{
	if (!World || !World->IsGameWorld() || Args.Num() != 1 || !Args[0].IsNumeric())
	{
		UE_LOG(LogTemp, Warning, TEXT("Usage: Gaters.Seed <integer>"));
		return;
	}

	const int32 Seed = FCString::Atoi(*Args[0]);
	const FString SeedPath = FPaths::ProjectSavedDir() / TEXT("TestSeed.txt");
	if (!FFileHelper::SaveStringToFile(FString::FromInt(Seed), *SeedPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Gaters.Seed could not write %s"), *SeedPath);
		return;
	}

	const FName LevelName(*UGameplayStatics::GetCurrentLevelName(World, true));
	UE_LOG(LogTemp, Display, TEXT("Gaters.Seed loading seed=%d level=%s"), Seed, *LevelName.ToString());
	UGameplayStatics::OpenLevel(World, LevelName);
}

FAutoConsoleCommandWithWorldAndArgs GatersSeedCommand(
	TEXT("Gaters.Seed"),
	TEXT("Reload the current generated world with an integer seed. Example: Gaters.Seed 53"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&SwitchSeed));
}

void UGatersTestSpawner::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	if (!InWorld.IsGameWorld() || !InWorld.GetMapName().Contains(TEXT("Lvl_GateGreybox")))
	{
		return;
	}
	ConfigureWorldLighting(InWorld);

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
			int32 GalleryRadius = 0;
			FParse::Value(FCommandLine::Get(), TEXT("GatersGalleryRadius="), GalleryRadius);
			for (TActorIterator<AGatersChunk> It(GetWorld()); It; ++It)
			{
				const int32 EffectiveRadius = GalleryRadius > 0
					? GalleryRadius
					: It->GalleryTerrainLoadRadius;
				const int32 ActiveCells = It->PrepareGalleryCapture(EffectiveRadius);
				UE_LOG(LogTemp, Display,
					TEXT("[GatersTestSpawner] gallery_stream radius=%d active=%d"),
					EffectiveRadius, ActiveCells);
				break;
			}
			float CameraOffsetXY = 26000.f;
			float CameraHeight = 40000.f;
			FParse::Value(FCommandLine::Get(), TEXT("GatersGalleryCameraOffset="), CameraOffsetXY);
			FParse::Value(FCommandLine::Get(), TEXT("GatersGalleryCameraHeight="), CameraHeight);
			const FVector CameraLocation =
				IslandOrigin + FVector(CameraOffsetXY, -CameraOffsetXY, CameraHeight);
			const FRotator CameraRotation = (IslandOrigin - CameraLocation).Rotation();
			if (ACameraActor* Camera = GetWorld()->SpawnActor<ACameraActor>(CameraLocation, CameraRotation))
			{
				if (APlayerController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0))
				{
					Controller->SetViewTarget(Camera);
					Pawn->SetActorHiddenInGame(true);
				}
			}
			UE_LOG(LogTemp, Display,
				TEXT("[GatersTestSpawner] gallery_camera offset=%.0f height=%.0f pitch=%.1f"),
				CameraOffsetXY, CameraHeight, CameraRotation.Pitch);
			const FString GalleryDir = FPaths::ProjectSavedDir() / TEXT("EnvironmentGallery");
			IFileManager::Get().MakeDirectory(*GalleryDir, true);
			const FString Screenshot = GalleryDir / FString::Printf(TEXT("seed-%d-beauty.png"), CurrentSeed);
			FScreenshotRequest::RequestScreenshot(Screenshot, false, false);
			UE_LOG(LogTemp, Display, TEXT("[GatersTestSpawner] gallery_beauty=%s"), *Screenshot);
			FTimerHandle TraversalCaptureHandle;
			GetWorld()->GetTimerManager().SetTimer(
				TraversalCaptureHandle, this, &UGatersTestSpawner::CaptureTraversalGallery, 1.f, false);
			return;
		}

		// gate pad plinth top is at +500 local; drop the player just above it
		Pawn->TeleportTo(IslandOrigin + FVector(0.f, 0.f, 640.f), FRotator(0.f, 0.f, 0.f));
	}
}

void UGatersTestSpawner::CaptureTraversalGallery()
{
	for (TActorIterator<AGatersChunk> It(GetWorld()); It; ++It)
	{
		It->DrawTraversalDebug(10.f);
		break;
	}
	const FString GalleryDir = FPaths::ProjectSavedDir() / TEXT("EnvironmentGallery");
	const FString Screenshot = GalleryDir / FString::Printf(TEXT("seed-%d-traversal.png"), CurrentSeed);
	FScreenshotRequest::RequestScreenshot(Screenshot, false, false);
	UE_LOG(LogTemp, Display, TEXT("[GatersTestSpawner] gallery_traversal=%s"), *Screenshot);
}
