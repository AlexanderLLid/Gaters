#include "GatersTerrainEvaluator.h"

FGatersTerrainEvaluation FGatersTerrainEvaluator::Evaluate(
	const FGatersEnvironment& Environment,
	float EvaluationSize)
{
	constexpr int32 SamplesPerAxis = 17;
	constexpr float BuildFootprintRadius = 900.f;
	constexpr float MaxBuildDrop = 350.f;
	constexpr float WaterClearance = 50.f;

	FGatersTerrainEvaluation Result;
	Result.MinHeight = TNumericLimits<float>::Max();
	Result.MaxHeight = -TNumericLimits<float>::Max();

	const float Extent = (EvaluationSize > 0.f ? EvaluationSize : Environment.ChunkSize) * 0.45f;
	const float Step = Extent * 2.f / static_cast<float>(SamplesPerAxis - 1);
	TArray<float> Heights;
	Heights.SetNumUninitialized(SamplesPerAxis * SamplesPerAxis);
	int32 WaterSamples = 0;
	int32 BuildableSamples = 0;
	int32 BelowDatumSamples = 0;
	float HeightSum = 0.f;

	for (int32 X = 0; X < SamplesPerAxis; ++X)
	{
		for (int32 Y = 0; Y < SamplesPerAxis; ++Y)
		{
			const FVector2D Point(-Extent + X * Step, -Extent + Y * Step);
			const float Height = Environment.HeightAt(Point);
			Heights[X * SamplesPerAxis + Y] = Height;
			Result.MinHeight = FMath::Min(Result.MinHeight, Height);
			Result.MaxHeight = FMath::Max(Result.MaxHeight, Height);
			HeightSum += Height;
			BelowDatumSamples += Height < 0.f ? 1 : 0;
			WaterSamples += Height <= Environment.WaterHeight ? 1 : 0;
			BuildableSamples +=
				Environment.FootprintDrop(Point, BuildFootprintRadius) <= MaxBuildDrop &&
				Environment.IsFootprintDry(Point, BuildFootprintRadius, WaterClearance) ? 1 : 0;
		}
	}

	float StepSum = 0.f;
	int32 StepCount = 0;
	for (int32 X = 0; X < SamplesPerAxis; ++X)
	{
		for (int32 Y = 0; Y < SamplesPerAxis; ++Y)
		{
			const float Height = Heights[X * SamplesPerAxis + Y];
			if (X + 1 < SamplesPerAxis)
			{
				const float NeighborStep = FMath::Abs(Height - Heights[(X + 1) * SamplesPerAxis + Y]);
				StepSum += NeighborStep;
				Result.MaxNeighborStep = FMath::Max(Result.MaxNeighborStep, NeighborStep);
				++StepCount;
			}
			if (Y + 1 < SamplesPerAxis)
			{
				const float NeighborStep = FMath::Abs(Height - Heights[X * SamplesPerAxis + Y + 1]);
				StepSum += NeighborStep;
				Result.MaxNeighborStep = FMath::Max(Result.MaxNeighborStep, NeighborStep);
				++StepCount;
			}
		}
	}

	const float SampleCount = static_cast<float>(Heights.Num());
	Result.WaterFraction = WaterSamples / SampleCount;
	Result.BuildableFraction = BuildableSamples / SampleCount;
	Result.MeanHeight = HeightSum / SampleCount;
	Result.BelowDatumFraction = BelowDatumSamples / SampleCount;
	Result.MeanNeighborStep = StepCount > 0 ? StepSum / static_cast<float>(StepCount) : 0.f;
	return Result;
}
