#include "GatersVisualMaterializer.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"

int32 FGatersVisualBatchPlan::NumInstances() const
{
	return Trees.Num() + Rocks.Num() + OpenClaims.Num() + ClaimedClaims.Num()
		+ VillageFoundations.Num() + VillageWalls.Num() + VillageRoofs.Num()
		+ VillageSpaces.Num();
}

namespace
{
void AddDoorFrame(
	const FString& StableId,
	const FTransform& Envelope,
	TArray<FGatersVisualInstance>& OutWalls)
{
	const FVector Scale = Envelope.GetScale3D();
	const bool bLongX = Scale.X >= Scale.Y;
	const float LongScale = bLongX ? Scale.X : Scale.Y;
	const float OpeningScale = LongScale * 0.45f;
	const float SideScale = (LongScale - OpeningScale) * 0.5f;
	const float OffsetCm = (OpeningScale + SideScale) * 50.f;
	const FVector LocalAxis = bLongX ? FVector(1, 0, 0) : FVector(0, 1, 0);
	for (const int32 Side : {-1, 1})
	{
		FTransform Post = Envelope;
		FVector PostScale = Scale;
		(bLongX ? PostScale.X : PostScale.Y) = SideScale;
		Post.SetScale3D(PostScale);
		Post.AddToTranslation(Envelope.GetRotation().RotateVector(LocalAxis * OffsetCm * Side));
		OutWalls.Add({StableId + (Side < 0 ? TEXT(":frame:left") : TEXT(":frame:right")), Post});
	}
	FTransform Lintel = Envelope;
	FVector LintelScale = Scale;
	(bLongX ? LintelScale.X : LintelScale.Y) = OpeningScale;
	LintelScale.Z = Scale.Z * 0.22f;
	Lintel.SetScale3D(LintelScale);
	Lintel.AddToTranslation(FVector(0.f, 0.f, (Scale.Z - LintelScale.Z) * 50.f));
	OutWalls.Add({StableId + TEXT(":frame:top"), Lintel});
}
}

FGatersVisualBatchPlan FGatersVisualMaterializer::Plan(
	const FGatersCompiledWorld& World,
	const TArray<FString>& DiffEntries)
{
	FGatersVisualBatchPlan Result;
	for (const FGatersCompiledNode& Node : World.Nodes)
	{
		if (Node.Kind == EGatersRecipeNodeKind::VillageSite)
		{
			FTransform Transform = Node.Transform;
			Transform.AddToTranslation(FVector(0.f, 0.f, 10.f));
			Transform.SetScale3D(FVector(5.f, 5.f, 0.2f));
			Result.VillageSpaces.Add({Node.NodeId, Transform});
			continue;
		}
		if (Node.Kind == EGatersRecipeNodeKind::SettlementModule)
		{
			if (Node.ContentKey == TEXT("building.foundation")
				|| Node.ContentKey == TEXT("building.floor"))
			{
				Result.VillageFoundations.Add({Node.NodeId, Node.Transform});
			}
			else if (Node.ContentKey == TEXT("building.wall"))
			{
				Result.VillageWalls.Add({Node.NodeId, Node.Transform});
			}
			else if (Node.ContentKey == TEXT("building.door"))
			{
				Result.VillageDoors.Add({Node.NodeId, Node.Transform});
				AddDoorFrame(Node.NodeId, Node.Transform, Result.VillageWalls);
			}
			else if (Node.ContentKey == TEXT("building.roof"))
			{
				Result.VillageRoofs.Add({Node.NodeId, Node.Transform});
			}
			continue;
		}
		if (Node.Kind == EGatersRecipeNodeKind::SettlementPath)
		{
			// Path identity remains semantic until a terrain-aware presentation adapter exists.
			continue;
		}
		const bool bScatter = Node.Kind == EGatersRecipeNodeKind::ScatterTree ||
			Node.Kind == EGatersRecipeNodeKind::ScatterRock;
		const bool bClaim = Node.Kind == EGatersRecipeNodeKind::BuildPlot;
		if (!bScatter && !bClaim)
		{
			continue;
		}

		if (Node.NodeId.IsEmpty())
		{
			continue;
		}
		const FString& StableId = Node.NodeId;
		if (bScatter && DiffEntries.Contains(TEXT("chop:") + StableId))
		{
			continue;
		}
		if (bScatter && Node.Representation != EGatersCompiledRepresentation::InstancedStatic &&
			Node.Representation != EGatersCompiledRepresentation::Placeholder)
		{
			continue;
		}

		FTransform Transform = Node.Transform;
		if (bScatter)
		{
			const bool bTree = Node.Kind == EGatersRecipeNodeKind::ScatterTree;
			TSoftObjectPtr<UStaticMesh>& SelectedMesh = bTree ? Result.TreeMesh : Result.RockMesh;
			if (SelectedMesh.IsNull() && !Node.Mesh.IsNull())
			{
				SelectedMesh = Node.Mesh;
			}
			(bTree ? Result.Trees : Result.Rocks).Add({StableId, Transform});
			continue;
		}

		Transform.AddToTranslation(FVector(0, 0, 20.f));
		Transform.SetScale3D(FVector(1.5f, 1.5f, 0.15f));
		const bool bClaimed = DiffEntries.Contains(TEXT("claim:") + StableId);
		(bClaimed ? Result.ClaimedClaims : Result.OpenClaims).Add({StableId, Transform});
	}
	return Result;
}

