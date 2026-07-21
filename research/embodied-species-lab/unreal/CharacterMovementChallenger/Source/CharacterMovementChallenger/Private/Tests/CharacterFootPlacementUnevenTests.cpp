#if WITH_DEV_AUTOMATION_TESTS

#include "CharacterFootPlacementAnimInstance.h"

#include "Animation/AnimSequence.h"
#include "Components/BoxComponent.h"
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
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace CharacterFootPlacementUneven
{
constexpr int32 ChallengerVersion = 1;
constexpr TCHAR CharacterLabMapName[] = TEXT("L_CharacterLab");
constexpr TCHAR SkeletalMeshPath[] = TEXT("/Game/Gaters/Generated/CharacterLab/SK_GeneratedHumanoid.SK_GeneratedHumanoid");
constexpr TCHAR WalkClipPath[] = TEXT("/Game/Gaters/Generated/CharacterLab/A_Walk.A_Walk");

const TCHAR* RequiredRoles[] = {
    TEXT("root"), TEXT("pelvis"),
    TEXT("foot_l"), TEXT("ball_l"), TEXT("ik_foot_l"),
    TEXT("foot_r"), TEXT("ball_r"), TEXT("ik_foot_r"),
};

struct FContract
{
    int32 RouteVersion = 0;
    float FixedDelta = 0.0f;
    float SettleSeconds = 0.0f;
    FVector Spawn = FVector::ZeroVector;
    float CapsuleRadius = 0.0f;
    float CapsuleHalfHeight = 0.0f;
    float SupportHeight = 0.0f;
    FVector SupportHalfExtent = FVector::ZeroVector;
    float RouteWalkSpeed = 0.0f;
    float RoutePhaseSeconds = 0.0f;
    float StepCenterOffset = 0.0f;
    FVector StepHalfExtent = FVector::ZeroVector;
    float SlopeCenterOffset = 0.0f;
    float SlopeCenterHeight = 0.0f;
    FVector SlopeHalfExtent = FVector::ZeroVector;
    float SlopePitch = 0.0f;
    float MaximumContactError = 0.0f;
    float MaximumPelvisOffset = 0.0f;
    float MinimumTargetDelta = 0.0f;
    float MinimumRouteDisplacement = 0.0f;
    float MinimumFloorHeightDelta = 0.0f;
    float MaximumFinalSpeed = 0.0f;
    FString GeneratedWalkClip;
    float AnimationPhaseA = 0.0f;
    float AnimationPhaseB = 0.0f;
    float MinimumAnimationTimeAdvance = 0.0f;
    float MinimumAnimatedThighDirectionDelta = 0.0f;
};

struct FFootSample
{
    bool bWalkableHit = false;
    float ContactError = 0.0f;
    float TargetDelta = 0.0f;
};

struct FCaseSample
{
    FString Name;
    FString MovementMode;
    bool bHasWalkableFloor = false;
    bool bHasNaN = false;
    float PelvisOffset = 0.0f;
    float Speed = 0.0f;
    FVector ActorLocation = FVector::ZeroVector;
    float CharacterFloorHeight = 0.0f;
    FFootSample Left;
    FFootSample Right;
};

struct FPoseSample
{
    FTransform Pelvis;
    FTransform LeftFoot;
    FTransform RightFoot;
    FTransform LeftTarget;
    FTransform RightTarget;
};

struct FAnimationSample
{
    FString Name;
    float AssetTime = 0.0f;
    FVector RawLeftThighEulerDegrees = FVector::ZeroVector;
    FVector RawRightThighEulerDegrees = FVector::ZeroVector;
    FVector LeftThighDirection = FVector::ZeroVector;
    FVector RightThighDirection = FVector::ZeroVector;
};

UWorld* FindWorld()
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

bool ReadVector(const TArray<TSharedPtr<FJsonValue>>* Values, FVector& OutVector)
{
    if (!Values || Values->Num() != 3)
    {
        return false;
    }
    OutVector = FVector((*Values)[0]->AsNumber(), (*Values)[1]->AsNumber(), (*Values)[2]->AsNumber());
    return true;
}

bool LoadContract(const FString& Path, FContract& OutContract, FString& OutError)
{
    FString Text;
    if (!FFileHelper::LoadFileToString(Text, *Path))
    {
        OutError = FString::Printf(TEXT("Could not read Foot Placement contract: %s"), *Path);
        return false;
    }
    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Root) || !Root.IsValid())
    {
        OutError = TEXT("Foot Placement contract is not valid JSON.");
        return false;
    }

    const TArray<TSharedPtr<FJsonValue>>* Spawn = nullptr;
    const TArray<TSharedPtr<FJsonValue>>* HalfExtent = nullptr;
    const TArray<TSharedPtr<FJsonValue>>* StepHalfExtent = nullptr;
    const TArray<TSharedPtr<FJsonValue>>* SlopeHalfExtent = nullptr;
    const TArray<TSharedPtr<FJsonValue>>* AnimationPhases = nullptr;
    if (Root->GetIntegerField(TEXT("schemaVersion")) != 1
        || !Root->TryGetArrayField(TEXT("spawnLocationCentimeters"), Spawn)
        || !Root->TryGetArrayField(TEXT("supportHalfExtentCentimeters"), HalfExtent)
        || !Root->TryGetArrayField(TEXT("stepHalfExtentCentimeters"), StepHalfExtent)
        || !Root->TryGetArrayField(TEXT("slopeHalfExtentCentimeters"), SlopeHalfExtent)
        || !Root->TryGetArrayField(TEXT("animationPhaseSeconds"), AnimationPhases)
        || AnimationPhases->Num() != 2
        || !ReadVector(Spawn, OutContract.Spawn)
        || !ReadVector(HalfExtent, OutContract.SupportHalfExtent)
        || !ReadVector(StepHalfExtent, OutContract.StepHalfExtent)
        || !ReadVector(SlopeHalfExtent, OutContract.SlopeHalfExtent))
    {
        OutError = TEXT("Foot Placement contract schema is incomplete.");
        return false;
    }

    OutContract.RouteVersion = Root->GetIntegerField(TEXT("routeVersion"));
    OutContract.FixedDelta = Root->GetNumberField(TEXT("fixedDeltaSeconds"));
    OutContract.SettleSeconds = Root->GetNumberField(TEXT("settleSeconds"));
    OutContract.CapsuleRadius = Root->GetNumberField(TEXT("capsuleRadiusCentimeters"));
    OutContract.CapsuleHalfHeight = Root->GetNumberField(TEXT("capsuleHalfHeightCentimeters"));
    OutContract.SupportHeight = Root->GetNumberField(TEXT("supportHeightCentimeters"));
    OutContract.RouteWalkSpeed = Root->GetNumberField(TEXT("routeWalkSpeedCentimetersPerSecond"));
    OutContract.RoutePhaseSeconds = Root->GetNumberField(TEXT("routePhaseSeconds"));
    OutContract.StepCenterOffset = Root->GetNumberField(TEXT("stepCenterForwardOffsetCentimeters"));
    OutContract.SlopeCenterOffset = Root->GetNumberField(TEXT("slopeCenterForwardOffsetCentimeters"));
    OutContract.SlopeCenterHeight = Root->GetNumberField(TEXT("slopeCenterHeightCentimeters"));
    OutContract.SlopePitch = Root->GetNumberField(TEXT("slopePitchDegrees"));
    OutContract.MaximumContactError = Root->GetNumberField(TEXT("maximumAbsoluteContactErrorCentimeters"));
    OutContract.MaximumPelvisOffset = Root->GetNumberField(TEXT("maximumAbsolutePelvisOffsetCentimeters"));
    OutContract.MinimumTargetDelta = Root->GetNumberField(TEXT("minimumTerrainTargetDeltaCentimeters"));
    OutContract.MinimumRouteDisplacement = Root->GetNumberField(TEXT("minimumRouteForwardDisplacementCentimeters"));
    OutContract.MinimumFloorHeightDelta = Root->GetNumberField(TEXT("minimumCharacterFloorHeightDeltaCentimeters"));
    OutContract.MaximumFinalSpeed = Root->GetNumberField(TEXT("maximumFinalSpeedCentimetersPerSecond"));
    OutContract.GeneratedWalkClip = Root->GetStringField(TEXT("generatedWalkClip"));
    OutContract.AnimationPhaseA = (*AnimationPhases)[0]->AsNumber();
    OutContract.AnimationPhaseB = (*AnimationPhases)[1]->AsNumber();
    OutContract.MinimumAnimationTimeAdvance = Root->GetNumberField(TEXT("minimumAnimationTimeAdvanceSeconds"));
    OutContract.MinimumAnimatedThighDirectionDelta = Root->GetNumberField(TEXT("minimumAnimatedThighDirectionDelta"));
    return OutContract.RouteVersion == 1 && OutContract.FixedDelta > 0.0f;
}

