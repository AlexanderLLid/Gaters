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
#include "GatersDebugMessages.h"
#include "GatersSettlementGenerator.h"
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

void SwitchVillageStage(const TArray<FString>& Args, UWorld* World)
{
	if (!World || !World->IsGameWorld() || Args.Num() != 1 || !Args[0].IsNumeric())
	{
		UE_LOG(LogTemp, Warning, TEXT("Usage: Gaters.VillageStage <0|1|2>"));
		return;
	}
	const int32 Stage = FCString::Atoi(*Args[0]);
	if (!FGatersSettlementGenerator::IsSupportedGrowthStage(Stage))
	{
		UE_LOG(LogTemp, Warning, TEXT("Usage: Gaters.VillageStage <0|1|2>"));
		return;
	}
	const FString StagePath = FPaths::ProjectSavedDir() / TEXT("TestVillageStage.txt");
	if (!FFileHelper::SaveStringToFile(FString::FromInt(Stage), *StagePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Gaters.VillageStage could not write %s"), *StagePath);
		return;
	}
	const FName LevelName(*UGameplayStatics::GetCurrentLevelName(World, true));
	UE_LOG(LogTemp, Display, TEXT("Gaters.VillageStage loading stage=%d level=%s"),
		Stage, *LevelName.ToString());
	UGameplayStatics::OpenLevel(World, LevelName);
}

void SwitchBuiltSites(const TArray<FString>& Args, UWorld* World)
{
	if (!World || !World->IsGameWorld() || Args.Num() != 1 ||
		(Args[0] != TEXT("0") && Args[0] != TEXT("1")))
	{
		UE_LOG(LogTemp, Warning, TEXT("Usage: Gaters.BuiltSites <0|1>"));
		return;
	}
	const FString ModePath = FPaths::ProjectSavedDir() / TEXT("TestBuiltSites.txt");
	if (!FFileHelper::SaveStringToFile(Args[0], *ModePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Gaters.BuiltSites could not write %s"), *ModePath);
		return;
	}
	const FName LevelName(*UGameplayStatics::GetCurrentLevelName(World, true));
	UE_LOG(LogTemp, Display, TEXT("Gaters.BuiltSites loading enabled=%s level=%s"),
		*Args[0], *LevelName.ToString());
	UGameplayStatics::OpenLevel(World, LevelName);
}

void SwitchLandforms(const TArray<FString>& Args, UWorld* World)
{
	if (!World || !World->IsGameWorld() || Args.Num() != 1
		|| (Args[0] != TEXT("0") && Args[0] != TEXT("1")))
	{
		UE_LOG(LogTemp, Warning, TEXT("Usage: Gaters.Landforms <0|1>"));
		return;
	}
	const FString ModePath = FPaths::ProjectSavedDir() / TEXT("TestLandforms.txt");
	if (!FFileHelper::SaveStringToFile(Args[0], *ModePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Gaters.Landforms could not write %s"), *ModePath);
		return;
	}
	const FName LevelName(*UGameplayStatics::GetCurrentLevelName(World, true));
	UE_LOG(LogTemp, Display, TEXT("Gaters.Landforms loading enabled=%s level=%s"),
		*Args[0], *LevelName.ToString());
	UGameplayStatics::OpenLevel(World, LevelName);
}

void SwitchDebugMessages(const TArray<FString>& Args, UWorld* World)
{
	if (!World || !World->IsGameWorld() || Args.Num() != 1
		|| (Args[0] != TEXT("0") && Args[0] != TEXT("1")))
	{
		UE_LOG(LogTemp, Warning, TEXT("Usage: Gaters.DebugMessages <0|1>"));
		return;
	}
	const bool bEnabled = Args[0] == TEXT("1");
	const FString ModePath = FPaths::ProjectSavedDir() / TEXT("TestDebugMessages.txt");
	if (!FFileHelper::SaveStringToFile(Args[0], *ModePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Gaters.DebugMessages could not write %s"), *ModePath);
		return;
	}
	FGatersDebugMessages::SetEnabled(bEnabled);
	UE_LOG(LogTemp, Display, TEXT("Gaters.DebugMessages enabled=%s"),
		bEnabled ? TEXT("yes") : TEXT("no"));
}

FAutoConsoleCommandWithWorldAndArgs GatersSeedCommand(
	TEXT("Gaters.Seed"),
	TEXT("Reload the current generated world with an integer seed. Example: Gaters.Seed 53"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&SwitchSeed));

FAutoConsoleCommandWithWorldAndArgs GatersVillageStageCommand(
	TEXT("Gaters.VillageStage"),
	TEXT("Reload the generated world at settlement growth stage 0, 1, or 2."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&SwitchVillageStage));

FAutoConsoleCommandWithWorldAndArgs GatersBuiltSitesCommand(
	TEXT("Gaters.BuiltSites"),
	TEXT("Reload the generated world with the Built Site layer disabled (0) or enabled (1)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&SwitchBuiltSites));

FAutoConsoleCommandWithWorldAndArgs GatersLandformsCommand(
	TEXT("Gaters.Landforms"),
	TEXT("Reload the generated world with landform-process preview disabled (0) or enabled (1)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&SwitchLandforms));

FAutoConsoleCommandWithWorldAndArgs GatersDebugMessagesCommand(
	TEXT("Gaters.DebugMessages"),
	TEXT("Show (1) or hide (0) persistent Gaters-only on-screen diagnostics."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&SwitchDebugMessages));
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
	int32 VillageStage = 0;
	FString VillageStageText;
	if (FFileHelper::LoadFileToString(
		VillageStageText, *(FPaths::ProjectSavedDir() / TEXT("TestVillageStage.txt"))))
	{
		VillageStage = FCString::Atoi(*VillageStageText);
	}
	FParse::Value(FCommandLine::Get(), TEXT("GatersVillageStage="), VillageStage);
	VillageStage = FGatersSettlementGenerator::IsSupportedGrowthStage(VillageStage)
		? VillageStage : 0;
	CurrentVillageStage = VillageStage;
	int32 BuiltSites = 1;
	FString BuiltSitesText;
	if (FFileHelper::LoadFileToString(
		BuiltSitesText, *(FPaths::ProjectSavedDir() / TEXT("TestBuiltSites.txt"))))
	{
		BuiltSites = FCString::Atoi(*BuiltSitesText);
	}
	FParse::Value(FCommandLine::Get(), TEXT("GatersBuiltSites="), BuiltSites);
	bCurrentBuiltSites = BuiltSites != 0;
	int32 Landforms = 0;
	FString LandformsText;
	if (FFileHelper::LoadFileToString(
		LandformsText, *(FPaths::ProjectSavedDir() / TEXT("TestLandforms.txt"))))
	{
		Landforms = FCString::Atoi(*LandformsText);
	}
	FParse::Value(FCommandLine::Get(), TEXT("GatersLandforms="), Landforms);
	bCurrentLandforms = Landforms != 0;
	FParse::Value(FCommandLine::Get(), TEXT("GatersArtifactLabel="), CurrentArtifactLabel);
	float LandformRelief = -1.f;
	float LandformVolcanism = -1.f;
	float LandformIce = -1.f;
	FParse::Value(FCommandLine::Get(), TEXT("GatersLandformRelief="), LandformRelief);
	FParse::Value(FCommandLine::Get(), TEXT("GatersLandformVolcanism="), LandformVolcanism);
	FParse::Value(FCommandLine::Get(), TEXT("GatersLandformIce="), LandformIce);
	LandformRelief = LandformRelief < 0.f ? -1.f : FMath::Clamp(LandformRelief, 0.f, 1.f);
	LandformVolcanism = LandformVolcanism < 0.f ? -1.f : FMath::Clamp(LandformVolcanism, 0.f, 1.f);
	LandformIce = LandformIce < 0.f ? -1.f : FMath::Clamp(LandformIce, 0.f, 1.f);
	int32 DebugMessages = 0;
	FString DebugMessagesText;
	if (FFileHelper::LoadFileToString(
		DebugMessagesText, *(FPaths::ProjectSavedDir() / TEXT("TestDebugMessages.txt"))))
	{
		DebugMessages = FCString::Atoi(*DebugMessagesText);
	}
	FParse::Value(FCommandLine::Get(), TEXT("GatersDebugMessages="), DebugMessages);
	FGatersDebugMessages::SetEnabled(DebugMessages != 0);

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
			Chunk->VillageGrowthStage = VillageStage;
			Chunk->bEnableBuiltSites = bCurrentBuiltSites;
			Chunk->bEnableLandformProcesses = bCurrentLandforms;
			Chunk->LandformReliefOverride = LandformRelief;
			Chunk->LandformVolcanismOverride = LandformVolcanism;
			Chunk->LandformIceOverride = LandformIce;
			Chunk->FinishSpawning(FTransform(IslandOrigin));
		}
	}
	UE_LOG(LogTemp, Display,
		TEXT("[GatersTestSpawner] island ready seed=%d village_stage=%d built_sites=%s landforms=%s legacy_removed=%d"),
		Seed, VillageStage, bCurrentBuiltSites ? TEXT("yes") : TEXT("no"),
		bCurrentLandforms ? TEXT("yes") : TEXT("no"), Removed);

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
			FString ModeSuffix = bCurrentBuiltSites ? TEXT("") : TEXT("-world-only");
			ModeSuffix += bCurrentLandforms ? TEXT("-landforms") : TEXT("");
			ModeSuffix += CurrentArtifactLabel.IsEmpty()
				? TEXT("") : TEXT("-") + CurrentArtifactLabel;
			const FString Screenshot = GalleryDir / FString::Printf(
				TEXT("seed-%d%s-beauty.png"), CurrentSeed, *ModeSuffix);
			FScreenshotRequest::RequestScreenshot(Screenshot, false, false);
			UE_LOG(LogTemp, Display, TEXT("[GatersTestSpawner] gallery_beauty=%s"), *Screenshot);
			FTimerHandle TraversalCaptureHandle;
			GetWorld()->GetTimerManager().SetTimer(
				TraversalCaptureHandle, this, &UGatersTestSpawner::CaptureTraversalGallery, 1.f, false);
			return;
		}

		// arrival marker plinth top is at +500 local; drop the player just above it
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
	FString ModeSuffix = bCurrentBuiltSites ? TEXT("") : TEXT("-world-only");
	ModeSuffix += bCurrentLandforms ? TEXT("-landforms") : TEXT("");
	ModeSuffix += CurrentArtifactLabel.IsEmpty()
		? TEXT("") : TEXT("-") + CurrentArtifactLabel;
	const FString Screenshot = GalleryDir / FString::Printf(
		TEXT("seed-%d%s-traversal.png"), CurrentSeed, *ModeSuffix);
	FScreenshotRequest::RequestScreenshot(Screenshot, false, false);
	UE_LOG(LogTemp, Display, TEXT("[GatersTestSpawner] gallery_traversal=%s"), *Screenshot);
}
