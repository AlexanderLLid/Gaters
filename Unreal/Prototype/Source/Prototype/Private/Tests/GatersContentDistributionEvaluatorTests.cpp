#if WITH_DEV_AUTOMATION_TESTS

#include "GatersContentDistributionEvaluator.h"
#include "GatersContentCellRecipe.h"
#include "GatersEnvironmentRecipe.h"
#include "Misc/AutomationTest.h"

namespace
{
bool HasIssue(
	const FGatersContentDistributionEvaluation& Evaluation,
	const FString& RuleId)
{
	return Evaluation.Issues.ContainsByPredicate(
		[&RuleId](const FGatersContentDistributionIssue& Issue)
		{
			return Issue.RuleId == RuleId;
		});
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersContentDistributionContractTest,
	"Gaters.Worldgen.ContentDistribution.Contract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersContentDistributionContractTest::RunTest(const FString& Parameters)
{
	const TArray<FGatersContentDistributionObservation> Observations = {
		{TEXT("sparse"), 0.10f, 0.00f, 1, 0, 10},
		{TEXT("mixed"), 0.25f, 0.25f, 2, 3, 10},
		{TEXT("rocky"), 0.00f, 0.90f, 0, 9, 10}};
	FGatersContentDistributionSettings Settings;
	Settings.MaxMeanDensityError = 0.11f;
	Settings.MaxMeanKindMixError = 0.15f;
	Settings.MinDensityCorrelation = 0.90f;
	Settings.MinObservations = 3;

	const FGatersContentDistributionEvaluation Evaluation =
		FGatersContentDistributionEvaluator::Evaluate(Observations, Settings);
	TestTrue(TEXT("matching external observations pass"), Evaluation.IsValid());
	TestEqual(TEXT("evaluation is versioned"), Evaluation.Version, 1);
	TestEqual(TEXT("all observations are measured"), Evaluation.ObservationCount, 3);
	TestTrue(TEXT("density error is bounded"),
		Evaluation.MeanDensityError <= Settings.MaxMeanDensityError);
	TestTrue(TEXT("kind mix error is bounded"),
		Evaluation.MeanKindMixError <= Settings.MaxMeanKindMixError);
	TestTrue(TEXT("density follows opportunity"),
		Evaluation.DensityCorrelation >= Settings.MinDensityCorrelation);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersContentDistributionCounterexampleTest,
	"Gaters.Worldgen.ContentDistribution.Counterexample",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersContentDistributionCounterexampleTest::RunTest(const FString& Parameters)
{
	FGatersContentDistributionSettings Settings;
	Settings.MaxMeanDensityError = 0.10f;
	Settings.MinDensityCorrelation = 0.50f;
	Settings.MinObservations = 3;
	const TArray<FGatersContentDistributionObservation> ConstantDensity = {
		{TEXT("low"), 0.10f, 0.00f, 5, 0, 10},
		{TEXT("middle"), 0.50f, 0.00f, 5, 0, 10},
		{TEXT("high"), 0.90f, 0.00f, 5, 0, 10}};
	const FGatersContentDistributionEvaluation Flat =
		FGatersContentDistributionEvaluator::Evaluate(ConstantDensity, Settings);
	TestFalse(TEXT("constant placement density cannot satisfy varied opportunity"),
		Flat.IsValid());
	TestTrue(TEXT("constant density reports correlation failure"),
		HasIssue(Flat, TEXT("content.density.correlation")));

	const TArray<FGatersContentDistributionObservation> ScarcityViolation = {
		{TEXT("declared-empty"), 0.f, 0.f, 1, 0, 10}};
	const FGatersContentDistributionEvaluation Scarcity =
		FGatersContentDistributionEvaluator::Evaluate(ScarcityViolation, Settings);
	TestFalse(TEXT("content in declared scarcity is rejected"), Scarcity.IsValid());
	TestTrue(TEXT("scarcity violation names its cause"),
		HasIssue(Scarcity, TEXT("content.scarcity.violated")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersContentDistributionEnvironmentRootTest,
	"Gaters.Worldgen.ContentDistribution.EnvironmentRoot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersContentDistributionEnvironmentRootTest::RunTest(const FString& Parameters)
{
	constexpr float WorldSize = 200000.f;
	constexpr float CellSize = 10000.f;
	TArray<FGatersContentDistributionObservation> Observations;
	for (const int32 Seed : {11, 29, 47, 83, 131})
	{
		const FGatersEnvironmentRecipe Environment =
			FGatersEnvironmentRecipeCompiler::Compile(Seed, WorldSize);
		for (int32 X = -2; X <= 2; ++X)
		{
			for (int32 Y = -2; Y <= 2; ++Y)
			{
				const FIntPoint Cell(X, Y);
				const FGatersContentCellRecipe Recipe =
					FGatersContentCellRecipe::Generate(Cell, CellSize, Environment);
				if (Recipe.Coverage.ReservedRejectedCount > 0
					|| Recipe.Coverage.WaterRejectedCount > 0
					|| Recipe.Coverage.SteepRejectedCount > 0
					|| Recipe.Coverage.IntentRejectedCount > 0)
				{
					continue;
				}
				int32 Trees = 0;
				int32 Rocks = 0;
				for (const FGatersContentCellPlacement& Placement : Recipe.Placements)
				{
					Trees += Placement.Kind == EGatersRecipeNodeKind::ScatterTree ? 1 : 0;
					Rocks += Placement.Kind == EGatersRecipeNodeKind::ScatterRock ? 1 : 0;
				}
				Observations.Add({
					FString::Printf(TEXT("seed:%d:cell:%d:%d"), Seed, X, Y),
					Recipe.VegetationOpportunity,
					Recipe.StoneOpportunity,
					Trees,
					Rocks,
					Recipe.MaxPlacements});
			}
		}
	}

	FGatersContentDistributionSettings Settings;
	Settings.MaxMeanDensityError = 0.25f;
	Settings.MaxMeanKindMixError = 0.40f;
	Settings.MinDensityCorrelation = 0.20f;
	Settings.MinObservations = 20;
	const FGatersContentDistributionEvaluation Evaluation =
		FGatersContentDistributionEvaluator::Evaluate(Observations, Settings);
	AddInfo(FString::Printf(
		TEXT("observations=%d density_error=%.3f kind_error=%.3f correlation=%.3f"),
		Evaluation.ObservationCount, Evaluation.MeanDensityError,
		Evaluation.MeanKindMixError, Evaluation.DensityCorrelation));
	for (const FGatersContentDistributionIssue& Issue : Evaluation.Issues)
	{
		AddError(FString::Printf(
			TEXT("rule=%s observation=%s observed=%.3f expected=%.3f limit=%.3f"),
			*Issue.RuleId, *Issue.ObservationId, Issue.Observed,
			Issue.Expected, Issue.Limit));
	}
	TestTrue(TEXT("held-out roots preserve physical content distribution"),
		Evaluation.IsValid());
	return true;
}

#endif
