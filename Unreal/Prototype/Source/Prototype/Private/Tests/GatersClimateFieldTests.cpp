#if WITH_DEV_AUTOMATION_TESTS

#include "GatersClimateField.h"
#include "GatersEnvironment.h"
#include "GatersEnvironmentBrief.h"
#include "GatersLandformProcessField.h"
#include "Misc/AutomationTest.h"

namespace
{
FGatersEnvironmentBrief FixedClimateBrief(
	const float Temperature = 0.55f,
	const float Moisture = 0.6f)
{
	FGatersEnvironmentBrief Brief;
	Brief.Global.Temperature = {Temperature, Temperature};
	Brief.Global.Moisture = {Moisture, Moisture};
	Brief.Global.SurfaceWater = {0.4f, 0.4f};
	Brief.Global.Relief = {0.45f, 0.45f};
	return Brief;
}

FGatersCompiledEnvironmentBrief CompileIntent(
	const FGatersEnvironmentBrief& Brief,
	const int32 Seed,
	const float WorldSize)
{
	return FGatersEnvironmentBriefCompiler::Compile(
		Brief, Seed, WorldSize).Intent;
}

bool HasIssue(
	const FGatersClimateCompileResult& Result,
	const TCHAR* RuleId)
{
	return Result.Issues.ContainsByPredicate(
		[RuleId](const FGatersClimateIssue& Issue)
		{
			return Issue.RuleId == RuleId;
		});
}

bool IsBounded(const float Value)
{
	return FMath::IsFinite(Value) && Value >= 0.f && Value <= 1.f;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersClimateFieldContractTest,
	"Gaters.Worldgen.ClimateField.Contract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersClimateFieldContractTest::RunTest(const FString& Parameters)
{
	constexpr int32 Seed = 83;
	constexpr float WorldSize = 120000.f;
	const FGatersEnvironment Environment =
		FGatersEnvironment::FromSeed(Seed, WorldSize);
	const FGatersCompiledEnvironmentBrief Intent = CompileIntent(
		FixedClimateBrief(), Seed, WorldSize);
	const FGatersLandformProcessRecipe Landform =
		FGatersLandformProcessField::Compile(Environment, Intent).Recipe;
	const FGatersClimateCompileResult A =
		FGatersClimateField::Compile(Environment, Intent, Landform);
	const FGatersClimateCompileResult B =
		FGatersClimateField::Compile(Environment, Intent, Landform);

	TestTrue(TEXT("compatible climate inputs compile"), A.IsValid());
	TestEqual(TEXT("climate recipe is versioned"),
		A.Recipe.Version, FGatersClimateRecipe::CurrentVersion);
	TestTrue(TEXT("same climate inputs compile exactly"), A.Recipe == B.Recipe);
	TestTrue(TEXT("prevailing wind is normalized"),
		FMath::IsNearlyEqual(A.Recipe.PrevailingWind.Size(), 1.f, 0.001f));

	const FVector2D Point(17000.f, -9000.f);
	const FGatersClimateSample SampleA =
		FGatersClimateField::Query(A.Recipe, Environment, Point);
	const FGatersClimateSample SampleB =
		FGatersClimateField::Query(A.Recipe, Environment, Point);
	TestTrue(TEXT("same climate query repeats exactly"), SampleA == SampleB);
	TestEqual(TEXT("climate samples the caller's accepted terrain exactly"),
		SampleA.Height, Environment.HeightAt(Point));

	for (int32 X = -4; X <= 4; ++X)
	{
		for (int32 Y = -4; Y <= 4; ++Y)
		{
			const FGatersClimateSample Sample = FGatersClimateField::Query(
				A.Recipe, Environment, FVector2D(X * 12000.f, Y * 12000.f));
			TestTrue(TEXT("temperature is bounded"), IsBounded(Sample.Temperature));
			TestTrue(TEXT("precipitation is bounded"), IsBounded(Sample.Precipitation));
			TestTrue(TEXT("wind exposure is bounded"), IsBounded(Sample.WindExposure));
			TestTrue(TEXT("seasonality is bounded"), IsBounded(Sample.Seasonality));
			TestTrue(TEXT("freeze-thaw is bounded"), IsBounded(Sample.FreezeThaw));
			TestTrue(TEXT("regional influence is bounded"),
				IsBounded(Sample.RegionInfluence));
		}
	}

	const int32 OtherSeed = 131;
	const FGatersEnvironment OtherEnvironment =
		FGatersEnvironment::FromSeed(OtherSeed, WorldSize);
	const FGatersCompiledEnvironmentBrief OtherIntent = CompileIntent(
		FixedClimateBrief(), OtherSeed, WorldSize);
	const FGatersLandformProcessRecipe OtherLandform =
		FGatersLandformProcessField::Compile(
			OtherEnvironment, OtherIntent).Recipe;
	const FGatersClimateCompileResult Different = FGatersClimateField::Compile(
		OtherEnvironment, OtherIntent, OtherLandform);
	TestTrue(TEXT("changed seed changes deterministic climate provenance"),
		Different.IsValid() && Different.Recipe != A.Recipe
			&& Different.Recipe.PrevailingWind != A.Recipe.PrevailingWind);

	FGatersCompiledEnvironmentBrief BadIntent = Intent;
	BadIntent.CompilerVersion = 1;
	TestTrue(TEXT("unsupported intent version is causal"), HasIssue(
		FGatersClimateField::Compile(Environment, BadIntent, Landform),
		TEXT("climate.intent_version")));
	BadIntent = Intent;
	BadIntent.Seed += 1;
	TestTrue(TEXT("intent seed mismatch is causal"), HasIssue(
		FGatersClimateField::Compile(Environment, BadIntent, Landform),
		TEXT("climate.provenance")));
	FGatersLandformProcessRecipe BadLandform = Landform;
	BadLandform.Version = 1;
	TestTrue(TEXT("unsupported landform version is causal"), HasIssue(
		FGatersClimateField::Compile(Environment, Intent, BadLandform),
		TEXT("climate.landform_version")));
	BadLandform = Landform;
	BadLandform.WorldSize += 1.f;
	TestTrue(TEXT("landform world-size mismatch is causal"), HasIssue(
		FGatersClimateField::Compile(Environment, Intent, BadLandform),
		TEXT("climate.provenance")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersClimateFieldPhysicalControlsTest,
	"Gaters.Worldgen.ClimateField.PhysicalControls",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersClimateFieldPhysicalControlsTest::RunTest(const FString& Parameters)
{
	FGatersClimateRecipe Recipe;
	Recipe.Seed = 7;
	Recipe.WorldSize = 120000.f;
	Recipe.PrevailingWind = FVector2D(1.f, 0.f);
	Recipe.SeasonalityBias = 0.55f;

	FGatersEnvironmentTargetProfile TemperateWet;
	TemperateWet.Temperature = 0.48f;
	TemperateWet.Moisture = 0.9f;
	TemperateWet.SurfaceWater = 0.6f;
	FGatersEnvironmentTargetProfile TemperateDry = TemperateWet;
	TemperateDry.Moisture = 0.1f;
	TemperateDry.SurfaceWater = 0.f;
	FGatersEnvironmentTargetProfile WarmWet = TemperateWet;
	WarmWet.Temperature = 0.95f;

	FGatersClimateTerrainEvidence Flat;
	Flat.Height = 200.f;
	Flat.UpwindHeight = 200.f;
	Flat.NeighborhoodMeanHeight = 200.f;
	Flat.bHasWater = true;
	Flat.WaterHeight = 0.f;
	const FGatersClimateSample Low =
		FGatersClimateField::Evaluate(Recipe, TemperateWet, Flat);
	FGatersClimateTerrainEvidence High = Flat;
	High.Height = 4200.f;
	High.UpwindHeight = 4200.f;
	High.NeighborhoodMeanHeight = 4200.f;
	const FGatersClimateSample Alpine =
		FGatersClimateField::Evaluate(Recipe, TemperateWet, High);
	TestTrue(TEXT("higher altitude lowers temperature"),
		Alpine.Temperature < Low.Temperature);

	const FGatersClimateSample Dry =
		FGatersClimateField::Evaluate(Recipe, TemperateDry, Flat);
	TestTrue(TEXT("declared moisture raises precipitation"),
		Low.Precipitation > Dry.Precipitation);

	FGatersClimateTerrainEvidence Windward = Flat;
	Windward.Height = 2500.f;
	Windward.UpwindHeight = 200.f;
	Windward.NeighborhoodMeanHeight = 1500.f;
	FGatersClimateTerrainEvidence Leeward = Flat;
	Leeward.Height = 200.f;
	Leeward.UpwindHeight = 2500.f;
	Leeward.NeighborhoodMeanHeight = 1500.f;
	const FGatersClimateSample WindwardSample =
		FGatersClimateField::Evaluate(Recipe, TemperateWet, Windward);
	const FGatersClimateSample LeewardSample =
		FGatersClimateField::Evaluate(Recipe, TemperateWet, Leeward);
	TestTrue(TEXT("rising upwind terrain raises precipitation"),
		WindwardSample.Precipitation > Low.Precipitation);
	TestTrue(TEXT("lee terrain lowers precipitation"),
		LeewardSample.Precipitation < Low.Precipitation);
	TestTrue(TEXT("terrain above its neighborhood raises wind exposure"),
		WindwardSample.WindExposure > Low.WindExposure);

	const FGatersClimateSample FreezeWet =
		FGatersClimateField::Evaluate(Recipe, TemperateWet, Flat);
	const FGatersClimateSample FreezeDry =
		FGatersClimateField::Evaluate(Recipe, TemperateDry, Flat);
	const FGatersClimateSample Warm =
		FGatersClimateField::Evaluate(Recipe, WarmWet, Flat);
	TestTrue(TEXT("wet near-freezing conditions raise freeze-thaw"),
		FreezeWet.FreezeThaw > FreezeDry.FreezeThaw
			&& FreezeWet.FreezeThaw > Warm.FreezeThaw);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersClimateFieldRegionalContinuityTest,
	"Gaters.Worldgen.ClimateField.RegionalContinuity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersClimateFieldRegionalContinuityTest::RunTest(const FString& Parameters)
{
	constexpr int32 Seed = 29;
	constexpr float WorldSize = 120000.f;
	FGatersEnvironmentBrief Brief = FixedClimateBrief(0.2f, 0.2f);
	FGatersEnvironmentRegionBrief Region;
	Region.Id = TEXT("climate-region");
	Region.CenterNormalized = FVector2D(0.15f, -0.1f);
	Region.RadiusFraction = 0.12f;
	Region.Profile = Brief.Global;
	Region.Profile.Temperature = {0.8f, 0.8f};
	Region.Profile.Moisture = {0.85f, 0.85f};
	Brief.Regions.Add(Region);
	const FGatersEnvironment Environment =
		FGatersEnvironment::FromSeed(Seed, WorldSize);
	const FGatersCompiledEnvironmentBrief Intent =
		CompileIntent(Brief, Seed, WorldSize);
	const FGatersLandformProcessRecipe Landform =
		FGatersLandformProcessField::Compile(Environment, Intent).Recipe;
	const FGatersClimateRecipe Recipe =
		FGatersClimateField::Compile(Environment, Intent, Landform).Recipe;

	FGatersClimateTerrainEvidence Evidence;
	Evidence.Height = 500.f;
	Evidence.UpwindHeight = 500.f;
	Evidence.NeighborhoodMeanHeight = 500.f;
	Evidence.bHasWater = Environment.HasWater();
	Evidence.WaterHeight = Environment.WaterHeight;
	float CoreInfluence = 0.f;
	float EdgeInfluence = 0.f;
	float OutsideInfluence = 0.f;
	const FVector2D Center = Intent.Regions[0].Center;
	const float Radius = Intent.Regions[0].Radius;
	const FGatersClimateSample Core = FGatersClimateField::Evaluate(
		Recipe,
		FGatersLandformProcessField::QueryProfile(
			Landform, Center, &CoreInfluence), Evidence);
	const FGatersClimateSample Edge = FGatersClimateField::Evaluate(
		Recipe,
		FGatersLandformProcessField::QueryProfile(
			Landform, Center + FVector2D(Radius * 0.85f, 0.f), &EdgeInfluence),
		Evidence);
	const FGatersClimateSample Outside = FGatersClimateField::Evaluate(
		Recipe,
		FGatersLandformProcessField::QueryProfile(
			Landform, Center + FVector2D(Radius * 1.2f, 0.f), &OutsideInfluence),
		Evidence);
	TestTrue(TEXT("regional climate core uses regional profile"),
		Core.Temperature > Edge.Temperature
			&& Core.Precipitation > Edge.Precipitation);
	TestTrue(TEXT("regional climate blends continuously at boundary"),
		Edge.Temperature > Outside.Temperature
			&& Edge.Precipitation > Outside.Precipitation);
	TestTrue(TEXT("regional influence comes from shared profile query"),
		CoreInfluence == 1.f && EdgeInfluence > 0.f && EdgeInfluence < 1.f
			&& OutsideInfluence == 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersClimateFieldHeldOutTest,
	"Gaters.Worldgen.ClimateField.HeldOut",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersClimateFieldHeldOutTest::RunTest(const FString& Parameters)
{
	FGatersClimateRecipe Recipe;
	Recipe.Seed = 101;
	Recipe.WorldSize = 120000.f;
	Recipe.PrevailingWind = FVector2D(1.f, 0.f);
	Recipe.SeasonalityBias = 0.55f;

	FGatersClimateTerrainEvidence Lowland;
	Lowland.Height = 250.f;
	Lowland.UpwindHeight = 250.f;
	Lowland.NeighborhoodMeanHeight = 250.f;
	Lowland.WaterHeight = 0.f;
	FGatersClimateTerrainEvidence Alpine = Lowland;
	Alpine.Height = 4500.f;
	Alpine.UpwindHeight = 4500.f;
	Alpine.NeighborhoodMeanHeight = 4500.f;
	FGatersClimateTerrainEvidence Maritime = Lowland;
	Maritime.bHasWater = true;
	FGatersClimateTerrainEvidence Continental = Lowland;
	Continental.bHasWater = false;
	FGatersClimateTerrainEvidence Windward = Lowland;
	Windward.Height = 2600.f;
	Windward.UpwindHeight = 100.f;
	Windward.NeighborhoodMeanHeight = 1600.f;
	FGatersClimateTerrainEvidence Leeward = Lowland;
	Leeward.Height = 100.f;
	Leeward.UpwindHeight = 2600.f;
	Leeward.NeighborhoodMeanHeight = 1600.f;

	FGatersEnvironmentTargetProfile Polar;
	Polar.Temperature = 0.08f;
	Polar.Moisture = 0.55f;
	FGatersEnvironmentTargetProfile Tropical = Polar;
	Tropical.Temperature = 0.92f;
	Tropical.Moisture = 0.85f;
	FGatersEnvironmentTargetProfile Temperate;
	Temperate.Temperature = 0.55f;
	Temperate.Moisture = 0.55f;
	FGatersEnvironmentTargetProfile Arid = Temperate;
	Arid.Moisture = 0.05f;
	FGatersEnvironmentTargetProfile Wet = Temperate;
	Wet.Moisture = 0.9f;
	Wet.SurfaceWater = 0.8f;

	const FGatersClimateSample PolarSample =
		FGatersClimateField::Evaluate(Recipe, Polar, Lowland);
	const FGatersClimateSample TropicalSample =
		FGatersClimateField::Evaluate(Recipe, Tropical, Lowland);
	const FGatersClimateSample LowlandSample =
		FGatersClimateField::Evaluate(Recipe, Temperate, Lowland);
	const FGatersClimateSample AlpineSample =
		FGatersClimateField::Evaluate(Recipe, Temperate, Alpine);
	const FGatersClimateSample AridSample =
		FGatersClimateField::Evaluate(Recipe, Arid, Continental);
	const FGatersClimateSample MaritimeSample =
		FGatersClimateField::Evaluate(Recipe, Wet, Maritime);
	const FGatersClimateSample ContinentalSample =
		FGatersClimateField::Evaluate(Recipe, Wet, Continental);
	const FGatersClimateSample WindwardSample =
		FGatersClimateField::Evaluate(Recipe, Wet, Windward);
	const FGatersClimateSample LeewardSample =
		FGatersClimateField::Evaluate(Recipe, Wet, Leeward);

	TestTrue(TEXT("polar evidence is colder than tropical evidence"),
		PolarSample.Temperature < TropicalSample.Temperature);
	TestTrue(TEXT("alpine evidence is colder than lowland evidence"),
		AlpineSample.Temperature < LowlandSample.Temperature);
	TestTrue(TEXT("maritime wet evidence is wetter than arid evidence"),
		MaritimeSample.Precipitation > AridSample.Precipitation);
	TestTrue(TEXT("maritime influence damps seasonality"),
		MaritimeSample.Seasonality < ContinentalSample.Seasonality);
	TestTrue(TEXT("windward terrain is wetter than leeward terrain"),
		WindwardSample.Precipitation > LeewardSample.Precipitation);
	TestTrue(TEXT("wet near-freezing evidence exceeds hot wet freeze-thaw"),
		LowlandSample.FreezeThaw > TropicalSample.FreezeThaw);

	const TArray<FGatersEnvironmentBrief> Briefs = {
		FixedClimateBrief(0.08f, 0.5f),
		FixedClimateBrief(0.28f, 0.8f),
		FixedClimateBrief(0.5f, 0.35f),
		FixedClimateBrief(0.72f, 0.7f),
		FixedClimateBrief(0.92f, 0.15f)};
	const int32 Seeds[] = {11, 29, 47, 83, 131};
	for (int32 CaseIndex = 0; CaseIndex < UE_ARRAY_COUNT(Seeds); ++CaseIndex)
	{
		const int32 Seed = Seeds[CaseIndex];
		const FGatersEnvironment Environment =
			FGatersEnvironment::FromSeed(Seed, Recipe.WorldSize);
		const FGatersCompiledEnvironmentBrief Intent = CompileIntent(
			Briefs[CaseIndex], Seed, Recipe.WorldSize);
		const FGatersLandformProcessRecipe Landform =
			FGatersLandformProcessField::Compile(Environment, Intent).Recipe;
		const FGatersClimateCompileResult Compiled =
			FGatersClimateField::Compile(Environment, Intent, Landform);
		TestTrue(TEXT("held-out seeded field compiles"), Compiled.IsValid());
		for (int32 X = -2; X <= 2; ++X)
		{
			for (int32 Y = -2; Y <= 2; ++Y)
			{
				const FVector2D Point(X * 18000.f, Y * 18000.f);
				const FGatersClimateSample A = FGatersClimateField::Query(
					Compiled.Recipe, Environment, Point);
				const FGatersClimateSample B = FGatersClimateField::Query(
					Compiled.Recipe, Environment, Point);
				TestTrue(TEXT("held-out coordinate repeats exactly"), A == B);
				TestEqual(TEXT("held-out climate preserves accepted terrain height"),
					A.Height, Environment.HeightAt(Point));
				for (const float Value : {
					A.Temperature, A.Precipitation, A.WindExposure,
					A.Seasonality, A.FreezeThaw, A.RegionInfluence})
				{
					TestTrue(TEXT("held-out climate signal is bounded"),
						IsBounded(Value));
				}
			}
		}
	}
	return true;
}

#endif
