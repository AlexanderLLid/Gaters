#if WITH_DEV_AUTOMATION_TESTS

#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Dom/JsonObject.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HAL/PlatformMisc.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Modules/ModuleManager.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace CharacterMovementFlatRoute
{
constexpr int32 ChallengerVersion = 1;
constexpr TCHAR CharacterLabMapName[] = TEXT("L_CharacterLab");
constexpr TCHAR CharacterLabMapPath[] = TEXT("/Game/Gaters/Generated/CharacterLab/L_CharacterLab");
constexpr TCHAR SkeletalMeshPath[] = TEXT("/Game/Gaters/Generated/CharacterLab/SK_GeneratedHumanoid.SK_GeneratedHumanoid");
constexpr TCHAR SkeletonPath[] = TEXT("/Game/Gaters/Generated/CharacterLab/SK_GeneratedHumanoid_Skeleton.SK_GeneratedHumanoid_Skeleton");
constexpr TCHAR PhysicsAssetPath[] = TEXT("/Game/Gaters/Generated/CharacterLab/SK_GeneratedHumanoid_PhysicsAsset.SK_GeneratedHumanoid_PhysicsAsset");
constexpr TCHAR IkRigPath[] = TEXT("/Game/Gaters/Generated/CharacterLab/IK_GeneratedHumanoid.IK_GeneratedHumanoid");

const TCHAR* PhaseNames[] = {
    TEXT("idle"), TEXT("walk"), TEXT("run"), TEXT("turn"),
    TEXT("stop"), TEXT("jump"), TEXT("fall"), TEXT("land"),
};
const TCHAR* ClipNames[] = {
    TEXT("A_Idle"), TEXT("A_Walk"), TEXT("A_Run"), TEXT("A_TurnLeft"),
    TEXT("A_Stop"), TEXT("A_Jump"), TEXT("A_Fall"), TEXT("A_Land"),
};
const TCHAR* ClipPaths[] = {
    TEXT("/Game/Gaters/Generated/CharacterLab/A_Idle.A_Idle"),
    TEXT("/Game/Gaters/Generated/CharacterLab/A_Walk.A_Walk"),
    TEXT("/Game/Gaters/Generated/CharacterLab/A_Run.A_Run"),
    TEXT("/Game/Gaters/Generated/CharacterLab/A_TurnLeft.A_TurnLeft"),
    TEXT("/Game/Gaters/Generated/CharacterLab/A_Stop.A_Stop"),
    TEXT("/Game/Gaters/Generated/CharacterLab/A_Jump.A_Jump"),
    TEXT("/Game/Gaters/Generated/CharacterLab/A_Fall.A_Fall"),
    TEXT("/Game/Gaters/Generated/CharacterLab/A_Land.A_Land"),
};

struct FRouteContract
{
    int32 RouteVersion = 0;
    float FixedDeltaSeconds = 0.0f;
    FVector SpawnLocation = FVector::ZeroVector;
    float CapsuleRadius = 0.0f;
    float CapsuleHalfHeight = 0.0f;
    float IdleSeconds = 0.0f;
    float WalkSeconds = 0.0f;
    float RunSeconds = 0.0f;
    float TurnSeconds = 0.0f;
    float StopSeconds = 0.0f;
    float LandSeconds = 0.0f;
    float WalkSpeed = 0.0f;
    float RunSpeed = 0.0f;
    float JumpSpeed = 0.0f;
    float MaximumAirborneSeconds = 0.0f;
    float MinimumForwardDisplacement = 0.0f;
    float MinimumTurnDisplacement = 0.0f;
    float MaximumFinalSpeed = 0.0f;
};

struct FRouteSample
{
    FString Phase;
    FString Clip;
    FVector Location = FVector::ZeroVector;
    FVector Velocity = FVector::ZeroVector;
    FVector Acceleration = FVector::ZeroVector;
    float FacingDegrees = 0.0f;
    FString MovementMode;
    bool bHasWalkableFloor = false;
};

UWorld* FindCharacterLabWorld()
{
    if (!GEngine)
    {
        return nullptr;
    }
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
    {
        UWorld* World = Context.World();
        if (World && World->GetMapName().Contains(CharacterLabMapName))
        {
            return World;
        }
    }
    return nullptr;
}

bool LoadRouteContract(const FString& Path, FRouteContract& OutContract, FString& OutError)
{
    FString Text;
    if (!FFileHelper::LoadFileToString(Text, *Path))
    {
        OutError = FString::Printf(TEXT("Could not read route contract: %s"), *Path);
        return false;
    }

    TSharedPtr<FJsonObject> Root;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Text);
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        OutError = TEXT("Route contract is not valid JSON.");
        return false;
    }

    const TArray<TSharedPtr<FJsonValue>>* Spawn = nullptr;
    const TSharedPtr<FJsonObject>* Durations = nullptr;
    if (Root->GetIntegerField(TEXT("schemaVersion")) != 1
        || !Root->TryGetArrayField(TEXT("spawnLocationCentimeters"), Spawn)
        || Spawn->Num() != 3
        || !Root->TryGetObjectField(TEXT("groundedPhaseSeconds"), Durations))
    {
        OutError = TEXT("Route contract schema is unsupported or incomplete.");
        return false;
    }

    OutContract.RouteVersion = Root->GetIntegerField(TEXT("routeVersion"));
    OutContract.FixedDeltaSeconds = Root->GetNumberField(TEXT("fixedDeltaSeconds"));
    OutContract.SpawnLocation = FVector(
        (*Spawn)[0]->AsNumber(), (*Spawn)[1]->AsNumber(), (*Spawn)[2]->AsNumber());
    OutContract.CapsuleRadius = Root->GetNumberField(TEXT("capsuleRadiusCentimeters"));
    OutContract.CapsuleHalfHeight = Root->GetNumberField(TEXT("capsuleHalfHeightCentimeters"));
    OutContract.IdleSeconds = (*Durations)->GetNumberField(TEXT("idle"));
    OutContract.WalkSeconds = (*Durations)->GetNumberField(TEXT("walk"));
    OutContract.RunSeconds = (*Durations)->GetNumberField(TEXT("run"));
    OutContract.TurnSeconds = (*Durations)->GetNumberField(TEXT("turn"));
    OutContract.StopSeconds = (*Durations)->GetNumberField(TEXT("stop"));
    OutContract.LandSeconds = (*Durations)->GetNumberField(TEXT("land"));
    OutContract.WalkSpeed = Root->GetNumberField(TEXT("walkSpeedCentimetersPerSecond"));
    OutContract.RunSpeed = Root->GetNumberField(TEXT("runSpeedCentimetersPerSecond"));
    OutContract.JumpSpeed = Root->GetNumberField(TEXT("jumpSpeedCentimetersPerSecond"));
    OutContract.MaximumAirborneSeconds = Root->GetNumberField(TEXT("maximumAirborneSeconds"));
    OutContract.MinimumForwardDisplacement = Root->GetNumberField(TEXT("minimumForwardDisplacementCentimeters"));
    OutContract.MinimumTurnDisplacement = Root->GetNumberField(TEXT("minimumTurnDisplacementCentimeters"));
    OutContract.MaximumFinalSpeed = Root->GetNumberField(TEXT("maximumFinalSpeedCentimetersPerSecond"));

    if (OutContract.RouteVersion != 1 || OutContract.FixedDeltaSeconds <= 0.0f)
    {
        OutError = TEXT("Route contract version or fixed delta is invalid.");
        return false;
    }
    return true;
}

