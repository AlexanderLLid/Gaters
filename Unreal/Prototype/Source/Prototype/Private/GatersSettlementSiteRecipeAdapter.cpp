#include "GatersSettlementSiteRecipeAdapter.h"

#include "GatersBuildingEvaluator.h"
#include "GatersBuildingGenerator.h"
#include "GatersSettlementEvaluator.h"

namespace
{
FVector PathGroundLocation(const FGatersTerrainSemanticField& Field, const FIntPoint& Cell)
{
	const int32 Half = Field.CellsPerAxis / 2;
	return FVector(
		(Cell.X - Half) * Field.CellSize,
		(Cell.Y - Half) * Field.CellSize,
		Field.At(Cell.X, Cell.Y).Height);
}

const TCHAR* RoleName(const EGatersSettlementRole Role)
{
	switch (Role)
	{
	case EGatersSettlementRole::Home: return TEXT("home");
	case EGatersSettlementRole::Community: return TEXT("community");
	case EGatersSettlementRole::Workshop: return TEXT("workshop");
	case EGatersSettlementRole::Storage: return TEXT("storage");
	default: checkNoEntry(); return TEXT("unknown");
	}
}

bool SettingsAreValid(const FGatersSettlementEvidenceSettings& Settings)
{
	return Settings.Version > 0 && !Settings.MovementModeId.IsEmpty()
		&& FMath::IsFinite(Settings.PathWidth) && Settings.PathWidth > 0.f
		&& FMath::IsFinite(Settings.OutdoorHeadroom) && Settings.OutdoorHeadroom > 0.f
		&& FMath::IsFinite(Settings.MaxConnectionLength)
		&& Settings.MaxConnectionLength > 0.f
		&& FMath::IsFinite(Settings.SlotClearanceRadius)
		&& Settings.SlotClearanceRadius > 0.f
		&& FMath::IsFinite(Settings.SlotClearanceHeight)
		&& Settings.SlotClearanceHeight > 0.f
		&& Settings.PathWidth * 0.5f >= Settings.SlotClearanceRadius
		&& Settings.OutdoorHeadroom >= Settings.SlotClearanceHeight;
}

void AddSlot(
	FGatersBuiltSiteRecipe& Recipe,
	const FGatersBuiltSiteSpace& Space,
	const FGatersSettlementEvidenceSettings& Settings,
	const TArray<FString>& SourceIds)
{
	FGatersBuiltSitePlacementSlot& Slot = Recipe.PlacementSlots.AddDefaulted_GetRef();
	Slot.Id = TEXT("slot:") + Space.Id;
	Slot.SpaceId = Space.Id;
	Slot.Location = Space.Center;
	Slot.ClearanceRadius = Settings.SlotClearanceRadius;
	Slot.ClearanceHeight = Settings.SlotClearanceHeight;
	Slot.Tags = Space.Tags.Contains(TEXT("indoors"))
		? TArray<FString>{TEXT("indoors"), TEXT("ground-supported")}
		: TArray<FString>{TEXT("outdoors"), TEXT("ground-supported")};
	Slot.SourceIds = SourceIds;
}

FGatersBuiltSiteSpace& AddRouteSpace(
	FGatersBuiltSiteRecipe& Recipe,
	const FString& Id,
	const FVector& Center,
	const float Width,
	const float Headroom,
	const TArray<FString>& Tags,
	const TArray<FString>& SourceIds,
	const FGatersSettlementEvidenceSettings& Settings,
	const bool bAddSlot = true)
{
	FGatersBuiltSiteSpace& Space = Recipe.Spaces.AddDefaulted_GetRef();
	Space.Id = Id;
	Space.Center = Center;
	Space.Extent = FVector(Width * 0.5f, Width * 0.5f, Headroom * 0.5f);
	Space.Tags = Tags;
	Space.SourceIds = SourceIds;
	if (bAddSlot && Space.Extent.X >= Settings.SlotClearanceRadius
		&& Space.Extent.Y >= Settings.SlotClearanceRadius
		&& Space.Extent.Z * 2.f >= Settings.SlotClearanceHeight)
	{
		AddSlot(Recipe, Space, Settings, SourceIds);
	}
	return Space;
}

void AddDirectedConnection(
	FGatersBuiltSiteRecipe& Recipe,
	const FString& FromId,
	const FString& ToId,
	const float Width,
	const float Headroom,
	const FString& MovementModeId,
	const TArray<FString>& SourceIds)
{
	FGatersBuiltSiteConnection& Connection = Recipe.Connections.AddDefaulted_GetRef();
	Connection.Id = FString::Printf(TEXT("connection:%s:to:%s"), *FromId, *ToId);
	Connection.FromSpaceId = FromId;
	Connection.ToSpaceId = ToId;
	Connection.Width = Width;
	Connection.Headroom = Headroom;
	Connection.MovementModeIds = {MovementModeId};
	Connection.Tags = {TEXT("clearance-proved")};
	Connection.SourceIds = SourceIds;

	const FGatersBuiltSiteSpace* From = Recipe.FindSpace(FromId);
	const FGatersBuiltSiteSpace* To = Recipe.FindSpace(ToId);
	check(From && To);
	FGatersBuiltSiteVisibility& Sight = Recipe.Visibility.AddDefaulted_GetRef();
	Sight.Id = TEXT("visibility:") + Connection.Id;
	Sight.FromSpaceId = FromId;
	Sight.ToSpaceId = ToId;
	Sight.Distance = FVector::Distance(From->Center, To->Center);
	Sight.FromHeight = FMath::Min(Headroom, 160.f);
	Sight.ToHeight = Sight.FromHeight;
	Sight.Tags = {TEXT("clear")};
	Sight.SourceIds = SourceIds;
}

void AddSegmentedRoute(
	FGatersBuiltSiteRecipe& Recipe,
	const FString& FromId,
	const FString& ToId,
	const float Width,
	const float Headroom,
	const FGatersSettlementEvidenceSettings& Settings,
	const TArray<FString>& SourceIds)
{
	const FGatersBuiltSiteSpace* From = Recipe.FindSpace(FromId);
	const FGatersBuiltSiteSpace* To = Recipe.FindSpace(ToId);
	check(From && To);
	const FVector FromCenter = From->Center;
	const FVector ToCenter = To->Center;
	const int32 SegmentCount = FMath::Max(1,
		FMath::CeilToInt(FVector::Distance(FromCenter, ToCenter)
			/ Settings.MaxConnectionLength));
	TArray<FString> SpaceIds{FromId};
	for (int32 Step = 1; Step < SegmentCount; ++Step)
	{
		const FString SpaceId = FString::Printf(
			TEXT("corridor:%s:to:%s:step:%d"), *FromId, *ToId, Step);
		AddRouteSpace(Recipe, SpaceId,
			FMath::Lerp(FromCenter, ToCenter, static_cast<float>(Step) / SegmentCount),
			Width, Headroom, {TEXT("route-clearance")}, SourceIds, Settings);
		SpaceIds.Add(SpaceId);
	}
	SpaceIds.Add(ToId);
	for (int32 Index = 0; Index + 1 < SpaceIds.Num(); ++Index)
	{
		AddDirectedConnection(Recipe, SpaceIds[Index], SpaceIds[Index + 1],
			Width, Headroom, Settings.MovementModeId, SourceIds);
		AddDirectedConnection(Recipe, SpaceIds[Index + 1], SpaceIds[Index],
			Width, Headroom, Settings.MovementModeId, SourceIds);
	}
}

void AddBlocker(
	FGatersBuiltSiteRecipe& Recipe,
	const FString& Id,
	const FBox& Bounds,
	const TArray<FString>& Tags,
	const TArray<FString>& SourceIds)
{
	FGatersBuiltSiteBlocker& Blocker = Recipe.Blockers.AddDefaulted_GetRef();
	Blocker.Id = Id;
	Blocker.Center = Bounds.GetCenter();
	Blocker.Extent = Bounds.GetExtent();
	Blocker.Tags = Tags;
	Blocker.SourceIds = SourceIds;
}

void AddBuildingBlockers(
	FGatersBuiltSiteRecipe& Recipe,
	const FGatersBuildingAssembly& Assembly)
{
	for (const FGatersBuildingModule& Module : Assembly.Modules)
	{
		if (Module.Kind == EGatersBuildingModuleKind::Wall)
		{
			AddBlocker(Recipe, Module.Id + TEXT(":blocker"),
				FBox(FVector(-50.f), FVector(50.f)).TransformBy(Module.Transform),
				{TEXT("solid"), TEXT("wall")}, {Assembly.BuildingId, Module.Id});
		}
		else if (Module.Kind == EGatersBuildingModuleKind::DoorWall)
		{
			const struct FFramePart
			{
				const TCHAR* Name;
				FVector Min;
				FVector Max;
			} Parts[] = {
				{TEXT("left"), FVector(-50.f, -50.f, -50.f), FVector(50.f, -22.5f, 28.f)},
				{TEXT("right"), FVector(-50.f, 22.5f, -50.f), FVector(50.f, 50.f, 28.f)},
				{TEXT("lintel"), FVector(-50.f, -22.5f, 28.f), FVector(50.f, 22.5f, 50.f)}};
			for (const FFramePart& Part : Parts)
			{
				AddBlocker(Recipe,
					FString::Printf(TEXT("%s:blocker:frame:%s"), *Module.Id, Part.Name),
					FBox(Part.Min, Part.Max).TransformBy(Module.Transform),
					{TEXT("solid"), TEXT("door-frame")},
					{Assembly.BuildingId, Module.Id});
			}
		}
	}
}

const FGatersBuiltSiteBlocker* FindIntersectingGeneratedBlocker(
	const FGatersBuiltSiteRecipe& Recipe,
	const FGatersBuiltSiteConnection& Connection,
	const FGatersSettlementEvidenceSettings& Settings)
{
	const FGatersBuiltSiteSpace* From = Recipe.FindSpace(Connection.FromSpaceId);
	const FGatersBuiltSiteSpace* To = Recipe.FindSpace(Connection.ToSpaceId);
	check(From && To);
	const float Radius = FMath::Min(Settings.SlotClearanceRadius, Connection.Width * 0.5f);
	const float HalfHeight = FMath::Min(
		Settings.SlotClearanceHeight, Connection.Headroom) * 0.5f;
	for (int32 Sample = 0; Sample <= 4; ++Sample)
	{
		const FVector Center = FMath::Lerp(
			From->Center, To->Center, static_cast<float>(Sample) / 4.f);
		for (const FGatersBuiltSiteBlocker& Blocker : Recipe.Blockers)
		{
			const FVector Min = Blocker.Center - Blocker.Extent;
			const FVector Max = Blocker.Center + Blocker.Extent;
			const float ClosestX = FMath::Clamp(Center.X, Min.X, Max.X);
			const float ClosestY = FMath::Clamp(Center.Y, Min.Y, Max.Y);
			const float HorizontalDistanceSquared =
				FMath::Square(Center.X - ClosestX) + FMath::Square(Center.Y - ClosestY);
			const bool bVerticalOverlap = Center.Z + HalfHeight > Min.Z
				&& Center.Z - HalfHeight < Max.Z;
			if (bVerticalOverlap && HorizontalDistanceSquared < FMath::Square(Radius))
			{
				return &Blocker;
			}
		}
	}
	return nullptr;
}

void FindDisconnectedIndoorSlots(
	const FGatersBuiltSiteRecipe& Recipe,
	const FString& MovementModeId,
	TArray<const FGatersBuiltSitePlacementSlot*>& OutDisconnected)
{
	OutDisconnected.Reset();
	TSet<FString> Reachable;
	for (const FGatersBuiltSiteSpace& Space : Recipe.Spaces)
	{
		if (Space.Tags.Contains(TEXT("path-centerline")))
		{
			Reachable.Add(Space.Id);
		}
	}
	bool bChanged = true;
	while (bChanged)
	{
		bChanged = false;
		for (const FGatersBuiltSiteConnection& Connection : Recipe.Connections)
		{
			if (Connection.MovementModeIds.Contains(MovementModeId)
				&& Reachable.Contains(Connection.FromSpaceId)
				&& !Reachable.Contains(Connection.ToSpaceId))
			{
				Reachable.Add(Connection.ToSpaceId);
				bChanged = true;
			}
		}
	}
	for (const FGatersBuiltSitePlacementSlot& Slot : Recipe.PlacementSlots)
	{
		const FGatersBuiltSiteSpace* Space = Recipe.FindSpace(Slot.SpaceId);
		if (Space && Space->Tags.Contains(TEXT("indoors")) && !Reachable.Contains(Space->Id))
		{
			OutDisconnected.Add(&Slot);
		}
	}
}
}