void TickCharacter(ACharacter* Character, float Delta)
{
    Character->GetCharacterMovement()->TickComponent(Delta, LEVELTICK_All, nullptr);
    USkeletalMeshComponent* Mesh = Character->GetMesh();
    Mesh->TickAnimation(Delta, false);
    Mesh->RefreshBoneTransforms();
}

void Settle(ACharacter* Character, const FContract& Contract)
{
    const int32 Frames = FMath::Max(1, FMath::CeilToInt(Contract.SettleSeconds / Contract.FixedDelta));
    for (int32 Frame = 0; Frame < Frames; ++Frame)
    {
        TickCharacter(Character, Contract.FixedDelta);
    }
}

void TickFor(ACharacter* Character, const FContract& Contract, float Seconds)
{
    const int32 Frames = FMath::Max(1, FMath::CeilToInt(Seconds / Contract.FixedDelta));
    for (int32 Frame = 0; Frame < Frames; ++Frame)
    {
        TickCharacter(Character, Contract.FixedDelta);
    }
}

FVector BoneDirection(const USkeletalMeshComponent* Mesh, const TCHAR* Parent, const TCHAR* Child)
{
    return (Mesh->GetSocketLocation(Child) - Mesh->GetSocketLocation(Parent)).GetSafeNormal();
}

