#pragma once

#include "CoreMinimal.h"
#include "GatersAssetContract.h"
#include "GatersWorldRecipe.h"

class FGatersContentCatalog;
class UStaticMesh;

enum class EGatersCompiledRepresentation : uint8
{
	Semantic,
	Placeholder,
	InstancedStatic,
	UniqueActor,
	SkeletalActor
};

struct PROTOTYPE_API FGatersCompiledNode
{
	bool TryGetStableIndex(const FString& ExpectedPrefix, int32& OutIndex) const;

	FString NodeId;
	EGatersRecipeNodeKind Kind = EGatersRecipeNodeKind::Gate;
	FTransform Transform = FTransform::Identity;
	FString ContentKey;
	FString AssetId;
	TOptional<FGatersAssetContract> AssetContract;
	EGatersCompiledRepresentation Representation = EGatersCompiledRepresentation::Semantic;
	TSoftObjectPtr<UStaticMesh> Mesh;
};

struct PROTOTYPE_API FGatersCompilerDiagnostic
{
	FString RuleId;
	FString NodeId;
	FString ContentKey;
	FString Message;
	bool bError = false;
};

struct PROTOTYPE_API FGatersCompiledWorld
{
	bool IsValid() const;
	bool MatchesRecipe(const FGatersWorldRecipe& Recipe) const;
	const FGatersCompiledNode* FindNode(const FString& NodeId) const;
	TArray<const FGatersCompiledNode*> FindNodes(EGatersRecipeNodeKind Kind) const;

	uint32 RecipeChecksum = 0;
	TArray<FGatersCompiledNode> Nodes;
	TArray<FGatersRecipeLink> Links;
	TArray<FGatersCompilerDiagnostic> Diagnostics;
};

struct PROTOTYPE_API FGatersWorldCompiler
{
	static FGatersCompiledWorld Compile(
		const FGatersWorldRecipe& Recipe,
		const FGatersContentCatalog& Catalog,
		const FString& StyleId);
};
