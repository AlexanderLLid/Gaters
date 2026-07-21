#include "GatersTerrainCell.h"

#include "Components/DynamicMeshComponent.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "DynamicMesh/DynamicMeshAttributeSet.h"
#include "GatersTerrainSemanticField.h"
#include "GatersTerrainPalette.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "Materials/MaterialInterface.h"
#include "UDynamicMesh.h"

AGatersTerrainCell::AGatersTerrainCell()
{
	PrimaryActorTick.bCanEverTick = false;
}

FVector2D AGatersTerrainCell::GlobalSamplePosition(
	const FIntPoint& InCell, const FVector2D& LocalPosition, float InCellSize)
{
	return FVector2D(InCell.X * InCellSize, InCell.Y * InCellSize) + LocalPosition;
}

void AGatersTerrainCell::Configure(
	const FGatersEnvironmentRecipe& InEnvironmentRecipe,
	const FIntPoint& InCell,
	float InCellSize,
	int32 InResolution,
	float InPadRadius,
	const FVector2D& InRouteTarget)
{
	Cell = InCell;
	CellSize = InCellSize;
	Resolution = InResolution;
	PadRadius = InPadRadius;
	RouteTarget = InRouteTarget;
	Environment = InEnvironmentRecipe.Terrain;
	Intent = InEnvironmentRecipe.Intent;
}

void AGatersTerrainCell::Build()
{
	UDynamicMeshComponent* Component = GetDynamicMeshComponent();
	UDynamicMesh* Mesh = Component->GetDynamicMesh();
	Mesh->Reset();

	FGeometryScriptPrimitiveOptions Options;
	UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendRectangleXY(
		Mesh, Options, FTransform::Identity, CellSize, CellSize, Resolution, Resolution);
	Mesh->EditMesh([this](UE::Geometry::FDynamicMesh3& EditMesh)
	{
		for (const int32 VertexId : EditMesh.VertexIndicesItr())
		{
			FVector3d Position = EditMesh.GetVertex(VertexId);
			const FVector2D Sample = GlobalSamplePosition(
				Cell, FVector2D(Position.X, Position.Y), CellSize);
			Position.Z = FGatersTerrainSemanticField::MaterializedHeightAt(
				Environment, Intent, Sample, PadRadius, RouteTarget);
			EditMesh.SetVertex(VertexId, Position);
		}

		if (!EditMesh.HasAttributes())
		{
			EditMesh.EnableAttributes();
		}
		if (!EditMesh.Attributes()->HasPrimaryColors())
		{
			EditMesh.Attributes()->EnablePrimaryColors();
		}

		UE::Geometry::FDynamicMeshNormalOverlay* Normals =
			EditMesh.Attributes()->PrimaryNormals();
		UE::Geometry::FDynamicMeshColorOverlay* Colors =
			EditMesh.Attributes()->PrimaryColors();
		Normals->ClearElements();
		Colors->ClearElements();

		TArray<int32> NormalElements;
		TArray<int32> ColorElements;
		NormalElements.SetNum(EditMesh.MaxVertexID());
		ColorElements.SetNum(EditMesh.MaxVertexID());
		const float SampleDistance = CellSize / static_cast<float>(Resolution);
		for (const int32 VertexId : EditMesh.VertexIndicesItr())
		{
			const FVector3d Position = EditMesh.GetVertex(VertexId);
			const FVector2D Sample = GlobalSamplePosition(
				Cell, FVector2D(Position.X, Position.Y), CellSize);
			const FVector Normal = FGatersTerrainSemanticField::MaterializedNormalAt(
				Environment, Intent, Sample, SampleDistance, PadRadius, RouteTarget);
			const FLinearColor Color = FGatersTerrainPalette::BlendColor(
				Environment.Type, Environment.WaterHeight,
				static_cast<float>(Position.Z), Normal.Z);
			NormalElements[VertexId] = Normals->AppendElement(FVector3f(Normal));
			ColorElements[VertexId] = Colors->AppendElement(
				FVector4f(Color.R, Color.G, Color.B, Color.A));
		}

		for (const int32 TriangleId : EditMesh.TriangleIndicesItr())
		{
			const UE::Geometry::FIndex3i Triangle = EditMesh.GetTriangle(TriangleId);
			Normals->SetTriangle(TriangleId, UE::Geometry::FIndex3i(
				NormalElements[Triangle.A], NormalElements[Triangle.B], NormalElements[Triangle.C]));
			Colors->SetTriangle(TriangleId, UE::Geometry::FIndex3i(
				ColorElements[Triangle.A], ColorElements[Triangle.B], ColorElements[Triangle.C]));
		}
	});

	if (UMaterialInterface* VertexColorMaterial = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Game/Gaters/Materials/M_TerrainVertexColor.M_TerrainVertexColor")))
	{
		Component->SetMaterial(0, VertexColorMaterial);
	}

	Component->SetCollisionProfileName(TEXT("BlockAll"));
	Component->EnableComplexAsSimpleCollision();
	// The occupied origin must collide immediately; surrounding streamed cells can cook
	// off-thread and are ready before the player reaches them.
	Component->bUseAsyncCooking = Cell != FIntPoint::ZeroValue;
	Component->UpdateCollision(false);
}