FGatersSettlementEvidenceSettings FGatersSettlementEvidenceSettings::Ground()
{
	FGatersSettlementEvidenceSettings Result;
	Result.MovementModeId = TEXT("ground");
	Result.PathWidth = 200.f;
	Result.OutdoorHeadroom = 300.f;
	Result.MaxConnectionLength = 150.f;
	Result.SlotClearanceRadius = 45.f;
	Result.SlotClearanceHeight = 180.f;
	return Result;
}

FGatersSettlementSiteRecipeCompilation FGatersSettlementSiteRecipeAdapter::Compile(
	const FGatersTerrainSemanticField& Field,
	const int32 Seed,
	const FGatersSettlementPlan& Plan,
	const FGatersSettlementEvidenceSettings& Settings)
{
	FGatersSettlementSiteRecipeCompilation Result;
	if (!SettingsAreValid(Settings))
	{
		Result.Diagnostics.Add(TEXT("site.evidence.settings physical evidence settings are invalid"));
		return Result;
	}
	const FGatersSettlementEvaluation Settlement =
		FGatersSettlementEvaluator::Evaluate(Field, Plan);
	for (const FGatersSettlementIssue& Issue : Settlement.Issues)
	{
		Result.Diagnostics.Add(FString::Printf(
			TEXT("%s subject=%s %s"), *Issue.RuleId, *Issue.SubjectId, *Issue.Message));
	}
	if (!Settlement.IsValid())
	{
		return Result;
	}

	FGatersBuiltSiteRecipe& Recipe = Result.Recipe;
	Recipe.SiteVersion = Plan.GrowthStage + 1;
	Recipe.GeneratorVersion = Plan.GeneratorVersion;
	Recipe.Seed = Seed;
	Recipe.SiteId = Plan.SiteId;
	Recipe.Kind = EGatersBuiltSiteKind::Settlement;
	Recipe.EvidenceCoverage = {true, true, true, true,
		{Plan.SiteId, FString::Printf(TEXT("settlement-evidence-settings:%d"), Settings.Version)}};

	TSet<FIntPoint> PathCells(Plan.PathCells);
	for (const FIntPoint& Cell : Plan.PathCells)
	{
		const FString Id = FGatersSettlementPlan::StablePathId(Cell);
		const FVector Ground = PathGroundLocation(Field, Cell);
		AddRouteSpace(Recipe, Id,
			Ground + FVector(0.f, 0.f, Settings.OutdoorHeadroom * 0.5f),
			Settings.PathWidth, Settings.OutdoorHeadroom,
			{TEXT("outdoors"), TEXT("path-centerline")}, {Id}, Settings);
	}
	for (const FIntPoint& Cell : Plan.PathCells)
	{
		const FString FromId = FGatersSettlementPlan::StablePathId(Cell);
		for (const FIntPoint Offset : {FIntPoint(1, 0), FIntPoint(0, 1)})
		{
			const FIntPoint Neighbor = Cell + Offset;
			if (!PathCells.Contains(Neighbor))
			{
				continue;
			}
			const FString ToId = FGatersSettlementPlan::StablePathId(Neighbor);
			AddSegmentedRoute(Recipe, FromId, ToId, Settings.PathWidth,
				Settings.OutdoorHeadroom, Settings, {FromId, ToId});
		}
	}

	for (const FGatersSettlementBuilding& Building : Plan.Buildings)
	{
		const FGatersBuildingAssembly Assembly =
			FGatersBuildingGenerator::Generate(Field, Building);
		const FGatersBuildingEvaluation BuildingEvaluation =
			FGatersBuildingEvaluator::Evaluate(Field, Assembly);
		if (!BuildingEvaluation.IsValid())
		{
			for (const FGatersBuildingIssue& Issue : BuildingEvaluation.Issues)
			{
				Result.Diagnostics.Add(FString::Printf(
					TEXT("%s subject=%s %s"), *Issue.RuleId, *Issue.SubjectId, *Issue.Message));
			}
			continue;
		}

		AddBuildingBlockers(Recipe, Assembly);
		FString GroundSpaceId;
		for (const FGatersBuildingUsableSpace& Usable : Assembly.UsableSpaces)
		{
			FGatersBuiltSiteSpace& Space = Recipe.Spaces.AddDefaulted_GetRef();
			Space.Id = Usable.FloorIndex == 0
				? Building.Id + TEXT(":space")
				: FString::Printf(TEXT("%s:space:floor:%d"), *Building.Id, Usable.FloorIndex);
			Space.Center = Usable.Center;
			Space.Extent = Usable.Extent;
			Space.SemanticRole = RoleName(Building.Role);
			Space.Tags = {TEXT("indoors"), TEXT("roofed"), TEXT("usable-space")};
			Space.SourceIds = Usable.SourceIds;
			if (Usable.FloorIndex == 0)
			{
				GroundSpaceId = Space.Id;
				AddSlot(Recipe, Space, Settings, Space.SourceIds);
			}
		}
		if (Assembly.Openings.IsEmpty() || GroundSpaceId.IsEmpty())
		{
			Result.Diagnostics.Add(FString::Printf(
				TEXT("site.entrance subject=%s building has no usable doorway"), *Building.Id));
			continue;
		}
		const FGatersBuildingOpening& Opening = Assembly.Openings[0];
		const FString OpeningSpaceId = Building.Id + TEXT(":opening-space");
		const FString ExteriorThresholdId = Building.Id + TEXT(":threshold:exterior");
		const FString InteriorThresholdId = Building.Id + TEXT(":threshold:interior");
		const FVector Outward = Opening.Transform.GetRotation().GetForwardVector().GetSafeNormal();
		const float ThresholdOffset = Opening.Depth * 0.5f
			+ Settings.SlotClearanceRadius + 1.f;
		AddRouteSpace(Recipe, OpeningSpaceId, Opening.Transform.GetLocation(),
			Opening.Width, Opening.Headroom,
			{TEXT("doorway-clearance")}, Opening.SourceIds, Settings, false);
		AddRouteSpace(Recipe, ExteriorThresholdId,
			Opening.Transform.GetLocation() + Outward * ThresholdOffset,
			Opening.Width, Opening.Headroom,
			{TEXT("outdoors"), TEXT("doorway-threshold")},
			Opening.SourceIds, Settings, false);
		AddRouteSpace(Recipe, InteriorThresholdId,
			Opening.Transform.GetLocation() - Outward * ThresholdOffset,
			Opening.Width, Opening.Headroom,
			{TEXT("indoors"), TEXT("doorway-threshold")},
			Opening.SourceIds, Settings, false);
		const FString EntranceId = FGatersSettlementPlan::StablePathId(Building.EntranceCell);
		if (!Recipe.FindSpace(EntranceId))
		{
			Result.Diagnostics.Add(FString::Printf(
				TEXT("site.entrance subject=%s building has no path-space doorway"), *Building.Id));
			continue;
		}
		AddSegmentedRoute(Recipe, EntranceId, ExteriorThresholdId, Opening.Width,
			Opening.Headroom, Settings, Opening.SourceIds);
		AddSegmentedRoute(Recipe, ExteriorThresholdId, OpeningSpaceId, Opening.Width,
			Opening.Headroom, Settings, Opening.SourceIds);
		AddSegmentedRoute(Recipe, OpeningSpaceId, InteriorThresholdId, Opening.Width,
			Opening.Headroom, Settings, Opening.SourceIds);
		AddSegmentedRoute(Recipe, InteriorThresholdId, GroundSpaceId, Opening.Width,
			Opening.Headroom, Settings, Opening.SourceIds);
	}
	if (!Result.Diagnostics.IsEmpty())
	{
		return Result;
	}
	for (const FGatersBuiltSiteConnection& Connection : Recipe.Connections)
	{
		if (const FGatersBuiltSiteBlocker* Blocker =
			FindIntersectingGeneratedBlocker(Recipe, Connection, Settings))
		{
			Result.Diagnostics.Add(FString::Printf(
				TEXT("site.connection.blocked subject=%s blocker=%s sources=%s blocker-sources=%s declared clearance intersects generated blocker"),
				*Connection.Id, *Blocker->Id,
				*FString::Join(Connection.SourceIds, TEXT(",")),
				*FString::Join(Blocker->SourceIds, TEXT(","))));
		}
	}
	if (!Result.Diagnostics.IsEmpty())
	{
		return Result;
	}
	TArray<const FGatersBuiltSitePlacementSlot*> DisconnectedIndoorSlots;
	FindDisconnectedIndoorSlots(Recipe, Settings.MovementModeId, DisconnectedIndoorSlots);
	for (const FGatersBuiltSitePlacementSlot* Slot : DisconnectedIndoorSlots)
	{
		Result.Diagnostics.Add(FString::Printf(
			TEXT("site.slot.disconnected subject=%s space=%s movement-mode=%s indoor neutral slot has no route from the site network"),
			*Slot->Id, *Slot->SpaceId, *Settings.MovementModeId));
	}
	if (!DisconnectedIndoorSlots.IsEmpty())
	{
		return Result;
	}

	FBox SiteBounds(ForceInit);
	for (const FGatersBuiltSiteSpace& Space : Recipe.Spaces)
	{
		SiteBounds += Space.Center - Space.Extent;
		SiteBounds += Space.Center + Space.Extent;
	}
	const FVector SiteSize = SiteBounds.GetSize();
	Recipe.SiteArea = SiteSize.X * SiteSize.Y;

	TArray<FGatersBuiltSiteIssue> Issues;
	if (!Recipe.Validate(Issues))
	{
		for (const FGatersBuiltSiteIssue& Issue : Issues)
		{
			Result.Diagnostics.Add(FString::Printf(
				TEXT("%s subject=%s %s"), *Issue.RuleId, *Issue.SubjectId, *Issue.Message));
		}
		return Result;
	}
	Result.bCompiled = true;
	return Result;
}