FString MovementModeName(const UCharacterMovementComponent* Movement)
{
    if (Movement->IsMovingOnGround())
    {
        return TEXT("Walking");
    }
    if (Movement->IsFalling())
    {
        return TEXT("Falling");
    }
    return Movement->GetMovementName();
}

FRouteSample CaptureSample(const TCHAR* Phase, const TCHAR* Clip, const ACharacter* Character)
{
    const UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
    FRouteSample Sample;
    Sample.Phase = Phase;
    Sample.Clip = Clip;
    Sample.Location = Character->GetActorLocation();
    Sample.Velocity = Movement->Velocity;
    Sample.Acceleration = Movement->GetCurrentAcceleration();
    Sample.FacingDegrees = Character->GetActorRotation().Yaw;
    Sample.MovementMode = MovementModeName(Movement);
    Sample.bHasWalkableFloor = Movement->CurrentFloor.IsWalkableFloor();
    return Sample;
}

void TickMovement(ACharacter* Character, float FixedDelta, const FVector& Input)
{
    if (!Input.IsNearlyZero())
    {
        Character->AddMovementInput(Input, 1.0f, true);
    }
    Character->GetCharacterMovement()->TickComponent(FixedDelta, LEVELTICK_All, nullptr);
}

void TickFor(ACharacter* Character, float FixedDelta, float Seconds, const FVector& Input)
{
    const int32 Frames = FMath::Max(1, FMath::CeilToInt(Seconds / FixedDelta));
    for (int32 Frame = 0; Frame < Frames; ++Frame)
    {
        TickMovement(Character, FixedDelta, Input);
    }
}

