#include "GatersWorldCompiler.h"

#include "GatersContentCatalog.h"

bool FGatersCompiledNode::TryGetStableIndex(
	const FString& ExpectedPrefix, int32& OutIndex) const
{
	FString Prefix;
	FString IndexText;
	if (!NodeId.Split(TEXT(":"), &Prefix, &IndexText) || Prefix != ExpectedPrefix || IndexText.IsEmpty())
	{
		return false;
	}
	for (const TCHAR Character : IndexText)
	{
		if (!FChar::IsDigit(Character))
		{
			return false;
		}
	}
	OutIndex = FCString::Atoi(*IndexText);
	return true;
}

bool FGatersCompiledWorld::IsValid() const
{
	return !Diagnostics.ContainsByPredicate([](const FGatersCompilerDiagnostic& Diagnostic)
	{
		return Diagnostic.bError;
	});
}

bool FGatersCompiledWorld::MatchesRecipe(const FGatersWorldRecipe& Recipe) const
{
	return RecipeChecksum == Recipe.Checksum() &&
		Nodes.Num() == Recipe.Nodes.Num() && Links.Num() == Recipe.Links.Num();
}

const FGatersCompiledNode* FGatersCompiledWorld::FindNode(const FString& NodeId) const
{
	return Nodes.FindByPredicate([&NodeId](const FGatersCompiledNode& Node)
	{
		return Node.NodeId == NodeId;
	});
}

TArray<const FGatersCompiledNode*> FGatersCompiledWorld::FindNodes(
	const EGatersRecipeNodeKind Kind) const
{
	TArray<const FGatersCompiledNode*> Result;
	for (const FGatersCompiledNode& Node : Nodes)
	{
		if (Node.Kind == Kind)
		{
			Result.Add(&Node);
		}
	}
	return Result;
}

FGatersCompiledWorld FGatersWorldCompiler::Compile(
	const FGatersWorldRecipe& Recipe,
	const FGatersContentCatalog& Catalog,
	const FString& StyleId)
{
	FGatersCompiledWorld World;
	World.RecipeChecksum = Recipe.Checksum();
	World.Links = Recipe.Links;
	World.Nodes.Reserve(Recipe.Nodes.Num());

	for (const FGatersRecipeNode& Source : Recipe.Nodes)
	{
		FGatersCompiledNode& Node = World.Nodes.AddDefaulted_GetRef();
		Node.NodeId = Source.Id;
		Node.Kind = Source.Kind;
		Node.Transform = FTransform(Source.Rotation, Source.Location, Source.Scale);
		Node.ContentKey = Source.ContentKey;

		const bool bRequiredAsset = Source.Kind == EGatersRecipeNodeKind::BasePiece;
		const bool bOptionalAsset =
			Source.Kind == EGatersRecipeNodeKind::ScatterTree ||
			Source.Kind == EGatersRecipeNodeKind::ScatterRock;
		if (!bRequiredAsset && !bOptionalAsset)
		{
			Node.Representation = EGatersCompiledRepresentation::Semantic;
			continue;
		}

		const TOptional<FGatersCatalogAsset> Asset = Catalog.Find(Source.ContentKey, StyleId);
		if (!Asset)
		{
			Node.Representation = EGatersCompiledRepresentation::Placeholder;
			World.Diagnostics.Add({
				bRequiredAsset ? TEXT("compiler.required_content") : TEXT("compiler.optional_placeholder"),
				Source.Id,
				Source.ContentKey,
				bRequiredAsset
					? TEXT("required recipe content has no compatible catalog asset")
					: TEXT("optional recipe content will use a placeholder"),
				bRequiredAsset});
			continue;
		}

		TArray<FString> ContractErrors;
		if (!Asset->Contract.Validate(ContractErrors))
		{
			Node.Representation = EGatersCompiledRepresentation::Placeholder;
			World.Diagnostics.Add({
				TEXT("compiler.invalid_contract"), Source.Id, Source.ContentKey,
				TEXT("selected catalog asset has an invalid contract"), true});
			continue;
		}
		Node.AssetId = Asset->Contract.AssetId;
		Node.AssetContract = Asset->Contract;
		Node.Mesh = Asset->Mesh;
		const bool bScatter = Source.Kind == EGatersRecipeNodeKind::ScatterTree ||
			Source.Kind == EGatersRecipeNodeKind::ScatterRock;
		if (bScatter)
		{
			const FGatersAssetContact* TerrainContact = Asset->Contract.Contacts.FindByPredicate(
				[](const FGatersAssetContact& Contact)
				{
					return Contact.Support == EGatersAssetContactSupport::Terrain;
				});
			if (TerrainContact)
			{
				const FVector ContactLocation = Node.Transform.TransformPosition(
					TerrainContact->Location * Asset->Contract.CentimetersPerUnit);
				Node.Transform.AddToTranslation(Source.Location - ContactLocation);
			}
		}
		if (Asset->Contract.bInstanceStatePersistent)
		{
			Node.Representation = EGatersCompiledRepresentation::UniqueActor;
		}
		else if (Asset->Contract.RenderClass == EGatersAssetRenderClass::InstancedStatic)
		{
			Node.Representation = EGatersCompiledRepresentation::InstancedStatic;
		}
		else if (Asset->Contract.RenderClass == EGatersAssetRenderClass::UniqueStatic)
		{
			Node.Representation = EGatersCompiledRepresentation::UniqueActor;
		}
		else
		{
			Node.Representation = EGatersCompiledRepresentation::SkeletalActor;
		}
	}
	return World;
}
