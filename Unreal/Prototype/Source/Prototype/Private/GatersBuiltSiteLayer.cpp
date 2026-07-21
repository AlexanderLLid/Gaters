#include "GatersBuiltSiteLayer.h"

#include "GatersSettlementEvaluator.h"
#include "GatersSettlementRecipeAdapter.h"
#include "GatersSettlementSiteRecipeAdapter.h"

FGatersBuiltSiteLayerResult FGatersBuiltSiteLayer::Generate(
	const FGatersTerrainSemanticField& Field,
	const int32 Seed,
	const FGatersSiteRoutePlan& Sites,
	const int32 GrowthStage)
{
	FGatersBuiltSiteLayerResult Result;
	const FGatersPlannedSite* Village = Sites.FindSite(TEXT("site:village:0"));
	if (!Village)
	{
		return Result;
	}

	Result.SiteCount = 1;
	Result.SourceIds.Add(Village->Id);
	const FGatersSettlementPlan Plan =
		FGatersSettlementGenerator::Generate(Field, Seed, *Village, GrowthStage);
	Result.SettlementGeneratorVersion = Plan.GeneratorVersion;
	Result.BuildingCount = Plan.Buildings.Num();
	Result.ParcelCount = Plan.Parcels.Num();
	Result.PathCount = Plan.PathCells.Num();
	Result.Diagnostics.Append(Plan.Diagnostics);
	for (const FGatersSettlementBuilding& Building : Plan.Buildings)
	{
		Result.SourceIds.Add(Building.Id);
	}
	for (const FGatersSettlementParcel& Parcel : Plan.Parcels)
	{
		Result.SourceIds.Add(Parcel.Id);
	}
	for (const FGatersSettlementGrowthFront& Front : Plan.GrowthFronts)
	{
		Result.SourceIds.Add(Front.Id);
	}
	for (const FIntPoint& Cell : Plan.PathCells)
	{
		Result.SourceIds.Add(FGatersSettlementPlan::StablePathId(Cell));
	}

	const FGatersSettlementEvaluation Evaluation =
		FGatersSettlementEvaluator::Evaluate(Field, Plan);
	Result.SettlementEvaluatorVersion = Evaluation.EvaluatorVersion;
	for (const FGatersSettlementIssue& Issue : Evaluation.Issues)
	{
		Result.Diagnostics.Add(FString::Printf(
			TEXT("%s subject=%s %s"), *Issue.RuleId, *Issue.SubjectId, *Issue.Message));
	}
	if (!Result.IsValid())
	{
		return Result;
	}
	FGatersSettlementSiteRecipeCompilation SiteCompilation =
		FGatersSettlementSiteRecipeAdapter::Compile(
			Field, Seed, Plan, FGatersSettlementEvidenceSettings::Ground());
	Result.Diagnostics.Append(SiteCompilation.Diagnostics);
	if (!SiteCompilation.bCompiled)
	{
		return Result;
	}
	Result.SiteRecipes.Add(MoveTemp(SiteCompilation.Recipe));

	FGatersSettlementRecipeCompilation Compilation =
		FGatersSettlementRecipeAdapter::Compile(Field, Plan);
	Result.ValidAssemblyCount = Compilation.ValidAssemblyCount;
	Result.ModuleCount = Compilation.ModuleCount;
	Result.Diagnostics.Append(Compilation.Diagnostics);
	if (!Compilation.bCompiled)
	{
		if (Result.Diagnostics.IsEmpty())
		{
			Result.Diagnostics.Add(TEXT("built-site layer recipe compilation failed"));
		}
		return Result;
	}
	Result.Nodes = MoveTemp(Compilation.Nodes);
	return Result;
}
