#include "GatersLandformResponseEvaluator.h"

#include "GatersEnvironment.h"
#include "GatersTerrainEvaluator.h"

FGatersLandformResponseEvaluation FGatersLandformResponseEvaluator::Evaluate(
	const FGatersEnvironment& Environment,
	TFunctionRef<float(const FVector2D&)> BaselineHeightAt,
	TFunctionRef<float(const FVector2D&)> ChallengerHeightAt,
	float EvaluationSize)
{
	constexpr int32 SamplesPerAxis = 33;
	constexpr float ChangeEpsilon = KINDA_SMALL_NUMBER;

	FGatersLandformResponseEvaluation Result;
	const float Size = EvaluationSize > 0.f ? EvaluationSize : Environment.ChunkSize;
	const float Extent = Size * 0.45f;
	const float Step = Extent * 2.f / static_cast<float>(SamplesPerAxis - 1);
	double SquaredDifferenceSum = 0.0;
	double DifferenceSum = 0.0;
	int32 PositiveSamples = 0;
	int32 NegativeSamples = 0;

	for (int32 X = 0; X < SamplesPerAxis; ++X)
	{
		for (int32 Y = 0; Y < SamplesPerAxis; ++Y)
		{
			const FVector2D Point(-Extent + X * Step, -Extent + Y * Step);
			const float BaselineHeight = BaselineHeightAt(Point);
			const float ChallengerHeight = ChallengerHeightAt(Point);
			if (!FMath::IsFinite(BaselineHeight) || !FMath::IsFinite(ChallengerHeight))
			{
				Result.Diagnostics.Add(TEXT("landform.response.non_finite"));
				return Result;
			}

			const float Difference = ChallengerHeight - BaselineHeight;
			SquaredDifferenceSum += static_cast<double>(Difference) * Difference;
			DifferenceSum += Difference;
			PositiveSamples += Difference > ChangeEpsilon ? 1 : 0;
			NegativeSamples += Difference < -ChangeEpsilon ? 1 : 0;
		}
	}

	const FGatersTerrainEvaluation Baseline = FGatersTerrainEvaluator::EvaluateHeightField(
		Environment, BaselineHeightAt, EvaluationSize);
	const FGatersTerrainEvaluation Challenger = FGatersTerrainEvaluator::EvaluateHeightField(
		Environment, ChallengerHeightAt, EvaluationSize);
	const float SampleCount = static_cast<float>(SamplesPerAxis * SamplesPerAxis);
	Result.RmsHeightDifference = static_cast<float>(FMath::Sqrt(SquaredDifferenceSum / SampleCount));
	Result.PositiveChangeFraction = PositiveSamples / SampleCount;
	Result.NegativeChangeFraction = NegativeSamples / SampleCount;
	Result.ReliefDelta = Challenger.Relief() - Baseline.Relief();
	Result.MaximumHeightDelta = Challenger.MaxHeight - Baseline.MaxHeight;
	Result.MeanHeightDelta = static_cast<float>(DifferenceSum / SampleCount);
	return Result;
}
