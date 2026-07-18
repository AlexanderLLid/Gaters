#include "GatersContentCatalog.h"

#include "GatersAssetIntake.h"
#include "Engine/StaticMesh.h"

namespace
{
void AddIssue(
	FGatersCatalogSelection& Selection,
	const TCHAR* RuleId,
	const FGatersCatalogAsset& Candidate,
	const TCHAR* Message)
{
	Selection.Issues.Add({RuleId, Candidate.Contract.AssetId, Message});
}

bool IsBetter(const FGatersCatalogAsset& Candidate, const FGatersCatalogAsset* Best)
{
	return !Best || Candidate.Contract.Version > Best->Contract.Version ||
		(Candidate.Contract.Version == Best->Contract.Version &&
			Candidate.Contract.AssetId.Compare(Best->Contract.AssetId) > 0);
}
}

bool FGatersContentCatalog::AddStaticMesh(
	UStaticMesh& Mesh,
	const FGatersAssetContract& Contract,
	TArray<FString>& OutErrors,
	const float BoundsToleranceCm)
{
	if (!FGatersAssetIntake::ValidateStaticMesh(Mesh, Contract, OutErrors, BoundsToleranceCm))
	{
		return false;
	}
	Assets.Add({Contract, TSoftObjectPtr<UStaticMesh>(&Mesh)});
	return true;
}

bool FGatersContentCatalog::AddPlaceholder(
	const FGatersAssetContract& Contract,
	TArray<FString>& OutErrors)
{
	if (!Contract.Validate(OutErrors))
	{
		return false;
	}
	Assets.Add({Contract, TSoftObjectPtr<UStaticMesh>()});
	return true;
}

bool FGatersContentCatalog::Withdraw(const FString& AssetId)
{
	return Assets.RemoveAll([&AssetId](const FGatersCatalogAsset& Asset)
	{
		return Asset.Contract.AssetId == AssetId;
	}) > 0;
}

TOptional<FGatersCatalogAsset> FGatersContentCatalog::Find(
	const FString& ContentKey,
	const FString& StyleId) const
{
	FGatersCatalogQuery Query;
	Query.ContentKey = ContentKey;
	Query.StyleId = StyleId;
	return Select(Query).Asset;
}

FGatersCatalogSelection FGatersContentCatalog::Select(const FGatersCatalogQuery& Query) const
{
	FGatersCatalogSelection Selection;
	const FGatersCatalogAsset* Best = nullptr;
	for (const FGatersCatalogAsset& Candidate : Assets)
	{
		if (Candidate.Contract.ContentKey != Query.ContentKey ||
			Candidate.Contract.StyleId != Query.StyleId)
		{
			continue;
		}

		bool bCompatible = true;
		if (Query.Collision.IsSet() && Candidate.Contract.Collision != Query.Collision.GetValue())
		{
			AddIssue(Selection, TEXT("catalog.collision"), Candidate,
				TEXT("candidate collision policy does not satisfy query"));
			bCompatible = false;
		}
		if (Query.RenderClass.IsSet() && Candidate.Contract.RenderClass != Query.RenderClass.GetValue())
		{
			AddIssue(Selection, TEXT("catalog.render_class"), Candidate,
				TEXT("candidate render class does not satisfy query"));
			bCompatible = false;
		}
		if (!Query.MaxBoundsExtent.IsZero() &&
			(Candidate.Contract.BoundsExtent.X > Query.MaxBoundsExtent.X ||
			 Candidate.Contract.BoundsExtent.Y > Query.MaxBoundsExtent.Y ||
			 Candidate.Contract.BoundsExtent.Z > Query.MaxBoundsExtent.Z))
		{
			AddIssue(Selection, TEXT("catalog.bounds"), Candidate,
				TEXT("candidate bounds exceed query maximum"));
			bCompatible = false;
		}
		for (const FString& RequiredPort : Query.RequiredPorts)
		{
			if (!Candidate.Contract.Ports.ContainsByPredicate([&RequiredPort](const FGatersAssetPort& Port)
			{
				return Port.Name == RequiredPort;
			}))
			{
				AddIssue(Selection, TEXT("catalog.port"), Candidate,
					TEXT("candidate is missing a required port"));
				bCompatible = false;
			}
		}
		if (bCompatible && IsBetter(Candidate, Best))
		{
			Best = &Candidate;
		}
	}
	if (Best)
	{
		Selection.Asset = *Best;
	}
	else if (Selection.Issues.IsEmpty())
	{
		Selection.Issues.Add({
			TEXT("catalog.no_match"), FString(), TEXT("no asset matches the semantic key and style")});
	}
	return Selection;
}
