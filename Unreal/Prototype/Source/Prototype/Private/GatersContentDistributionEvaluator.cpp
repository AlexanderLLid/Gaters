#include "GatersContentDistributionEvaluator.h"

FGatersContentDistributionEvaluation FGatersContentDistributionEvaluator::Evaluate(
	const TArray<FGatersContentDistributionObservation>& Observations,
	const FGatersContentDistributionSettings& Settings)
{
	FGatersContentDistributionEvaluation Result;
	TArray<FVector2D> DensityPairs;
	float DensityError = 0.f;
	float KindError = 0.f;
	int32 KindSamples = 0;

	for (const FGatersContentDistributionObservation& Observation : Observations)
	{
		const int32 Placed = Observation.TreeCount + Observation.RockCount;
		if (Observation.Capacity <= 0 || Observation.TreeCount < 0
			|| Observation.RockCount < 0 || Placed > Observation.Capacity)
		{
			Result.Issues.Add({TEXT("content.observation.invalid"), Observation.Id,
				static_cast<float>(Placed), 0.f, static_cast<float>(Observation.Capacity)});
			continue;
		}

		const float ExpectedDensity = FMath::Clamp(
			Observation.VegetationOpportunity + Observation.StoneOpportunity, 0.f, 1.f);
		const float ActualDensity = static_cast<float>(Placed) / Observation.Capacity;
		DensityPairs.Add(FVector2D(ExpectedDensity, ActualDensity));
		DensityError += FMath::Abs(ExpectedDensity - ActualDensity);
		if (ExpectedDensity <= KINDA_SMALL_NUMBER && Placed > 0)
		{
			Result.Issues.Add({TEXT("content.scarcity.violated"), Observation.Id,
				ActualDensity, 0.f, 0.f});
		}

		const float OpportunityTotal = Observation.VegetationOpportunity
			+ Observation.StoneOpportunity;
		if (OpportunityTotal > KINDA_SMALL_NUMBER && Placed > 0)
		{
			const float ExpectedTreeFraction = Observation.VegetationOpportunity
				/ OpportunityTotal;
			const float ActualTreeFraction = static_cast<float>(Observation.TreeCount) / Placed;
			KindError += FMath::Abs(ExpectedTreeFraction - ActualTreeFraction);
			++KindSamples;
		}
	}

	Result.ObservationCount = DensityPairs.Num();
	Result.MeanDensityError = Result.ObservationCount > 0
		? DensityError / Result.ObservationCount
		: 0.f;
	Result.MeanKindMixError = KindSamples > 0 ? KindError / KindSamples : 0.f;
	if (Result.ObservationCount < Settings.MinObservations)
	{
		Result.Issues.Add({TEXT("content.observations.insufficient"), {},
			static_cast<float>(Result.ObservationCount),
			static_cast<float>(Settings.MinObservations),
			static_cast<float>(Settings.MinObservations)});
	}
	if (Result.MeanDensityError > Settings.MaxMeanDensityError)
	{
		Result.Issues.Add({TEXT("content.density.error"), {},
			Result.MeanDensityError, 0.f, Settings.MaxMeanDensityError});
	}
	if (Result.MeanKindMixError > Settings.MaxMeanKindMixError)
	{
		Result.Issues.Add({TEXT("content.kind-mix.error"), {},
			Result.MeanKindMixError, 0.f, Settings.MaxMeanKindMixError});
	}

	if (Result.ObservationCount >= 2)
	{
		FVector2D Mean = FVector2D::ZeroVector;
		for (const FVector2D& Pair : DensityPairs)
		{
			Mean += Pair;
		}
		Mean /= Result.ObservationCount;
		float Covariance = 0.f;
		float ExpectedVariance = 0.f;
		float ActualVariance = 0.f;
		for (const FVector2D& Pair : DensityPairs)
		{
			const float X = Pair.X - Mean.X;
			const float Y = Pair.Y - Mean.Y;
			Covariance += X * Y;
			ExpectedVariance += X * X;
			ActualVariance += Y * Y;
		}
		const float Denominator = FMath::Sqrt(ExpectedVariance * ActualVariance);
		Result.DensityCorrelation = Denominator > SMALL_NUMBER
			? Covariance / Denominator
			: 0.f;
		if (ExpectedVariance > SMALL_NUMBER
			&& Result.DensityCorrelation < Settings.MinDensityCorrelation)
		{
			Result.Issues.Add({TEXT("content.density.correlation"), {},
				Result.DensityCorrelation, Settings.MinDensityCorrelation,
				Settings.MinDensityCorrelation});
		}
	}
	return Result;
}
