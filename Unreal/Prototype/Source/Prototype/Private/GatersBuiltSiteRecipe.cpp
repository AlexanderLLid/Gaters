#include "GatersBuiltSiteRecipe.h"

#include "Misc/Crc.h"

namespace
{
FString EncodeString(const FString& Value)
{
	return FString::Printf(TEXT("%d:%s"), Value.Len(), *Value);
}

void AppendStrings(FString& Result, const TCHAR* Label, const TArray<FString>& Values)
{
	for (const FString& Value : Values)
	{
		Result += FString::Printf(TEXT(";%s=%s"), Label, *EncodeString(Value));
	}
}

bool HasValidProvenance(const TArray<FString>& SourceIds)
{
	return !SourceIds.IsEmpty() && !SourceIds.ContainsByPredicate([](const FString& Id)
	{
		return Id.IsEmpty();
	});
}

bool HasDuplicateOrEmptyString(const TArray<FString>& Values)
{
	TSet<FString> Seen;
	for (const FString& Value : Values)
	{
		if (Value.IsEmpty() || Seen.Contains(Value))
		{
			return true;
		}
		Seen.Add(Value);
	}
	return false;
}

bool HasInvalidComponent(const FVector& Value)
{
	return !FMath::IsFinite(Value.X) || !FMath::IsFinite(Value.Y)
		|| !FMath::IsFinite(Value.Z) || Value.X < 0.f || Value.Y < 0.f || Value.Z < 0.f;
}

bool IsFiniteVector(const FVector& Value)
{
	return FMath::IsFinite(Value.X) && FMath::IsFinite(Value.Y)
		&& FMath::IsFinite(Value.Z);
}

bool IsInvalidDimension(const float Value)
{
	return !FMath::IsFinite(Value) || Value < 0.f;
}
}

const FGatersBuiltSiteSpace* FGatersBuiltSiteRecipe::FindSpace(const FString& Id) const
{
	return Spaces.FindByPredicate([&Id](const FGatersBuiltSiteSpace& Space)
	{
		return Space.Id == Id;
	});
}

FString FGatersBuiltSiteRecipe::CanonicalText() const
{
	FString Result = FString::Printf(
		TEXT("contract=%d;site-version=%d;generator=%d;seed=%d;site=%s;kind=%d;area=%.3f"),
		ContractVersion, SiteVersion, GeneratorVersion, Seed, *EncodeString(SiteId),
		static_cast<int32>(Kind), SiteArea);
	Result += FString::Printf(TEXT(";coverage=%d,%d,%d,%d"),
		EvidenceCoverage.bPlacement, EvidenceCoverage.bTraversalClearance,
		EvidenceCoverage.bVisibility, EvidenceCoverage.bBlockers);
	AppendStrings(Result, TEXT("coverage-source"), EvidenceCoverage.SourceIds);
	for (const FGatersBuiltSiteSpace& Space : Spaces)
	{
		Result += FString::Printf(TEXT(";space=%s,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f"),
			*EncodeString(Space.Id), Space.Center.X, Space.Center.Y, Space.Center.Z,
			Space.Extent.X, Space.Extent.Y, Space.Extent.Z);
		Result += FString::Printf(TEXT(";space-role=%s"), *EncodeString(Space.SemanticRole));
		AppendStrings(Result, TEXT("space-tag"), Space.Tags);
		AppendStrings(Result, TEXT("space-source"), Space.SourceIds);
	}
	for (const FGatersBuiltSiteConnection& Connection : Connections)
	{
		Result += FString::Printf(
			TEXT(";connection=%s,%s,%s,%.3f,%.3f,%.3f,%.3f"),
			*EncodeString(Connection.Id), *EncodeString(Connection.FromSpaceId),
			*EncodeString(Connection.ToSpaceId),
			Connection.Width, Connection.Headroom, Connection.MaxStepHeight,
			Connection.MaxJumpDistance);
		AppendStrings(Result, TEXT("connection-mode"), Connection.MovementModeIds);
		AppendStrings(Result, TEXT("connection-blocker"), Connection.BlockerIds);
		AppendStrings(Result, TEXT("connection-tag"), Connection.Tags);
		AppendStrings(Result, TEXT("connection-source"), Connection.SourceIds);
	}
	for (const FGatersBuiltSiteVisibility& Sight : Visibility)
	{
		Result += FString::Printf(TEXT(";visibility=%s,%s,%s,%.3f,%.3f,%.3f"),
			*EncodeString(Sight.Id), *EncodeString(Sight.FromSpaceId),
			*EncodeString(Sight.ToSpaceId), Sight.Distance,
			Sight.FromHeight, Sight.ToHeight);
		AppendStrings(Result, TEXT("visibility-blocker"), Sight.BlockerIds);
		AppendStrings(Result, TEXT("visibility-tag"), Sight.Tags);
		AppendStrings(Result, TEXT("visibility-source"), Sight.SourceIds);
	}
	for (const FGatersBuiltSiteBlocker& Blocker : Blockers)
	{
		Result += FString::Printf(TEXT(";blocker=%s,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f"),
			*EncodeString(Blocker.Id), Blocker.Center.X, Blocker.Center.Y, Blocker.Center.Z,
			Blocker.Extent.X, Blocker.Extent.Y, Blocker.Extent.Z);
		AppendStrings(Result, TEXT("blocker-tag"), Blocker.Tags);
		AppendStrings(Result, TEXT("blocker-source"), Blocker.SourceIds);
	}
	for (const FGatersBuiltSitePlacementSlot& Slot : PlacementSlots)
	{
		Result += FString::Printf(TEXT(";slot=%s,%s,%.3f,%.3f,%.3f,%.3f,%.3f"),
			*EncodeString(Slot.Id), *EncodeString(Slot.SpaceId),
			Slot.Location.X, Slot.Location.Y, Slot.Location.Z,
			Slot.ClearanceRadius, Slot.ClearanceHeight);
		AppendStrings(Result, TEXT("slot-tag"), Slot.Tags);
		AppendStrings(Result, TEXT("slot-source"), Slot.SourceIds);
	}
	return Result;
}

