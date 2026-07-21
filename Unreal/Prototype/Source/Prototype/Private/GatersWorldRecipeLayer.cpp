#include "GatersWorldRecipeLayer.h"

FGatersWorldRecipeLayerComposition FGatersWorldRecipeLayerComposer::Append(
	FGatersWorldRecipe& Recipe,
	const FGatersWorldRecipeLayer& Layer)
{
	FGatersWorldRecipeLayerComposition Result;
	if (Layer.Nodes.IsEmpty() && Layer.Diagnostics.IsEmpty())
	{
		Result.bComposed = true;
		return Result;
	}
	if (!Layer.bGenerated || Layer.LayerId.IsEmpty()
		|| Layer.SchemaVersion <= 0 || Layer.GeneratorVersion <= 0)
	{
		Result.Diagnostics.Add(TEXT("layer metadata is invalid"));
	}
	Result.Diagnostics.Append(Layer.Diagnostics);

	TSet<FString> Ids;
	for (const FGatersRecipeNode& Node : Recipe.Nodes)
	{
		Ids.Add(Node.Id);
	}
	for (const FGatersRecipeNode& Node : Layer.Nodes)
	{
		if (Node.Id.IsEmpty() || Ids.Contains(Node.Id))
		{
			Result.Diagnostics.Add(FString::Printf(
				TEXT("duplicate or empty recipe node identity: %s"), *Node.Id));
		}
		Ids.Add(Node.Id);
	}
	if (!Result.Diagnostics.IsEmpty())
	{
		return Result;
	}

	Recipe.Nodes.Append(Layer.Nodes);
	Result.bComposed = true;
	Result.AppendedNodeCount = Layer.Nodes.Num();
	return Result;
}
