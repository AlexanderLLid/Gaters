#include "CharacterFootPlacementAnimInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CharacterFootPlacementAnimInstance)

namespace
{
void ConfigureLeg(
    FAnimNode_TwoBoneIK& Leg,
    const TCHAR* FootBone,
    const TCHAR* TargetBone,
    const FVector& JointTarget)
{
    Leg.IKBone.BoneName = FName(FootBone);
    Leg.EffectorTarget = FBoneSocketTarget(FName(TargetBone), false);
    Leg.EffectorLocationSpace = BCS_BoneSpace;
    Leg.JointTargetLocationSpace = BCS_ComponentSpace;
    Leg.JointTargetLocation = JointTarget;
    Leg.bAllowStretching = false;
    Leg.bTakeRotationFromEffectorSpace = true;
    Leg.SetAlpha(1.0f);
}
}

FCharacterFootPlacementAnimInstanceProxy::FCharacterFootPlacementAnimInstanceProxy(
    UAnimInstance* Instance,
    FAnimNode_SequencePlayer_Standalone* Source,
    FAnimNode_ConvertLocalToComponentSpace* LocalToComponent,
    FAnimNode_FootPlacement* FootPlacement,
    FAnimNode_TwoBoneIK* LeftLeg,
    FAnimNode_TwoBoneIK* RightLeg,
    FAnimNode_ConvertComponentToLocalSpace* Root)
    : FAnimInstanceProxy(Instance)
    , SourceNode(Source)
    , LocalToComponentNode(LocalToComponent)
    , FootPlacementNode(FootPlacement)
    , LeftLegNode(LeftLeg)
    , RightLegNode(RightLeg)
    , RootNode(Root)
{
    LocalToComponentNode->LocalPose.SetLinkNode(SourceNode);
    FootPlacementNode->ComponentPose.SetLinkNode(LocalToComponentNode);
    LeftLegNode->ComponentPose.SetLinkNode(FootPlacementNode);
    RightLegNode->ComponentPose.SetLinkNode(LeftLegNode);
    RootNode->ComponentPose.SetLinkNode(RightLegNode);
}

FAnimNode_Base* FCharacterFootPlacementAnimInstanceProxy::GetCustomRootNode()
{
    return RootNode;
}

void FCharacterFootPlacementAnimInstanceProxy::GetCustomNodes(TArray<FAnimNode_Base*>& OutNodes)
{
    OutNodes.Append({
        SourceNode,
        LocalToComponentNode,
        FootPlacementNode,
        LeftLegNode,
        RightLegNode,
        RootNode,
    });
}

UCharacterFootPlacementAnimInstance::UCharacterFootPlacementAnimInstance(
    const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bUseMultiThreadedAnimationUpdate = false;
    Source.SetLoopAnimation(true);
    Source.SetPlayRate(1.0f);

    FootPlacement.PlantSpeedMode = EWarpingEvaluationMode::Graph;
    FootPlacement.IKFootRootBone.BoneName = TEXT("root");
    FootPlacement.PelvisBone.BoneName = TEXT("pelvis");
    FootPlacement.PelvisSettings.bEnableInterpolation = false;
    FootPlacement.InterpolationSettings.bEnableFloorInterpolation = false;
    FootPlacement.InterpolationSettings.bEnableSeparationInterpolation = false;
    FootPlacement.TraceSettings.bDisableComplexTrace = true;
    FootPlacement.TraceSettings.SimpleTraceChannel = UEngineTypes::ConvertToTraceType(ECC_Visibility);
    FootPlacement.PlantSettings.LockType = EFootPlacementLockType::PivotAroundAnkle;
    FootPlacement.SetAlpha(1.0f);

    FFootPlacemenLegDefinition& Left = FootPlacement.LegDefinitions.AddDefaulted_GetRef();
    Left.FKFootBone.BoneName = TEXT("foot_l");
    Left.BallBone.BoneName = TEXT("ball_l");
    Left.IKFootBone.BoneName = TEXT("ik_foot_l");

    FFootPlacemenLegDefinition& Right = FootPlacement.LegDefinitions.AddDefaulted_GetRef();
    Right.FKFootBone.BoneName = TEXT("foot_r");
    Right.BallBone.BoneName = TEXT("ball_r");
    Right.IKFootBone.BoneName = TEXT("ik_foot_r");

    ConfigureLeg(LeftLeg, TEXT("foot_l"), TEXT("ik_foot_l"), FVector(16.0, -50.0, 45.0));
    ConfigureLeg(RightLeg, TEXT("foot_r"), TEXT("ik_foot_r"), FVector(-16.0, -50.0, 45.0));
}

void UCharacterFootPlacementAnimInstance::SetSourceSequence(UAnimSequenceBase* Sequence)
{
    Source.SetSequence(Sequence);
}

UAnimSequenceBase* UCharacterFootPlacementAnimInstance::GetSourceSequence() const
{
    return Source.GetSequence();
}

float UCharacterFootPlacementAnimInstance::GetSourceAssetTimeSeconds() const
{
    return Source.GetCurrentAssetTime();
}

FAnimInstanceProxy* UCharacterFootPlacementAnimInstance::CreateAnimInstanceProxy()
{
    return new FCharacterFootPlacementAnimInstanceProxy(
        this,
        &Source,
        &LocalToComponent,
        &FootPlacement,
        &LeftLeg,
        &RightLeg,
        &Root);
}
