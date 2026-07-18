#if WITH_DEV_AUTOMATION_TESTS

#include "GatersAssetContract.h"
#include "Misc/AutomationTest.h"

#include <limits>

namespace
{
FGatersAssetContract MakeValidAsset()
{
	FGatersAssetContract Asset;
	Asset.AssetId = TEXT("fixture.wood-foundation.1");
	Asset.ContentKey = TEXT("building.foundation.wood");
	Asset.StyleId = TEXT("gaters.clean-midpoly-painted");
	Asset.CentimetersPerUnit = 1.f;
	Asset.BoundsCenter = FVector(15, 0, 0);
	Asset.BoundsExtent = FVector(200, 200, 25);
	Asset.ClearanceExtent = FVector(210, 210, 30);
	Asset.Collision = EGatersAssetCollision::Simple;
	Asset.RenderClass = EGatersAssetRenderClass::InstancedStatic;
	Asset.bSourceRegenerable = true;
	Asset.bInstanceStatePersistent = true;
	Asset.Contacts.Add({TEXT("ground"), FVector(0, 0, -25), FVector::UpVector});
	Asset.Ports.Add({TEXT("edge.north"), FTransform(FVector(200, 0, 0)), FVector(20, 200, 100)});
	return Asset;
}

bool HasError(const TArray<FString>& Errors, const TCHAR* Fragment)
{
	return Errors.ContainsByPredicate([Fragment](const FString& Error)
	{
		return Error.Contains(Fragment);
	});
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersAssetContractValidationTest,
	"Gaters.Content.AssetContract.Validation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersAssetContractValidationTest::RunTest(const FString& Parameters)
{
	TArray<FString> Errors;
	const FGatersAssetContract Valid = MakeValidAsset();
	TestTrue(TEXT("complete asset contract is valid"), Valid.Validate(Errors));
	TestEqual(TEXT("valid contract has no diagnostics"), Errors.Num(), 0);
	TestTrue(TEXT("regeneration and instance persistence are independent"),
		Valid.bSourceRegenerable && Valid.bInstanceStatePersistent);

	FGatersAssetContract BadVersion = Valid;
	BadVersion.SchemaVersion = 0;
	TestFalse(TEXT("schema version is required"), BadVersion.Validate(Errors));
	TestTrue(TEXT("version diagnostic is actionable"), HasError(Errors, TEXT("positive schema")));

	FGatersAssetContract MissingAssetId = Valid;
	MissingAssetId.AssetId.Reset();
	TestFalse(TEXT("asset identity is required"), MissingAssetId.Validate(Errors));
	TestTrue(TEXT("identity diagnostic is actionable"), HasError(Errors, TEXT("requires AssetId")));

	FGatersAssetContract MissingStyle = Valid;
	MissingStyle.StyleId.Reset();
	TestFalse(TEXT("style identity is required"), MissingStyle.Validate(Errors));
	TestTrue(TEXT("style diagnostic is actionable"), HasError(Errors, TEXT("StyleId")));

	FGatersAssetContract MissingKey = Valid;
	MissingKey.ContentKey.Reset();
	TestFalse(TEXT("semantic content key is required"), MissingKey.Validate(Errors));
	TestTrue(TEXT("content-key diagnostic is actionable"), HasError(Errors, TEXT("requires ContentKey")));

	FGatersAssetContract BadScale = Valid;
	BadScale.CentimetersPerUnit = 0.f;
	TestFalse(TEXT("positive authoring scale is required"), BadScale.Validate(Errors));
	TestTrue(TEXT("scale diagnostic is actionable"), HasError(Errors, TEXT("CentimetersPerUnit")));

	FGatersAssetContract BadOrientation = Valid;
	BadOrientation.UpAxis = BadOrientation.ForwardAxis;
	TestFalse(TEXT("forward and up must be orthogonal"), BadOrientation.Validate(Errors));
	TestTrue(TEXT("orientation diagnostic is actionable"), HasError(Errors, TEXT("orthogonal")));

	FGatersAssetContract SourceOrientation = Valid;
	SourceOrientation.ForwardAxis = FVector::RightVector;
	TestFalse(TEXT("source tools must normalize into canonical axes"), SourceOrientation.Validate(Errors));
	TestTrue(TEXT("canonical-axis diagnostic is actionable"), HasError(Errors, TEXT("canonical")));

	FGatersAssetContract BadBounds = Valid;
	BadBounds.BoundsExtent.Z = 0.f;
	TestFalse(TEXT("positive bounds are required"), BadBounds.Validate(Errors));
	TestTrue(TEXT("bounds diagnostic is actionable"), HasError(Errors, TEXT("BoundsExtent")));

	FGatersAssetContract BadCenter = Valid;
	BadCenter.BoundsCenter.X = std::numeric_limits<double>::quiet_NaN();
	TestFalse(TEXT("finite bounds center is required"), BadCenter.Validate(Errors));
	TestTrue(TEXT("center diagnostic is actionable"), HasError(Errors, TEXT("BoundsCenter")));

	FGatersAssetContract NoContacts = Valid;
	NoContacts.Contacts.Reset();
	TestFalse(TEXT("at least one physical contact is required"), NoContacts.Validate(Errors));
	TestTrue(TEXT("contact diagnostic is actionable"), HasError(Errors, TEXT("at least one contact")));

	FGatersAssetContract BadContact = Valid;
	BadContact.Contacts[0].Normal = FVector::ZeroVector;
	TestFalse(TEXT("contact normals must be normalized"), BadContact.Validate(Errors));
	TestTrue(TEXT("contact diagnostic names the contact"), HasError(Errors, TEXT("contact ground")));

	FGatersAssetContract OutsideContact = Valid;
	OutsideContact.Contacts[0].Location.Z = -1000.f;
	TestFalse(TEXT("contacts must lie on or inside measured bounds"), OutsideContact.Validate(Errors));
	TestTrue(TEXT("contact-surface diagnostic names the contact"), HasError(Errors, TEXT("contact ground")));

	FGatersAssetContract DuplicateContact = Valid;
	DuplicateContact.Contacts.Add(Valid.Contacts[0]);
	TestFalse(TEXT("contact names are unique"), DuplicateContact.Validate(Errors));
	TestTrue(TEXT("duplicate contact diagnostic names it"), HasError(Errors, TEXT("duplicate contact ground")));

	FGatersAssetContract BadClearance = Valid;
	BadClearance.ClearanceExtent.X = -1.f;
	TestFalse(TEXT("clearance cannot be negative"), BadClearance.Validate(Errors));
	TestTrue(TEXT("clearance diagnostic is actionable"), HasError(Errors, TEXT("ClearanceExtent")));

	FGatersAssetContract BadCollision = Valid;
	BadCollision.Collision = static_cast<EGatersAssetCollision>(255);
	TestFalse(TEXT("collision policy must be declared"), BadCollision.Validate(Errors));
	TestTrue(TEXT("collision diagnostic is actionable"), HasError(Errors, TEXT("Collision")));

	FGatersAssetContract BadRenderClass = Valid;
	BadRenderClass.RenderClass = static_cast<EGatersAssetRenderClass>(255);
	TestFalse(TEXT("render class must be declared"), BadRenderClass.Validate(Errors));
	TestTrue(TEXT("render-class diagnostic is actionable"), HasError(Errors, TEXT("RenderClass")));

	FGatersAssetContract DuplicatePort = Valid;
	DuplicatePort.Ports.Add(Valid.Ports[0]);
	TestFalse(TEXT("port names are unique"), DuplicatePort.Validate(Errors));
	TestTrue(TEXT("port diagnostic names the duplicate"), HasError(Errors, TEXT("duplicate port edge.north")));

	FGatersAssetContract BadPort = Valid;
	BadPort.Ports[0].ClearanceExtent.Y = -1.f;
	TestFalse(TEXT("port clearance cannot be negative"), BadPort.Validate(Errors));
	TestTrue(TEXT("port diagnostic names the port"), HasError(Errors, TEXT("port edge.north")));

	FGatersAssetContract BadPortRotation = Valid;
	BadPortRotation.Ports[0].Transform.SetRotation(FQuat(0, 0, 0, 0));
	TestFalse(TEXT("port rotations must be normalized"), BadPortRotation.Validate(Errors));
	TestTrue(TEXT("socket-transform diagnostic names the port"), HasError(Errors, TEXT("port edge.north")));
	return true;
}

#endif
