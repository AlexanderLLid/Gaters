#include "GatersAssetContract.h"

namespace
{
bool IsPositive(const FVector& Value)
{
	return !Value.ContainsNaN() && Value.X > 0 && Value.Y > 0 && Value.Z > 0;
}

bool IsNonNegative(const FVector& Value)
{
	return !Value.ContainsNaN() && Value.X >= 0 && Value.Y >= 0 && Value.Z >= 0;
}
}

bool FGatersAssetContract::Validate(TArray<FString>& OutErrors) const
{
	OutErrors.Reset();
	if (SchemaVersion <= 0 || Version <= 0)
	{
		OutErrors.Add(TEXT("asset contract requires positive schema and asset versions"));
	}
	if (AssetId.IsEmpty())
	{
		OutErrors.Add(TEXT("asset contract requires AssetId"));
	}
	if (ContentKey.IsEmpty())
	{
		OutErrors.Add(TEXT("asset contract requires ContentKey"));
	}
	if (StyleId.IsEmpty() || StyleVersion <= 0)
	{
		OutErrors.Add(TEXT("asset contract requires a versioned StyleId"));
	}
	if (!FMath::IsFinite(CentimetersPerUnit) || CentimetersPerUnit <= 0)
	{
		OutErrors.Add(TEXT("asset contract requires positive CentimetersPerUnit"));
	}
	if (!ForwardAxis.IsNormalized() || !UpAxis.IsNormalized())
	{
		OutErrors.Add(TEXT("asset contract orientation axes must be normalized"));
	}
	else if (!FMath::IsNearlyZero(FVector::DotProduct(ForwardAxis, UpAxis)))
	{
		OutErrors.Add(TEXT("asset contract forward and up axes must be orthogonal"));
	}
	else if (!ForwardAxis.Equals(FVector::ForwardVector) || !UpAxis.Equals(FVector::UpVector))
	{
		OutErrors.Add(TEXT("asset contract orientation must use canonical +X forward and +Z up axes"));
	}
	if (!IsPositive(BoundsExtent))
	{
		OutErrors.Add(TEXT("asset contract requires positive finite BoundsExtent"));
	}
	if (BoundsCenter.ContainsNaN())
	{
		OutErrors.Add(TEXT("asset contract requires finite BoundsCenter"));
	}
	if (!IsNonNegative(ClearanceExtent))
	{
		OutErrors.Add(TEXT("asset contract requires non-negative finite ClearanceExtent"));
	}
	if (Collision != EGatersAssetCollision::None &&
		Collision != EGatersAssetCollision::Simple &&
		Collision != EGatersAssetCollision::Complex)
	{
		OutErrors.Add(TEXT("asset contract has invalid Collision policy"));
	}
	if (RenderClass != EGatersAssetRenderClass::InstancedStatic &&
		RenderClass != EGatersAssetRenderClass::UniqueStatic &&
		RenderClass != EGatersAssetRenderClass::Skeletal)
	{
		OutErrors.Add(TEXT("asset contract has invalid RenderClass"));
	}
	if (Contacts.IsEmpty())
	{
		OutErrors.Add(TEXT("asset contract requires at least one contact"));
	}

	TSet<FString> Names;
	for (const FGatersAssetContact& Contact : Contacts)
	{
		if (Contact.Name.IsEmpty())
		{
			OutErrors.Add(TEXT("asset contact requires a name"));
		}
		else if (Names.Contains(Contact.Name))
		{
			OutErrors.Add(FString::Printf(TEXT("asset contract has duplicate contact %s"), *Contact.Name));
		}
		Names.Add(Contact.Name);
		const FVector FromCenter = (Contact.Location - BoundsCenter).GetAbs();
		if (Contact.Location.ContainsNaN() || !Contact.Normal.IsNormalized() ||
			FromCenter.X > BoundsExtent.X || FromCenter.Y > BoundsExtent.Y ||
			FromCenter.Z > BoundsExtent.Z)
		{
			OutErrors.Add(FString::Printf(TEXT("contact %s requires an in-bounds location and normalized normal"), *Contact.Name));
		}
		if (Contact.Support != EGatersAssetContactSupport::Terrain &&
			Contact.Support != EGatersAssetContactSupport::Attachment)
		{
			OutErrors.Add(FString::Printf(TEXT("contact %s has invalid support intent"), *Contact.Name));
		}
	}

	Names.Reset();
	for (const FGatersAssetPort& Port : Ports)
	{
		if (Port.Name.IsEmpty())
		{
			OutErrors.Add(TEXT("asset port requires a name"));
		}
		else if (Names.Contains(Port.Name))
		{
			OutErrors.Add(FString::Printf(TEXT("asset contract has duplicate port %s"), *Port.Name));
		}
		Names.Add(Port.Name);
		if (!Port.Transform.IsValid() || !IsPositive(Port.Transform.GetScale3D()) ||
			!IsNonNegative(Port.ClearanceExtent))
		{
			OutErrors.Add(FString::Printf(TEXT("port %s has invalid transform or clearance"), *Port.Name));
		}
	}
	return OutErrors.IsEmpty();
}
