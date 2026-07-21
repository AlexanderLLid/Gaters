#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CharacterPhysicsProfileAdapterLibrary.generated.h"

class UPhysicsAsset;
class USkeletalMesh;

namespace Gaters::CharacterLab
{
CHARACTERPHYSICSPROFILEADAPTER_API bool ValidateTopology(
    const TSet<FName>& SkeletonBones,
    const TArray<FName>& BodyBones,
    const TArray<FName>& ParentBones,
    const TArray<FName>& ChildBones,
    FString& OutError);
}

UCLASS()
class CHARACTERPHYSICSPROFILEADAPTER_API UCharacterPhysicsProfileAdapterLibrary
    : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "Gaters|CharacterLab")
    static int32 GetAdapterVersion() { return 1; }

    UFUNCTION(BlueprintCallable, Category = "Gaters|CharacterLab")
    static FString RebuildPhysicsAssetTopology(
        USkeletalMesh* SkeletalMesh,
        UPhysicsAsset* PhysicsAsset,
        const TArray<FName>& BodyBones,
        const TArray<FName>& ParentBones,
        const TArray<FName>& ChildBones);
};
