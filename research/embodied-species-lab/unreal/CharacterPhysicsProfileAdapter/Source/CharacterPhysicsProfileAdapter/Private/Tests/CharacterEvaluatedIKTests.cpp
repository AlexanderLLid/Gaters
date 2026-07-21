#if WITH_DEV_AUTOMATION_TESTS

#include "Dom/JsonObject.h"
#include "Engine/SkeletalMesh.h"
#include "HAL/PlatformMisc.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Rig/IKRigDataTypes.h"
#include "Rig/IKRigDefinition.h"
#include "Rig/IKRigProcessor.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
constexpr TCHAR SkeletalMeshPath[] =
    TEXT("/Game/Gaters/Generated/CharacterLab/SK_GeneratedHumanoid.SK_GeneratedHumanoid");
constexpr TCHAR IKRigPath[] =
    TEXT("/Game/Gaters/Generated/CharacterLab/IK_GeneratedHumanoid.IK_GeneratedHumanoid");

struct FIKContract
{
    double GoalOffset = 0.0;
    double MinimumDrivenDisplacement = 0.0;
    double MaximumGoalError = 0.0;
    double MaximumOppositeFootDisplacement = 0.0;
};

struct FFootMeasurement
{
    double DrivenDisplacement = 0.0;
    double GoalError = 0.0;
    double OppositeFootDisplacement = 0.0;
};

bool LoadContract(const FString& Path, FIKContract& OutContract, FString& OutError)
{
    FString Json;
    if (!FFileHelper::LoadFileToString(Json, *Path))
    {
        OutError = FString::Printf(TEXT("Could not read IK contract: %s"), *Path);
        return false;
    }

    TSharedPtr<FJsonObject> Root;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        OutError = TEXT("IK contract is not valid JSON");
        return false;
    }

    double SchemaVersion = 0.0;
    if (!Root->TryGetNumberField(TEXT("schemaVersion"), SchemaVersion)
        || SchemaVersion != 1.0
        || !Root->TryGetNumberField(TEXT("goalOffsetCentimeters"), OutContract.GoalOffset)
        || !Root->TryGetNumberField(
            TEXT("minimumDrivenDisplacementCentimeters"),
            OutContract.MinimumDrivenDisplacement)
        || !Root->TryGetNumberField(
            TEXT("maximumGoalErrorCentimeters"),
            OutContract.MaximumGoalError)
        || !Root->TryGetNumberField(
            TEXT("maximumOppositeFootDisplacementCentimeters"),
            OutContract.MaximumOppositeFootDisplacement))
    {
        OutError = TEXT("IK contract schema or thresholds are invalid");
        return false;
    }
    return true;
}

FIKRigGoal MakeGoal(
    const FName GoalName,
    const FName BoneName,
    const FTransform& Transform)
{
    return FIKRigGoal(
        GoalName,
        BoneName,
        Transform.GetTranslation(),
        Transform.GetRotation(),
        1.0f,
        1.0f,
        EIKRigGoalSpace::Component,
        EIKRigGoalSpace::Component,
        true);
}

FFootMeasurement EvaluateFoot(
    FIKRigProcessor& Processor,
    const TArray<FTransform>& ReferencePose,
    const int32 ChallengedIndex,
    const int32 OppositeIndex,
    const FName ChallengedGoal,
    const FName ChallengedBone,
    const FName OppositeGoal,
    const FName OppositeBone,
    const double GoalOffset)
{
    FTransform ChallengedTarget = ReferencePose[ChallengedIndex];
    ChallengedTarget.AddToTranslation(FVector(0.0, 0.0, GoalOffset));
    const FTransform& OppositeTarget = ReferencePose[OppositeIndex];

    Processor.SetInputPoseToRefPose();
    Processor.SetIKGoal(MakeGoal(ChallengedGoal, ChallengedBone, ChallengedTarget));
    Processor.SetIKGoal(MakeGoal(OppositeGoal, OppositeBone, OppositeTarget));
    Processor.Solve();

    TArray<FTransform> OutputPose;
    Processor.GetOutputPoseGlobal(OutputPose);
    FFootMeasurement Measurement;
    Measurement.DrivenDisplacement = FVector::Distance(
        OutputPose[ChallengedIndex].GetTranslation(),
        ReferencePose[ChallengedIndex].GetTranslation());
    Measurement.GoalError = FVector::Distance(
        OutputPose[ChallengedIndex].GetTranslation(),
        ChallengedTarget.GetTranslation());
    Measurement.OppositeFootDisplacement = FVector::Distance(
        OutputPose[OppositeIndex].GetTranslation(),
        ReferencePose[OppositeIndex].GetTranslation());
    return Measurement;
}

