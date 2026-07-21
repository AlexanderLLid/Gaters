#include "GatersSettlementGrowth.h"

namespace
{
FString BuildingText(const FGatersSettlementBuilding& Building)
{
	return FString::Printf(TEXT("%s,%s,%s,%d,%d,%s,%d,%d,%d,%d,%.3f,%d,%d,%d"),
		*Building.Id, *Building.ParcelId, *Building.GrowthFrontId,
		Building.IntroducedStage, static_cast<int32>(Building.Role), *Building.ContentKey,
		Building.Cell.X, Building.Cell.Y,
		Building.EntranceCell.X, Building.EntranceCell.Y, Building.Yaw,
		Building.FootprintWidthCells, Building.FootprintDepthCells, Building.FloorCount);
}

void AddIssue(
	FGatersSettlementGrowthEvaluation& Evaluation,
	const TCHAR* RuleId,
	const FString& SubjectId,
	const TCHAR* Message)
{
	Evaluation.Issues.Add({RuleId, SubjectId, Message});
}
}

FString FGatersSettlementGrowthPatch::CanonicalText() const
{
	FString Result = FString::Printf(TEXT("v=%d;planned=%d;source=%d;target=%d;base={%s}"),
		Version, bPlanned ? 1 : 0, SourceStage, TargetStage, *SourceCanonicalText);
	for (const FGatersSettlementBuilding& Building : AddedBuildings)
	{
		Result += TEXT(";building=") + BuildingText(Building);
	}
	for (const FGatersSettlementParcel& Parcel : AddedParcels)
	{
		Result += FString::Printf(TEXT(";parcel=%s,%d,%d,%d,%s,%s"),
			*Parcel.Id, Parcel.IntroducedStage, Parcel.Cell.X, Parcel.Cell.Y,
			*Parcel.SupportKey, *Parcel.AccessKey);
	}
	for (const FIntPoint& Cell : AddedPathCells)
	{
		Result += TEXT(";path=") + FGatersSettlementPlan::StablePathId(Cell);
	}
	return Result;
}

FGatersSettlementGrowthPatch FGatersSettlementGrowthPlanner::Plan(
	const FGatersTerrainSemanticField& Field,
	const int32 Seed,
	const FGatersPlannedSite& VillageSite,
	const FGatersSettlementPlan& Existing,
	const int32 TargetStage)
{
	FGatersSettlementGrowthPatch Result;
	Result.SourceStage = Existing.GrowthStage;
	Result.TargetStage = TargetStage;
	Result.SourceCanonicalText = Existing.CanonicalText();
	if (TargetStage != Existing.GrowthStage + 1
		|| !FGatersSettlementGenerator::IsSupportedGrowthStage(TargetStage))
	{
		Result.Diagnostics.Add(TEXT("growth must advance exactly one stage"));
		return Result;
	}

	const FGatersSettlementPlan ExpectedSource = FGatersSettlementGenerator::Generate(
		Field, Seed, VillageSite, Existing.GrowthStage);
	if (!ExpectedSource.bGenerated || Existing.CanonicalText() != ExpectedSource.CanonicalText())
	{
		Result.Diagnostics.Add(TEXT("source plan does not match its deterministic stage"));
		return Result;
	}
	const FGatersSettlementPlan Target = FGatersSettlementGenerator::Generate(
		Field, Seed, VillageSite, TargetStage);
	if (!Target.bGenerated || Target.Buildings.Num() < Existing.Buildings.Num())
	{
		Result.Diagnostics.Add(TEXT("target stage could not be generated"));
		return Result;
	}
	for (int32 Index = 0; Index < Existing.Buildings.Num(); ++Index)
	{
		if (BuildingText(Existing.Buildings[Index]) != BuildingText(Target.Buildings[Index]))
		{
			Result.Diagnostics.Add(TEXT("target stage changed an existing building"));
			return Result;
		}
	}

	Result.AddedBuildings.Append(
		Target.Buildings.GetData() + Existing.Buildings.Num(),
		Target.Buildings.Num() - Existing.Buildings.Num());
	Result.AddedParcels.Append(
		Target.Parcels.GetData() + Existing.Parcels.Num(),
		Target.Parcels.Num() - Existing.Parcels.Num());
	const TSet<FIntPoint> ExistingPaths(Existing.PathCells);
	for (const FIntPoint& Cell : Target.PathCells)
	{
		if (!ExistingPaths.Contains(Cell))
		{
			Result.AddedPathCells.Add(Cell);
		}
	}
	Result.bPlanned = true;
	return Result;
}

