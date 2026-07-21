#pragma once

#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimNode_SequencePlayer.h"
#include "Animation/AnimNodeSpaceConversions.h"
#include "BoneControllers/AnimNode_FootPlacement.h"
#include "BoneControllers/AnimNode_TwoBoneIK.h"

#include "CharacterFootPlacementAnimInstance.generated.h"

USTRUCT()
struct CHARACTERMOVEMENTCHALLENGER_API FCharacterFootPlacementAnimInstanceProxy : public FAnimInstanceProxy
{
    GENERATED_BODY()

    FCharacterFootPlacementAnimInstanceProxy() = default;
    FCharacterFootPlacementAnimInstanceProxy(
        UAnimInstance* Instance,
        FAnimNode_SequencePlayer_Standalone* Source,
        FAnimNode_ConvertLocalToComponentSpace* LocalToComponent,
        FAnimNode_FootPlacement* FootPlacement,
        FAnimNode_TwoBoneIK* LeftLeg,
        FAnimNode_TwoBoneIK* RightLeg,
        FAnimNode_ConvertComponentToLocalSpace* Root);

    virtual FAnimNode_Base* GetCustomRootNode() override;
    virtual void GetCustomNodes(TArray<FAnimNode_Base*>& OutNodes) override;

private:
    FAnimNode_SequencePlayer_Standalone* SourceNode = nullptr;
    FAnimNode_ConvertLocalToComponentSpace* LocalToComponentNode = nullptr;
    FAnimNode_FootPlacement* FootPlacementNode = nullptr;
    FAnimNode_TwoBoneIK* LeftLegNode = nullptr;
    FAnimNode_TwoBoneIK* RightLegNode = nullptr;
    FAnimNode_ConvertComponentToLocalSpace* RootNode = nullptr;
};

UCLASS(Transient, NotBlueprintType, NotBlueprintable)
class CHARACTERMOVEMENTCHALLENGER_API UCharacterFootPlacementAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UCharacterFootPlacementAnimInstance(const FObjectInitializer& ObjectInitializer);
    void SetSourceSequence(UAnimSequenceBase* Sequence);
    UAnimSequenceBase* GetSourceSequence() const;
    float GetSourceAssetTimeSeconds() const;

protected:
    virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;

private:
    UPROPERTY(Transient)
    FAnimNode_SequencePlayer_Standalone Source;

    UPROPERTY(Transient)
    FAnimNode_ConvertLocalToComponentSpace LocalToComponent;

    UPROPERTY(Transient)
    FAnimNode_FootPlacement FootPlacement;

    UPROPERTY(Transient)
    FAnimNode_TwoBoneIK LeftLeg;

    UPROPERTY(Transient)
    FAnimNode_TwoBoneIK RightLeg;

    UPROPERTY(Transient)
    FAnimNode_ConvertComponentToLocalSpace Root;
};
