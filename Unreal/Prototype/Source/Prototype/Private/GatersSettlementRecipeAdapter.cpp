#include "GatersSettlementRecipeAdapter.h"

#include "GatersBuildingEvaluator.h"
#include "GatersBuildingGenerator.h"
#include "GatersSettlementEvaluator.h"

int32 FGatersSettlementRecipeCompilation::Count(EGatersRecipeNodeKind Kind) const
{
	int32 Result = 0;
	for (const FGatersRecipeNode& Node : Nodes)
	{
		Result += Node.Kind == Kind ? 1 : 0;
	}
	return Result;
}

FGatersSettlementRecipeCompilation FGatersSettlementRecipeAdapter::Compile(
	const FGatersTerrainSemanticField& Field,
	const FGatersSettlementPlan& Plan)
{
	FGatersSettlementRecipeCompilation Result;
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

	for (const FGatersSettlementParcel& Parcel : Plan.Parcels)
	{
		FGatersRecipeNode Node{Parcel.Id, EGatersRecipeNodeKind::SettlementParcel, Parcel.Location};
		Node.ContentKey = Parcel.SupportKey;
		Result.Nodes.Add(MoveTemp(Node));
	}
	for (const FGatersSettlementGrowthFront& Front : Plan.GrowthFronts)
	{
		FGatersRecipeNode Node{Front.Id, EGatersRecipeNodeKind::SettlementGrowthFront, Plan.CenterLocation};
		Node.Rotation.Yaw = -157.5f + Front.Sector * 45.f;
		Node.ContentKey = TEXT("settlement.growth-front");
		Result.Nodes.Add(MoveTemp(Node));
	}

	for (const FGatersSettlementBuilding& Building : Plan.Buildings)
	{
		const FGatersBuildingAssembly Assembly =
			FGatersBuildingGenerator::Generate(Field, Building);
		const FGatersBuildingEvaluation Evaluation =
			FGatersBuildingEvaluator::Evaluate(Field, Assembly);
		if (!Evaluation.IsValid())
		{
			for (const FGatersBuildingIssue& Issue : Evaluation.Issues)
			{
				Result.Diagnostics.Add(FString::Printf(
					TEXT("%s subject=%s %s"), *Issue.RuleId, *Issue.SubjectId, *Issue.Message));
			}
			continue;
		}
		++Result.ValidAssemblyCount;
		Result.ModuleCount += Assembly.Modules.Num();
		for (const FGatersBuildingModule& Module : Assembly.Modules)
		{
			FGatersRecipeNode Node{
				Module.Id,
				EGatersRecipeNodeKind::SettlementModule,
				Module.Transform.GetLocation()};
			Node.Rotation = Module.Transform.Rotator();
			Node.Scale = Module.Transform.GetScale3D();
			Node.ContentKey = Module.ContentKey;
			Result.Nodes.Add(MoveTemp(Node));
		}
	}

	const int32 Half = Field.CellsPerAxis / 2;
	for (const FIntPoint& Cell : Plan.PathCells)
	{
		const FVector Location(
			(Cell.X - Half) * Field.CellSize,
			(Cell.Y - Half) * Field.CellSize,
			Field.At(Cell.X, Cell.Y).Height);
		FGatersRecipeNode Node{
			FGatersSettlementPlan::StablePathId(Cell),
			EGatersRecipeNodeKind::SettlementPath,
			Location};
		Node.Scale = FVector(4.5f, 4.5f, 0.12f);
		Result.Nodes.Add(MoveTemp(Node));
	}

	Result.bCompiled = Result.ValidAssemblyCount == Plan.Buildings.Num() &&
		Result.Diagnostics.IsEmpty();
	return Result;
}
