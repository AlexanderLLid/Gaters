#include "GatersVisualMaterializer.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"

int32 FGatersVisualBatchPlan::NumInstances() const
{
	return Trees.Num() + Rocks.Num() + OpenClaims.Num() + ClaimedClaims.Num();
}

FGatersVisualBatchPlan FGatersVisualMaterializer::Plan(
	const FGatersCompiledWorld& World,
	const TArray<FString>& DiffEntries)
{
	FGatersVisualBatchPlan Result;
	for (const FGatersCompiledNode& Node : World.Nodes)
	{
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
			TEXT("/Game/Gaters/Materials/MI_Claimed.MI_Claimed"), &Plan.ClaimedClaims, nullptr}
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