FString MeasurementJson(const FFootMeasurement& Measurement)
{
    return FString::Printf(
        TEXT("{\n")
        TEXT("      \"drivenDisplacementCentimeters\": %.6f,\n")
        TEXT("      \"goalErrorCentimeters\": %.6f,\n")
        TEXT("      \"oppositeFootDisplacementCentimeters\": %.6f\n")
        TEXT("    }"),
        Measurement.DrivenDisplacement,
        Measurement.GoalError,
        Measurement.OppositeFootDisplacement);
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterEvaluatedIKTest,
    "Gaters.CharacterLab.EvaluatedIK.GeneratedHumanoid",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharacterEvaluatedIKTest::RunTest(const FString& Parameters)
{
    const FString EvidenceEnvironment =
        FPlatformMisc::GetEnvironmentVariable(TEXT("GATERS_IK_EVIDENCE"));
    const FString ContractEnvironment =
        FPlatformMisc::GetEnvironmentVariable(TEXT("GATERS_IK_CONTRACT"));
    if (EvidenceEnvironment.IsEmpty() || ContractEnvironment.IsEmpty())
    {
        AddError(TEXT("GATERS_IK_EVIDENCE and GATERS_IK_CONTRACT are required"));
        return false;
    }

    FString EvidencePath = FPaths::ConvertRelativePathToFull(EvidenceEnvironment);
    FString AllowedRoot = FPaths::ConvertRelativePathToFull(
        FPaths::ProjectSavedDir() / TEXT("CharacterLab"));
    FPaths::NormalizeFilename(EvidencePath);
    FPaths::NormalizeDirectoryName(AllowedRoot);
    if (!EvidencePath.StartsWith(AllowedRoot + TEXT("/")))
    {
        AddError(FString::Printf(
            TEXT("IK evidence must remain under %s: %s"),
            *AllowedRoot,
            *EvidencePath));
        return false;
    }

    FIKContract Contract;
    FString ContractError;
    if (!LoadContract(
        FPaths::ConvertRelativePathToFull(ContractEnvironment),
        Contract,
        ContractError))
    {
        AddError(ContractError);
        return false;
    }

    USkeletalMesh* SkeletalMesh = LoadObject<USkeletalMesh>(nullptr, SkeletalMeshPath);
    UIKRigDefinition* IKRig = LoadObject<UIKRigDefinition>(nullptr, IKRigPath);
    if (!TestNotNull(TEXT("Generated Skeletal Mesh loads"), SkeletalMesh)
        || !TestNotNull(TEXT("Generated IK Rig loads"), IKRig))
    {
        return false;
    }

    FIKRigProcessor Processor;
    Processor.Initialize(IKRig, SkeletalMesh, FIKRigGoalContainer());
    if (!TestTrue(TEXT("IK Rig processor initializes"), Processor.IsInitialized()))
    {
        return false;
    }

    const FIKRigSkeleton& Skeleton = Processor.GetSkeleton();
    const int32 LeftIndex = Skeleton.GetBoneIndexFromName(TEXT("foot_l"));
    const int32 RightIndex = Skeleton.GetBoneIndexFromName(TEXT("foot_r"));
    if (!TestTrue(TEXT("Left foot exists"), LeftIndex != INDEX_NONE)
        || !TestTrue(TEXT("Right foot exists"), RightIndex != INDEX_NONE))
    {
        return false;
    }
    const TArray<FTransform> ReferencePose = Skeleton.RefPoseGlobal;

    const FFootMeasurement Left = EvaluateFoot(
        Processor,
        ReferencePose,
        LeftIndex,
        RightIndex,
        TEXT("foot_l_goal"),
        TEXT("foot_l"),
        TEXT("foot_r_goal"),
        TEXT("foot_r"),
        Contract.GoalOffset);
    const FFootMeasurement Right = EvaluateFoot(
        Processor,
        ReferencePose,
        RightIndex,
        LeftIndex,
        TEXT("foot_r_goal"),
        TEXT("foot_r"),
        TEXT("foot_l_goal"),
        TEXT("foot_l"),
        Contract.GoalOffset);

    const FString Evidence = FString::Printf(
        TEXT("{\n")
        TEXT("  \"schemaVersion\": 1,\n")
        TEXT("  \"skeletalMesh\": \"%s\",\n")
        TEXT("  \"ikRig\": \"%s\",\n")
        TEXT("  \"solverCount\": %d,\n")
        TEXT("  \"goalOffsetCentimeters\": %.6f,\n")
        TEXT("  \"feet\": {\n")
        TEXT("    \"left\": %s,\n")
        TEXT("    \"right\": %s\n")
        TEXT("  }\n")
        TEXT("}\n"),
        SkeletalMeshPath,
        IKRigPath,
        IKRig->GetSolverStructs().Num(),
        Contract.GoalOffset,
        *MeasurementJson(Left),
        *MeasurementJson(Right));
    if (!FFileHelper::SaveStringToFile(
        Evidence,
        *EvidencePath,
        FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
    {
        AddError(FString::Printf(TEXT("Could not write IK evidence: %s"), *EvidencePath));
        return false;
    }

    auto CheckMeasurement = [this, &Contract](
        const TCHAR* Side,
        const FFootMeasurement& Measurement)
    {
        TestTrue(
            *FString::Printf(TEXT("%s driven foot moves"), Side),
            Measurement.DrivenDisplacement >= Contract.MinimumDrivenDisplacement);
        TestTrue(
            *FString::Printf(TEXT("%s driven foot reaches goal"), Side),
            Measurement.GoalError <= Contract.MaximumGoalError);
        TestTrue(
            *FString::Printf(TEXT("%s opposite foot remains planted"), Side),
            Measurement.OppositeFootDisplacement
                <= Contract.MaximumOppositeFootDisplacement);
    };
    CheckMeasurement(TEXT("Left"), Left);
    CheckMeasurement(TEXT("Right"), Right);
    return !HasAnyErrors();
}

#endif