uint32 FGatersBuiltSiteRecipe::Checksum() const
{
	return FCrc::StrCrc32(*CanonicalText());
}

bool FGatersBuiltSiteRecipe::Validate(TArray<FGatersBuiltSiteIssue>& OutIssues) const
{
	OutIssues.Reset();
	auto AddIssue = [&OutIssues](const TCHAR* RuleId, const FString& SubjectId,
		const TCHAR* Message)
	{
		OutIssues.Add({RuleId, SubjectId, Message});
	};
	if (ContractVersion < 1 || SiteVersion < 1 || GeneratorVersion < 1 || SiteId.IsEmpty())
	{
		AddIssue(TEXT("site.identity"), SiteId, TEXT("site identity or version is invalid"));
	}
	if (IsInvalidDimension(SiteArea))
	{
		AddIssue(TEXT("site.dimensions"), SiteId, TEXT("site area is invalid"));
	}
	const bool bHasMeasuredCoverage = EvidenceCoverage.bPlacement
		|| EvidenceCoverage.bTraversalClearance || EvidenceCoverage.bVisibility
		|| EvidenceCoverage.bBlockers;
	if (bHasMeasuredCoverage && !HasValidProvenance(EvidenceCoverage.SourceIds))
	{
		AddIssue(TEXT("site.evidence.provenance"), SiteId,
			TEXT("measured evidence coverage has no stable source id"));
	}

	TSet<FString> ElementIds;
	auto RegisterElement = [&](const FString& Id, const TArray<FString>& SourceIds)
	{
		if (Id.IsEmpty())
		{
			AddIssue(TEXT("site.identity"), SiteId, TEXT("site element id is empty"));
		}
		else if (ElementIds.Contains(Id))
		{
			AddIssue(TEXT("site.duplicate-id"), Id, TEXT("site element id is duplicated"));
		}
		else
		{
			ElementIds.Add(Id);
		}
		if (!HasValidProvenance(SourceIds))
		{
			AddIssue(TEXT("site.provenance"), Id, TEXT("site element has no stable source id"));
		}
	};

	TSet<FString> SpaceIds;
	for (const FGatersBuiltSiteSpace& Space : Spaces)
	{
		RegisterElement(Space.Id, Space.SourceIds);
		SpaceIds.Add(Space.Id);
		if (!IsFiniteVector(Space.Center) || HasInvalidComponent(Space.Extent))
		{
			AddIssue(TEXT("site.dimensions"), Space.Id, TEXT("space dimensions are invalid"));
		}
	}

	TSet<FString> BlockerIds;
	for (const FGatersBuiltSiteBlocker& Blocker : Blockers)
	{
		RegisterElement(Blocker.Id, Blocker.SourceIds);
		BlockerIds.Add(Blocker.Id);
		if (!IsFiniteVector(Blocker.Center) || HasInvalidComponent(Blocker.Extent)
			|| Blocker.Extent.X == 0.f || Blocker.Extent.Y == 0.f || Blocker.Extent.Z == 0.f)
		{
			AddIssue(TEXT("site.dimensions"), Blocker.Id, TEXT("blocker dimensions are invalid"));
		}
	}

	auto ValidateReferences = [&](const FString& Id, const FString& FromSpaceId,
		const FString& ToSpaceId, const TArray<FString>& ReferencedBlockers)
	{
		if (!SpaceIds.Contains(FromSpaceId) || !SpaceIds.Contains(ToSpaceId))
		{
			AddIssue(TEXT("site.reference"), Id, TEXT("site relation references a missing space"));
		}
		for (const FString& BlockerId : ReferencedBlockers)
		{
			if (!BlockerIds.Contains(BlockerId))
			{
				AddIssue(TEXT("site.reference"), Id, TEXT("site relation references a missing blocker"));
			}
		}
	};

	for (const FGatersBuiltSiteConnection& Connection : Connections)
	{
		RegisterElement(Connection.Id, Connection.SourceIds);
		ValidateReferences(Connection.Id, Connection.FromSpaceId, Connection.ToSpaceId,
			Connection.BlockerIds);
		if (Connection.MovementModeIds.IsEmpty()
			|| HasDuplicateOrEmptyString(Connection.MovementModeIds))
		{
			AddIssue(TEXT("site.movement-mode"), Connection.Id,
				TEXT("connection movement modes are empty or duplicated"));
		}
		if (IsInvalidDimension(Connection.Width) || IsInvalidDimension(Connection.Headroom)
			|| IsInvalidDimension(Connection.MaxStepHeight)
			|| IsInvalidDimension(Connection.MaxJumpDistance))
		{
			AddIssue(TEXT("site.dimensions"), Connection.Id,
				TEXT("connection traversal dimension is negative"));
		}
		if (EvidenceCoverage.bTraversalClearance
			&& (Connection.Width <= 0.f || Connection.Headroom <= 0.f))
		{
			AddIssue(TEXT("site.evidence.traversal"), Connection.Id,
				TEXT("covered traversal requires positive width and headroom"));
		}
	}
	for (const FGatersBuiltSiteVisibility& Sight : Visibility)
	{
		RegisterElement(Sight.Id, Sight.SourceIds);
		ValidateReferences(Sight.Id, Sight.FromSpaceId, Sight.ToSpaceId, Sight.BlockerIds);
		if (IsInvalidDimension(Sight.Distance) || IsInvalidDimension(Sight.FromHeight)
			|| IsInvalidDimension(Sight.ToHeight))
		{
			AddIssue(TEXT("site.dimensions"), Sight.Id,
				TEXT("visibility dimension is negative"));
		}
	}
	for (const FGatersBuiltSitePlacementSlot& Slot : PlacementSlots)
	{
		RegisterElement(Slot.Id, Slot.SourceIds);
		const FGatersBuiltSiteSpace* Space = FindSpace(Slot.SpaceId);
		if (!Space)
		{
			AddIssue(TEXT("site.reference"), Slot.Id, TEXT("slot references a missing space"));
		}
		if (!IsFiniteVector(Slot.Location) || !FMath::IsFinite(Slot.ClearanceRadius)
			|| !FMath::IsFinite(Slot.ClearanceHeight) || Slot.ClearanceRadius <= 0.f
			|| Slot.ClearanceHeight <= 0.f)
		{
			AddIssue(TEXT("site.dimensions"), Slot.Id, TEXT("slot clearance is invalid"));
		}
		else if (Space)
		{
			const FVector Delta = (Slot.Location - Space->Center).GetAbs();
			if (Delta.X + Slot.ClearanceRadius > Space->Extent.X
				|| Delta.Y + Slot.ClearanceRadius > Space->Extent.Y
				|| Delta.Z + Slot.ClearanceHeight * 0.5f > Space->Extent.Z)
			{
				AddIssue(TEXT("site.containment"), Slot.Id,
					TEXT("slot clearance exceeds its containing space"));
			}
		}
	}
	return OutIssues.IsEmpty();
}
