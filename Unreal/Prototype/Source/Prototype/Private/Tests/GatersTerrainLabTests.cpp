#if WITH_DEV_AUTOMATION_TESTS

#include "GatersTerrainField.h"
#include "GatersTerrainLab.h"
#include "Components/DynamicMeshComponent.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersTerrainFieldTest,
	"Gaters.TerrainLab.Field",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersTerrainFieldTest::RunTest(const FString& Parameters)
{
	FGatersTerrainField Original(16, 100.f);
	Original.Generate(73);

	FGatersTerrainField SameSeed(16, 100.f);
	SameSeed.Generate(73);
	TestEqual(TEXT("same seed produces the same density checksum"),
		Original.DensityChecksum(), SameSeed.DensityChecksum());
	TestNotEqual(TEXT("neighboring cells have different stable IDs"),
		Original.CellId(2, 3, 4), Original.CellId(3, 3, 4));
	UE::Geometry::FDynamicMesh3 UncutMesh;
	SameSeed.BuildMesh(UncutMesh);
	int32 UpwardTriangles = 0;
	for (const int32 TriangleId : UncutMesh.TriangleIndicesItr())
	{
		UpwardTriangles += UncutMesh.GetTriNormal(TriangleId).Z > 0.0 ? 1 : 0;
	}
	TestTrue(TEXT("the terrain surface has consistently upward-facing triangles"),
		UpwardTriangles * 100 >= UncutMesh.TriangleCount() * 95);

	const int32 SolidBefore = Original.SolidSampleCount();
	const FGatersTerrainEdit Cut{ FVector(0.f, 0.f, 100.f), 350.f };
	Original.Apply(Cut);
	TestTrue(TEXT("a spherical cut removes solid samples"),
		Original.SolidSampleCount() < SolidBefore);

	FGatersTerrainField Replayed(16, 100.f);
	Replayed.Generate(73);
	Replayed.Apply(Cut);
	TestEqual(TEXT("seed plus the same edit replays exactly"),
		Original.DensityChecksum(), Replayed.DensityChecksum());

	UE::Geometry::FDynamicMesh3 MeshA;
	UE::Geometry::FDynamicMesh3 MeshB;
	Original.BuildMesh(MeshA);
	Replayed.BuildMesh(MeshB);
	TestTrue(TEXT("edited density extracts a visible surface"), MeshA.TriangleCount() > 0);
	TestEqual(TEXT("replayed edit extracts the same triangle count"),
		MeshA.TriangleCount(), MeshB.TriangleCount());

	const FGatersTerrainContact Contact{
		FVector2D(250.f, -150.f), 300.f, 200.f, 1000.f };
	FGatersTerrainField Fitted(16, 100.f);
	Fitted.Generate(73);
	const float DropBeforeFit = Fitted.FootprintDrop(Contact.Center, Contact.Radius);
	FGatersTerrainContactResult FitResult;
	TestTrue(TEXT("contact solver accepts a footprint within its deformation budget"),
		Fitted.TryFitContact(Contact, FitResult));
	TestEqual(TEXT("accepted contact reports its status"), FitResult.Status,
		EGatersTerrainContactStatus::Accepted);
	TestTrue(TEXT("accepted contact levels its core footprint"),
		Fitted.FootprintDrop(Contact.Center, Contact.Radius) < DropBeforeFit * 0.35f);

	FGatersTerrainField SameFit(16, 100.f);
	SameFit.Generate(73);
	FGatersTerrainContactResult SameFitResult;
	SameFit.TryFitContact(Contact, SameFitResult);
	TestEqual(TEXT("same seed and contact recipe replay exactly"),
		Fitted.DensityChecksum(), SameFit.DensityChecksum());

	FGatersTerrainField Rejected(16, 100.f);
	Rejected.Generate(73);
	const uint64 BeforeReject = Rejected.DensityChecksum();
	FGatersTerrainContactResult RejectResult;
	TestFalse(TEXT("contact solver rejects a footprint beyond its deformation budget"),
		Rejected.TryFitContact({ Contact.Center, Contact.Radius, Contact.BlendRadius, 0.f },
			RejectResult));
	TestEqual(TEXT("rejected contact explains the failure"), RejectResult.Status,
		EGatersTerrainContactStatus::TooUneven);
	TestEqual(TEXT("rejected contact leaves terrain untouched"),
		Rejected.DensityChecksum(), BeforeReject);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersTerrainLabActorTest,
	"Gaters.TerrainLab.Actor",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersTerrainLabActorTest::RunTest(const FString& Parameters)
{
	const AGatersTerrainLab* Defaults = GetDefault<AGatersTerrainLab>();
	TestEqual(TEXT("Valheim lab defaults to a larger density grid"), Defaults->CellsPerAxis, 32);
	TestEqual(TEXT("Valheim lab defaults to a larger cell size"), Defaults->CellSize, 250.f);
	TestEqual(TEXT("Valheim lab defaults to a visible cut radius"), Defaults->CutRadius, 900.f);
	TestTrue(TEXT("contact fitting is visible by default"), Defaults->bShowContactDemo);

	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AGatersTerrainLab* Lab = World->SpawnActor<AGatersTerrainLab>();
	TestNotNull(TEXT("terrain lab can be placed in a world"), Lab);
	const int32 Before = Lab->GetDynamicMeshComponent()->GetDynamicMesh()->GetMeshRef().TriangleCount();
	TestTrue(TEXT("placed terrain lab generates a surface"), Before > 0);
	TestEqual(TEXT("demo fits two contacts"), Lab->AcceptedContacts, 2);
	TestEqual(TEXT("demo exposes one rejected contact"), Lab->RejectedContacts, 1);

	Lab->ApplyTestCut();
	const int32 After = Lab->GetDynamicMeshComponent()->GetDynamicMesh()->GetMeshRef().TriangleCount();
	TestNotEqual(TEXT("editor cut button visibly changes the extracted mesh"), Before, After);
	TestEqual(TEXT("terrain lab uses blocking collision"),
		Lab->GetDynamicMeshComponent()->GetCollisionProfileName(), FName(TEXT("BlockAll")));
	const UMaterialInterface* TerrainMaterial = Lab->GetDynamicMeshComponent()->GetMaterial(0);
	TestNotNull(TEXT("terrain lab assigns a natural landscape material"), TerrainMaterial);
	if (TerrainMaterial)
	{
		TestEqual(TEXT("terrain lab reuses the polygonal landscape material"), TerrainMaterial->GetPathName(),
			FString(TEXT("/Game/EasyBuildingSystem/Materials/Instances/Polygonal/MI_Polygonal_Landscape.MI_Polygonal_Landscape")));
	}

	Lab->ResetTerrain();
	TestEqual(TEXT("reset restores the original surface"),
		Lab->GetDynamicMeshComponent()->GetDynamicMesh()->GetMeshRef().TriangleCount(), Before);

	Lab->StartCutDemo();
	TestTrue(TEXT("play-mode demo begins from intact terrain"), Lab->IsCutDemoRunning());
	TestEqual(TEXT("demo starts before any terrain is removed"),
		Lab->GetDynamicMeshComponent()->GetDynamicMesh()->GetMeshRef().TriangleCount(), Before);
	Lab->AdvanceCutDemo(Lab->CutDemoDuration);
	TestFalse(TEXT("demo stops after reaching its configured duration"), Lab->IsCutDemoRunning());
	TestTrue(TEXT("demo completes with the cut applied"), Lab->bCutApplied);
	TestNotEqual(TEXT("demo visibly changes the extracted mesh"),
		Lab->GetDynamicMeshComponent()->GetDynamicMesh()->GetMeshRef().TriangleCount(), Before);

	World->DestroyWorld(true);
	GEngine->DestroyWorldContext(World);
	return true;
}

#endif
