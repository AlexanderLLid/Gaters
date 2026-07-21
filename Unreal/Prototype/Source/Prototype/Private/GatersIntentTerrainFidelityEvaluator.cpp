#include "GatersIntentTerrainFidelityEvaluator.h"

FGatersIntentTerrainFidelityEvaluation FGatersIntentTerrainFidelityEvaluator::Evaluate(
	const FGatersEnvironment& Environment,
	const FGatersWorldIntentRecipe& Intent,
	const TArray<FGatersIntentTerrainObservation>& Observations,
	const float HeightTolerance)
{
	check(HeightTolerance >= 0.f);
	FGatersIntentTerrainFidelityEvaluation Result;
	for (int32 RegionIndex = 0; RegionIndex < Intent.Regions.Num(); ++RegionIndex)
	{
		const FGatersWorldRegionIntent& Region = Intent.Regions[RegionIndex];
		const FGatersIntentTerrainObservation* Closest = nullptr;
		float ClosestDistance = TNumericLimits<float>::Max();
		for (const FGatersIntentTerrainObservation& Observation : Observations)
		{
			const float Distance = FVector2D::Distance(Observation.Point, Region.Center);
			if (Distance < ClosestDistance)
			{
				Closest = &Observation;
				ClosestDistance = Distance;
			}
		}
		const float CoreRadius = RegionIndex == 0 ? Environment.ChunkSize * 0.075f : Region.Radius * 0.5f;
		if (!Closest || ClosestDistance > CoreRadius)
		{
			Result.Diagnostics.Add(FString::Printf(
				TEXT("intent terrain missing core evidence region=%s"), *Region.Id));
			continue;
		}

		++Result.CoveredRegionCount;
		const float ExpectedHeight = RegionIndex == 0
			? Environment.HeightAt(Closest->Point)
			: Environment.WithProfile(
				Region.TerrainTendency, Region.HydrologyTendency).HeightAt(
					Closest->Point - Region.Center);
		const float Error = FMath::Abs(Closest->Height - ExpectedHeight);
		Result.MaxHeightError = FMath::Max(Result.MaxHeightError, Error);
		if (Error > HeightTolerance)
		{
			Result.Diagnostics.Add(FString::Printf(
				TEXT("intent terrain mismatch region=%s point=(%.0f,%.0f) expected=%.3f observed=%.3f error=%.3f"),
				*Region.Id, Closest->Point.X, Closest->Point.Y,
				ExpectedHeight, Closest->Height, Error));
		}
	}
	Result.bValid = Result.CoveredRegionCount == Intent.Regions.Num()
		&& Result.Diagnostics.IsEmpty();
	return Result;
}