bool FGatersVisualMaterializer::Materialize(
	AActor& Owner,
	USceneComponent& Parent,
	const FGatersVisualBatchPlan& Plan,
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>& InOutComponents,
	TArray<FString>& OutErrors)
{
	struct FBatch
	{
		const TCHAR* Name;
		const TCHAR* MeshPath;
		const TCHAR* MaterialPath;
		const TArray<FGatersVisualInstance>* Instances;
		const TSoftObjectPtr<UStaticMesh>* SelectedMesh;
	};
	const FBatch Batches[] = {
		{TEXT("GatersTrees"), TEXT("/Engine/BasicShapes/Cone.Cone"),
			TEXT("/Game/Gaters/Materials/MI_ScatterTree.MI_ScatterTree"), &Plan.Trees, &Plan.TreeMesh},
		{TEXT("GatersRocks"), TEXT("/Engine/BasicShapes/Sphere.Sphere"),
			TEXT("/Game/Gaters/Materials/MI_ScatterRock.MI_ScatterRock"), &Plan.Rocks, &Plan.RockMesh},
		{TEXT("GatersOpenClaims"), TEXT("/Engine/BasicShapes/Cube.Cube"),
			TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"), &Plan.OpenClaims, nullptr},
		{TEXT("GatersClaimedClaims"), TEXT("/Engine/BasicShapes/Cube.Cube"),
			TEXT("/Game/Gaters/Materials/MI_Claimed.MI_Claimed"), &Plan.ClaimedClaims, nullptr},
		{TEXT("GatersVillageFoundations"), TEXT("/Engine/BasicShapes/Cube.Cube"),
			TEXT("/Game/Gaters/Materials/MI_ScatterRock.MI_ScatterRock"), &Plan.VillageFoundations, nullptr},
		{TEXT("GatersVillageWalls"), TEXT("/Engine/BasicShapes/Cube.Cube"),
			TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"), &Plan.VillageWalls, nullptr},
		{TEXT("GatersVillageRoofs"), TEXT("/Engine/BasicShapes/Cube.Cube"),
			TEXT("/Game/Gaters/Materials/MI_Claimed.MI_Claimed"), &Plan.VillageRoofs, nullptr},
		{TEXT("GatersVillageSpaces"), TEXT("/Engine/BasicShapes/Cylinder.Cylinder"),
			TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"), &Plan.VillageSpaces, nullptr}
	};

	while (InOutComponents.Num() < UE_ARRAY_COUNT(Batches))
	{
		InOutComponents.Add(nullptr);
	}
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(Batches); ++Index)
	{
		const FBatch& Batch = Batches[Index];
		UInstancedStaticMeshComponent* Component = InOutComponents[Index];
		const bool bCatalogMesh = Batch.SelectedMesh && !Batch.SelectedMesh->IsNull();
		UStaticMesh* Mesh = bCatalogMesh
			? Batch.SelectedMesh->LoadSynchronous()
			: LoadObject<UStaticMesh>(nullptr, Batch.MeshPath);
		UMaterialInterface* Material = bCatalogMesh
			? nullptr
			: LoadObject<UMaterialInterface>(nullptr, Batch.MaterialPath);
		if (!Mesh || (!bCatalogMesh && !Material))
		{
			if (Component)
			{
				Component->ClearInstances();
				Component->SetVisibility(false);
			}
			OutErrors.Add(FString::Printf(TEXT("batch=%s missing=%s%s"), Batch.Name,
				Mesh ? TEXT("") : TEXT("mesh"),
				(bCatalogMesh || Material) ? TEXT("") : TEXT(" material")));
			continue;
		}

		if (!Component)
		{
			Component = NewObject<UInstancedStaticMeshComponent>(&Owner, Batch.Name);
			Component->AttachToComponent(&Parent, FAttachmentTransformRules::KeepRelativeTransform);
			Component->SetMobility(Parent.Mobility);
			Owner.AddInstanceComponent(Component);
			if (Owner.GetWorld())
			{
				Component->RegisterComponent();
			}
			InOutComponents[Index] = Component;
		}
		Component->SetVisibility(true);
		Component->SetStaticMesh(Mesh);
		Component->SetMaterial(0, Material);
		const bool bInteractive = Index < 3;
		Component->SetCollisionEnabled(
			bInteractive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
		Component->SetCollisionResponseToAllChannels(
			bInteractive ? ECollisionResponse::ECR_Overlap : ECollisionResponse::ECR_Ignore);
		Component->SetGenerateOverlapEvents(bInteractive);
		Component->bMultiBodyOverlap = bInteractive;
		Component->ClearInstances();
		for (const FGatersVisualInstance& Instance : *Batch.Instances)
		{
			Component->AddInstance(Instance.Transform);
		}
	}
	return OutErrors.IsEmpty();
}

FGatersVisualInteraction FGatersVisualMaterializer::InteractionAt(
	const FGatersVisualBatchPlan& Plan,
	const EGatersVisualBatch Batch,
	const int32 InstanceIndex)
{
	const TArray<FGatersVisualInstance>* Instances = nullptr;
	EGatersVisualInteractionKind Kind = EGatersVisualInteractionKind::None;
	switch (Batch)
	{
	case EGatersVisualBatch::Trees:
		Instances = &Plan.Trees;
		Kind = EGatersVisualInteractionKind::Chop;
		break;
	case EGatersVisualBatch::Rocks:
		Instances = &Plan.Rocks;
		Kind = EGatersVisualInteractionKind::Chop;
		break;
	case EGatersVisualBatch::OpenClaims:
		Instances = &Plan.OpenClaims;
		Kind = EGatersVisualInteractionKind::Claim;
		break;
	case EGatersVisualBatch::ClaimedClaims:
		return {};
	}
	if (!Instances || !Instances->IsValidIndex(InstanceIndex))
	{
		return {};
	}
	const FString& StableId = (*Instances)[InstanceIndex].StableId;
	return {
		Kind,
		StableId,
		(Kind == EGatersVisualInteractionKind::Chop ? TEXT("chop:") : TEXT("claim:")) +
			StableId};
}
