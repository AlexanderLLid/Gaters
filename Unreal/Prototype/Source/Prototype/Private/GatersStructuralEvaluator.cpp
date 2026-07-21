#include "GatersStructuralEvaluator.h"

#include "GatersWorldRecipe.h"

namespace
{
void AddIssue(
	FGatersStructuralEvaluation& Evaluation,
	const TCHAR* RuleId,
	FString Message,
	const FString& NodeId = FString())
{
	FGatersStructuralIssue& Issue = Evaluation.Issues.AddDefaulted_GetRef();
	Issue.RuleId = RuleId;
	Issue.Message = MoveTemp(Message);
	if (!NodeId.IsEmpty())
	{
		Issue.NodeIds.Add(NodeId);
	}
}

FGatersStructuralIssue& AddLinkIssue(
	FGatersStructuralEvaluation& Evaluation,
	const TCHAR* RuleId,
	FString Message,
	const FGatersRecipeLink& Link)
{
	FGatersStructuralIssue& Issue = Evaluation.Issues.AddDefaulted_GetRef();
	Issue.RuleId = RuleId;
	Issue.Message = MoveTemp(Message);
	Issue.LinkIds.Add(Link.Id);
	if (!Link.FromNodeId.IsEmpty())
	{
		Issue.NodeIds.Add(Link.FromNodeId);
	}
	if (!Link.ToNodeId.IsEmpty() && Link.ToNodeId != Link.FromNodeId)
	{
		Issue.NodeIds.Add(Link.ToNodeId);
	}
	return Issue;
}

bool IsFinite(const FVector& Value)
{
	return FMath::IsFinite(Value.X) && FMath::IsFinite(Value.Y) && FMath::IsFinite(Value.Z);
}

bool IsFinite(const FRotator& Value)
{
	return FMath::IsFinite(Value.Pitch) && FMath::IsFinite(Value.Yaw) && FMath::IsFinite(Value.Roll);
}

struct FContractedPiece
{
	const FGatersRecipeNode* Node = nullptr;
	FBox Bounds;
};
}

FGatersStructuralEvaluation FGatersStructuralEvaluator::Evaluate(
	const FGatersWorldRecipe& Recipe,
	const FGatersStructuralLimits& Limits)
{
	FGatersStructuralContext Context;
	Context.Limits = Limits;
	Context.bValidateAssetContracts = false;
	return Evaluate(Recipe, Context);
}

