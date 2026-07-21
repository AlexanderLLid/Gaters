#if WITH_DEV_AUTOMATION_TESTS

#include "GatersEnvironment.h"
#include "GatersLandformResponseEvaluator.h"
#include "Misc/AutomationTest.h"

#include <limits>

namespace
{
constexpr float WorldSize = 400000.f;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersLandformResponseIdentityTest,
	"Gaters.Worldgen.LandformResponse.Identity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersLandformResponseIdentityTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment Context = FGatersEnvironment::FromSeed(11, WorldSize)
		.WithProfile(EGatersEnvironment::Lowlands, EGatersHydrology::Dry);
	const auto Flat = [](const FVector2D&) { return 100.f; };
	const FGatersLandformResponseEvaluation Result =
		FGatersLandformResponseEvaluator::Evaluate(Context, Flat, Flat, 100000.f);

	TestTrue(TEXT("identical finite fields evaluate"), Result.IsValid());
	TestEqual(TEXT("identical fields have zero RMS response"),
		Result.RmsHeightDifference, 0.f);
	TestEqual(TEXT("identical fields have zero relief response"),
		Result.ReliefDelta, 0.f);
	TestEqual(TEXT("identical fields have no positive response"),
		Result.PositiveChangeFraction, 0.f);
	TestEqual(TEXT("identical fields have no negative response"),
		Result.NegativeChangeFraction, 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersLandformResponseSensitivityTest,
	"Gaters.Worldgen.LandformResponse.Sensitivity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersLandformResponseSensitivityTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment Context = FGatersEnvironment::FromSeed(29, WorldSize)
		.WithProfile(EGatersEnvironment::Lowlands, EGatersHydrology::Dry);
	const auto Baseline = [](const FVector2D&) { return 0.f; };
	const auto Uplift = [](const FVector2D& Point)
	{
		return 1000.f + Point.X * 0.01f;
	};
	const auto Glacial = [](const FVector2D& Point)
	{
		const float NormalizedY = Point.Y / 9000.f;
		return -1200.f * FMath::Exp(-NormalizedY * NormalizedY);
	};

	const FGatersLandformResponseEvaluation UpliftResult =
		FGatersLandformResponseEvaluator::Evaluate(
			Context, Baseline, Uplift, 100000.f);
	const FGatersLandformResponseEvaluation GlacialResult =
		FGatersLandformResponseEvaluator::Evaluate(
			Context, Baseline, Glacial, 100000.f);

	TestTrue(TEXT("uplift response is valid"), UpliftResult.IsValid());
	TestTrue(TEXT("uplift raises all sampled terrain"),
		UpliftResult.PositiveChangeFraction > 0.99f);
	TestTrue(TEXT("uplift adds measurable relief"), UpliftResult.ReliefDelta > 500.f);
	TestTrue(TEXT("glacial response is valid"), GlacialResult.IsValid());
	TestTrue(TEXT("glacial carving has negative coverage"),
		GlacialResult.NegativeChangeFraction > 0.1f);
	TestTrue(TEXT("glacial carving lowers mean height"),
		GlacialResult.MeanHeightDelta < -100.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersLandformResponseNonFiniteTest,
	"Gaters.Worldgen.LandformResponse.NonFinite",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersLandformResponseNonFiniteTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment Context = FGatersEnvironment::FromSeed(47, WorldSize);
	const auto Baseline = [](const FVector2D&) { return 0.f; };
	const auto Broken = [](const FVector2D& Point)
	{
		return Point.IsNearlyZero()
			? std::numeric_limits<float>::quiet_NaN()
			: 0.f;
	};
	const FGatersLandformResponseEvaluation Result =
		FGatersLandformResponseEvaluator::Evaluate(
			Context, Baseline, Broken, 100000.f);

	TestFalse(TEXT("non-finite field is rejected"), Result.IsValid());
	TestTrue(TEXT("non-finite rejection is causal"), Result.Diagnostics.Contains(
		TEXT("landform.response.non_finite")));
	return true;
}

#endif
