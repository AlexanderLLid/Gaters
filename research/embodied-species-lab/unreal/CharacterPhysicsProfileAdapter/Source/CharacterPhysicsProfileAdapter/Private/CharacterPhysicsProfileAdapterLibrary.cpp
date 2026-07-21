#include "CharacterPhysicsProfileAdapterLibrary.h"

#include "AnimationRuntime.h"
#include "Engine/SkeletalMesh.h"
#include "PhysicsAssetUtils.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/PhysicsConstraintTemplate.h"
#include "PhysicsEngine/SkeletalBodySetup.h"

namespace Gaters::CharacterLab
{
bool ValidateTopology(
    const TSet<FName>& SkeletonBones,
    const TArray<FName>& BodyBones,
    const TArray<FName>& ParentBones,
    const TArray<FName>& ChildBones,
    FString& OutError)
{
    OutError.Reset();
    if (BodyBones.IsEmpty())
    {
        OutError = TEXT("BodyBones must not be empty");
        return false;
    }
    if (ParentBones.Num() != ChildBones.Num())
    {
        OutError = TEXT("ParentBones and ChildBones must have equal length");
        return false;
    }
    if (ParentBones.Num() != BodyBones.Num() - 1)
    {
        OutError = TEXT("A connected body tree requires exactly body count minus one joints");
        return false;
    }

    TSet<FName> BodySet;
    for (const FName Bone : BodyBones)
    {
        if (Bone.IsNone())
        {
            OutError = TEXT("Body bone names must not be empty");
            return false;
        }
        if (!SkeletonBones.Contains(Bone))
        {
            OutError = FString::Printf(TEXT("Body references unknown skeleton bone: %s"), *Bone.ToString());
            return false;
        }
        if (BodySet.Contains(Bone))
        {
            OutError = FString::Printf(TEXT("Profile contains duplicate body bone: %s"), *Bone.ToString());
            return false;
        }
        BodySet.Add(Bone);
    }

    TSet<FString> JointPairs;
    TMap<FName, FName> ParentByChild;
    TMap<FName, TArray<FName>> ChildrenByParent;
    for (int32 Index = 0; Index < ParentBones.Num(); ++Index)
    {
        const FName Parent = ParentBones[Index];
        const FName Child = ChildBones[Index];
        if (Parent.IsNone() || Child.IsNone())
        {
            OutError = TEXT("Joint bone names must not be empty");
            return false;
        }
        if (!SkeletonBones.Contains(Parent) || !SkeletonBones.Contains(Child))
        {
            const FName UnknownBone = !SkeletonBones.Contains(Parent) ? Parent : Child;
            OutError = FString::Printf(
                TEXT("Joint references unknown skeleton bone: %s"),
                *UnknownBone.ToString());
            return false;
        }
        if (!BodySet.Contains(Parent) || !BodySet.Contains(Child))
        {
            OutError = FString::Printf(
                TEXT("Joint references body outside profile: %s -> %s"),
                *Parent.ToString(),
                *Child.ToString());
            return false;
        }
        if (Parent == Child)
        {
            OutError = FString::Printf(TEXT("Joint cannot connect a body to itself: %s"), *Parent.ToString());
            return false;
        }

        const FString Pair = Parent.ToString() + TEXT("\x1f") + Child.ToString();
        if (JointPairs.Contains(Pair))
        {
            OutError = FString::Printf(TEXT("Profile contains duplicate joint pair: %s"), *Pair);
            return false;
        }
        if (ParentByChild.Contains(Child))
        {
            OutError = FString::Printf(TEXT("Body has multiple parents: %s"), *Child.ToString());
            return false;
        }
        JointPairs.Add(Pair);
        ParentByChild.Add(Child, Parent);
        ChildrenByParent.FindOrAdd(Parent).Add(Child);
    }

    TArray<FName> Roots;
    for (const FName Body : BodyBones)
    {
        if (!ParentByChild.Contains(Body))
        {
            Roots.Add(Body);
        }
    }
    if (Roots.Num() != 1)
    {
        OutError = TEXT("Topology must contain exactly one root body");
        return false;
    }

    TSet<FName> Visited;
    TArray<FName> Queue{Roots[0]};
    for (int32 QueueIndex = 0; QueueIndex < Queue.Num(); ++QueueIndex)
    {
        const FName Current = Queue[QueueIndex];
        if (Visited.Contains(Current))
        {
            continue;
        }
        Visited.Add(Current);
        if (const TArray<FName>* Children = ChildrenByParent.Find(Current))
        {
            Queue.Append(*Children);
        }
    }
    if (Visited.Num() != BodySet.Num())
    {
        OutError = TEXT("Topology must be connected from its single root");
        return false;
    }
    return true;
}
}