FGatersStructuralEvaluation FGatersStructuralEvaluator::Evaluate(
	const FGatersWorldRecipe& Recipe,
	const FGatersStructuralContext& Context)
{
	FGatersStructuralEvaluation Evaluation;
	if (Recipe.SchemaVersion <= 0)
	{
		AddIssue(Evaluation, TEXT("recipe.schema_version"), TEXT("schema version must be positive"));
	}
	if (Recipe.GeneratorVersion <= 0)
	{
		AddIssue(Evaluation, TEXT("recipe.generator_version"), TEXT("generator version must be positive"));
	}
	if (Recipe.Nodes.Num() > Context.Limits.MaxNodes)
	{
		FGatersStructuralIssue& Issue = Evaluation.Issues.AddDefaulted_GetRef();
		Issue.RuleId = TEXT("recipe.node_budget");
		Issue.Message = FString::Printf(
			TEXT("recipe node count %d exceeds limit %d"), Recipe.Nodes.Num(), Context.Limits.MaxNodes);
		Issue.bHasMeasurement = true;
		Issue.Measured = Recipe.Nodes.Num();
		Issue.Limit = Context.Limits.MaxNodes;
	}

	TSet<FString> Ids;
	int32 ArrivalCount = 0;
	int32 BaseCount = 0;
	const FGatersRecipeNode* Arrival = nullptr;
	const FGatersRecipeNode* Base = nullptr;
	for (const FGatersRecipeNode& Node : Recipe.Nodes)
	{
		if (Node.Id.IsEmpty())
		{
			AddIssue(Evaluation, TEXT("node.id.required"), TEXT("node id must not be empty"));
		}
		else if (Ids.Contains(Node.Id))
		{
			AddIssue(Evaluation, TEXT("node.id.unique"),
				FString::Printf(TEXT("duplicate node id %s"), *Node.Id), Node.Id);
		}
		Ids.Add(Node.Id);

		if (!IsFinite(Node.Location))
		{
			AddIssue(Evaluation, TEXT("node.transform.location_finite"),
				FString::Printf(TEXT("node %s has non-finite location"), *Node.Id), Node.Id);
		}
		if (!IsFinite(Node.Rotation))
		{
			AddIssue(Evaluation, TEXT("node.transform.rotation_finite"),
				FString::Printf(TEXT("node %s has non-finite rotation"), *Node.Id), Node.Id);
		}
		if (!IsFinite(Node.Scale))
		{
			AddIssue(Evaluation, TEXT("node.transform.scale_finite"),
				FString::Printf(TEXT("node %s has non-finite scale"), *Node.Id), Node.Id);
		}
		else if (Node.Scale.X <= 0 || Node.Scale.Y <= 0 || Node.Scale.Z <= 0)
		{
			AddIssue(Evaluation, TEXT("node.transform.scale_positive"),
				FString::Printf(TEXT("node %s requires positive scale"), *Node.Id), Node.Id);
		}

		if ((Node.Kind == EGatersRecipeNodeKind::BasePiece ||
			Node.Kind == EGatersRecipeNodeKind::RouteWaypoint ||
			Node.Kind == EGatersRecipeNodeKind::SettlementModule ||
			Node.Kind == EGatersRecipeNodeKind::SettlementParcel ||
			Node.Kind == EGatersRecipeNodeKind::SettlementGrowthFront) && Node.ContentKey.IsEmpty())
		{
			const TCHAR* KindName = Node.Kind == EGatersRecipeNodeKind::BasePiece
				? TEXT("BasePiece")
				: Node.Kind == EGatersRecipeNodeKind::RouteWaypoint
					? TEXT("RouteWaypoint")
					: Node.Kind == EGatersRecipeNodeKind::SettlementModule
						? TEXT("SettlementModule")
						: Node.Kind == EGatersRecipeNodeKind::SettlementParcel
							? TEXT("SettlementParcel") : TEXT("SettlementGrowthFront");
			AddIssue(Evaluation, TEXT("node.content.required"),
				FString::Printf(TEXT("%s node %s requires ContentKey"), KindName, *Node.Id), Node.Id);
		}

		if (Node.Kind == EGatersRecipeNodeKind::Arrival)
		{
			++ArrivalCount;
			Arrival = &Node;
		}
		else if (Node.Kind == EGatersRecipeNodeKind::BaseSite)
		{
			++BaseCount;
			Base = &Node;
		}
	}

	TArray<FContractedPiece> ContractedPieces;
	if (Context.bValidateAssetContracts)
	{
		for (const FGatersRecipeNode& Node : Recipe.Nodes)
		{
			if (Node.Kind != EGatersRecipeNodeKind::BasePiece)
			{
				continue;
			}
			const FGatersAssetContract* Contract = Context.AssetContracts.Find(Node.ContentKey);
			if (!Contract)
			{
				AddIssue(Evaluation, TEXT("node.contract.missing"), FString::Printf(
					TEXT("BasePiece node %s cannot resolve asset contract %s"),
					*Node.Id, *Node.ContentKey), Node.Id);
				continue;
			}
			TArray<FString> ContractErrors;
			if (!Contract->Validate(ContractErrors))
			{
				AddIssue(Evaluation, TEXT("node.contract.invalid"), FString::Printf(
					TEXT("BasePiece node %s references an invalid asset contract"), *Node.Id), Node.Id);
				continue;
			}

			const FVector Center = Contract->BoundsCenter * Contract->CentimetersPerUnit;
			const FVector Extent = Contract->BoundsExtent * Contract->CentimetersPerUnit;
			const FBox LocalBounds(Center - Extent, Center + Extent);
			const FTransform NodeTransform(Node.Rotation, Node.Location, Node.Scale);
			ContractedPieces.Add({&Node, LocalBounds.TransformBy(NodeTransform)});
		}
	}

	TSet<FString> LinkIds;
	for (const FGatersRecipeLink& Link : Recipe.Links)
	{
		if (Link.Id.IsEmpty())
		{
			AddLinkIssue(Evaluation, TEXT("link.id.required"),
				TEXT("link id must not be empty"), Link);
		}
		else if (LinkIds.Contains(Link.Id))
		{
			AddLinkIssue(Evaluation, TEXT("link.id.unique"),
				FString::Printf(TEXT("duplicate link id %s"), *Link.Id), Link);
		}
		LinkIds.Add(Link.Id);

		const FGatersRecipeNode* FromNode = Recipe.FindNode(Link.FromNodeId);
		const FGatersRecipeNode* ToNode = Recipe.FindNode(Link.ToNodeId);
		if (!FromNode || !ToNode)
		{
			AddLinkIssue(Evaluation, TEXT("link.node.missing"), FString::Printf(
				TEXT("link %s references a missing node"), *Link.Id), Link);
			continue;
		}
		if (FromNode == ToNode)
		{
			AddLinkIssue(Evaluation, TEXT("link.node.distinct"), FString::Printf(
				TEXT("link %s must connect distinct nodes"), *Link.Id), Link);
		}
		if (Link.FromPort.IsEmpty() || Link.ToPort.IsEmpty())
		{
			AddLinkIssue(Evaluation, TEXT("link.port.required"), FString::Printf(
				TEXT("link %s requires both port names"), *Link.Id), Link);
			continue;
		}
		if (!Context.bValidateAssetContracts)
		{
			continue;
		}

		const FGatersAssetContract* FromContract = Context.AssetContracts.Find(FromNode->ContentKey);
		const FGatersAssetContract* ToContract = Context.AssetContracts.Find(ToNode->ContentKey);
		if (!FromContract || !ToContract)
		{
			AddLinkIssue(Evaluation, TEXT("link.contract.missing"), FString::Printf(
				TEXT("link %s cannot resolve both asset contracts"), *Link.Id), Link);
			continue;
		}
		TArray<FString> ContractErrors;
		if (!FromContract->Validate(ContractErrors) || !ToContract->Validate(ContractErrors))
		{
			AddLinkIssue(Evaluation, TEXT("link.contract.invalid"), FString::Printf(
				TEXT("link %s references an invalid asset contract"), *Link.Id), Link);
			continue;
		}

		const FGatersAssetPort* FromPort = FromContract->Ports.FindByPredicate(
			[&Link](const FGatersAssetPort& Port) { return Port.Name == Link.FromPort; });
		const FGatersAssetPort* ToPort = ToContract->Ports.FindByPredicate(
			[&Link](const FGatersAssetPort& Port) { return Port.Name == Link.ToPort; });
		if (!FromPort || !ToPort)
		{
			AddLinkIssue(Evaluation, TEXT("link.port.missing"), FString::Printf(
				TEXT("link %s references a missing contract port"), *Link.Id), Link);
			continue;
		}

		const FTransform FromTransform(FromNode->Rotation, FromNode->Location, FromNode->Scale);
		const FTransform ToTransform(ToNode->Rotation, ToNode->Location, ToNode->Scale);
		const FVector FromLocation = FromTransform.TransformPosition(
			FromPort->Transform.GetLocation() * FromContract->CentimetersPerUnit);
		const FVector ToLocation = ToTransform.TransformPosition(
			ToPort->Transform.GetLocation() * ToContract->CentimetersPerUnit);
		const double Distance = FVector::Distance(FromLocation, ToLocation);
		if (Distance > Context.PortToleranceCm)
		{
			FGatersStructuralIssue& Issue = AddLinkIssue(
				Evaluation, TEXT("link.port.alignment"), FString::Printf(
					TEXT("link %s port distance %.3f exceeds tolerance %.3f"),
					*Link.Id, Distance, Context.PortToleranceCm), Link);
			Issue.bHasMeasurement = true;
			Issue.Measured = Distance;
			Issue.Limit = Context.PortToleranceCm;
		}
	}

	if (ContractedPieces.Num() > 1)
	{
		TSet<FString> PieceIds;
		for (const FContractedPiece& Piece : ContractedPieces)
		{
			PieceIds.Add(Piece.Node->Id);
		}
		TSet<FString> Reached;
		Reached.Add(ContractedPieces[0].Node->Id);
		bool bAdded = true;
		while (bAdded)
		{
			bAdded = false;
			for (const FGatersRecipeLink& Link : Recipe.Links)
			{
				if (!PieceIds.Contains(Link.FromNodeId) || !PieceIds.Contains(Link.ToNodeId))
				{
					continue;
				}
				if (Reached.Contains(Link.FromNodeId) && !Reached.Contains(Link.ToNodeId))
				{
					Reached.Add(Link.ToNodeId);
					bAdded = true;
				}
				else if (Reached.Contains(Link.ToNodeId) && !Reached.Contains(Link.FromNodeId))
				{
					Reached.Add(Link.FromNodeId);
					bAdded = true;
				}
			}
		}
		if (Reached.Num() != ContractedPieces.Num())
		{
			FGatersStructuralIssue& Issue = Evaluation.Issues.AddDefaulted_GetRef();
			Issue.RuleId = TEXT("recipe.piece_connectivity");
			Issue.Message = FString::Printf(
				TEXT("contracted BasePieces span %d of %d reachable nodes"),
				Reached.Num(), ContractedPieces.Num());
			Issue.bHasMeasurement = true;
			Issue.Measured = Reached.Num();
			Issue.Limit = ContractedPieces.Num();
			for (const FContractedPiece& Piece : ContractedPieces)
			{
				if (!Reached.Contains(Piece.Node->Id))
				{
					Issue.NodeIds.Add(Piece.Node->Id);
				}
			}
		}
	}

	for (int32 A = 0; A < ContractedPieces.Num(); ++A)
	{
		for (int32 B = A + 1; B < ContractedPieces.Num(); ++B)
		{
			const FBox& BoxA = ContractedPieces[A].Bounds;
			const FBox& BoxB = ContractedPieces[B].Bounds;
			const double PenetrationX = FMath::Min(BoxA.Max.X, BoxB.Max.X) - FMath::Max(BoxA.Min.X, BoxB.Min.X);
			const double PenetrationY = FMath::Min(BoxA.Max.Y, BoxB.Max.Y) - FMath::Max(BoxA.Min.Y, BoxB.Min.Y);
			const double PenetrationZ = FMath::Min(BoxA.Max.Z, BoxB.Max.Z) - FMath::Max(BoxA.Min.Z, BoxB.Min.Z);
			const double MinPenetration = FMath::Min3(PenetrationX, PenetrationY, PenetrationZ);
			if (MinPenetration <= Context.OverlapToleranceCm)
			{
				continue;
			}
			FGatersStructuralIssue& Issue = Evaluation.Issues.AddDefaulted_GetRef();
			Issue.RuleId = TEXT("node.bounds.overlap");
			Issue.Message = FString::Printf(
				TEXT("nodes %s and %s overlap by %.3f cm beyond tolerance %.3f"),
				*ContractedPieces[A].Node->Id,
				*ContractedPieces[B].Node->Id,
				MinPenetration,
				Context.OverlapToleranceCm);
			Issue.NodeIds = {ContractedPieces[A].Node->Id, ContractedPieces[B].Node->Id};
			Issue.bHasMeasurement = true;
			Issue.Measured = MinPenetration;
			Issue.Limit = Context.OverlapToleranceCm;
		}
	}

	if (ArrivalCount != 1)
	{
		AddIssue(Evaluation, TEXT("recipe.arrival.cardinality"),
			TEXT("recipe must contain exactly one Arrival node"));
	}
	else if (!Arrival->Location.Equals(FVector::ZeroVector))
	{
		AddIssue(Evaluation, TEXT("recipe.arrival.location"),
			TEXT("Arrival node must be at the local origin"), Arrival->Id);
	}

	const int32 ExpectedBaseCount = Recipe.bHasBaseSite ? 1 : 0;
	if (BaseCount != ExpectedBaseCount)
	{
		AddIssue(Evaluation, TEXT("recipe.base.cardinality"), FString::Printf(
			TEXT("recipe must contain %d BaseSite node(s)"), ExpectedBaseCount));
	}
	else if (Base)
	{
		const FVector ExpectedLocation(
			Recipe.BaseSite,
			static_cast<double>(Recipe.BaseSiteHeight));
		if (!Base->Location.Equals(ExpectedLocation))
		{
			AddIssue(Evaluation, TEXT("recipe.base.location"),
				TEXT("BaseSite node does not match BaseSite"), Base->Id);
		}
	}
	return Evaluation;
}
