#if WITH_DEV_AUTOMATION_TESTS

#include "GatersBiomeOpportunityField.h"

#include "GatersEnvironmentRecipe.h"
#include "Misc/AutomationTest.h"

namespace
{
void TestBounded(
	FAutomationTestBase& Test,
	const TCHAR* Label,
	const FGatersBiomeOpportunitySample& Sample)
{
	for (const float Value : {
		Sample.Vegetation,
		Sample.Stone,
		Sample.Landmark,
		Sample.TravelFriction})
	{
		Test.TestTrue(Label,
			FMath::IsFinite(Value) && Value >= 0.f && Value <= 1.f);
	}
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersBiomeOpportunityFieldContractTest,
	"Gaters.Worldgen.BiomeOpportunities.Contract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersBiomeOpportunityFieldContractTest::RunTest(
	const FString& Parameters)
{
	FGatersBiomeSample Land;
	Land.NormalZ = 0.98f;
	Land.Moisture = 0.8f;
	Land.Exposure = 0.1f;
	Land.BiomeKey = TEXT("land");
	FGatersClimateSample Temperate;
	Temperate.Temperature = 0.55f;
	Temperate.Precipitation = 0.8f;
	FGatersSurfaceConditionSample Soil;
	Soil.Soil = 0.9f;
	Soil.Sediment = 0.25f;
	Soil.Saturation = 0.2f;
	const FGatersBiomeOpportunitySample Green =
		FGatersBiomeOpportunityField::Evaluate(Land, Temperate, Soil);

	FGatersSurfaceConditionSample ExposedRock;
	ExposedRock.Rock = 0.9f;
	ExposedRock.Cliff = 0.55f;
	ExposedRock.Ridge = 0.5f;
	const FGatersBiomeOpportunitySample Rocky =
		FGatersBiomeOpportunityField::Evaluate(Land, Temperate, ExposedRock);

	FGatersSurfaceConditionSample Frozen = Soil;
	Frozen.Snow = 0.8f;
	Frozen.Ice = 0.9f;
	FGatersClimateSample Cold = Temperate;
	Cold.Temperature = 0.05f;
	const FGatersBiomeOpportunitySample Icy =
		FGatersBiomeOpportunityField::Evaluate(Land, Cold, Frozen);

	FGatersSurfaceConditionSample Saturated = Soil;
	Saturated.Saturation = 1.f;
	const FGatersBiomeOpportunitySample Boggy =
		FGatersBiomeOpportunityField::Evaluate(Land, Temperate, Saturated);

	FGatersBiomeSample Water = Land;
	Water.BiomeKey = TEXT("water");
	FGatersSurfaceConditionSample Submerged;
	Submerged.Evidence.WaterDepth = 1.f;
	const FGatersBiomeOpportunitySample Wet =
		FGatersBiomeOpportunityField::Evaluate(Water, Temperate, Submerged);

	TestEqual(TEXT("same physical evidence is deterministic"), Green,
		FGatersBiomeOpportunityField::Evaluate(Land, Temperate, Soil));
	TestTrue(TEXT("warm moist soil favors vegetation"),
		Green.Vegetation > Green.Stone);
	TestTrue(TEXT("exposed rock increases stone opportunity"),
		Rocky.Stone > Green.Stone);
	TestTrue(TEXT("exposed relief increases landmark opportunity"),
		Rocky.Landmark > Green.Landmark);
	TestTrue(TEXT("snow and ice increase travel friction"),
		Icy.TravelFriction > Green.TravelFriction);
	TestTrue(TEXT("saturation increases travel friction"),
		Boggy.TravelFriction > Green.TravelFriction);
	TestTrue(TEXT("frozen stress reduces vegetation"),
		Icy.Vegetation < Green.Vegetation);
	TestEqual(TEXT("water produces no land vegetation opportunity"),
		Wet.Vegetation, 0.f);
	TestEqual(TEXT("water produces no land stone opportunity"), Wet.Stone, 0.f);
	TestTrue(TEXT("water remains maximally difficult for land travel"),
		FMath::IsNearlyEqual(Wet.TravelFriction, 1.f));
	for (const FGatersBiomeOpportunitySample* Sample :
		{&Green, &Rocky, &Icy, &Boggy, &Wet})
	{
		TestBounded(*this, TEXT("physical opportunity weights are bounded"), *Sample);
	}

	const FGatersBiomeOpportunitySample Empty =
		FGatersBiomeOpportunityField::Evaluate({}, {}, {});
	TestEqual(TEXT("barren evidence invents no vegetation"), Empty.Vegetation, 0.f);
	TestEqual(TEXT("barren evidence invents no stone"), Empty.Stone, 0.f);
	TestEqual(TEXT("barren evidence invents no landmark"), Empty.Landmark, 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersBiomeOpportunityRootTest,
	"Gaters.Worldgen.BiomeOpportunities.Root",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersBiomeOpportunityRootTest::RunTest(const FString& Parameters)
{
	for (const int32 Seed : {11, 29, 47, 83, 131})
	{
		const FGatersEnvironmentRecipe A =
			FGatersEnvironmentRecipeCompiler::Compile(Seed, 120000.f);
		const FGatersEnvironmentRecipe B =
			FGatersEnvironmentRecipeCompiler::Compile(Seed, 120000.f);
		const FGatersBiomeOpportunityCompileResult Direct =
			FGatersBiomeOpportunityField::Compile(A);
		TestTrue(FString::Printf(TEXT("seed %d opportunity recipe compiles"), Seed),
			Direct.IsValid());
		TestEqual(FString::Printf(TEXT("seed %d opportunity recipe repeats"), Seed),
			A.BiomeOpportunities, B.BiomeOpportunities);
		TestEqual(FString::Printf(TEXT("seed %d direct recipe matches root"), Seed),
			A.BiomeOpportunities, Direct.Recipe);
		for (const FVector2D Point : {
			FVector2D::ZeroVector,
			FVector2D(12000.f, -9000.f),
			FVector2D(-22000.f, 18000.f)})
		{
			const FGatersBiomeSample Biome = A.QueryBiome(Point);
			const FGatersClimateSample Climate = A.QueryClimate(Point);
			const FGatersSurfaceConditionSample Surface =
				A.QuerySurfaceConditions(Point);
			const FGatersBiomeOpportunitySample Opportunity =
				A.QueryOpportunities(Point);
			TestEqual(TEXT("root opportunity query delegates exactly"), Opportunity,
				FGatersBiomeOpportunityField::Evaluate(Biome, Climate, Surface));
			TestEqual(TEXT("same root opportunity query repeats exactly"), Opportunity,
				B.QueryOpportunities(Point));
			TestBounded(*this, TEXT("root opportunity weights are bounded"),
				Opportunity);
		}
	}
	return true;
}

#endif
