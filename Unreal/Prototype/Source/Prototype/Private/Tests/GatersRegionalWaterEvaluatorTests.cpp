#if WITH_DEV_AUTOMATION_TESTS

#include "GatersRegionalWaterEvaluator.h"
#include "GatersRegionalWaterRecipe.h"
#include "Misc/AutomationTest.h"

namespace
{
bool HasDiagnostic(const TArray<FString>& Diagnostics, const TCHAR* Pattern)
{
	for (const FString& Diagnostic : Diagnostics)
	{
		if (Diagnostic.Contains(Pattern))
		{
			return true;
		}
	}
	return false;
}

FGatersWorldIntentRecipe WetIntent(const int32 Seed, const float WorldSize)
{
	FGatersWorldIntentRecipe Intent = FGatersWorldIntentRecipe::Generate(Seed, WorldSize);
	Intent.Regions[1].TerrainTendency = EGatersEnvironment::Lowlands;
	Intent.Regions[1].HydrologyTendency = EGatersHydrology::Lakes;
	Intent.Regions[2].TerrainTendency = EGatersEnvironment::Archipelago;
	Intent.Regions[2].HydrologyTendency = EGatersHydrology::Ocean;
	return Intent;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersRegionalWaterEvaluatorAcceptTest,
	"Gaters.Worldgen.RegionalWater.EvaluatorAcceptsPhysicalFit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersRegionalWaterEvaluatorAcceptTest::RunTest(const FString& Parameters)
{
	constexpr int32 Seed = 73;
	constexpr float WorldSize = 400000.f;
	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(Seed, WorldSize);
	const FGatersWorldIntentRecipe Intent = WetIntent(Seed, WorldSize);
	const FGatersRegionalWaterRecipe Recipe = FGatersRegionalWaterRecipe::Generate(
		Environment, Intent);
	const FGatersRegionalWaterEvaluation Evaluation = FGatersRegionalWaterEvaluator::Evaluate(
		Environment, Intent, Recipe);
	TestEqual(TEXT("water-fit contract is versioned"), Evaluation.Version, 1);
	TestTrue(TEXT("physically fitted water passes"), Evaluation.bValid);
	TestEqual(TEXT("expected surface count is causal"), Evaluation.ExpectedSurfaceCount, 3);
	TestEqual(TEXT("all surfaces intersect submerged terrain"),
		Evaluation.SubmergedSurfaceCount, Recipe.Surfaces.Num());
	TestTrue(TEXT("valid water has no diagnostics"), Evaluation.Diagnostics.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersRegionalWaterEvaluatorRejectTest,
	"Gaters.Worldgen.RegionalWater.EvaluatorRejectsPhysicalMismatch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersRegionalWaterEvaluatorRejectTest::RunTest(const FString& Parameters)
{
	constexpr int32 Seed = 73;
	constexpr float WorldSize = 400000.f;
	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(Seed, WorldSize);
	FGatersWorldIntentRecipe Intent = WetIntent(Seed, WorldSize);
	const FGatersRegionalWaterRecipe Good = FGatersRegionalWaterRecipe::Generate(
		Environment, Intent);

	FGatersRegionalWaterRecipe Missing = Good;
	Missing.Surfaces.Pop();
	const FGatersRegionalWaterEvaluation MissingEvaluation =
		FGatersRegionalWaterEvaluator::Evaluate(Environment, Intent, Missing);
	TestFalse(TEXT("missing declared water fails"), MissingEvaluation.bValid);
	TestTrue(TEXT("missing water has a causal diagnostic"),
		HasDiagnostic(MissingEvaluation.Diagnostics, TEXT("count")));

	FGatersRegionalWaterRecipe WrongDatum = Good;
	WrongDatum.Surfaces[0].Height += 10000.f;
	const FGatersRegionalWaterEvaluation DatumEvaluation =
		FGatersRegionalWaterEvaluator::Evaluate(Environment, Intent, WrongDatum);
	TestFalse(TEXT("wrong water datum fails"), DatumEvaluation.bValid);
	TestTrue(TEXT("wrong datum has a causal diagnostic"),
		HasDiagnostic(DatumEvaluation.Diagnostics, TEXT("datum")));

	Intent.Regions[1].HydrologyTendency = EGatersHydrology::Dry;
	const FGatersRegionalWaterEvaluation DryLeak =
		FGatersRegionalWaterEvaluator::Evaluate(Environment, Intent, Good);
	TestFalse(TEXT("water leaking into dry intent fails"), DryLeak.bValid);
	TestTrue(TEXT("dry leak has a causal diagnostic"),
		HasDiagnostic(DryLeak.Diagnostics, TEXT("dry")));

	const FGatersRegionalWaterEvaluation ElevatedComposedTerrain =
		FGatersRegionalWaterEvaluator::Evaluate(
			Environment,
			WetIntent(Seed, WorldSize),
			Good,
			[](const FVector2D&) { return 10000.f; });
	TestFalse(TEXT("injected composed terrain controls physical fit"),
		ElevatedComposedTerrain.bValid);
	TestTrue(TEXT("unsupported composed terrain is causal"),
		HasDiagnostic(
			ElevatedComposedTerrain.Diagnostics,
			TEXT("no submerged terrain")));
	return true;
}

#endif