FString UCharacterPhysicsProfileAdapterLibrary::RebuildPhysicsAssetTopology(
    USkeletalMesh* SkeletalMesh,
    UPhysicsAsset* PhysicsAsset,
    const TArray<FName>& BodyBones,
    const TArray<FName>& ParentBones,
    const TArray<FName>& ChildBones)
{
    if (!SkeletalMesh || !PhysicsAsset)
    {
        return TEXT("SkeletalMesh and PhysicsAsset must not be null");
    }

    TSet<FName> SkeletonBones;
    const FReferenceSkeleton& ReferenceSkeleton = SkeletalMesh->GetRefSkeleton();
    for (int32 Index = 0; Index < ReferenceSkeleton.GetRawBoneNum(); ++Index)
    {
        SkeletonBones.Add(ReferenceSkeleton.GetBoneName(Index));
    }
    FString Error;
    if (!Gaters::CharacterLab::ValidateTopology(
            SkeletonBones, BodyBones, ParentBones, ChildBones, Error))
    {
        return Error;
    }

    PhysicsAsset->Modify();
    SkeletalMesh->Modify();
    for (int32 Index = PhysicsAsset->ConstraintSetup.Num() - 1; Index >= 0; --Index)
    {
        FPhysicsAssetUtils::DestroyConstraint(PhysicsAsset, Index);
    }
    for (int32 Index = PhysicsAsset->SkeletalBodySetups.Num() - 1; Index >= 0; --Index)
    {
        FPhysicsAssetUtils::DestroyBody(PhysicsAsset, Index);
    }

    FPhysAssetCreateParams Params;
    Params.MinBoneSize = 0.0f;
    Params.GeomType = EFG_Box;
    Params.VertWeight = EVW_DominantWeight;
    Params.bAlwaysUseVertices = true;
    Params.bIncludeChildBones = true;
    Params.bAutoOrientToBone = true;
    Params.bCreateConstraints = false;
    Params.bWalkPastSmall = false;
    Params.bBodyForAll = true;
    Params.bDisableCollisionsByDefault = true;

    FText NativeError;
    if (!FPhysicsAssetUtils::CreateFromSkeletalMesh(
            PhysicsAsset, SkeletalMesh, Params, NativeError, false, false))
    {
        return NativeError.ToString();
    }

    TSet<FName> RequiredBodies;
    for (const FName Bone : BodyBones)
    {
        RequiredBodies.Add(Bone);
    }
    for (int32 Index = PhysicsAsset->SkeletalBodySetups.Num() - 1; Index >= 0; --Index)
    {
        const USkeletalBodySetup* BodySetup = PhysicsAsset->SkeletalBodySetups[Index];
        if (!BodySetup || !RequiredBodies.Contains(BodySetup->BoneName))
        {
            FPhysicsAssetUtils::DestroyBody(PhysicsAsset, Index);
        }
    }
    for (const FName Bone : BodyBones)
    {
        if (PhysicsAsset->FindBodyIndex(Bone) == INDEX_NONE)
        {
            return FString::Printf(TEXT("Native body generation omitted profile bone: %s"), *Bone.ToString());
        }
    }
    if (PhysicsAsset->SkeletalBodySetups.Num() != BodyBones.Num())
    {
        return TEXT("Native body count differs from the validated profile");
    }

    for (int32 Index = 0; Index < ParentBones.Num(); ++Index)
    {
        const FName ParentBone = ParentBones[Index];
        const FName ChildBone = ChildBones[Index];
        const int32 ConstraintIndex = FPhysicsAssetUtils::CreateNewConstraint(
            PhysicsAsset, ChildBone);
        if (!PhysicsAsset->ConstraintSetup.IsValidIndex(ConstraintIndex))
        {
            return FString::Printf(TEXT("Could not create native constraint for child: %s"), *ChildBone.ToString());
        }

        UPhysicsConstraintTemplate* Constraint = PhysicsAsset->ConstraintSetup[ConstraintIndex];
        FConstraintInstance& Instance = Constraint->DefaultInstance;
        Instance.ConstraintBone1 = ChildBone;
        Instance.ConstraintBone2 = ParentBone;

        const int32 ParentIndex = ReferenceSkeleton.FindBoneIndex(ParentBone);
        const int32 ChildIndex = ReferenceSkeleton.FindBoneIndex(ChildBone);
        const FTransform ParentCS = FAnimationRuntime::GetComponentSpaceTransformRefPose(
            ReferenceSkeleton, ParentIndex);
        const FTransform ChildCS = FAnimationRuntime::GetComponentSpaceTransformRefPose(
            ReferenceSkeleton, ChildIndex);
        Instance.SetRefFrame(EConstraintFrame::Frame1, FTransform::Identity);
        Instance.SetRefFrame(
            EConstraintFrame::Frame2,
            ChildCS.GetRelativeTransform(ParentCS));
        Constraint->SetDefaultProfile(Instance);
    }

    if (PhysicsAsset->ConstraintSetup.Num() != ParentBones.Num())
    {
        return TEXT("Native constraint count differs from the validated profile");
    }
    for (int32 Index = 0; Index < PhysicsAsset->ConstraintSetup.Num(); ++Index)
    {
        const FConstraintInstance& Instance = PhysicsAsset->ConstraintSetup[Index]->DefaultInstance;
        if (Instance.ConstraintBone2 != ParentBones[Index]
            || Instance.ConstraintBone1 != ChildBones[Index])
        {
            return TEXT("Native constraint pair differs from the validated profile");
        }
    }

    PhysicsAsset->UpdateBodySetupIndexMap();
    PhysicsAsset->UpdateBoundsBodiesArray();
    SkeletalMesh->SetPhysicsAsset(PhysicsAsset);
    PhysicsAsset->MarkPackageDirty();
    SkeletalMesh->MarkPackageDirty();
    PhysicsAsset->PostEditChange();
    SkeletalMesh->PostEditChange();
    return FString();
}