FGatersSettlementGrowthEvaluation FGatersSettlementGrowthEvaluator::Evaluate(
	const FGatersTerrainSemanticField& Field,
	const FGatersSettlementPlan& Existing,
	const FGatersSettlementGrowthPatch& Patch)
{
	FGatersSettlementGrowthEvaluation Result;
	Result.ExpandedPlan = Existing;
	if (!Patch.bPlanned)
	{
		AddIssue(Result, TEXT("settlement.growth.patch.invalid"), Existing.SiteId,
			TEXT("growth planner did not produce a complete patch"));
		return Result;
	}
	if (Patch.SourceStage != Existing.GrowthStage
		|| Patch.SourceCanonicalText != Existing.CanonicalText())
	{
		AddIssue(Result, TEXT("settlement.growth.source.mismatch"), Existing.SiteId,
			TEXT("growth patch does not apply to this exact settlement stage"));
	}
	if (Patch.TargetStage != Patch.SourceStage + 1
		|| !FGatersSettlementGenerator::IsSupportedGrowthStage(Patch.TargetStage))
	{
		AddIssue(Result, TEXT("settlement.growth.stage.invalid"), Existing.SiteId,
			TEXT("growth patch must advance exactly one supported stage"));
	}

	TSet<FString> BuildingIds;
	TSet<FString> ParcelIds;
	for (const FGatersSettlementParcel& Parcel : Existing.Parcels)
	{
		ParcelIds.Add(Parcel.Id);
	}
	for (const FGatersSettlementParcel& Parcel : Patch.AddedParcels)
	{
		if (Parcel.Id.IsEmpty() || ParcelIds.Contains(Parcel.Id))
		{
			AddIssue(Result, TEXT("settlement.growth.parcel.duplicate"), Parcel.Id,
				TEXT("growth additions require new stable parcel identities"));
		}
		ParcelIds.Add(Parcel.Id);
		Result.ExpandedPlan.Parcels.Add(Parcel);
	}
	for (const FGatersSettlementBuilding& Building : Existing.Buildings)
	{
		BuildingIds.Add(Building.Id);
	}
	for (const FGatersSettlementBuilding& Building : Patch.AddedBuildings)
	{
		if (Building.Id.IsEmpty() || BuildingIds.Contains(Building.Id))
		{
			AddIssue(Result, TEXT("settlement.growth.identity.duplicate"), Building.Id,
				TEXT("growth additions require new stable building identities"));
		}
		BuildingIds.Add(Building.Id);
		Result.ExpandedPlan.Buildings.Add(Building);
	}

	TSet<FIntPoint> PathCells(Result.ExpandedPlan.PathCells);
	for (const FIntPoint& Cell : Patch.AddedPathCells)
	{
		if (PathCells.Contains(Cell))
		{
			AddIssue(Result, TEXT("settlement.growth.path.duplicate"),
				FGatersSettlementPlan::StablePathId(Cell),
				TEXT("growth patch repeats an existing path identity"));
		}
		PathCells.Add(Cell);
	}
	Result.ExpandedPlan.PathCells = PathCells.Array();
	Result.ExpandedPlan.PathCells.Sort([](const FIntPoint& A, const FIntPoint& B)
	{
		return A.X == B.X ? A.Y < B.Y : A.X < B.X;
	});
	Result.ExpandedPlan.GrowthStage = Patch.TargetStage;
	Result.ExpandedPlan.bGenerated = true;

	if (Result.ExpandedPlan.Buildings.Num()
		!= FGatersSettlementGenerator::ExpectedBuildingCount(Patch.TargetStage))
	{
		AddIssue(Result, TEXT("settlement.growth.count.invalid"), Existing.SiteId,
			TEXT("growth patch does not produce the target stage building count"));
	}
	const FGatersSettlementEvaluation Settlement =
		FGatersSettlementEvaluator::Evaluate(Field, Result.ExpandedPlan);
	Result.Issues.Append(Settlement.Issues);
	return Result;
}