void PlayClip(USkeletalMeshComponent* Mesh, UAnimSequence* Clip, bool bLoop)
{
    Mesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
    Mesh->PlayAnimation(Clip, bLoop);
}

TArray<TSharedPtr<FJsonValue>> VectorJson(const FVector& Vector)
{
    return {
        MakeShared<FJsonValueNumber>(FMath::RoundToDouble(Vector.X * 1000000.0) / 1000000.0),
        MakeShared<FJsonValueNumber>(FMath::RoundToDouble(Vector.Y * 1000000.0) / 1000000.0),
        MakeShared<FJsonValueNumber>(FMath::RoundToDouble(Vector.Z * 1000000.0) / 1000000.0),
    };
}

bool WriteEvidence(
    const FString& Path,
    const FRouteContract& Contract,
    const ACharacter* Character,
    const TArray<FRouteSample>& Samples,
    bool bEditorAdapterLoaded)
{
    TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
    Root->SetNumberField(TEXT("schemaVersion"), 1);
    Root->SetNumberField(TEXT("challengerVersion"), ChallengerVersion);
    Root->SetNumberField(TEXT("routeVersion"), Contract.RouteVersion);
    Root->SetStringField(TEXT("map"), CharacterLabMapPath);

    TSharedRef<FJsonObject> Assets = MakeShared<FJsonObject>();
    Assets->SetStringField(TEXT("skeletalMesh"), SkeletalMeshPath);
    Assets->SetStringField(TEXT("skeleton"), SkeletonPath);
    Assets->SetStringField(TEXT("physicsAsset"), PhysicsAssetPath);
    Assets->SetStringField(TEXT("ikRig"), IkRigPath);
    TSharedRef<FJsonObject> Clips = MakeShared<FJsonObject>();
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(PhaseNames); ++Index)
    {
        Clips->SetStringField(PhaseNames[Index], ClipPaths[Index]);
    }
    Assets->SetObjectField(TEXT("clips"), Clips);
    Root->SetObjectField(TEXT("assets"), Assets);

    const UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
    TSharedRef<FJsonObject> Runtime = MakeShared<FJsonObject>();
    Runtime->SetStringField(TEXT("actorClass"), Character->GetClass()->GetName());
    Runtime->SetStringField(TEXT("movementComponentClass"), Movement->GetClass()->GetName());
    Runtime->SetStringField(TEXT("updatedComponentClass"), Movement->UpdatedComponent->GetClass()->GetName());
    Runtime->SetBoolField(TEXT("editorAdapterLoaded"), bEditorAdapterLoaded);
    Root->SetObjectField(TEXT("runtime"), Runtime);

    TArray<TSharedPtr<FJsonValue>> ObservedPhases;
    TArray<TSharedPtr<FJsonValue>> SampleValues;
    for (const FRouteSample& Sample : Samples)
    {
        ObservedPhases.Add(MakeShared<FJsonValueString>(Sample.Phase));
        TSharedRef<FJsonObject> Value = MakeShared<FJsonObject>();
        Value->SetStringField(TEXT("phase"), Sample.Phase);
        Value->SetStringField(TEXT("clip"), Sample.Clip);
        Value->SetArrayField(TEXT("locationCentimeters"), VectorJson(Sample.Location));
        Value->SetArrayField(TEXT("velocityCentimetersPerSecond"), VectorJson(Sample.Velocity));
        Value->SetArrayField(TEXT("accelerationCentimetersPerSecondSquared"), VectorJson(Sample.Acceleration));
        Value->SetNumberField(TEXT("facingDegrees"), FMath::RoundToDouble(Sample.FacingDegrees * 1000000.0) / 1000000.0);
        Value->SetStringField(TEXT("movementMode"), Sample.MovementMode);
        Value->SetBoolField(TEXT("hasWalkableFloor"), Sample.bHasWalkableFloor);
        SampleValues.Add(MakeShared<FJsonValueObject>(Value));
    }
    Root->SetArrayField(TEXT("observedPhases"), ObservedPhases);
    Root->SetArrayField(TEXT("samples"), SampleValues);

    TSharedRef<FJsonObject> Final = MakeShared<FJsonObject>();
    Final->SetArrayField(TEXT("locationCentimeters"), VectorJson(Character->GetActorLocation()));
    Final->SetNumberField(TEXT("speedCentimetersPerSecond"), FMath::RoundToDouble(Movement->Velocity.Size() * 1000000.0) / 1000000.0);
    Final->SetStringField(TEXT("movementMode"), MovementModeName(Movement));
    Final->SetBoolField(TEXT("hasWalkableFloor"), Movement->CurrentFloor.IsWalkableFloor());
    Root->SetObjectField(TEXT("final"), Final);

    FString Text;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Text);
    if (!FJsonSerializer::Serialize(Root, Writer))
    {
        return false;
    }
    return FFileHelper::SaveStringToFile(
        Text + LINE_TERMINATOR,
        *Path,
        FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterMovementFlatRouteTest,
    "Gaters.CharacterLab.CharacterMovement.FlatRoute",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCharacterMovementFlatRouteTest::RunTest(const FString& Parameters)
{
    using namespace CharacterMovementFlatRoute;

    const FString ContractPath = FPlatformMisc::GetEnvironmentVariable(TEXT("GATERS_CHARACTER_MOVEMENT_CONTRACT"));
    const FString EvidencePath = FPlatformMisc::GetEnvironmentVariable(TEXT("GATERS_CHARACTER_MOVEMENT_EVIDENCE"));
    if (ContractPath.IsEmpty() || EvidencePath.IsEmpty())
    {
        AddError(TEXT("GATERS_CHARACTER_MOVEMENT_CONTRACT and GATERS_CHARACTER_MOVEMENT_EVIDENCE are required."));
        return false;
    }

    FRouteContract Contract;
    FString ContractError;
    if (!LoadRouteContract(ContractPath, Contract, ContractError))
    {
        AddError(ContractError);
        return false;
    }

    UWorld* World = FindCharacterLabWorld();
    if (!TestNotNull(TEXT("Generated CharacterLab map is loaded"), World))
    {
        return false;
    }

    USkeletalMesh* SkeletalMesh = LoadObject<USkeletalMesh>(nullptr, SkeletalMeshPath);
    USkeleton* Skeleton = LoadObject<USkeleton>(nullptr, SkeletonPath);
    UPhysicsAsset* PhysicsAsset = LoadObject<UPhysicsAsset>(nullptr, PhysicsAssetPath);
    UObject* IkRig = LoadObject<UObject>(nullptr, IkRigPath);
    TArray<UAnimSequence*> Clips;
    if (!TestNotNull(TEXT("Generated skeletal mesh is loadable"), SkeletalMesh)
        || !TestNotNull(TEXT("Generated skeleton is loadable"), Skeleton)
        || !TestNotNull(TEXT("Generated Physics Asset is loadable"), PhysicsAsset)
        || !TestNotNull(TEXT("Generated IK Rig is loadable"), IkRig))
    {
        return false;
    }
    for (const TCHAR* ClipPath : ClipPaths)
    {
        UAnimSequence* Clip = LoadObject<UAnimSequence>(nullptr, ClipPath);
        if (!TestNotNull(*FString::Printf(TEXT("Generated clip is loadable: %s"), ClipPath), Clip))
        {
            return false;
        }
        Clips.Add(Clip);
    }

    ACharacter* Character = World->SpawnActor<ACharacter>(ACharacter::StaticClass(), Contract.SpawnLocation, FRotator::ZeroRotator);
    if (!TestNotNull(TEXT("Native ACharacter spawns"), Character))
    {
        return false;
    }

    USkeletalMeshComponent* Mesh = Character->GetMesh();
    UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
    Character->GetCapsuleComponent()->SetCapsuleSize(Contract.CapsuleRadius, Contract.CapsuleHalfHeight);
    Mesh->SetRelativeLocation(FVector(0.0, 0.0, -Contract.CapsuleHalfHeight));
    Mesh->SetSkeletalMeshAsset(SkeletalMesh);
    Mesh->SetPhysicsAsset(PhysicsAsset, false);
    Movement->bRunPhysicsWithNoController = true;
    Movement->bOrientRotationToMovement = true;
    Movement->RotationRate = FRotator(0.0, 720.0, 0.0);
    Movement->JumpZVelocity = Contract.JumpSpeed;
    Movement->BrakingDecelerationWalking = 2048.0f;
    Movement->SetMovementMode(MOVE_Falling);

    TestEqual(TEXT("Generated mesh uses the imported Skeleton"), SkeletalMesh->GetSkeleton(), Skeleton);
    TestEqual(TEXT("ACharacter owns native CharacterMovement"), Movement->GetClass()->GetName(), FString(TEXT("CharacterMovementComponent")));
    TestTrue(TEXT("CharacterMovement updates the capsule"), Movement->UpdatedComponent.Get() == Character->GetCapsuleComponent());
    const bool bEditorAdapterLoaded = FModuleManager::Get().IsModuleLoaded(TEXT("CharacterPhysicsProfileAdapter"));
    TestFalse(TEXT("Editor-only intake adapter is not loaded"), bEditorAdapterLoaded);

    const int32 SettleFrames = FMath::CeilToInt(2.0f / Contract.FixedDeltaSeconds);
    for (int32 Frame = 0; Frame < SettleFrames && !Movement->IsMovingOnGround(); ++Frame)
    {
        TickMovement(Character, Contract.FixedDeltaSeconds, FVector::ZeroVector);
    }
    FHitResult DownwardHit;
    const FVector TraceStart = Character->GetActorLocation();
    const bool bDownwardTraceHit = World->LineTraceSingleByChannel(
        DownwardHit,
        TraceStart,
        TraceStart - FVector(0.0, 0.0, 1000.0),
        ECC_Visibility);
    FFindFloorResult DirectFloor;
    Movement->FindFloor(Character->GetActorLocation(), DirectFloor, false);
    AddInfo(FString::Printf(
        TEXT("Floor diagnostic worldType=%d begunPlay=%d actorTick=%d movementRegistered=%d movementActive=%d movementTick=%d capsuleCollision=%d location=%s velocity=%s mode=%s currentFloor=%d directFloor=%d traceHit=%d traceActor=%s traceImpact=%s"),
        static_cast<int32>(World->WorldType),
        World->HasBegunPlay() ? 1 : 0,
        Character->IsActorTickEnabled() ? 1 : 0,
        Movement->IsRegistered() ? 1 : 0,
        Movement->IsActive() ? 1 : 0,
        Movement->IsComponentTickEnabled() ? 1 : 0,
        static_cast<int32>(Character->GetCapsuleComponent()->GetCollisionEnabled()),
        *Character->GetActorLocation().ToString(),
        *Movement->Velocity.ToString(),
        *MovementModeName(Movement),
        Movement->CurrentFloor.IsWalkableFloor() ? 1 : 0,
        DirectFloor.IsWalkableFloor() ? 1 : 0,
        bDownwardTraceHit ? 1 : 0,
        bDownwardTraceHit && DownwardHit.GetActor() ? *DownwardHit.GetActor()->GetName() : TEXT("none"),
        *DownwardHit.ImpactPoint.ToString()));
    if (!TestTrue(TEXT("CharacterMovement discovers the flat floor"), Movement->IsMovingOnGround() && Movement->CurrentFloor.IsWalkableFloor()))
    {
        Character->Destroy();
        return false;
    }

    TArray<FRouteSample> Samples;
    PlayClip(Mesh, Clips[0], true);
    TickFor(Character, Contract.FixedDeltaSeconds, Contract.IdleSeconds, FVector::ZeroVector);
    Samples.Add(CaptureSample(PhaseNames[0], ClipNames[0], Character));

    Movement->MaxWalkSpeed = Contract.WalkSpeed;
    PlayClip(Mesh, Clips[1], true);
    TickFor(Character, Contract.FixedDeltaSeconds, Contract.WalkSeconds, FVector::ForwardVector);
    Samples.Add(CaptureSample(PhaseNames[1], ClipNames[1], Character));

    Movement->MaxWalkSpeed = Contract.RunSpeed;
    PlayClip(Mesh, Clips[2], true);
    TickFor(Character, Contract.FixedDeltaSeconds, Contract.RunSeconds, FVector::ForwardVector);
    Samples.Add(CaptureSample(PhaseNames[2], ClipNames[2], Character));

    Movement->MaxWalkSpeed = Contract.WalkSpeed;
    PlayClip(Mesh, Clips[3], false);
    TickFor(Character, Contract.FixedDeltaSeconds, Contract.TurnSeconds, -FVector::RightVector);
    Samples.Add(CaptureSample(PhaseNames[3], ClipNames[3], Character));

    PlayClip(Mesh, Clips[4], false);
    TickFor(Character, Contract.FixedDeltaSeconds, Contract.StopSeconds, FVector::ZeroVector);
    Samples.Add(CaptureSample(PhaseNames[4], ClipNames[4], Character));

    PlayClip(Mesh, Clips[5], false);
    TestTrue(TEXT("Native CharacterMovement accepts the jump"), Movement->DoJump(false, Contract.FixedDeltaSeconds));
    const int32 AirborneFrames = FMath::CeilToInt(Contract.MaximumAirborneSeconds / Contract.FixedDeltaSeconds);
    for (int32 Frame = 0; Frame < AirborneFrames && !Movement->IsFalling(); ++Frame)
    {
        TickMovement(Character, Contract.FixedDeltaSeconds, FVector::ZeroVector);
    }
    Samples.Add(CaptureSample(PhaseNames[5], ClipNames[5], Character));

    PlayClip(Mesh, Clips[6], true);
    for (int32 Frame = 0; Frame < AirborneFrames && Movement->IsFalling() && Movement->Velocity.Z > 0.0f; ++Frame)
    {
        TickMovement(Character, Contract.FixedDeltaSeconds, FVector::ZeroVector);
    }
    Samples.Add(CaptureSample(PhaseNames[6], ClipNames[6], Character));
    for (int32 Frame = 0; Frame < AirborneFrames && Movement->IsFalling(); ++Frame)
    {
        TickMovement(Character, Contract.FixedDeltaSeconds, FVector::ZeroVector);
    }

    PlayClip(Mesh, Clips[7], false);
    TickFor(Character, Contract.FixedDeltaSeconds, Contract.LandSeconds, FVector::ZeroVector);
    Samples.Add(CaptureSample(PhaseNames[7], ClipNames[7], Character));

    const double ForwardDisplacement = Samples[2].Location.X - Samples[0].Location.X;
    const double TurnDisplacement = FMath::Abs(Samples[3].Location.Y - Samples[2].Location.Y);
    TestTrue(TEXT("Idle, walk, run, turn, stop, and land stay grounded"),
        Samples[0].bHasWalkableFloor && Samples[1].bHasWalkableFloor
        && Samples[2].bHasWalkableFloor && Samples[3].bHasWalkableFloor
        && Samples[4].bHasWalkableFloor && Samples[7].bHasWalkableFloor);
    TestTrue(TEXT("Walk and run move the capsule forward"), ForwardDisplacement >= Contract.MinimumForwardDisplacement);
    TestTrue(TEXT("Turn moves the capsule laterally"), TurnDisplacement >= Contract.MinimumTurnDisplacement);
    TestEqual(TEXT("Jump enters native falling mode"), Samples[5].MovementMode, FString(TEXT("Falling")));
    TestEqual(TEXT("Fall remains in native falling mode"), Samples[6].MovementMode, FString(TEXT("Falling")));
    TestEqual(TEXT("Land returns to native walking mode"), Samples[7].MovementMode, FString(TEXT("Walking")));
    TestTrue(TEXT("Route finishes stopped"), Movement->Velocity.Size() <= Contract.MaximumFinalSpeed);
    TestTrue(TEXT("Stable movement evidence is written"), WriteEvidence(EvidencePath, Contract, Character, Samples, bEditorAdapterLoaded));

    Character->Destroy();
    return true;
}

#endif