FAnimationSample CaptureAnimationSample(
    const TCHAR* Name,
    const USkeletalMeshComponent* Mesh,
    const UCharacterFootPlacementAnimInstance* AnimInstance,
    const UAnimSequence* WalkClip)
{
    FAnimationSample Sample;
    Sample.Name = Name;
    Sample.AssetTime = AnimInstance->GetSourceAssetTimeSeconds();
    const FReferenceSkeleton& SequenceSkeleton = WalkClip->GetSkeleton()->GetReferenceSkeleton();
    const FAnimExtractContext ExtractContext(Sample.AssetTime, false, {}, true);
    FTransform RawLeftThigh;
    FTransform RawRightThigh;
    WalkClip->GetBoneTransform(
        RawLeftThigh,
        FSkeletonPoseBoneIndex(SequenceSkeleton.FindBoneIndex(TEXT("thigh_l"))),
        ExtractContext,
        false);
    WalkClip->GetBoneTransform(
        RawRightThigh,
        FSkeletonPoseBoneIndex(SequenceSkeleton.FindBoneIndex(TEXT("thigh_r"))),
        ExtractContext,
        false);
    Sample.RawLeftThighEulerDegrees = RawLeftThigh.GetRotation().Euler();
    Sample.RawRightThighEulerDegrees = RawRightThigh.GetRotation().Euler();
    Sample.LeftThighDirection = BoneDirection(Mesh, TEXT("thigh_l"), TEXT("shin_l"));
    Sample.RightThighDirection = BoneDirection(Mesh, TEXT("thigh_r"), TEXT("shin_r"));
    return Sample;
}

FPoseSample CapturePose(const USkeletalMeshComponent* Mesh)
{
    FPoseSample Sample;
    Sample.Pelvis = Mesh->GetSocketTransform(TEXT("pelvis"), RTS_World);
    Sample.LeftFoot = Mesh->GetSocketTransform(TEXT("foot_l"), RTS_World);
    Sample.RightFoot = Mesh->GetSocketTransform(TEXT("foot_r"), RTS_World);
    Sample.LeftTarget = Mesh->GetSocketTransform(TEXT("ik_foot_l"), RTS_World);
    Sample.RightTarget = Mesh->GetSocketTransform(TEXT("ik_foot_r"), RTS_World);
    return Sample;
}

AActor* SpawnSupport(UWorld* World, const FVector& FootLocation, float FloorHeight, const FContract& Contract)
{
    AActor* Actor = World->SpawnActor<AActor>();
    if (!Actor)
    {
        return nullptr;
    }
    UBoxComponent* Box = NewObject<UBoxComponent>(Actor, TEXT("FootPlacementSupport"));
    Actor->SetRootComponent(Box);
    Actor->AddInstanceComponent(Box);
    Box->SetBoxExtent(Contract.SupportHalfExtent);
    Box->SetWorldLocation(FVector(
        FootLocation.X,
        FootLocation.Y,
        FloorHeight + Contract.SupportHeight - Contract.SupportHalfExtent.Z));
    Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Box->SetCollisionObjectType(ECC_WorldStatic);
    Box->SetCollisionResponseToAllChannels(ECR_Ignore);
    Box->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    Box->SetGenerateOverlapEvents(false);
    Box->RegisterComponent();
    return Actor;
}

