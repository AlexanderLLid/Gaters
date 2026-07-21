#include "GatersSettlementEvaluator.h"

#include "GatersTerrainNavigation.h"

namespace
{
bool IsValidCell(const FGatersTerrainSemanticField& Field, const FIntPoint& Cell)
{
	return Cell.X >= 0 && Cell.X < Field.CellsPerAxis
		&& Cell.Y >= 0 && Cell.Y < Field.CellsPerAxis;
}

void AddIssue(
	FGatersSettlementEvaluation& Evaluation,
	const TCHAR* RuleId,
	const FString& SubjectId,
	const TCHAR* Message)
{
	Evaluation.Issues.Add({RuleId, SubjectId, Message});
}
}

FString FGatersSettlementEvaluation::Summary() const
{
	return FString::Printf(
		TEXT("valid=%s buildings=%d homes=%d roles=%d entrances=%d paths=%d orientations=%d radius=%.1f issues=%d"),
		IsValid() ? TEXT("yes") : TEXT("no"),
		BuildingCount,
		HomeCount,
		CoveredRoleCount,
		ReachableEntranceCount,
		PathCellCount,
		OrientationBucketCount,
		MaxBuildingRadiusCells,
		Issues.Num());
}

FGatersSettlementEvaluation FGatersSettlementEvaluator::Evaluate(
	const FGatersTerrainSemanticField& Field,
	const FGatersSettlementPlan& Plan)
{
	FGatersSettlementEvaluation Result;
	Result.BuildingCount = Plan.Buildings.Num();
	Result.PathCellCount = Plan.PathCells.Num();
	if (!Plan.bGenerated)
	{
		AddIssue(Result, TEXT("settlement.plan.invalid"), Plan.SiteId,
			TEXT("settlement generator did not produce a complete candidate"));
	}
	if (!IsValidCell(Field, Plan.CenterCell)
		|| !FGatersTerrainNavigation::IsWalkable(
			Field.At(Plan.CenterCell.X, Plan.CenterCell.Y).Type))
	{
		AddIssue(Result, TEXT("settlement.center.unwalkable"), Plan.SiteId,
			TEXT("public center is outside walkable terrain"));
	}

	TSet<FString> Ids;
	TSet<FIntPoint> BuildingCells;
	TSet<FIntPoint> DeclaredPaths(Plan.PathCells);
	TSet<EGatersSettlementRole> Roles;
	TSet<int32> OrientationBuckets;
	TSet<int32> FloorBuckets;
	TSet<int32> FootprintBuckets;
	TSet<FString> ParcelIds;
	TSet<FString> GrowthFrontIds;
	TSet<int32> GrowthFrontSectors;
	for (const FGatersSettlementGrowthFront& Front : Plan.GrowthFronts)
	{
		if (Front.Id.IsEmpty() || GrowthFrontIds.Contains(Front.Id)
			|| Front.Sector < 0 || Front.Sector > 7
			|| GrowthFrontSectors.Contains(Front.Sector))
		{
			AddIssue(Result, TEXT("settlement.growth-front.invalid"), Front.Id,
				TEXT("growth fronts require unique identities and direction sectors"));
		}
		GrowthFrontIds.Add(Front.Id);
		GrowthFrontSectors.Add(Front.Sector);
	}
	if (Plan.GrowthFronts.Num() != 4)
	{
		AddIssue(Result, TEXT("settlement.growth-front.count"), Plan.SiteId,
			TEXT("land settlements require four immutable directional growth fronts"));
	}
	for (const FGatersSettlementParcel& Parcel : Plan.Parcels)
	{
		if (Parcel.Id.IsEmpty() || ParcelIds.Contains(Parcel.Id)
			|| Parcel.SupportKey != TEXT("support.ground")
			|| Parcel.AccessKey != TEXT("navigation.ground"))
		{
			AddIssue(Result, TEXT("settlement.parcel.invalid"), Parcel.Id,
				TEXT("land parcels require unique identity and ground support/access contracts"));
		}
		ParcelIds.Add(Parcel.Id);
	}
	for (const FGatersSettlementBuilding& Building : Plan.Buildings)
	{
		const FGatersSettlementParcel* Parcel = Plan.FindParcel(Building.ParcelId);
		if (!Parcel || Parcel->Cell != Building.Cell
			|| Parcel->EntranceCell != Building.EntranceCell
			|| Parcel->IntroducedStage != Building.IntroducedStage)
		{
			AddIssue(Result, TEXT("settlement.parcel.mismatch"), Building.Id,
				TEXT("building placement must be owned by one matching semantic parcel"));
		}
		if (Building.IntroducedStage < 0 || Building.IntroducedStage > Plan.GrowthStage
			|| (Building.IntroducedStage > 0
				&& !GrowthFrontIds.Contains(Building.GrowthFrontId)))
		{
			AddIssue(Result, TEXT("settlement.growth-front.missing"), Building.Id,
				TEXT("expanded buildings require a declared immutable growth front"));
		}
		Result.HomeCount += Building.Role == EGatersSettlementRole::Home ? 1 : 0;
		Roles.Add(Building.Role);
		const FIntPoint CenterOffset = Building.Cell - Plan.CenterCell;
		const float CenterAngle = FMath::RadiansToDegrees(FMath::Atan2(
			static_cast<float>(CenterOffset.Y), static_cast<float>(CenterOffset.X)));
		const int32 Bucket = FMath::FloorToInt(
			FMath::Fmod(CenterAngle + 405.f, 360.f) / 90.f);
		OrientationBuckets.Add(Bucket);
		FloorBuckets.Add(Building.FloorCount);
		FootprintBuckets.Add(Building.FootprintWidthCells * 10 + Building.FootprintDepthCells);
		if (Building.FloorCount < 1 || Building.FloorCount > 3
			|| Building.FootprintWidthCells < 1 || Building.FootprintWidthCells > 2
			|| Building.FootprintDepthCells < 1 || Building.FootprintDepthCells > 2)
		{
			AddIssue(Result, TEXT("settlement.massing.invalid"), Building.Id,
				TEXT("building massing is outside the greybox construction contract"));
		}
		if (Building.Id.IsEmpty() || Ids.Contains(Building.Id))
		{
			AddIssue(Result, TEXT("settlement.identity.duplicate"), Building.Id,
				TEXT("building IDs must be non-empty and unique"));
		}
		Ids.Add(Building.Id);
		if (BuildingCells.Contains(Building.Cell))
		{
			AddIssue(Result, TEXT("settlement.building.overlap"), Building.Id,
				TEXT("two buildings occupy the same terrain cell"));
		}
		BuildingCells.Add(Building.Cell);
		const float Radius = static_cast<float>((Building.Cell - Plan.CenterCell).Size());
		Result.MaxBuildingRadiusCells = FMath::Max(Result.MaxBuildingRadiusCells, Radius);
		if (Radius > FGatersSettlementGenerator::MaxRadiusCells(Plan.GrowthStage))
		{
			AddIssue(Result, TEXT("settlement.layout.sprawl"), Building.Id,
				TEXT("building is too far from public space for the compact village contract"));
		}

		const bool bEntranceWalkable = IsValidCell(Field, Building.EntranceCell)
			&& FGatersTerrainNavigation::IsWalkable(
				Field.At(Building.EntranceCell.X, Building.EntranceCell.Y).Type);
		const FGatersTerrainPath Path = bEntranceWalkable
			? FGatersTerrainNavigation::FindPath(Field, Building.EntranceCell, Plan.CenterCell)
			: FGatersTerrainPath();
		if (!bEntranceWalkable || !Path.bFound)
		{
			AddIssue(Result, TEXT("settlement.entrance.unreachable"), Building.Id,
				TEXT("building entrance cannot reach public space"));
			continue;
		}
		++Result.ReachableEntranceCount;
		if (!Path.Cells.ContainsByPredicate([&DeclaredPaths](const FIntPoint& Cell)
		{
			return !DeclaredPaths.Contains(Cell);
		}))
		{
			continue;
		}
		AddIssue(Result, TEXT("settlement.path.missing"), Building.Id,
			TEXT("declared settlement paths do not connect the entrance to public space"));
	}
	Result.CoveredRoleCount = Roles.Num();
	Result.OrientationBucketCount = OrientationBuckets.Num();
	Result.FloorBucketCount = FloorBuckets.Num();
	Result.FootprintBucketCount = FootprintBuckets.Num();
	if (Plan.Parcels.Num() != Plan.Buildings.Num())
	{
		AddIssue(Result, TEXT("settlement.parcel.count"), Plan.SiteId,
			TEXT("every building requires exactly one semantic parcel"));
	}
	if (Result.OrientationBucketCount < 3)
	{
		AddIssue(Result, TEXT("settlement.orientation.collapsed"), Plan.SiteId,
			TEXT("building entrances do not surround public space"));
	}
	if (!FGatersSettlementGenerator::IsSupportedGrowthStage(Plan.GrowthStage)
		|| Result.BuildingCount != FGatersSettlementGenerator::ExpectedBuildingCount(Plan.GrowthStage)
		|| Result.HomeCount != FGatersSettlementGenerator::ExpectedHomeCount(Plan.GrowthStage)
		|| Result.CoveredRoleCount != 4)
	{
		AddIssue(Result, TEXT("settlement.roles.missing"), Plan.SiteId,
			TEXT("settlement must contain three homes and one of each shared role"));
	}
	for (const FIntPoint& Cell : Plan.PathCells)
	{
		if (!IsValidCell(Field, Cell)
			|| !FGatersTerrainNavigation::IsWalkable(Field.At(Cell.X, Cell.Y).Type))
		{
			AddIssue(Result, TEXT("settlement.path.unwalkable"),
				FString::Printf(TEXT("%d,%d"), Cell.X, Cell.Y),
				TEXT("declared settlement path crosses blocked terrain"));
		}
	}
	return Result;
}
