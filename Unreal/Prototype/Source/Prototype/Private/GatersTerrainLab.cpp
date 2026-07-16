#include "GatersTerrainLab.h"

#include "GatersTerrainField.h"
#include "Components/DynamicMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GeometryScript/MeshNormalsFunctions.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UDynamicMesh.h"

AGatersTerrainLab::AGatersTerrainLab()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(false);

	ContactObjectA = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FittedContactA"));
	ContactObjectA->SetupAttachment(GetRootComponent());
	ContactObjectA->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ContactObjectB = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FittedContactB"));
	ContactObjectB->SetupAttachment(GetRootComponent());
	ContactObjectB->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RejectedObject = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RejectedContact"));
	RejectedObject->SetupAttachment(GetRootComponent());
	RejectedObject->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGatersTerrainLab::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RebuildTerrain();
}

void AGatersTerrainLab::RebuildTerrain()
{
	BuildTerrain(bCutApplied ? FMath::Max(CutRadius, 1.f) : 0.f);
}

void AGatersTerrainLab::BuildTerrain(float ActiveCutRadius)
{
	const int32 Resolution = FMath::Clamp(CellsPerAxis, 4, 48);
	const float Spacing = FMath::Max(CellSize, 10.f);
	FGatersTerrainField Field(Resolution, Spacing);
	Field.Generate(Seed);
	if (ActiveCutRadius > 0.f)
	{
		Field.Apply({ CutCenter, ActiveCutRadius });
	}

	AcceptedContacts = 0;
	RejectedContacts = 0;
	UStaticMeshComponent* ContactObjects[] = { ContactObjectA, ContactObjectB, RejectedObject };
	const float Extent = Resolution * Spacing;
	const FGatersTerrainContact Contacts[] = {
		{ FVector2D(-Extent * 0.24f, -Extent * 0.12f), Extent * 0.075f,
			Extent * 0.055f, Extent * 0.25f },
		{ FVector2D(Extent * 0.18f, Extent * 0.20f), Extent * 0.09f,
			Extent * 0.06f, Extent * 0.25f },
		{ FVector2D(Extent * 0.08f, -Extent * 0.28f), Extent * 0.09f,
			Extent * 0.06f, 0.f }
	};
	UMaterialInterface* MarkerBaseMaterial = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(Contacts); ++Index)
	{
		UStaticMeshComponent* Object = ContactObjects[Index];
		Object->SetVisibility(bShowContactDemo);
		if (!bShowContactDemo)
		{
			continue;
		}

		FGatersTerrainContactResult Result;
		const bool bAccepted = Field.TryFitContact(Contacts[Index], Result);
		AcceptedContacts += bAccepted ? 1 : 0;
		RejectedContacts += bAccepted ? 0 : 1;
		Object->SetStaticMesh(Index == 1 ? Cylinder : Cube);
		const float ObjectHeight = Index == 1 ? 900.f : 700.f;
		Object->SetRelativeScale3D(FVector(10.f, 10.f, ObjectHeight / 100.f));
		Object->SetRelativeLocation(Result.Placement + FVector(0.f, 0.f, ObjectHeight * 0.5f));

		if (MarkerBaseMaterial)
		{
			UMaterialInstanceDynamic* MarkerMaterial =
				UMaterialInstanceDynamic::Create(MarkerBaseMaterial, this);
			MarkerMaterial->SetVectorParameterValue(TEXT("Color"), bAccepted
				? FLinearColor(0.06f, 0.65f, 0.14f)
				: FLinearColor(0.85f, 0.04f, 0.02f));
			Object->SetMaterial(0, MarkerMaterial);
		}
	}

	UDynamicMeshComponent* Component = GetDynamicMeshComponent();
	UDynamicMesh* TargetMesh = Component->GetDynamicMesh();
	TargetMesh->Reset();
	TargetMesh->EditMesh([&Field](UE::Geometry::FDynamicMesh3& Mesh)
	{
		Field.BuildMesh(Mesh);
	});

	UGeometryScriptLibrary_MeshNormalsFunctions::SetPerVertexNormals(TargetMesh);
	GeneratedTriangles = TargetMesh->GetMeshRef().TriangleCount();

	if (UMaterialInterface* LandscapeMaterial = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Game/EasyBuildingSystem/Materials/Instances/Polygonal/MI_Polygonal_Landscape.MI_Polygonal_Landscape")))
	{
		Component->SetMaterial(0, LandscapeMaterial);
	}
	else if (UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
	{
		UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		Material->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.30f, 0.32f, 0.18f));
		Component->SetMaterial(0, Material);
	}

	Component->SetCollisionProfileName(TEXT("BlockAll"));
	Component->EnableComplexAsSimpleCollision();
	Component->UpdateCollision(false);
	UE_LOG(LogTemp, Display, TEXT("[TerrainLab] seed=%d cut_radius=%.0f triangles=%d checksum=%llu"),
		Seed, ActiveCutRadius, GeneratedTriangles, Field.DensityChecksum());
}

void AGatersTerrainLab::ApplyTestCut()
{
	bCutApplied = true;
	RebuildTerrain();
}

void AGatersTerrainLab::ResetTerrain()
{
	bCutApplied = false;
	RebuildTerrain();
}

void AGatersTerrainLab::BeginPlay()
{
	Super::BeginPlay();
	if (bPlayCutDemoOnBeginPlay)
	{
		StartCutDemo();
	}
}

void AGatersTerrainLab::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	AdvanceCutDemo(DeltaSeconds);
}

void AGatersTerrainLab::StartCutDemo()
{
	bCutApplied = false;
	bCutDemoRunning = true;
	DemoElapsed = 0.f;
	DemoRebuildElapsed = 0.f;
	BuildTerrain(0.f);
	SetActorTickEnabled(true);
}

void AGatersTerrainLab::AdvanceCutDemo(float DeltaSeconds)
{
	if (!bCutDemoRunning)
	{
		return;
	}
	DemoElapsed += FMath::Max(DeltaSeconds, 0.f);
	DemoRebuildElapsed += FMath::Max(DeltaSeconds, 0.f);
	const float Duration = FMath::Max(CutDemoDuration, 0.1f);
	const bool bFinished = DemoElapsed >= Duration;
	if (!bFinished && DemoRebuildElapsed < FMath::Max(CutDemoUpdateSeconds, 0.01f))
	{
		return;
	}

	DemoRebuildElapsed = 0.f;
	const float Radius = FMath::Lerp(0.f, FMath::Max(CutRadius, 1.f),
		FMath::Clamp(DemoElapsed / Duration, 0.f, 1.f));
	BuildTerrain(Radius);
	if (bFinished)
	{
		bCutApplied = true;
		bCutDemoRunning = false;
		SetActorTickEnabled(false);
	}
}