AActor* SpawnRouteSurface(
    UWorld* World,
    const FVector& RouteOrigin,
    float ForwardOffset,
    float CenterHeight,
    const FVector& HalfExtent,
    float PitchDegrees)
{
    AActor* Actor = World->SpawnActor<AActor>();
    if (!Actor)
    {
        return nullptr;
    }
    UBoxComponent* Box = NewObject<UBoxComponent>(Actor, TEXT("FootPlacementRouteSurface"));
    Actor->SetRootComponent(Box);
    Actor->AddInstanceComponent(Box);
    Box->SetBoxExtent(HalfExtent);
    Box->SetWorldLocation(RouteOrigin + FVector(ForwardOffset, 0.0f, CenterHeight));
    Box->SetWorldRotation(FRotator(PitchDegrees, 0.0f, 0.0f));
    Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Box->SetCollisionObjectType(ECC_WorldStatic);
    Box->SetCollisionResponseToAllChannels(ECR_Block);
    Box->SetGenerateOverlapEvents(false);
    Box->RegisterComponent();
    return Actor;
}

void DriveForward(ACharacter* Character, const FContract& Contract)
{
    const int32 Frames = FMath::Max(1, FMath::CeilToInt(Contract.RoutePhaseSeconds / Contract.FixedDelta));
    for (int32 Frame = 0; Frame < Frames; ++Frame)
    {
        Character->AddMovementInput(FVector::ForwardVector, 1.0f, true);
        TickCharacter(Character, Contract.FixedDelta);
    }
}

FFootSample MeasureFoot(
    UWorld* World,
    const UCharacterMovementComponent* Movement,
    const FTransform& BaselineFoot,
    const FTransform& BaselineTarget,
    const FTransform& Foot,
    const FTransform& Target,
    float FlatFloorHeight)
{
    FHitResult Hit;
    const FVector FootLocation = Foot.GetLocation();
    const bool bHit = World->LineTraceSingleByChannel(
        Hit,
        FootLocation + FVector(0.0, 0.0, 100.0),
        FootLocation - FVector(0.0, 0.0, 100.0),
        ECC_Visibility);
    const float TerrainDelta = bHit ? Hit.ImpactPoint.Z - FlatFloorHeight : 0.0f;

    FFootSample Sample;
    Sample.bWalkableHit = bHit && Movement->IsWalkable(Hit);
    Sample.ContactError = (FootLocation.Z - BaselineFoot.GetLocation().Z) - TerrainDelta;
    Sample.TargetDelta = FMath::Abs(Target.GetLocation().Z - BaselineTarget.GetLocation().Z);
    return Sample;
}

FCaseSample CaptureCase(
    const TCHAR* Name,
    UWorld* World,
    ACharacter* Character,
    const FPoseSample& Baseline,
    float FlatFloorHeight)
{
    const FPoseSample Pose = CapturePose(Character->GetMesh());
    const UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
    FCaseSample Sample;
    Sample.Name = Name;
    Sample.MovementMode = Movement->IsMovingOnGround() ? TEXT("Walking") : Movement->GetMovementName();
    Sample.bHasWalkableFloor = Movement->CurrentFloor.IsWalkableFloor();
    Sample.PelvisOffset = Pose.Pelvis.GetLocation().Z - Baseline.Pelvis.GetLocation().Z;
    Sample.Speed = Movement->Velocity.Size();
    Sample.ActorLocation = Character->GetActorLocation();
    Sample.CharacterFloorHeight = Movement->CurrentFloor.HitResult.ImpactPoint.Z;
    Sample.Left = MeasureFoot(
        World, Movement, Baseline.LeftFoot, Baseline.LeftTarget,
        Pose.LeftFoot, Pose.LeftTarget, FlatFloorHeight);
    Sample.Right = MeasureFoot(
        World, Movement, Baseline.RightFoot, Baseline.RightTarget,
        Pose.RightFoot, Pose.RightTarget, FlatFloorHeight);
    Sample.bHasNaN = Pose.Pelvis.ContainsNaN() || Pose.LeftFoot.ContainsNaN()
        || Pose.RightFoot.ContainsNaN() || Pose.LeftTarget.ContainsNaN() || Pose.RightTarget.ContainsNaN();
    return Sample;
}

