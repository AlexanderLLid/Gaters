#if WITH_DEV_AUTOMATION_TESTS

#include "GatersEnvironment.h"
#include "GatersTerrainMorphologyEvaluator.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersTerrainMorphologyContractTest,
	"Gaters.Worldgen.TerrainMorphology.Contract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersTerrainMorphologyContractTest::RunTest(const FString& Parameters)
{
	const auto Circle = [](const FVector2D& Point)
	{
		return 4000.f - static_cast<float>(Point.Size());
	};
	const auto Irregular = [](const FVector2D& Point)
	{
		const float Angle = FMath::Atan2(Point.Y, Point.X);
		const float Radius = 4000.f
			+ FMath::Sin(Angle * 3.f) * 1200.f
			+ FMath::Sin(Angle * 7.f + 0.4f) * 650.f;
		return Radius - static_cast<float>(Point.Size());
	};

	const FGatersTerrainMorphologyEvaluation Round =
		FGatersTerrainMorphologyEvaluator::Evaluate(Circle, 7000.f, {0.f});
	const FGatersTerrainMorphologyEvaluation Natural =
		FGatersTerrainMorphologyEvaluator::Evaluate(Irregular, 7000.f, {0.f});

	TestEqual(TEXT("morphology contract is versioned"), Round.EvaluatorVersion, 1);
	TestTrue(TEXT("closed circle is evaluated"), Round.ClosedContourCount > 0);
	TestTrue(TEXT("radial primitive scores as compact"),
		Round.MaximumClosedContourCircularity >= 0.70f);
	TestTrue(TEXT("irregular contour scores below a radial primitive"),
		Natural.MaximumClosedContourCircularity
			< Round.MaximumClosedContourCircularity - 0.08f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersArchipelagoMorphologyRegressionTest,
	"Gaters.Worldgen.TerrainMorphology.Archipelago131",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersArchipelagoMorphologyRegressionTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(131, 400000.f);
	TestEqual(TEXT("held-out seed remains an archipelago"),
		Environment.Type, EGatersEnvironment::Archipelago);

	const FGatersTerrainMorphologyEvaluation Morphology =
		FGatersTerrainMorphologyEvaluator::Evaluate(
			[&Environment](const FVector2D& Point)
			{
				return Environment.HeightAt(Point);
			},
			20000.f,
			{
				Environment.WaterHeight + 50.f,
				Environment.WaterHeight + 300.f,
				Environment.WaterHeight + 600.f
			});

	AddInfo(FString::Printf(TEXT(
		"seed 131 closed=%d max-circularity=%.3f max-samples=%d largest-samples=%d largest-circularity=%.3f"),
		Morphology.ClosedContourCount,
		Morphology.MaximumClosedContourCircularity,
		Morphology.MostCircularContourSamples,
		Morphology.LargestClosedContourSamples,
		Morphology.LargestClosedContourCircularity));
	TestTrue(TEXT("seed 131 exposes a closed-island contour"),
		Morphology.ClosedContourCount >= 2);
	TestTrue(TEXT("seed 131 does not expose a circular generator stamp"),
		Morphology.MaximumClosedContourCircularity < 0.86f);
	FVector2D BaseSite;
	TestTrue(TEXT("seed 131 retains a viable base site"), Environment.FindBaseSite(
		6000.f, 10800.f, 900.f, 350.f, BaseSite));
	return true;
}

#endif
