#pragma once

#include "CoreMinimal.h"

enum class EGatersAssetCollision : uint8
{
	None,
	Simple,
	Complex
};

enum class EGatersAssetRenderClass : uint8
{
	InstancedStatic,
	UniqueStatic,
	Skeletal
};

enum class EGatersAssetContactSupport : uint8
{
	Terrain,
	Attachment
};

struct PROTOTYPE_API FGatersAssetContact
{
	FString Name;
	FVector Location = FVector::ZeroVector;
	FVector Normal = FVector::UpVector;
	EGatersAssetContactSupport Support = EGatersAssetContactSupport::Terrain;
};

struct PROTOTYPE_API FGatersAssetPort
{
	FString Name;
	FTransform Transform = FTransform::Identity;
	FVector ClearanceExtent = FVector::ZeroVector;
};

// Tool-independent mechanical facts. Spatial values use a right-handed contract space,
// CentimetersPerUnit converts them to centimeters, and +X forward / +Z up is canonical;
// source tools normalize into this space before Unreal import. Unreal assets and Actors
// are derived outputs, not part of the contract identity.
struct PROTOTYPE_API FGatersAssetContract
{
	bool Validate(TArray<FString>& OutErrors) const;

	int32 SchemaVersion = 1;
	int32 Version = 1;
	FString AssetId;
	FString ContentKey;
	FString StyleId;
	int32 StyleVersion = 1;
	float CentimetersPerUnit = 1.f;
	FVector ForwardAxis = FVector::ForwardVector;
	FVector UpAxis = FVector::UpVector;
	FVector BoundsCenter = FVector::ZeroVector;
	FVector BoundsExtent = FVector::ZeroVector;
	FVector ClearanceExtent = FVector::ZeroVector;
	EGatersAssetCollision Collision = EGatersAssetCollision::None;
	EGatersAssetRenderClass RenderClass = EGatersAssetRenderClass::UniqueStatic;
	bool bSourceRegenerable = true;
	bool bInstanceStatePersistent = false;
	TArray<FGatersAssetContact> Contacts;
	TArray<FGatersAssetPort> Ports;
};