double Rounded(double Value)
{
    return FMath::RoundToDouble(Value * 1000000.0) / 1000000.0;
}

TArray<TSharedPtr<FJsonValue>> VectorJson(const FVector& Vector)
{
    return {
        MakeShared<FJsonValueNumber>(Rounded(Vector.X)),
        MakeShared<FJsonValueNumber>(Rounded(Vector.Y)),
        MakeShared<FJsonValueNumber>(Rounded(Vector.Z)),
    };
}

TSharedRef<FJsonObject> FootJson(const FFootSample& Foot)
{
    TSharedRef<FJsonObject> Value = MakeShared<FJsonObject>();
    Value->SetBoolField(TEXT("walkableHit"), Foot.bWalkableHit);
    Value->SetNumberField(TEXT("contactErrorCentimeters"), Rounded(Foot.ContactError));
    Value->SetNumberField(TEXT("terrainTargetDeltaCentimeters"), Rounded(Foot.TargetDelta));
    return Value;
}

bool WriteEvidence(
    const FString& Path,
    const FContract& Contract,
    const TArray<FCaseSample>& Cases,
    bool bEditorAdapterLoaded,
    bool bAnimated,
    const UAnimSequence* WalkClip,
    const TArray<FAnimationSample>& AnimationSamples)
{
    TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
    Root->SetNumberField(TEXT("schemaVersion"), 1);
    Root->SetNumberField(TEXT("challengerVersion"), ChallengerVersion);
    Root->SetNumberField(TEXT("routeVersion"), Contract.RouteVersion);
    Root->SetBoolField(TEXT("footPlacementExperimental"), true);
    Root->SetBoolField(TEXT("editorAdapterLoaded"), bEditorAdapterLoaded);

    TSharedRef<FJsonObject> Nodes = MakeShared<FJsonObject>();
    Nodes->SetStringField(TEXT("footPlacement"), TEXT("FAnimNode_FootPlacement"));
    Nodes->SetStringField(TEXT("legSolver"), TEXT("FAnimNode_TwoBoneIK"));
    Nodes->SetStringField(TEXT("movement"), TEXT("CharacterMovementComponent"));
    if (bAnimated)
    {
        Nodes->SetStringField(TEXT("source"), TEXT("FAnimNode_SequencePlayer_Standalone"));
    }
    Root->SetObjectField(TEXT("nativeNodes"), Nodes);

    if (bAnimated)
    {
        Root->SetNumberField(TEXT("animationEvidenceVersion"), 1);
        TSharedRef<FJsonObject> SourceAnimation = MakeShared<FJsonObject>();
        SourceAnimation->SetStringField(TEXT("asset"), WalkClipPath);
        SourceAnimation->SetStringField(TEXT("name"), WalkClip ? WalkClip->GetName() : TEXT(""));
        SourceAnimation->SetBoolField(TEXT("looping"), true);
        Root->SetObjectField(TEXT("sourceAnimation"), SourceAnimation);

        TArray<TSharedPtr<FJsonValue>> AnimationValues;
        for (const FAnimationSample& Sample : AnimationSamples)
        {
            TSharedRef<FJsonObject> Value = MakeShared<FJsonObject>();
            Value->SetStringField(TEXT("name"), Sample.Name);
            Value->SetNumberField(TEXT("assetTimeSeconds"), Rounded(Sample.AssetTime));
            Value->SetArrayField(TEXT("rawLeftThighEulerDegrees"), VectorJson(Sample.RawLeftThighEulerDegrees));
            Value->SetArrayField(TEXT("rawRightThighEulerDegrees"), VectorJson(Sample.RawRightThighEulerDegrees));
            Value->SetArrayField(TEXT("leftThighDirection"), VectorJson(Sample.LeftThighDirection));
            Value->SetArrayField(TEXT("rightThighDirection"), VectorJson(Sample.RightThighDirection));
            AnimationValues.Add(MakeShared<FJsonValueObject>(Value));
        }
        Root->SetArrayField(TEXT("animationSamples"), AnimationValues);
    }

    TSharedRef<FJsonObject> Roles = MakeShared<FJsonObject>();
    Roles->SetStringField(TEXT("ikRoot"), TEXT("root"));
    Roles->SetStringField(TEXT("pelvis"), TEXT("pelvis"));
    TSharedRef<FJsonObject> LeftRoles = MakeShared<FJsonObject>();
    LeftRoles->SetStringField(TEXT("fkFoot"), TEXT("foot_l"));
    LeftRoles->SetStringField(TEXT("ball"), TEXT("ball_l"));
    LeftRoles->SetStringField(TEXT("ikFoot"), TEXT("ik_foot_l"));
    Roles->SetObjectField(TEXT("left"), LeftRoles);
    TSharedRef<FJsonObject> RightRoles = MakeShared<FJsonObject>();
    RightRoles->SetStringField(TEXT("fkFoot"), TEXT("foot_r"));
    RightRoles->SetStringField(TEXT("ball"), TEXT("ball_r"));
    RightRoles->SetStringField(TEXT("ikFoot"), TEXT("ik_foot_r"));
    Roles->SetObjectField(TEXT("right"), RightRoles);
    Root->SetObjectField(TEXT("roles"), Roles);

    TArray<TSharedPtr<FJsonValue>> CaseValues;
    for (const FCaseSample& Sample : Cases)
    {
        TSharedRef<FJsonObject> Value = MakeShared<FJsonObject>();
        Value->SetStringField(TEXT("name"), Sample.Name);
        Value->SetStringField(TEXT("movementMode"), Sample.MovementMode);
        Value->SetBoolField(TEXT("hasWalkableFloor"), Sample.bHasWalkableFloor);
        Value->SetBoolField(TEXT("hasNaN"), Sample.bHasNaN);
        Value->SetNumberField(TEXT("pelvisOffsetCentimeters"), Rounded(Sample.PelvisOffset));
        Value->SetNumberField(TEXT("speedCentimetersPerSecond"), Rounded(Sample.Speed));
        Value->SetArrayField(TEXT("actorLocationCentimeters"), VectorJson(Sample.ActorLocation));
        Value->SetNumberField(TEXT("characterFloorHeightCentimeters"), Rounded(Sample.CharacterFloorHeight));
        TSharedRef<FJsonObject> Feet = MakeShared<FJsonObject>();
        Feet->SetObjectField(TEXT("left"), FootJson(Sample.Left));
        Feet->SetObjectField(TEXT("right"), FootJson(Sample.Right));
        Value->SetObjectField(TEXT("feet"), Feet);
        CaseValues.Add(MakeShared<FJsonValueObject>(Value));
    }
    Root->SetArrayField(TEXT("cases"), CaseValues);

    FString Text;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Text);
    if (!FJsonSerializer::Serialize(Root, Writer))
    {
        return false;
    }
    return FFileHelper::SaveStringToFile(Text + TEXT("\n"), *Path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterFootPlacementUnevenTest,
    "Gaters.CharacterLab.CharacterMovement.UnevenFootPlacement",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCharacterFootPlacementUnevenTest::RunTest(const FString& Parameters)
{
    using namespace CharacterFootPlacementUneven;
    const FString ContractPath = FPlatformMisc::GetEnvironmentVariable(TEXT("GATERS_FOOT_PLACEMENT_CONTRACT"));
    const FString EvidencePath = FPlatformMisc::GetEnvironmentVariable(TEXT("GATERS_FOOT_PLACEMENT_EVIDENCE"));
    const bool bAnimated = FPlatformMisc::GetEnvironmentVariable(TEXT("GATERS_FOOT_PLACEMENT_ANIMATED")) == TEXT("1");
    FContract Contract;
    FString ContractError;
    if (!TestTrue(TEXT("Foot Placement contract loads"),
        !ContractPath.IsEmpty() && !EvidencePath.IsEmpty() && LoadContract(ContractPath, Contract, ContractError)))
    {
        AddError(ContractError);
        return false;
    }

    UWorld* World = FindWorld();
    USkeletalMesh* SkeletalMesh = LoadObject<USkeletalMesh>(nullptr, SkeletalMeshPath);
    UAnimSequence* WalkClip = bAnimated ? LoadObject<UAnimSequence>(nullptr, WalkClipPath) : nullptr;
    if (!TestNotNull(TEXT("CharacterLab world is loaded"), World)
        || !TestNotNull(TEXT("Generated skeletal mesh is loadable"), SkeletalMesh))
    {
        return false;
    }
    if (bAnimated && !TestNotNull(TEXT("Generated A_Walk is loadable"), WalkClip))
    {
        return false;
    }
    const FReferenceSkeleton& Skeleton = SkeletalMesh->GetRefSkeleton();
    for (const TCHAR* Role : RequiredRoles)
    {
        TestTrue(*FString::Printf(TEXT("Generated skeleton contains %s"), Role),
            Skeleton.FindBoneIndex(FName(Role)) != INDEX_NONE);
    }

    ACharacter* Character = World->SpawnActor<ACharacter>(ACharacter::StaticClass(), Contract.Spawn, FRotator::ZeroRotator);
    if (!TestNotNull(TEXT("Native ACharacter spawns"), Character))
    {
        return false;
    }
    USkeletalMeshComponent* Mesh = Character->GetMesh();
    UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
    Character->GetCapsuleComponent()->SetCapsuleSize(Contract.CapsuleRadius, Contract.CapsuleHalfHeight);
    Mesh->SetRelativeLocation(FVector(0.0, 0.0, -Contract.CapsuleHalfHeight));
    Mesh->SetSkeletalMeshAsset(SkeletalMesh);
    Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    Mesh->SetAnimInstanceClass(UCharacterFootPlacementAnimInstance::StaticClass());
    Movement->bRunPhysicsWithNoController = true;
    Movement->SetMovementMode(MOVE_Falling);
    Settle(Character, Contract);

    if (!TestTrue(TEXT("CharacterMovement retains the flat walkable floor"),
        Movement->IsMovingOnGround() && Movement->CurrentFloor.IsWalkableFloor()))
    {
        Character->Destroy();
        return false;
    }
    const bool bEditorAdapterLoaded = FModuleManager::Get().IsModuleLoaded(TEXT("CharacterPhysicsProfileAdapter"));
    TestFalse(TEXT("Editor intake adapter remains unloaded"), bEditorAdapterLoaded);

    TArray<FAnimationSample> AnimationSamples;
    if (bAnimated)
    {
        UCharacterFootPlacementAnimInstance* AnimInstance =
            Cast<UCharacterFootPlacementAnimInstance>(Mesh->GetAnimInstance());
        if (!TestNotNull(TEXT("Native animated Foot Placement instance is active"), AnimInstance))
        {
            Character->Destroy();
            return false;
        }
        AnimInstance->SetSourceSequence(WalkClip);
        AnimInstance->InitializeAnimation();
        TickFor(Character, Contract, Contract.AnimationPhaseA);
        AnimationSamples.Add(CaptureAnimationSample(TEXT("phase_a"), Mesh, AnimInstance, WalkClip));
        TickFor(Character, Contract, Contract.AnimationPhaseB - Contract.AnimationPhaseA);
        AnimationSamples.Add(CaptureAnimationSample(TEXT("phase_b"), Mesh, AnimInstance, WalkClip));
        TestTrue(TEXT("Generated A_Walk is the native graph source"), AnimInstance->GetSourceSequence() == WalkClip);
        TestTrue(TEXT("Generated walk asset time advances"),
            AnimationSamples[1].AssetTime - AnimationSamples[0].AssetTime >= Contract.MinimumAnimationTimeAdvance);
        TestTrue(TEXT("Evaluated left thigh changes across gait phases"),
            FVector::Distance(AnimationSamples[0].LeftThighDirection, AnimationSamples[1].LeftThighDirection)
                >= Contract.MinimumAnimatedThighDirectionDelta);
        TestTrue(TEXT("Evaluated right thigh changes across gait phases"),
            FVector::Distance(AnimationSamples[0].RightThighDirection, AnimationSamples[1].RightThighDirection)
                >= Contract.MinimumAnimatedThighDirectionDelta);
    }

    const float FlatFloorHeight = Movement->CurrentFloor.HitResult.ImpactPoint.Z;
    const FPoseSample Baseline = CapturePose(Mesh);
    TArray<FCaseSample> Cases;

    AActor* LeftSupport = SpawnSupport(World, Baseline.LeftFoot.GetLocation(), FlatFloorHeight, Contract);
    TestNotNull(TEXT("Left support spawns"), LeftSupport);
    Settle(Character, Contract);
    Cases.Add(CaptureCase(TEXT("split_left_high"), World, Character, Baseline, FlatFloorHeight));
    if (LeftSupport)
    {
        LeftSupport->Destroy();
    }
    Settle(Character, Contract);

    AActor* RightSupport = SpawnSupport(World, Baseline.RightFoot.GetLocation(), FlatFloorHeight, Contract);
    TestNotNull(TEXT("Right support spawns"), RightSupport);
    Settle(Character, Contract);
    Cases.Add(CaptureCase(TEXT("split_right_high"), World, Character, Baseline, FlatFloorHeight));
    if (RightSupport)
    {
        RightSupport->Destroy();
    }
    Settle(Character, Contract);

    const FVector RouteOrigin(Contract.Spawn.X, Contract.Spawn.Y, FlatFloorHeight);
    AActor* StepSurface = SpawnRouteSurface(
        World,
        RouteOrigin,
        Contract.StepCenterOffset,
        Contract.SupportHeight - Contract.StepHalfExtent.Z,
        Contract.StepHalfExtent,
        0.0f);
    AActor* SlopeSurface = SpawnRouteSurface(
        World,
        RouteOrigin,
        Contract.SlopeCenterOffset,
        Contract.SlopeCenterHeight,
        Contract.SlopeHalfExtent,
        Contract.SlopePitch);
    TestNotNull(TEXT("Solid step surface spawns"), StepSurface);
    TestNotNull(TEXT("Solid slope surface spawns"), SlopeSurface);

    Movement->MaxWalkSpeed = Contract.RouteWalkSpeed;
    Movement->BrakingDecelerationWalking = 2048.0f;
    DriveForward(Character, Contract);
    Cases.Add(CaptureCase(TEXT("step"), World, Character, Baseline, FlatFloorHeight));
    DriveForward(Character, Contract);
    Cases.Add(CaptureCase(TEXT("slope"), World, Character, Baseline, FlatFloorHeight));
    Settle(Character, Contract);
    Cases.Add(CaptureCase(TEXT("stop"), World, Character, Baseline, FlatFloorHeight));

    TestTrue(TEXT("Left-high terrain moves the native left IK target"),
        Cases[0].Left.TargetDelta >= Contract.MinimumTargetDelta);
    TestTrue(TEXT("Right-high terrain moves the native right IK target"),
        Cases[1].Right.TargetDelta >= Contract.MinimumTargetDelta);
    for (const FCaseSample& Sample : Cases)
    {
        TestTrue(*FString::Printf(TEXT("%s retains bilateral walkable traces"), *Sample.Name),
            Sample.bHasWalkableFloor && Sample.Left.bWalkableHit && Sample.Right.bWalkableHit);
        TestTrue(*FString::Printf(TEXT("%s left foot meets contact bound"), *Sample.Name),
            FMath::Abs(Sample.Left.ContactError) <= Contract.MaximumContactError);
        TestTrue(*FString::Printf(TEXT("%s right foot meets contact bound"), *Sample.Name),
            FMath::Abs(Sample.Right.ContactError) <= Contract.MaximumContactError);
        TestTrue(*FString::Printf(TEXT("%s pelvis remains bounded"), *Sample.Name),
            !Sample.bHasNaN && FMath::Abs(Sample.PelvisOffset) <= Contract.MaximumPelvisOffset);
    }
    float MinimumFloorHeight = TNumericLimits<float>::Max();
    float MaximumFloorHeight = TNumericLimits<float>::Lowest();
    for (const FCaseSample& Sample : Cases)
    {
        MinimumFloorHeight = FMath::Min(MinimumFloorHeight, Sample.CharacterFloorHeight);
        MaximumFloorHeight = FMath::Max(MaximumFloorHeight, Sample.CharacterFloorHeight);
    }
    TestTrue(TEXT("CharacterMovement moves the capsule through the route"),
        Cases.Last().ActorLocation.X - Cases[0].ActorLocation.X >= Contract.MinimumRouteDisplacement);
    TestTrue(TEXT("CharacterMovement reports uneven floor heights"),
        MaximumFloorHeight - MinimumFloorHeight >= Contract.MinimumFloorHeightDelta);
    TestTrue(TEXT("Uneven route finishes stopped"), Movement->Velocity.Size() <= Contract.MaximumFinalSpeed);
    TestTrue(TEXT("Stable Foot Placement evidence is written"),
        WriteEvidence(EvidencePath, Contract, Cases, bEditorAdapterLoaded, bAnimated, WalkClip, AnimationSamples));

    if (StepSurface)
    {
        StepSurface->Destroy();
    }
    if (SlopeSurface)
    {
        SlopeSurface->Destroy();
    }
    Character->Destroy();
    return true;
}

#endif
