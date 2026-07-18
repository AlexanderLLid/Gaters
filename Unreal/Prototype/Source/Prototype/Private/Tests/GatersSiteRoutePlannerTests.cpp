#if WITH_DEV_AUTOMATION_TESTS

#include "GatersSiteRoutePlanner.h"
#include "GatersWorldRecipe.h"
#include "Misc/AutomationTest.h"

namespace
{
FGatersTerrainSemanticField MakePlannerField(std::initializer_list<const TCHAR*> Rows)
{
	FGatersTerrainSemanticField Field;
	Field.CellsPerAxis = static_cast<int32>(Rows.size());
	Field.CellSize = 500.f;
	Field.Cells.SetNum(Field.CellsPerAxis * Field.CellsPerAxis);
	int32 Y = 0;
	for (const TCHAR* Row : Rows)
	{
		check(FCString::Strlen(Row) == Field.CellsPerAxis);
		for (int32 X = 0; X < Field.CellsPerAxis; ++X)
		{
			FGatersTerrainSemanticSample& Sample = Field.Cells[X * Field.CellsPerAxis + Y];
			Sample.Height = X * 10.f + Y;
			switch (Row[X])
			{
			case TEXT('.'): Sample.Type = EGatersTerrainSemantic::Flat; break;
			case TEXT('s'): Sample.Type = EGatersTerrainSemantic::Slope; break;
			case TEXT('#'): Sample.Type = EGatersTerrainSemantic::Steep; break;
			case TEXT('~'): Sample.Type = EGatersTerrainSemantic::Water; break;
			default: checkNoEntry();
			}
		}
		++Y;
	}
	return Field;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersSiteRoutePlannerTest,
	"Gaters.Worldgen.SiteRoutePlanner.Contract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersSiteRoutePlannerTest::RunTest(const FString& Parameters)
{
	const FGatersTerrainSemanticField Open = MakePlannerField({
		TEXT("........."), TEXT("........."), TEXT("........."),
		TEXT("........."), TEXT("........."), TEXT("........."),
		TEXT("........."), TEXT("........."), TEXT(".........") });
	const FGatersSiteRoutePlan A = FGatersSiteRoutePlanner::Plan(
		Open, 73, FVector2D(1000.f, 0.f));
	const FGatersSiteRoutePlan B = FGatersSiteRoutePlanner::Plan(
		Open, 73, FVector2D(1000.f, 0.f));

	TestEqual(TEXT("planner contract is versioned"), A.PlannerVersion, 1);
	TestTrue(TEXT("open terrain produces a valid plan"), A.bValid);
	TestEqual(TEXT("plan contains arrival, village, raid base, and landmark"), A.Sites.Num(), 4);
	TestEqual(TEXT("plan connects three required site pairs"), A.Routes.Num(), 3);
	TestEqual(TEXT("same seed preserves sites"), A.Sites, B.Sites);
	TestEqual(TEXT("same seed preserves routes"), A.Routes, B.Routes);
	TestNotNull(TEXT("arrival has a stable ID"), A.FindSite(TEXT("site:arrival")));
	TestNotNull(TEXT("village has a stable ID"), A.FindSite(TEXT("site:village:0")));
	TestNotNull(TEXT("raid base has a stable ID"), A.FindSite(TEXT("site:raid-base:0")));
	TestNotNull(TEXT("landmark has a stable ID"), A.FindSite(TEXT("site:landmark:0")));
	for (const FGatersPlannedRoute& Route : A.Routes)
	{
		TestTrue(*FString::Printf(TEXT("route %s is reachable"), *Route.Id), Route.Path.bFound);
		TestTrue(*FString::Printf(TEXT("route %s has both endpoints"), *Route.Id),
			Route.Path.Cells.Num() >= 2);
	}

	const FGatersTerrainSemanticField Blocked = MakePlannerField({
		TEXT(".....~..."), TEXT(".....~..."), TEXT(".....~..."),
		TEXT(".....~..."), TEXT(".....~..."), TEXT(".....~..."),
		TEXT(".....~..."), TEXT(".....~..."), TEXT(".....~...") });
	const FGatersSiteRoutePlan BlockedPlan = FGatersSiteRoutePlanner::Plan(
		Blocked, 73, FVector2D(1500.f, 0.f));
	TestFalse(TEXT("unreachable raid base rejects the plan"), BlockedPlan.bValid);
	TestTrue(TEXT("blocked plan explains the base failure"),
		BlockedPlan.Diagnostics.Contains(TEXT("raid base is unreachable from arrival")));

	const FGatersTerrainSemanticField Scarce = MakePlannerField({
		TEXT("#########"), TEXT("#########"), TEXT("#########"),
		TEXT("#########"), TEXT("####...##"), TEXT("#########"),
		TEXT("#########"), TEXT("#########"), TEXT("#########") });
	const FGatersSiteRoutePlan ScarcePlan = FGatersSiteRoutePlanner::Plan(
		Scarce, 73, FVector2D(1000.f, 0.f));
	TestFalse(TEXT("terrain without a village footprint rejects the plan"), ScarcePlan.bValid);
	TestTrue(TEXT("scarce plan names the missing village"),
		ScarcePlan.Diagnostics.Contains(TEXT("no valid village site")));

	TSet<uint64> Layouts;
	for (int32 Seed = 0; Seed < 128; ++Seed)
	{
		const FGatersWorldRecipe Recipe = FGatersWorldRecipe::Generate(
			Seed, 400000.f, 6000.f, 10800.f, 900.f, 350.f);
		const FGatersEnvironment Environment = Recipe.CreateEnvironment();
		const FGatersTerrainSemanticField Field = FGatersTerrainSemanticField::Build(
			Environment, 61, 500.f, 1000.f, 0.94f, 0.77f, Recipe.BaseSite);
		const FGatersSiteRoutePlan Plan = FGatersSiteRoutePlanner::Plan(
			Field, Seed, Recipe.BaseSite);
		if (!TestTrue(*FString::Printf(TEXT("seed %d produces a valid site plan: %s"),
			Seed, *FString::Join(Plan.Diagnostics, TEXT(", "))), Plan.bValid))
		{
			continue;
		}
		const FGatersPlannedSite* Village = Plan.FindSite(TEXT("site:village:0"));
		const FGatersPlannedSite* Landmark = Plan.FindSite(TEXT("site:landmark:0"));
		if (!Village || !Landmark)
		{
			AddError(FString::Printf(TEXT("seed %d omitted a required planned site"), Seed));
			continue;
		}
		const uint64 Layout = (static_cast<uint64>(static_cast<uint32>(Village->Cell.X)) << 48)
			| (static_cast<uint64>(static_cast<uint32>(Village->Cell.Y)) << 32)
			| (static_cast<uint64>(static_cast<uint32>(Landmark->Cell.X)) << 16)
			| static_cast<uint32>(Landmark->Cell.Y);
		Layouts.Add(Layout);
	}
	AddInfo(FString::Printf(TEXT("site-plan layouts=%d across 128 seeds"), Layouts.Num()));
	TestTrue(TEXT("seed sweep produces broad site-layout variety"), Layouts.Num() >= 96);
	return true;
}

#endif
