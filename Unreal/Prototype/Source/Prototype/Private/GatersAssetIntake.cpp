#include "GatersAssetIntake.h"

#include "GatersAssetContract.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "PhysicsEngine/BodySetup.h"

bool FGatersAssetIntake::ValidateStaticMesh(
	const UStaticMesh& Mesh,
	const FGatersAssetContract& Contract,
	TArray<FString>& OutErrors,
	const float BoundsToleranceCm)
{
	if (!Contract.Validate(OutErrors))
	{
		return false;
	}
	if (Contract.RenderClass != EGatersAssetRenderClass::InstancedStatic &&
		Contract.RenderClass != EGatersAssetRenderClass::UniqueStatic)
	{
		OutErrors.Add(TEXT("StaticMesh intake requires a static RenderClass"));
	}

	const FBoxSphereBounds Bounds = Mesh.GetBounds();
	const FVector ExpectedCenterCm = Contract.BoundsCenter * Contract.CentimetersPerUnit;
	const FVector ExpectedExtentCm = Contract.BoundsExtent * Contract.CentimetersPerUnit;
	if (!Bounds.Origin.Equals(ExpectedCenterCm, BoundsToleranceCm))
	{
		OutErrors.Add(FString::Printf(
			TEXT("measured BoundsCenter %s does not match declared %s"),
			*Bounds.Origin.ToString(), *ExpectedCenterCm.ToString()));
	}
	if (!Bounds.BoxExtent.Equals(ExpectedExtentCm, BoundsToleranceCm))
	{
		OutErrors.Add(FString::Printf(
			TEXT("measured BoundsExtent %s does not match declared %s"),
			*Bounds.BoxExtent.ToString(), *ExpectedExtentCm.ToString()));
	}
	for (const FGatersAssetPort& Port : Contract.Ports)
	{
		if (!Mesh.FindSocket(FName(*Port.Name)))
		{
			OutErrors.Add(FString::Printf(TEXT("missing declared static-mesh socket %s"), *Port.Name));
		}
	}
	const UBodySetup* BodySetup = Mesh.GetBodySetup();
	const int32 SimpleShapeCount = BodySetup ? BodySetup->AggGeom.GetElementCount() : 0;
	const ECollisionTraceFlag TraceFlag = BodySetup
		? static_cast<ECollisionTraceFlag>(BodySetup->CollisionTraceFlag.GetValue())
		: CTF_UseDefault;
	const bool bUsesComplexAsSimple = TraceFlag == CTF_UseComplexAsSimple;
	const bool bCollisionMatches =
		(Contract.Collision == EGatersAssetCollision::None && SimpleShapeCount == 0 && !bUsesComplexAsSimple) ||
		(Contract.Collision == EGatersAssetCollision::Simple && SimpleShapeCount > 0 && !bUsesComplexAsSimple) ||
		(Contract.Collision == EGatersAssetCollision::Complex && bUsesComplexAsSimple);
	if (!bCollisionMatches)
	{
		OutErrors.Add(FString::Printf(
			TEXT("collision policy mismatch: declared=%d simpleShapes=%d traceFlag=%d"),
			static_cast<int32>(Contract.Collision), SimpleShapeCount, static_cast<int32>(TraceFlag)));
	}
	return OutErrors.IsEmpty();
}

bool FGatersAssetIntake::ValidateSkeletalMesh(
	const USkeletalMesh& Mesh,
	const FGatersAssetContract& Contract,
	TArray<FString>& OutErrors,
	const float BoundsToleranceCm)
{
	if (!Contract.Validate(OutErrors))
	{
		return false;
	}
	if (Contract.RenderClass != EGatersAssetRenderClass::Skeletal)
	{
		OutErrors.Add(TEXT("SkeletalMesh intake requires Skeletal RenderClass"));
	}

	const FBoxSphereBounds Bounds = Mesh.GetBounds();
	const FVector ExpectedCenterCm = Contract.BoundsCenter * Contract.CentimetersPerUnit;
	const FVector ExpectedExtentCm = Contract.BoundsExtent * Contract.CentimetersPerUnit;
	if (!Bounds.Origin.Equals(ExpectedCenterCm, BoundsToleranceCm))
	{
		OutErrors.Add(FString::Printf(
			TEXT("measured BoundsCenter %s does not match declared %s"),
			*Bounds.Origin.ToString(), *ExpectedCenterCm.ToString()));
	}
	if (!Bounds.BoxExtent.Equals(ExpectedExtentCm, BoundsToleranceCm))
	{
		OutErrors.Add(FString::Printf(
			TEXT("measured BoundsExtent %s does not match declared %s"),
			*Bounds.BoxExtent.ToString(), *ExpectedExtentCm.ToString()));
	}

	for (const FGatersAssetPort& Port : Contract.Ports)
	{
		const FName Name(*Port.Name);
		if (!Mesh.FindSocket(Name) && Mesh.GetRefSkeleton().FindBoneIndex(Name) == INDEX_NONE)
		{
			OutErrors.Add(FString::Printf(
				TEXT("missing declared skeletal socket or bone %s"), *Port.Name));
		}
	}

	const bool bHasPhysicsAsset = Mesh.GetPhysicsAsset() != nullptr;
	const bool bCollisionMatches =
		(Contract.Collision == EGatersAssetCollision::None && !bHasPhysicsAsset) ||
		(Contract.Collision == EGatersAssetCollision::Simple && bHasPhysicsAsset);
	if (!bCollisionMatches)
	{
		OutErrors.Add(FString::Printf(
			TEXT("collision policy mismatch: declared=%d physicsAsset=%d"),
			static_cast<int32>(Contract.Collision), bHasPhysicsAsset ? 1 : 0));
	}
	return OutErrors.IsEmpty();
}
