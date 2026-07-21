#if WITH_DEV_AUTOMATION_TESTS

#include "GatersIntentTerrainFidelityEvaluator.h"
#include "GatersIntentTerrainField.h"
#include "Misc/AutomationTest.h"

namespace
{
TArray<FGatersIntentTerrainObservation> ObserveCores(
	const FGatersEnvironment& Environment,
	const FGatersWorldIntentRecipe& Intent)
{
	TArray<FGatersIntentTerrainObservation> Result;
	Result.Add({ FVector2D::ZeroVector,
		FGatersIntentTerrainField::Query(Environment, Intent, FVector2D::ZeroVector).Height });
	for (int32 Index = 1; Index < Intent.Regions.Num(); ++Index)
	{
		const FVector2D Point = Intent.Regions[Index].Center;
		Result.Add({ Point, FGatersIntentTerrainField::Query(Environment, Intent, Point).Height });
	}
	return Result;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersIntentTerrainFidelityAcceptTest,
	"Gaters.Worldgen.IntentTerrainFidelity.AcceptsDeclaredTerrain",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersIntentTerrainFidelityAcceptTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(73, 400000.f);
	const FGatersWorldIntentRecipe Intent = FGatersWorldIntentRecipe::Generate(73, 400000.f);
	const FGatersIntentTerrainFidelityEvaluation Evaluation =
		FGatersIntentTerrainFidelityEvaluator::Evaluate(
			Environment, Intent, ObserveCores(Environment, Intent));
	TestEqual(TEXT("fidelity contract is versioned"), Evaluation.Version, 1);
	TestTrue(TEXT("matching observations pass"), Evaluation.bValid);
	TestEqual(TEXT("every declared region has core evidence"),
		Evaluation.CoveredRegionCount, Intent.Regions.Num());
	TestEqual(TEXT("matching observations have no diagnostics"),
		Evaluation.Diagnostics.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersIntentTerrainFidelityRejectTest,
	"Gaters.Worldgen.IntentTerrainFidelity.RejectsMismatch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersIntentTerrainFidelityRejectTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(73, 400000.f);
	const FGatersWorldIntentRecipe Intent = FGatersWorldIntentRecipe::Generate(73, 400000.f);
	TArray<FGatersIntentTerrainObservation> Observations = ObserveCores(Environment, Intent);
	Observations[1].Height += 100.f;
	const FGatersIntentTerrainFidelityEvaluation Corrupted =
		FGatersIntentTerrainFidelityEvaluator::Evaluate(Environment, Intent, Observations);
	TestFalse(TEXT("corrupted regional terrain fails"), Corrupted.bValid);
	TestTrue(TEXT("mismatch names the responsible region"),
		!Corrupted.Diagnostics.IsEmpty() &&
		Corrupted.Diagnostics[0].Contains(Intent.Regions[1].Id));

	Observations = ObserveCores(Environment, Intent);
	Observations.RemoveAt(2);
	const FGatersIntentTerrainFidelityEvaluation Missing =
		FGatersIntentTerrainFidelityEvaluator::Evaluate(Environment, Intent, Observations);
	TestFalse(TEXT("missing regional evidence fails"), Missing.bValid);
	TestEqual(TEXT("missing evidence reduces covered regions"),
		Missing.CoveredRegionCount, Intent.Regions.Num() - 1);
	return true;
}

#endif
