#if WITH_DEV_AUTOMATION_TESTS

#include "GatersAssetContract.h"
#include "GatersAssetIntake.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Misc/AutomationTest.h"

namespace
{
FGatersAssetContract ImportedFoundationContract()
{
	FGatersAssetContract Contract;
	Contract.AssetId = TEXT("fixture.wood-foundation.1");
	Contract.ContentKey = TEXT("building.foundation.wood");
	Contract.StyleId = TEXT("gaters.clean-midpoly-painted");
	Contract.BoundsCenter = FVector(0, 0, 25);
	Contract.BoundsExtent = FVector(200, 200, 25);
	Contract.ClearanceExtent = FVector(200, 200, 25);
	Contract.Collision = EGatersAssetCollision::Simple;
	Contract.RenderClass = EGatersAssetRenderClass::InstancedStatic;
	Contract.Contacts.Add({TEXT("ground"), FVector::ZeroVector, FVector::UpVector});
	return Contract;
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
	FGatersStaticMeshIntakeTest,
	"Gaters.Content.AssetIntake.StaticMesh",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersStaticMeshIntakeTest::RunTest(const FString& Parameters)
{
	const UStaticMesh* Mesh = LoadObject<UStaticMesh>(
		nullptr, TEXT("/Game/Gaters/Generated/Fixtures/SM_WoodFoundation.SM_WoodFoundation"));
	TestNotNull(TEXT("headless importer produced the fixture mesh"), Mesh);
	if (!Mesh)
	{
		return false;
	}

	TArray<FString> Errors;
	const FGatersAssetContract Matching = ImportedFoundationContract();
	TestTrue(TEXT("matching imported mesh passes intake"),
		FGatersAssetIntake::ValidateStaticMesh(*Mesh, Matching, Errors, 0.1f));
	TestEqual(TEXT("matching mesh has no diagnostics"), Errors.Num(), 0);

	FGatersAssetContract MeterScaled = Matching;
	MeterScaled.CentimetersPerUnit = 2.f;
	MeterScaled.BoundsCenter /= MeterScaled.CentimetersPerUnit;
	MeterScaled.BoundsExtent /= MeterScaled.CentimetersPerUnit;
	MeterScaled.ClearanceExtent /= MeterScaled.CentimetersPerUnit;
	TestTrue(TEXT("contract units are converted to centimeters"),
		FGatersAssetIntake::ValidateStaticMesh(*Mesh, MeterScaled, Errors, 0.1f));
	TestEqual(TEXT("scaled contract has no diagnostics"), Errors.Num(), 0);

	FGatersAssetContract WrongNoneCollision = Matching;
	WrongNoneCollision.Collision = EGatersAssetCollision::None;
	TestFalse(TEXT("mesh with simple collision cannot satisfy a none policy"),
		FGatersAssetIntake::ValidateStaticMesh(*Mesh, WrongNoneCollision, Errors, 0.1f));
	TestTrue(TEXT("none-policy diagnostic reports collision"), HasError(Errors, TEXT("collision")));

	FGatersAssetContract WrongComplexCollision = Matching;
	WrongComplexCollision.Collision = EGatersAssetCollision::Complex;
	TestFalse(TEXT("mesh with simple collision cannot satisfy a complex policy"),
		FGatersAssetIntake::ValidateStaticMesh(*Mesh, WrongComplexCollision, Errors, 0.1f));
	TestTrue(TEXT("complex-policy diagnostic reports collision"), HasError(Errors, TEXT("collision")));

	FGatersAssetContract WrongPivot = Matching;
	WrongPivot.BoundsCenter.X += 50.f;
	TestFalse(TEXT("wrong declared pivot is rejected"),
		FGatersAssetIntake::ValidateStaticMesh(*Mesh, WrongPivot, Errors, 0.1f));
	TestTrue(TEXT("pivot diagnostic is actionable"), HasError(Errors, TEXT("BoundsCenter")));

	FGatersAssetContract MissingSocket = Matching;
	MissingSocket.Ports.Add({TEXT("edge.north"), FTransform::Identity, FVector::ZeroVector});
	TestFalse(TEXT("missing declared socket is rejected"),
		FGatersAssetIntake::ValidateStaticMesh(*Mesh, MissingSocket, Errors, 0.1f));
	TestTrue(TEXT("socket diagnostic names the port"), HasError(Errors, TEXT("edge.north")));

	FGatersAssetContract WrongClass = Matching;
	WrongClass.RenderClass = EGatersAssetRenderClass::Skeletal;
	TestFalse(TEXT("skeletal contract cannot accept a static mesh"),
		FGatersAssetIntake::ValidateStaticMesh(*Mesh, WrongClass, Errors, 0.1f));
	TestTrue(TEXT("render-class diagnostic is actionable"), HasError(Errors, TEXT("RenderClass")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersSkeletalMeshIntakeTest,
	"Gaters.Content.AssetIntake.SkeletalMesh",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersSkeletalMeshIntakeTest::RunTest(const FString& Parameters)
{
	const USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(
		nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	TestNotNull(TEXT("Manny skeletal fixture exists"), Mesh);
	if (!Mesh)
	{
		return false;
	}

	const FBoxSphereBounds Bounds = Mesh->GetBounds();
	const FName RootBone = Mesh->GetRefSkeleton().GetBoneName(0);
	FGatersAssetContract Matching;
	Matching.AssetId = TEXT("fixture.manny.1");
	Matching.ContentKey = TEXT("character.body.humanoid");
	Matching.StyleId = TEXT("gaters.clean-midpoly-painted");
	Matching.BoundsCenter = Bounds.Origin;
	Matching.BoundsExtent = Bounds.BoxExtent;
	Matching.ClearanceExtent = Bounds.BoxExtent;
	Matching.Collision = Mesh->GetPhysicsAsset()
		? EGatersAssetCollision::Simple : EGatersAssetCollision::None;
	Matching.RenderClass = EGatersAssetRenderClass::Skeletal;
	Matching.Contacts.Add({TEXT("body"), Bounds.Origin, FVector::UpVector});
	Matching.Ports.Add({RootBone.ToString(), FTransform::Identity, FVector::ZeroVector});

	TArray<FString> Errors;
	TestTrue(TEXT("matching skeletal mesh passes intake"),
		FGatersAssetIntake::ValidateSkeletalMesh(*Mesh, Matching, Errors, 0.1f));
	TestEqual(TEXT("matching skeletal mesh has no diagnostics"), Errors.Num(), 0);

	FGatersAssetContract WrongClass = Matching;
	WrongClass.RenderClass = EGatersAssetRenderClass::UniqueStatic;
	TestFalse(TEXT("static contract cannot accept skeletal mesh"),
		FGatersAssetIntake::ValidateSkeletalMesh(*Mesh, WrongClass, Errors, 0.1f));
	TestTrue(TEXT("skeletal render-class diagnostic is actionable"),
		HasError(Errors, TEXT("RenderClass")));

	FGatersAssetContract WrongBounds = Matching;
	WrongBounds.BoundsExtent.X += 10.f;
	TestFalse(TEXT("wrong skeletal bounds are rejected"),
		FGatersAssetIntake::ValidateSkeletalMesh(*Mesh, WrongBounds, Errors, 0.1f));
	TestTrue(TEXT("skeletal bounds diagnostic is actionable"),
		HasError(Errors, TEXT("BoundsExtent")));

	FGatersAssetContract WrongCollision = Matching;
	WrongCollision.Collision = Matching.Collision == EGatersAssetCollision::Simple
		? EGatersAssetCollision::None : EGatersAssetCollision::Simple;
	TestFalse(TEXT("wrong skeletal collision policy is rejected"),
		FGatersAssetIntake::ValidateSkeletalMesh(*Mesh, WrongCollision, Errors, 0.1f));
	TestTrue(TEXT("skeletal collision diagnostic is actionable"),
		HasError(Errors, TEXT("collision")));

	FGatersAssetContract MissingBone = Matching;
	MissingBone.Ports[0].Name = TEXT("bone.that.does.not.exist");
	TestFalse(TEXT("missing declared skeletal port is rejected"),
		FGatersAssetIntake::ValidateSkeletalMesh(*Mesh, MissingBone, Errors, 0.1f));
	TestTrue(TEXT("skeletal port diagnostic names the port"),
		HasError(Errors, TEXT("bone.that.does.not.exist")));
	return true;
}

#endif
