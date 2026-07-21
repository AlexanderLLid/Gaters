#if WITH_DEV_AUTOMATION_TESTS

#include "GatersSettlementEvaluator.h"
#include "GatersSettlementGenerator.h"
#include "GatersSiteRoutePlanner.h"
#include "GatersWorldRecipe.h"
#include "Misc/AutomationTest.h"

namespace
{
FGatersTerrainSemanticField MakeSettlementField(
	const int32 Side,
	const EGatersTerrainSemantic Fill = EGatersTerrainSemantic::Flat)
{
	FGatersTerrainSemanticField Field;
	Field.CellsPerAxis = Side;
	Field.CellSize = 500.f;
	Field.Cells.SetNum(Side * Side);
	for (int32 X = 0; X < Side; ++X)
	{
		for (int32 Y = 0; Y < Side; ++Y)
		{
			FGatersTerrainSemanticSample& Sample = Field.Cells[X * Side + Y];
			Sample.Type = Fill;
			Sample.Height = X * 5.f + Y;
		}
	}
	return Field;
}

FGatersPlannedSite VillageSite(const FGatersTerrainSemanticField& Field)
{
	const FIntPoint Cell(Field.CellsPerAxis / 2, Field.CellsPerAxis / 2);
	return {
		TEXT("site:village:0"),
		EGatersPlannedSiteKind::Village,
		Cell,
		FVector(0.f, 0.f, Field.At(Cell.X, Cell.Y).Height)};
}

bool HasSettlementIssue(
	const FGatersSettlementEvaluation& Evaluation,
	const TCHAR* RuleId)
{
	return Evaluation.Issues.ContainsByPredicate(
		[RuleId](const FGatersSettlementIssue& Issue)
		{
			return Issue.RuleId == RuleId;
		});
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersSettlementContractTest,
	"Gaters.Worldgen.Settlement.Contract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersSettlementContractTest::RunTest(const FString& Parameters)
{
	const FGatersTerrainSemanticField Open = MakeSettlementField(17);
	const FGatersPlannedSite Site = VillageSite(Open);
	const FGatersSettlementPlan A = FGatersSettlementGenerator::Generate(Open, 73, Site);
	const FGatersSettlementPlan B = FGatersSettlementGenerator::Generate(Open, 73, Site);

	TestEqual(TEXT("settlement generator is versioned"), A.GeneratorVersion, 3);
	TestEqual(TEXT("settlement profile is versioned"), A.Profile.Version, 1);
	TestTrue(TEXT("height bias is normalized"),
		A.Profile.HeightBias >= 0.f && A.Profile.HeightBias <= 1.f);
	TestTrue(TEXT("footprint bias is normalized"),
		A.Profile.FootprintBias >= 0.f && A.Profile.FootprintBias <= 1.f);
	TestTrue(TEXT("density bias is normalized"),
		A.Profile.DensityBias >= 0.f && A.Profile.DensityBias <= 1.f);
	TestTrue(TEXT("open terrain produces a settlement"), A.bGenerated);
	TestEqual(TEXT("settlement has the six-role slice"), A.Buildings.Num(), 6);
	TestEqual(TEXT("same seed preserves canonical layout"), A.CanonicalText(), B.CanonicalText());
	TestTrue(TEXT("settlement emits connecting path cells"), A.PathCells.Num() > 0);

	const FGatersSettlementEvaluation Evaluation =
		FGatersSettlementEvaluator::Evaluate(Open, A);
	TestEqual(TEXT("settlement evaluator is versioned"), Evaluation.EvaluatorVersion, 3);
	TestTrue(TEXT("open settlement passes independent evaluation"), Evaluation.IsValid());
	TestEqual(TEXT("three homes are present"), Evaluation.HomeCount, 3);
	TestEqual(TEXT("all four roles are covered"), Evaluation.CoveredRoleCount, 4);
	TestEqual(TEXT("every entrance reaches public space"),
		Evaluation.ReachableEntranceCount, A.Buildings.Num());
	TestTrue(TEXT("buildings surround public space instead of collapsing to one side"),
		Evaluation.OrientationBucketCount >= 3);
	TestTrue(TEXT("village stays compact around public space"),
		Evaluation.MaxBuildingRadiusCells <= 6.f);
	TestTrue(TEXT("profile assigns valid building footprints"),
		A.Buildings.ContainsByPredicate([](const FGatersSettlementBuilding& Building)
		{
			return Building.FootprintWidthCells >= 1 && Building.FootprintWidthCells <= 2
				&& Building.FootprintDepthCells >= 1 && Building.FootprintDepthCells <= 2;
		}));
	TestTrue(TEXT("profile assigns valid floor counts"),
		A.Buildings.ContainsByPredicate([](const FGatersSettlementBuilding& Building)
		{
			return Building.FloorCount >= 1 && Building.FloorCount <= 3;
		}));
	TSet<FIntPoint> BuildingCells;
	for (const FGatersSettlementBuilding& Building : A.Buildings)
	{
		BuildingCells.Add(Building.Cell);
		const FVector2D Forward(
			FMath::Cos(FMath::DegreesToRadians(Building.Yaw)),
			FMath::Sin(FMath::DegreesToRadians(Building.Yaw)));
		const FVector2D TowardEntrance(
			static_cast<float>(Building.EntranceCell.X - Building.Cell.X),
			static_cast<float>(Building.EntranceCell.Y - Building.Cell.Y));
		TestTrue(*FString::Printf(TEXT("%s facade faces its selected entrance"), *Building.Id),
			FVector2D::DotProduct(Forward, TowardEntrance.GetSafeNormal()) > 0.999f);
	}
	TestFalse(TEXT("building cells never become public path cells"),
		A.PathCells.ContainsByPredicate([&BuildingCells](const FIntPoint& Cell)
		{
			return BuildingCells.Contains(Cell);
		}));

	const FGatersSettlementPlan Different =
		FGatersSettlementGenerator::Generate(Open, 74, Site);
	TestNotEqual(TEXT("different seeds vary the settlement layout"),
		A.CanonicalText(), Different.CanonicalText());
	TestNotEqual(TEXT("different seeds vary the settlement profile"),
		A.Profile.CanonicalText(), Different.Profile.CanonicalText());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersSettlementProfileVarietyTest,
	"Gaters.Worldgen.Settlement.ProfileVariety",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersSettlementProfileVarietyTest::RunTest(const FString& Parameters)
{
	const FGatersTerrainSemanticField Open = MakeSettlementField(17);
	const FGatersPlannedSite Site = VillageSite(Open);
	int32 SingleFloorVillages = 0;
	int32 TallVillages = 0;
	TSet<FString> Profiles;
	for (int32 Seed = 0; Seed < 64; ++Seed)
	{
		const FGatersSettlementPlan Plan = FGatersSettlementGenerator::Generate(Open, Seed, Site);
		Profiles.Add(Plan.Profile.CanonicalText());
		int32 MaxFloors = 0;
		for (const FGatersSettlementBuilding& Building : Plan.Buildings)
		{
			MaxFloors = FMath::Max(MaxFloors, Building.FloorCount);
		}
		SingleFloorVillages += MaxFloors == 1 ? 1 : 0;
		TallVillages += MaxFloors >= 3 ? 1 : 0;
	}
	TestTrue(TEXT("held-out profiles include low villages"), SingleFloorVillages > 0);
	TestTrue(TEXT("held-out profiles include tall villages"), TallVillages > 0);
	TestTrue(TEXT("profiles remain diverse"), Profiles.Num() >= 48);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersSettlementGrowthContractTest,
	"Gaters.Worldgen.Settlement.GrowthContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersSettlementGrowthContractTest::RunTest(const FString& Parameters)
{
	const FGatersTerrainSemanticField Open = MakeSettlementField(31);
	const FGatersPlannedSite Site = VillageSite(Open);
	const FGatersSettlementPlan Hamlet =
		FGatersSettlementGenerator::Generate(Open, 73, Site, 0);
	const FGatersSettlementPlan Village =
		FGatersSettlementGenerator::Generate(Open, 73, Site, 1);
	const FGatersSettlementPlan Town =
		FGatersSettlementGenerator::Generate(Open, 73, Site, 2);

	TestTrue(TEXT("all bounded growth stages generate"),
		Hamlet.bGenerated && Village.bGenerated && Town.bGenerated);
	TestEqual(TEXT("stage zero keeps the existing six-building slice"),
		Hamlet.Buildings.Num(), 6);
	TestTrue(TEXT("each growth stage adds buildings"),
		Hamlet.Buildings.Num() < Village.Buildings.Num()
		&& Village.Buildings.Num() < Town.Buildings.Num());
	TestEqual(TEXT("plans record the requested growth stage"), Town.GrowthStage, 2);
	TestEqual(TEXT("every hamlet building occupies an explicit parcel"),
		Hamlet.Parcels.Num(), Hamlet.Buildings.Num());
	TestEqual(TEXT("growth fronts are fixed before expansion"),
		Hamlet.GrowthFronts.Num(), 4);
	TestEqual(TEXT("later stages preserve the same growth fronts"),
		Hamlet.GrowthFronts, Town.GrowthFronts);
	bool bExpansionUsesDeclaredParcelsAndFronts = true;
	TSet<FString> FrontIds;
	for (const FGatersSettlementGrowthFront& Front : Hamlet.GrowthFronts)
	{
		FrontIds.Add(Front.Id);
	}
	for (const FGatersSettlementBuilding& Building : Town.Buildings)
	{
		const FGatersSettlementParcel* Parcel = Town.FindParcel(Building.ParcelId);
		bExpansionUsesDeclaredParcelsAndFronts &= Parcel
			&& Parcel->Cell == Building.Cell
			&& Parcel->IntroducedStage == Building.IntroducedStage;
		if (Building.IntroducedStage > 0)
		{
			bExpansionUsesDeclaredParcelsAndFronts &=
				FrontIds.Contains(Building.GrowthFrontId);
		}
	}
	TestTrue(TEXT("expanded buildings consume declared parcels and growth fronts"),
		bExpansionUsesDeclaredParcelsAndFronts);

	auto TestPreservedPrefix = [this](
		const TCHAR* Label,
		const FGatersSettlementPlan& Earlier,
		const FGatersSettlementPlan& Later)
	{
		bool bPreserved = Earlier.Buildings.Num() <= Later.Buildings.Num();
		for (int32 Index = 0; bPreserved && Index < Earlier.Buildings.Num(); ++Index)
		{
			const FGatersSettlementBuilding& A = Earlier.Buildings[Index];
			const FGatersSettlementBuilding& B = Later.Buildings[Index];
			bPreserved = A.Id == B.Id && A.Role == B.Role
				&& A.ContentKey == B.ContentKey && A.Cell == B.Cell
				&& A.EntranceCell == B.EntranceCell && A.Location == B.Location
				&& A.Yaw == B.Yaw
				&& A.FootprintWidthCells == B.FootprintWidthCells
				&& A.FootprintDepthCells == B.FootprintDepthCells
				&& A.FloorCount == B.FloorCount;
		}
		TestTrue(Label, bPreserved);
	};
	TestPreservedPrefix(TEXT("village preserves every hamlet building"), Hamlet, Village);
	TestPreservedPrefix(TEXT("town preserves every village building"), Village, Town);

	TSet<FString> VillagePathIds;
	for (const FIntPoint& Cell : Village.PathCells)
	{
		VillagePathIds.Add(FGatersSettlementPlan::StablePathId(Cell));
	}
	bool bPathsPreserved = true;
	for (const FIntPoint& Cell : Hamlet.PathCells)
	{
		bPathsPreserved &= VillagePathIds.Contains(
			FGatersSettlementPlan::StablePathId(Cell));
	}
	TestTrue(TEXT("growth preserves every existing path identity"), bPathsPreserved);

	const FGatersSettlementPlan Invalid =
		FGatersSettlementGenerator::Generate(Open, 73, Site, 3);
	TestFalse(TEXT("growth beyond the bounded contract is rejected"), Invalid.bGenerated);
	TestTrue(TEXT("invalid growth stage is diagnosed"),
		Invalid.Diagnostics.Contains(TEXT("growth stage is outside the supported contract")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersSettlementCounterexampleTest,
	"Gaters.Worldgen.Settlement.Counterexamples",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersSettlementCounterexampleTest::RunTest(const FString& Parameters)
{
	FGatersTerrainSemanticField Scarce = MakeSettlementField(17, EGatersTerrainSemantic::Steep);
	const FIntPoint Center(8, 8);
	for (int32 X = Center.X - 1; X <= Center.X + 1; ++X)
	{
		for (int32 Y = Center.Y - 1; Y <= Center.Y + 1; ++Y)
		{
			Scarce.Cells[X * Scarce.CellsPerAxis + Y].Type = EGatersTerrainSemantic::Flat;
		}
	}
	const FGatersSettlementPlan Rejected =
		FGatersSettlementGenerator::Generate(Scarce, 73, VillageSite(Scarce));
	TestFalse(TEXT("terrain without six building sites is rejected"), Rejected.bGenerated);
	TestTrue(TEXT("scarce terrain reports capacity failure"),
		Rejected.Diagnostics.Contains(TEXT("not enough reachable building sites")));

	FGatersTerrainSemanticField Open = MakeSettlementField(17);
	FGatersSettlementPlan Corrupt =
		FGatersSettlementGenerator::Generate(Open, 73, VillageSite(Open));
	TestTrue(TEXT("positive fixture exists before corruption"), Corrupt.bGenerated);
	if (Corrupt.Buildings.IsEmpty())
	{
		return false;
	}
	const FIntPoint Entrance = Corrupt.Buildings[0].EntranceCell;
	Open.Cells[Entrance.X * Open.CellsPerAxis + Entrance.Y].Type =
		EGatersTerrainSemantic::Water;
	const FGatersSettlementEvaluation Evaluation =
		FGatersSettlementEvaluator::Evaluate(Open, Corrupt);
	TestFalse(TEXT("water-blocked entrance is rejected"), Evaluation.IsValid());
	TestTrue(TEXT("entrance failure is causal"),
		HasSettlementIssue(Evaluation, TEXT("settlement.entrance.unreachable")));

	const FGatersTerrainSemanticField OrientationField = MakeSettlementField(17);
	FGatersSettlementPlan Collapsed = FGatersSettlementGenerator::Generate(
		OrientationField, 73, VillageSite(OrientationField));
	const FIntPoint CollapsedOffsets[] = {
		FIntPoint(2, -1), FIntPoint(2, 0), FIntPoint(2, 1),
		FIntPoint(3, -1), FIntPoint(3, 0), FIntPoint(3, 1)};
	for (int32 Index = 0; Index < Collapsed.Buildings.Num(); ++Index)
	{
		FGatersSettlementBuilding& Building = Collapsed.Buildings[Index];
		Building.Cell = Collapsed.CenterCell + CollapsedOffsets[Index];
		for (FGatersSettlementParcel& Parcel : Collapsed.Parcels)
		{
			if (Parcel.Id == Building.ParcelId)
			{
				Parcel.Cell = Building.Cell;
				break;
			}
		}
	}
	const FGatersSettlementEvaluation CollapsedEvaluation =
		FGatersSettlementEvaluator::Evaluate(OrientationField, Collapsed);
	TestTrue(TEXT("collapsed facade directions are rejected"),
		HasSettlementIssue(CollapsedEvaluation, TEXT("settlement.orientation.collapsed")));

	FGatersSettlementPlan Sprawled = Corrupt;
	Sprawled.Buildings[0].Cell = FIntPoint(16, 16);
	const FGatersSettlementEvaluation SprawledEvaluation =
		FGatersSettlementEvaluator::Evaluate(MakeSettlementField(17), Sprawled);
	TestTrue(TEXT("sprawled building placement is rejected"),
		HasSettlementIssue(SprawledEvaluation, TEXT("settlement.layout.sprawl")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersSettlementGeneratedTerrainTest,
	"Gaters.Worldgen.Settlement.GeneratedTerrain",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersSettlementGeneratedTerrainTest::RunTest(const FString& Parameters)
{
	for (const int32 Seed : {0, 2, 4, 7, 53})
	{
		const FGatersWorldRecipe Recipe = FGatersWorldRecipe::Generate(
			Seed, 400000.f, 6000.f, 10800.f, 900.f, 350.f);
		const FGatersEnvironment Environment = Recipe.CreateEnvironment();
		const FGatersTerrainSemanticField Field = FGatersTerrainSemanticField::Build(
			Environment, 61, 500.f, 1000.f, 0.94f, 0.77f, Recipe.BaseSite);
		const FGatersSiteRoutePlan Sites =
			FGatersSiteRoutePlanner::Plan(Field, Seed, Recipe.BaseSite);
		const FGatersPlannedSite* Site = Sites.FindSite(TEXT("site:village:0"));
		if (!TestNotNull(*FString::Printf(TEXT("seed %d has a village site"), Seed), Site))
		{
			continue;
		}
		const FGatersSettlementPlan Settlement =
			FGatersSettlementGenerator::Generate(Field, Seed, *Site);
		const FGatersSettlementEvaluation Evaluation =
			FGatersSettlementEvaluator::Evaluate(Field, Settlement);
		TestTrue(*FString::Printf(TEXT("seed %d generates a settlement: %s"), Seed,
			*FString::Join(Settlement.Diagnostics, TEXT(", "))), Settlement.bGenerated);
		TestTrue(*FString::Printf(TEXT("seed %d settlement evaluates: %s"), Seed,
			*Evaluation.Summary()), Evaluation.IsValid());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersSettlementHeldOutSweepTest,
	"Gaters.Worldgen.Settlement.HeldOutSweep",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersSettlementHeldOutSweepTest::RunTest(const FString& Parameters)
{
	constexpr int32 SeedCount = 64;
	TSet<FString> RelativeLayouts;
	int32 ValidCount = 0;
	for (int32 Seed = 1000; Seed < 1000 + SeedCount; ++Seed)
	{
		const FGatersWorldRecipe Recipe = FGatersWorldRecipe::Generate(
			Seed, 400000.f, 6000.f, 10800.f, 900.f, 350.f);
		const FGatersEnvironment Environment = Recipe.CreateEnvironment();
		const FGatersTerrainSemanticField Field = FGatersTerrainSemanticField::Build(
			Environment, 61, 500.f, 1000.f, 0.94f, 0.77f, Recipe.BaseSite);
		const FGatersSiteRoutePlan Sites =
			FGatersSiteRoutePlanner::Plan(Field, Seed, Recipe.BaseSite);
		const FGatersPlannedSite* Site = Sites.FindSite(TEXT("site:village:0"));
		if (!Site)
		{
			AddError(FString::Printf(TEXT("held-out seed %d has no village site"), Seed));
			continue;
		}

		const FGatersSettlementPlan Settlement =
			FGatersSettlementGenerator::Generate(Field, Seed, *Site);
		const FGatersSettlementEvaluation Evaluation =
			FGatersSettlementEvaluator::Evaluate(Field, Settlement);
		if (!Evaluation.IsValid())
		{
			AddError(FString::Printf(TEXT("held-out seed %d failed: %s"),
				Seed, *Evaluation.Summary()));
			continue;
		}
		++ValidCount;

		TArray<FString> Parts;
		for (const FGatersSettlementBuilding& Building : Settlement.Buildings)
		{
			const FIntPoint Cell = Building.Cell - Settlement.CenterCell;
			const FIntPoint Entrance = Building.EntranceCell - Settlement.CenterCell;
			const int32 YawBucket = FMath::FloorToInt(
				FMath::Fmod(Building.Yaw + 405.f, 360.f) / 45.f);
			Parts.Add(FString::Printf(TEXT("%d:%d,%d:%d,%d:%d"),
				static_cast<int32>(Building.Role), Cell.X, Cell.Y,
				Entrance.X, Entrance.Y, YawBucket));
		}
		Parts.Sort();
		RelativeLayouts.Add(FString::Join(Parts, TEXT(";")));
	}

	TestEqual(TEXT("all held-out settlements satisfy the structural evaluator"),
		ValidCount, SeedCount);
	TestTrue(TEXT("held-out seeds do not collapse to one relative village layout"),
		RelativeLayouts.Num() >= SeedCount * 3 / 4);
	AddInfo(FString::Printf(TEXT("held-out settlements valid=%d/%d unique-layouts=%d"),
		ValidCount, SeedCount, RelativeLayouts.Num()));
	return true;
}

#endif
